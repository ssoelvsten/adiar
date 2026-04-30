#ifndef ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
#define ADIAR_INTERNAL_ALGORITHMS_REPLACE_H

#include "adiar/bdd.h"
#include "adiar/bdd/bdd.h"
#include "adiar/exec_policy.h"
#include "adiar/internal/algorithms/nested_sweeping.h"
#include "adiar/internal/data_structures/levelized_priority_queue.h"
#include "adiar/internal/data_structures/sorter.h"
#include "adiar/internal/data_structures/vector.h"
#include "adiar/internal/data_types/level_info.h"
#include "adiar/internal/data_types/ptr.h"
#include "adiar/internal/data_types/request.h"
#include "adiar/internal/data_types/uid.h"
#include "adiar/internal/io/arc_ofstream.h"
#include "adiar/internal/io/shared_file_ptr.h"
#include "adiar/internal/memory.h"
#include "adiar/internal/unreachable.h"
#include <cstddef>
#include <functional>
#include <iostream>
#include <sys/types.h>
#include <utility>
#include <vector>

#include <adiar/exception.h>
#include <adiar/functional.h>
#include <adiar/type_traits.h>
#include <adiar/types.h>

#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/levelized_ifstream.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_ifstream.h>
#include <adiar/internal/io/node_ofstream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::replace_t stats_replace;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Helper Functions

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief A total mapping function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using replace_func = function<typename T::label_type(typename T::label_type)>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replaces the level of a single pointer with the one provided by the map `m`.
  ///
  /// \details All other information, e.g. level-identifier, terminal value, and taint flag, are
  ///          preserved as-is.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline ptr_uint64
  __replace(const ptr_uint64& p, const replace_func<ptr_uint64>& m)
  {
    return p.is_node() ? replace(p, m(p.level())) : p;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline uid_uint64
  __replace(const uid_uint64& u, const replace_func<ptr_uint64>& m)
  {
    return uid_uint64::unsafe(__replace(u.as_ptr(), m));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replaces the level of a single node and its children pointers.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline node
  __replace(const node& n, const replace_func<node>& m)
  {
    return { __replace(n.uid(), m), __replace(n.low(), m), __replace(n.high(), m) };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Infer the replace type.
  //////////////////////////////////////////////////////////////////////////////////////////////////

  template <typename Policy, typename LevelInfoStream, typename ReplaceFunction>
  replace_type
  __replace__infer_type(LevelInfoStream& ls, const ReplaceFunction& m)
  {
    using label_type        = typename Policy::label_type;
    using signed_label_type = typename Policy::signed_label_type;
    using result_type       = typename ReplaceFunction::result_type;

    constexpr bool is_total_map   = is_same<result_type, label_type>;
    constexpr bool is_partial_map = is_same<result_type, optional<label_type>>;

    static_assert(is_total_map || is_partial_map);

    bool identity  = true;
    bool shift     = true;
    bool monotone  = true;
    bool jump_down = true;
    bool jump_up = true;
    bool adj_swap  = true;

    typename Policy::label_type last_jump = Policy::pointer_type::nil().level();
    //typename Policy::label_type adj_node  = 0;

    label_type prev_before = Policy::max_label + 1;
    label_type prev_after  = Policy::max_label + 1;

    label_type seen_swap_before = Policy::max_label + 1;
    label_type seen_swap_after  = Policy::max_label + 1;
    bool seen_set = false;

    signed_label_type prev_diff = 0;

    while (ls.can_pull()) {
      const label_type next_before     = ls.pull().level();
      const result_type next_after_opt = m(next_before);

      label_type next_after;
      if constexpr (is_partial_map) {
        if (!next_after_opt.has_value()) { continue; }
        next_after = *next_after_opt;
      } else {
        next_after = next_after_opt;
      }

      const signed_label_type next_diff =
        static_cast<signed_label_type>(next_before) - static_cast<signed_label_type>(next_after);

      if (shift) {
        shift &= Policy::max_label < prev_before || prev_diff == next_diff;
        prev_diff = next_diff;
      }

      identity &= next_before == next_after;
      monotone &= Policy::max_label < prev_before || prev_after < next_after;

      if(next_before != next_after){ //level is moved check
        //JUMP_DOWN checks
        jump_down &= (next_before < next_after ); //levels are only moved down
        jump_down &= (last_jump == Policy::pointer_type::nil().level() || last_jump < next_before); //check overlap: if moved, then you should be below last jump target
        //Jump_Up check
        jump_up &= (next_before > next_after ); //levels are only moved up
        jump_up &= (last_jump == Policy::pointer_type::nil().level() || last_jump < next_after); //check overlap: if moved, then you should be above last jump target
        last_jump = (jump_down) ? next_after : next_before;

        //Adjacent swap checks - currently only detects when both adjacent variables exist in bdd
        //otherwise it's considered a jump down
        if (seen_set && (next_before == seen_swap_after) && (next_after == seen_swap_before)){
          seen_set = false;
        } else if (seen_set) {
          adj_swap = false;
        } else {
          seen_swap_before = next_before;
          seen_swap_after = next_after;
          seen_set = true;
        }
        
      } else if (seen_set) {adj_swap = false;}

      // Todo: swap 
      prev_before = next_before;
      prev_after  = next_after;
    }

    if (!monotone) {
      if (jump_up) { return replace_type::Jump_Up;}
      if (jump_down) { return replace_type::Jump_Down;}
      if (adj_swap) {return replace_type::Swap_Adjacent;}
      return replace_type::Non_Monotone; } //TODO: missing handling swap
    if (!shift) { return replace_type::Monotone; }
    if (!identity) { return replace_type::Shift; }
    return replace_type::Identity;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Infer the replace type.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename ReplaceFunction>
  replace_type
  replace__infer_type(const typename Policy::dd_type& dd, const ReplaceFunction& m)
  {
    level_info_ifstream<false> ls(dd);
    return __replace__infer_type<Policy>(ls, m);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Infer the replace type.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename ReplaceFunction>
  replace_type
  replace__infer_type(const typename Policy::__dd_type& __dd, const ReplaceFunction& m)
  {
    level_info_ifstream<true> ls(__dd);
    return __replace__infer_type<Policy>(ls, m);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Algorithms

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace the level in constant time
  ///
  /// \remark This requires that the mapping, `m`, is *monotonic* and *affine*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::dd_type
  __replace__shift_return(const typename Policy::dd_type& dd, const replace_func<Policy>& m)
  {
    adiar_assert(!dd->is_terminal());
    
    const typename Policy::signed_label_type topvar         = dd_topvar(dd);
    const typename Policy::signed_label_type shifted_topvar = m(topvar);

    return typename Policy::dd_type(
      dd.file_ptr(), dd.is_negated(), dd.shift() + (shifted_topvar - topvar));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace the level of all nodes in a single linear scan.
  ///
  /// \remark This requires that the mapping, `m`, is *monotonic*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::dd_type
  __replace__monotonic_scan(const typename Policy::dd_type& dd, const replace_func<Policy>& m)
  {
    adiar_assert(!dd->is_terminal());

    // Set up outputs
    shared_levelized_file<typename Policy::node_type> out_file;
    node_ofstream out(out_file);

    out.unsafe_set_sorted(dd->sorted);
    out.unsafe_set_indexable(dd->indexable);

    out.unsafe_set_1level_cut(
      { dd->max_1level_cut[cut::Internal],
        dd->max_1level_cut[dd.is_negated() ? cut::Internal_True : cut::Internal_False],
        dd->max_1level_cut[dd.is_negated() ? cut::Internal_False : cut::Internal_True],
        dd->max_1level_cut[cut::All] });

    { // Copy over nodes (in "reverse" to still follow the same order on disk)
      node_ifstream<true> in_nodes(dd);
      while (in_nodes.can_pull()) { out.unsafe_push(__replace(in_nodes.pull(), m)); }
    }
    { // Copy over levels (also in "reverse")
      level_info_ifstream<true> in_levels(dd);
      while (in_levels.can_pull()) {
        const level_info li = in_levels.pull();
        out.unsafe_push(level_info(m(li.level()), li.width()));
      }
    }

    return out_file;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class replace_reduce_policy : public Policy
  {
  private:
    const replace_func<Policy>& _m;

  public:
    replace_reduce_policy(const replace_func<Policy>& m)
      : _m(m)
    {}

    constexpr inline typename Policy::label_type
    map_level(typename Policy::label_type x) const
    {
      return this->_m(x);
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::dd_type
  __replace__monotonic_reduce(const exec_policy& ep,
                              const typename Policy::__dd_type& __dd,
                              const replace_func<Policy>& m)
  {
    replace_reduce_policy<Policy> policy(m);
    return reduce(ep, policy, std::move(__dd));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: Nested Sweeping for non-monotonic reorderings.
  //

  // for allowing testing prints
  constexpr bool debug_enabled = false;

  //types
  template <uint8_t nodes_carried>
  using cor_req_t = request_data<2, with_parent_and_level, nodes_carried>;

  template <memory_mode mem_mode>
  using cor_priority_queue_2_t = priority_queue<mem_mode, cor_req_t<1>,
                                request_data_second_lt<cor_req_t<1>>>;

  template <size_t LookAhead, memory_mode MemMode, size_t x = 1>
  using cor_lvl_priority_queue_t =
    levelized_priority_queue<cor_req_t<0>,
                                  request_data_first_lt<cor_req_t<0>>,
                                  LookAhead,
                                  MemMode,
                                  x,
                                  std::less<node::label_type>,
                                  0u>;

//--------------------------helper functions for non-monotone replace ---------------------          
  template<typename Policy, typename pq_t>
  inline void 
  pusher(pq_t& pq, 
         arc_ofstream& out_stream, 
         ptr_uint64 source, 
         const tuple<typename Policy::pointer_type> target, 
         const typename Policy::label_type level) {
    adiar_assert(!(target[0].is_nil() && target[1].is_nil()), "both targets cannot be nil!");

    if (target[0].is_nil() && target[1].is_terminal()){
        const arc a =  {source, target[1]};
        if (debug_enabled) std::cout << "pushing term arc: " << a << "\n";
        out_stream.push_terminal(a);
        return;
    }
    if (target[1].is_nil() && target[0].is_terminal()){
        const arc a =  {source, target[0]};
        if (debug_enabled) std::cout << "pushing term arc: " << a << "\n";
        out_stream.push_terminal(a);
        return;
    }
    if (target[0].is_terminal() && target[1].is_terminal() && target[0] == target[1]) {
        //push leaf arc from current
        const arc alow =  {source, target[0]};
        if (debug_enabled) std::cout << "pushing term arc: " << alow << "\n";
        out_stream.push_terminal(alow);
        return;
    } else {
      //Non-terminal: push request from current
      const cor_req_t<0> lreq({target[0],target[1]},{},{source, level});
      pq.push(lreq);
    }
  }

  template<typename Policy, typename pq_t, uint8_t nc>
  inline void 
  internal_pusher(pq_t& pq, 
                  arc_ofstream& aw, 
                  typename Policy::uid_type out_uid, 
                  const tuple<typename Policy::pointer_type> target,
                  const typename Policy::label_type level)
  {
    while(pq.has_top() ) {
      const cor_req_t<nc>& r1 = pq.top(); 
      if (r1.target != target || r1.data.level != level) break;

      pq.pop(); //non-levelized has no pull so simulate with pop..
      if (debug_enabled) std::cout << "has popped: " << r1 << "\n";

      if (r1.data.source.level() != ptr_uint64::nil().level()) {//push to out!
        arc in{r1.data.source , out_uid};
        if (debug_enabled) std::cout << "has pushed internal: " << in << "\n";
        aw.push_internal(in);
      } 
    }
  }

  template<typename Policy>
  tuple<tuple<typename Policy::pointer_type>>
  reqFor(tuple<typename Policy::pointer_type> t, node v , 
         typename Policy::pointer_type low,
         typename Policy::pointer_type high) {
    if (debug_enabled) std::cout << "running reqFor with " << t << "\n";
    const bdd::pointer_type tl = t[0];
    const bdd::pointer_type th = t[1];

    if (tl.is_terminal() && th.is_terminal()) {
      if(debug_enabled) std::cout << " \t both terminal case\n";
      return { {tl,tl}, {th,th} };
    }

    if (th.is_terminal() || tl.level() < th.level()) {
      if(debug_enabled) std::cout << " \t th terminal or tl less case\n";
      return { {v.low(), th}, {v.high(), th}};
    }

    if (tl.is_terminal() || tl.level() > th.level()) {
      if(debug_enabled) std::cout << " \t tl terminal or th less case\n";
      return { {tl, v.low()}, {tl, v.high()}};
    }

    if (v.uid() == tl && v.uid() == th ) {
      if(debug_enabled) std::cout << " \t both are v case \n";
      return { {v.low(), v.low()}, {v.high(), v.high()}};
    }
    //low high must exist!
    if(v.uid() == tl) {
      if(debug_enabled) std::cout << " \t lt is v case \n";
      return { {v.low(), low}, {v.high(), high}};
    }

    if(v.uid() == th) {
      if(debug_enabled) std::cout << " \t lh is v case \n";
      return { {low, v.low()}, {high, v.high()}};
    }
    throw invalid_argument("reqFor: Unexpected case missing!");
  }

  //find top req from given PQs
  template<typename PQ1, typename PQ2>
  cor_req_t<1> getNext(PQ1& pq1 , PQ2& pq2)
  {
    if (pq1.can_pull()) {
      ptr_uint64 l_uid(pq1.top().data.level, 0);  //for treating level like uid for comp
      ptr_uint64 min_pq1 = std::min(pq1.top().target.first() , l_uid);

      if(pq2.empty() || min_pq1 < pq2.top().target.second()) {
        cor_req_t<1> r = { pq1.top().target, 
                          { { { node::pointer_type::nil(), node::pointer_type::nil() } } }, 
                          pq1.top().data};
        if(debug_enabled) std::cout << "takes req " << r << " from pq1\n";
        return r;
      } 
    } 
    if(debug_enabled) std::cout << "takes req " << pq2.top() << " from pq2\n";
    return pq2.top();
  }

  //------------------------------------- correctify logic for single level ------------------------------------------

  //helper type for diff cases
  enum class label_indicator : signed char
  {
    NORMAL = 1, // label should just be the current level
    SHIFT_BACK = 2, // for use in swap_adj - label should be current level / 2
    DEC= 3, //adj_swap -> level -1 (for xj, extar layers)
  };

  template<typename Policy>
  typename Policy::label_type
  create_label(label_indicator li, typename Policy::label_type label){
    switch (li) {
      case label_indicator::NORMAL: return label;
      case label_indicator::SHIFT_BACK: return label/2;
      case label_indicator::DEC: return label -1;
    }
    adiar_unreachable();
    return 0;
  }


  template <typename Policy, typename In, typename Out, typename PQ1, typename PQ2>
  void
  correctify_single_level(In& in, Out& aw,
                          PQ1& pq1, PQ2& pq2,
                          typename Policy::node_type& v,
                          const label_indicator li)
  {
    using label_t   = typename Policy::label_type;
    using uid_t     = typename Policy::uid_type;
    using ptr_t     = typename Policy::pointer_type;
    using children_t = typename Policy::children_type;

    //does all the correctify stuff for single level - for use both in general non-monotone replace and special cases jump_down and adj_swap
    //vars
    label_t label = pq1.current_level();
    label_t id = -1; 

    // --- Helpers ------------------------------------------------------------
    auto update_label_and_id = [&](const ptr_uint64& tseek) {
      id = (label == tseek.label()) ? (id + 1) : 0; 
      label = tseek.label() ;
      if(debug_enabled) std::cout << "label, id: " << label << "," << id << "\n";
    };

    auto compute_tseek = [&](const cor_req_t<1>& r) -> ptr_uint64 {
      const ptr_uint64 level_uid(r.data.level, 0);
      return (r.empty_carry())
        ? std::min(r.target.first(), level_uid)
        : r.target.second();
    };

    // --- Main Loop ------------------------------------------------------------

    while (!pq1.empty_level() || pq2.has_top()) {
      const cor_req_t<1> r = getNext(pq1, pq2); //pull next req
      const ptr_uint64 tseek = compute_tseek(r);  //updating tseek, v

      //CASE found correct layer!
      //FIXED: now >= to facilitate adj swap extra layer
      // as far as i can tell this change should have no impact in any other case 
      if (r.target.first().level() >= r.data.level) {   
        if (debug_enabled) std::cout << "enters copy case\n";

        //SUBCASE suppresible node case - push one req
        if (r.target[0] == r.target[1]) {
          if (debug_enabled)std::cout << "skip surpressible node! \n";

          pq1.pop(); //remove req without pushing internal -- NOTE: how do we know it comes from pq1?

          //(source -> (target[0], nil))
          const cor_req_t<0> r1 = {{r.target[0],node::pointer_type::nil()}, {}, {r.data.source}};
          if (debug_enabled)std::cout << "pushes req " << r1 << "\n";
          pq1.push(r1);

          continue;
        }
        //SUBCASE not surpressible
        update_label_and_id(tseek);

        //push copy reqs
        const label_t out_label = create_label<Policy>(li, label);
        const uid_t out_uid(out_label, id);

        const label_t nil_lbl = node::pointer_type::nil().level();
        const tuple<ptr_t> tl = {r.target[0],node::pointer_type::nil()};
        const tuple<ptr_t> th = {r.target[1],node::pointer_type::nil()};
        pusher<Policy>(pq1, aw, out_uid.as_ptr(false), tl, nil_lbl);
        pusher<Policy>(pq1, aw, out_uid.as_ptr(true),  th, nil_lbl);

        // forward incoming
        internal_pusher<Policy, PQ1, 0>(pq1, aw, out_uid, r.target,  r.data.level);

        continue;
      }

      //FIX: moved this down -> technically it's not too dangerous but very confusing to have this above the correct layer case 
      // when considering the extra level in adj_swap. it wont be updated to anything more than the smallest node on the potential real level
      // that the extra level acts like it is, buut by putting the update down here it is never run on extra level since extra level always goes in the 
      // correct layer case
      while (v.uid() < tseek && in.can_pull()) { v = in.pull(); if (debug_enabled) std::cout << "has stepped v forward to " << v.uid() << "\n";}

      //CASE should move request from PQ1 to PQ2
      //big copy-paste from prod + small changes 
      if ( r.empty_carry() 
           && r.target[0].is_node() && r.target[1].is_node()
           && r.target[0].label() == r.target[1].label()
           && r.target[0].id() != r.target[1].id()) 
      {
        if(debug_enabled) std::cout << "enters pq2 push if-statement! r is" << r << "\n";
        if(debug_enabled) std::cout << "top of pq1 is" << pq1.top() << "\n";

        const children_t children = v.children();

        while (pq1.has_top() && pq1.top().target == r.target) {
          if(debug_enabled) std::cout << "enters pq2 while\n";
          const cor_req_t<1> nr = {r.target, { children }, pq1.top().data};
          pq2.push(nr);
          pq1.pop();
        }
        continue;
      }

      //CASE wrong layer
        if (debug_enabled) std::cout << "wrong layer case \n";
        update_label_and_id(tseek);

        tuple<tuple<ptr_t>> reqs = reqFor<Policy>(r.target, v, r.node_carry[0][0], r.node_carry[0][1]);
        tuple<ptr_t> rlow = reqs[0]; 
        tuple<ptr_t> rhigh = reqs[1]; 

        const label_t out_label = create_label<Policy>(li, label);
        const uid_t out_uid(out_label, id);

        // Forward outgoing
        pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(false), rlow, r.data.level);
        pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(true), rhigh, r.data.level);

        // Forward incoming
        internal_pusher<Policy, PQ1, 0>(pq1, aw, out_uid, r.target,  r.data.level);
        internal_pusher<Policy, PQ2, 1>(pq2, aw, out_uid, r.target,  r.data.level);
      }
      
      const label_t out_label = create_label<Policy>(li, label);
      if (id >= 0) { aw.push(level_info(out_label, id+1)); }
      if(debug_enabled) std::cout << "finished work for level "  << label << "\n";
      
  }


  //--------------------------------------- JUMP_DOWN special case ---------------------------------------

  template <typename Policy, typename PQ1, typename PQ2>
  inline typename Policy::__dd_type
  replace_jump_down_sweep(const typename Policy::dd_type& dd, 
                          replace_func<Policy> m,
                          exec_policy ep,
                          size_t pq1_mem,
                          size_t max_pq1_size,
                          size_t pq2_mem,
                          size_t max_pq2_size) 
  {
    using label_t = typename Policy::label_type;
    if (debug_enabled) std::cout << "start jump_down special case! \n";

    //setup input
    node_ifstream<> in(dd);
    node v = in.pull(); //there must be at least 2 nodes else couldn't be this case

    //setup output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);
    out_arcs->max_1level_cut = 0;

    //finding jump_down levels and targets
    //TODO maybe move vec building to seperate function?
    //vecs to fill
    std::vector<label_t> jump_starts, jump_targets;
    {
      //open info file
      level_info_ifstream<> info_in(dd);
      while (info_in.can_pull()){
        const level_info l = info_in.pull();
        const label_t lbl  = l.label();
        const label_t tgt  = m(lbl);
        if (debug_enabled) std::cout << "found level " << l << "\n";
        if (tgt > lbl) {
          jump_starts.push_back(lbl);
          jump_targets.push_back(tgt);
        }
      }
    }

    //build generators
    const generator<label_t> level_gen = make_generator(jump_starts.begin(), jump_starts.end());
    const generator<label_t> target_gen = make_generator(jump_targets.begin(), jump_targets.end());

    optional<label_t> next_jump_down = level_gen();

    //setup PQs
    PQ1 pq1({dd,target_gen}, pq1_mem , max_pq1_size, stats_replace.lpq);
    PQ2 pq2(pq2_mem, max_pq2_size);

    //init req
    cor_req_t<0> init_r;
    if (v.uid().level() == *next_jump_down) {
      if (debug_enabled) {std::cout << "CASE first level moves down\n";}
      //push proper req
       init_r = {{v.low(), v.high()},{},{ptr_uint64::nil(), m(v.uid().level())}};
       next_jump_down = level_gen();
    } else {
      if (debug_enabled) {std::cout << "CASE first level doesn't moves down\n";}
      //just init copy req
       init_r = {{v.uid(),node::pointer_type::nil() },
                 {},{ptr_uint64::nil(), node::pointer_type::nil().level()}};
    }
    if (debug_enabled) std::cout << "init jump_down req: " << init_r << "\n";
    pq1.push(init_r); 

    while(!pq1.empty()) {
      pq1.setup_next_level();
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq1.size());
      const label_t cur_label = pq1.current_level();

      if (cur_label == next_jump_down) {
      // CASE jump down start 
        if (debug_enabled) std::cout << "found jump down level " << cur_label << "\n";
        //push reqs
        while(!pq1.empty_level()) {
          const cor_req_t<0> req = pq1.pull();
          const ptr_uint64 t_uid(req.data.level, 0);
          const ptr_uint64 tseek = std::min(req.target.first(), t_uid);
          while (v.uid() < tseek && in.can_pull()) { v = in.pull(); }
          const cor_req_t<0> n_req = {{v.low(), v.high()},{},{req.data.source, m(cur_label)}};
          if (debug_enabled) std::cout << "pushing req to PQ1 " << n_req << "\n";
          pq1.push(n_req);
        }
        //update next_jump_down
        next_jump_down = level_gen();
        //no level update since this level no longer exists
        continue;
      }
      // CASE normal layer --> Do normal correctify for this level
      correctify_single_level<Policy>(in, aw, pq1, pq2,  v, label_indicator::NORMAL);
    }
    return typename Policy::__dd_type(out_arcs,ep); 
  }

// ------------------------------------- Adj Swap special case -------------------------------------

template<typename PQ1, typename sorter_t>
class adj_swap_pq_decorator{
  //decorator very like up_pq_decorator but pushes to sorter when level is min in req
  public:
    using value_type = typename PQ1::value_type;
    using value_comp_type = typename PQ1::value_comp_type;
    static constexpr memory_mode mem_mode = PQ1::mem_mode;
    using level_type = typename value_type::pointer_type::label_type;
    static constexpr level_type no_label = PQ1::no_label;
  private:
    PQ1& _pq1; //ref to pq1
    sorter_t& _sorter; //ref to sorter

  //constructor
  public:
    adj_swap_pq_decorator(PQ1& pq1, sorter_t& sorter) :
    _pq1(pq1), 
    _sorter(sorter)
    {}

  size_t terminals(const bool terminal_value) const {return _pq1.terminals(terminal_value);}
  size_t size() const {return _pq1.size() + _sorter.size();}

  size_t size_without_terminals() const{
    return size() - terminals(false) - terminals(true);
  }

  bool empty_level() const          { return  _pq1.empty_level();}
  bool can_pull() const             { return  _pq1.can_pull();}
  cor_req_t<0> pull()               { return  _pq1.pull(); }
  bool has_top() const              { return  _pq1.has_top(); }
  cor_req_t<0> top()                { return  _pq1.top(); }
  void pop()                        { return  _pq1.pop(); }
  bool has_current_level() const    { return  _pq1.has_current_level(); }
  level_type current_level() const  { return  _pq1.current_level(); }
  bool empty() const { return size() == 0u;}  

  void setup_next_level(level_type stop_level = no_label)
    { _pq1.setup_next_level(stop_level);}

  //pushing
  void push(const cor_req_t<0> r){
    if (r.data.level <= r.target.first().level()){
      if(debug_enabled) std::cout << "pushing to sorter: " << r << "\n";
      _sorter.push(r);
    } else {
      if(debug_enabled) std::cout << "pushing to pq1: " << r << "\n";
      _pq1.push(r);
    }
  }

};


template <typename Policy, typename PQ1, typename PQ2, typename sorter_t>
inline typename Policy::__dd_type
replace_adj_swap_sweep(const typename Policy::dd_type& dd, 
                          replace_func<Policy> m,
                          exec_policy ep,
                          size_t pq1_mem,
                          size_t max_pq1_size,
                          size_t pq2_mem,
                          size_t max_pq2_size,
                          size_t sorter_mem,
                          size_t sorter_max) {
  //ok so:
  //layers above xi might push to sorter instead of pq
  //when we reach level xj first run stuff for xj, then handle all reqs in sorter
  //these will be in the "correct level" case always
  //when done reset sorter for next swap

  //setup input
    node_ifstream<> in(dd);
    node v = in.pull();
    
  //setup output
  shared_levelized_file<arc> out_arcs;
  arc_ofstream aw(out_arcs);   
  out_arcs->max_1level_cut = 0;
  
  //identifying swaps -> being put in this special case we already know we have only non-overlapping swaps
    level_info_ifstream<> info_in(dd);
    //top of adj swap, bot of adj swap,  fresh layer below bottom (need to load PQ with these)
    std::vector<typename Policy::label_type> swap_starts, swap_end; 
    while (info_in.can_pull()){
      level_info l = info_in.pull();
      if (debug_enabled) std::cout << "found level " << l << "\n";
      if (m(l.label()) > l.label()) {
        swap_starts.push_back(l.label());
        swap_end.push_back(m(l.label()));
      }
    }

    const generator<typename Policy::label_type> level_gen = make_generator(swap_starts.begin(), swap_starts.end());
    const generator<typename Policy::label_type> end_gen = make_generator(swap_end.begin(), swap_end.end());
    optional<typename Policy::label_type> next_swap = level_gen();
    optional<typename Policy::label_type> next_target = end_gen();


    //setup PQs
    PQ1 pq1({dd}, pq1_mem , max_pq1_size, stats_replace.lpq);
    PQ2 pq2(pq2_mem, max_pq2_size);

    //setup sorter
    sorter_t sorter(sorter_mem, sorter_max);
    
    //decorator to sometimes push to sorter instead
    adj_swap_pq_decorator<PQ1, sorter_t> apq(pq1, sorter);

    cor_req_t<0> init_req;
    if(v.uid().label() == next_swap){
      //push 2-ary to children
      if(debug_enabled) std::cout << "root is part of a swap\n";
      init_req = {{v.low(), v.high()},{},{ptr_uint64::nil(),next_target.value() +1}};
      
    } else {
      //just push 1-ary
      if(debug_enabled) std::cout << "root is NOT part of a swap\n";
      init_req = {{v.uid(), ptr_uint64::nil()},{},{ptr_uint64::nil()}};
    }
    apq.push(init_req);

    while(!apq.empty()){
      apq.setup_next_level();
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, apq.size());
      const typename Policy::label_type label = apq.current_level();
      if(debug_enabled) std::cout << "starting work for level" << label <<"\n";

      if(label == next_swap){ 
        if (debug_enabled) std::cout << "found top of next adjacent swap: " << label << "\n";
        //we just pushing 2 ary reqs
         while(!apq.empty_level()){
          const cor_req_t<0> req = apq.pull();
          if (debug_enabled) std::cout << "found req " << req << "\n";
          const ptr_uint64 t_uid(req.data.level, 0);
          ptr_uint64 tseek = std::min(req.target.first(), t_uid);
          while (v.uid() < tseek && in.can_pull()) { v = in.pull(); }
          const cor_req_t<0> n_req = {{v.low(), v.high()},{},{req.data.source, next_target.value() +1 }};
          apq.push(n_req);
        }

      } else if (label == next_target) {
        if (debug_enabled) std::cout << "found next target level: " << label << "\n";
        //SHOULD
        //(1) handle level xj as correctify but
        //    (a) never in correct layer case!
        //    (b) when pushing reqs, if level is min, push to pq3 instead (handled by the new policy)
        //    (c) out_label should be next_swap aka current level -1 (handled by label_indicator)
        correctify_single_level<Policy>(in, aw, apq, pq2, v,  label_indicator::DEC);
        //make sure to update cut since we just continue here 
        out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, apq.size());
        //(2) handle the extra level as correctify but
        //    (a) always in correct layer case!
        //    (b) pulls requests from sorter
        //    (c) out_label should be current level -1 aka next_target
        sorter.sort();
        typename Policy::label_type id = 0;
        while(sorter.can_pull()){
          const cor_req_t<0> extra_r = sorter.top();
          if (extra_r.target[0] == extra_r.target[1]) {
            if (debug_enabled)std::cout << "skip surpressible node! \n";
            apq.pop();
            const cor_req_t<0> r1 = {{extra_r.target[0],node::pointer_type::nil()}, {}, {extra_r.data.source}};
            if (debug_enabled)std::cout << "pushes req " << r1 << "\n";
            apq.push(r1);
            continue;
          }
 
          const typename Policy::uid_type out_uid(next_target.value(), id);
          id++;
          pusher<Policy>(apq, aw, out_uid.as_ptr(false), {extra_r.target[0], node::pointer_type::nil()}, node::pointer_type::nil().level());
          pusher<Policy>(apq, aw, out_uid.as_ptr(true),  {extra_r.target[1], node::pointer_type::nil()}, node::pointer_type::nil().level());

          // forward incoming
          //TODO find a way to use internal pusher here?
          while (sorter.has_top() && sorter.top().target == extra_r.target){
            const cor_req_t<0> r1 = sorter.pull();
            if (debug_enabled) std::cout << "has pulled: " << r1 << "\n";
            if (r1.data.source.level() != ptr_uint64::nil().level()) {
              if (debug_enabled) std::cout << "has pushed internal: " << arc{r1.data.source, out_uid} << "\n";
              aw.push({unflag(r1.data.source), out_uid});
              //TODO : unfalg here because we use roots sorter class from nested sweeping directly
              // could define own class to avoid
            }
          }

        }
        //update level info
        if (id >= 0 ) {aw.push(level_info(next_target.value(), id+1));}
        //now update all the values!
        next_swap = level_gen();
        next_target = end_gen();
        sorter.reset();

      } else {
         if (debug_enabled) std::cout << "found regular level: " << label << "\n";
         correctify_single_level<Policy>(in, aw, apq, pq2, v,  label_indicator::NORMAL);
      }

    }
    if (debug_enabled) std::cout << "finished all levels :D \n";
    return typename Policy::__dd_type(out_arcs,ep); 




}

//NOTE TO SELF
//all levels are multiplied by 2 to ensure that the extra level we work with for each swap is free
//the initial doubling means that we do an extra sweep - could be done with affine shift instead to avoid (TODO?)
//shiftign back again is doen on the fly as arcs are output
  template <typename Policy, typename PQ1, typename PQ2>
  typename Policy::__dd_type
  replace_adj_swap_sweep_double(const typename Policy::dd_type& dd, 
                          replace_func<Policy> m,
                          exec_policy ep,
                          size_t pq1_mem,
                          size_t max_pq1_size,
                          size_t pq2_mem,
                          size_t max_pq2_size) {
    if (debug_enabled) std::cout << "\n start adj_swap special case! \n";
    //makign room for intermediate layers
    replace_func<Policy> shift_forward = [](int x){ return x*2;};
    //TODO - do this as a affine shift??
    const typename Policy::dd_type dd_shifted = bdd_replace(dd, shift_forward,replace_type::Monotone); //expeeensive                       

    //setup input
    node_ifstream<> in(dd_shifted);
    node v = in.pull();
    
    //setup output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);   

    //identifying swaps -> being put in this special case we already know we have only non-overlapping swaps
    level_info_ifstream<> info_in(dd_shifted);
    std::vector<typename Policy::label_type> swap_starts; //top of adj swap
    std::vector<typename Policy::label_type> swap_end; // bot of adj swap
    std::vector<typename Policy::label_type> swap_extra; // fresh layer below bottom (need to load PQ with these)
    while (info_in.can_pull()){
      level_info l = info_in.pull();
      if (debug_enabled) std::cout << "found level " << l << "\n";
      if (m(l.label()/2) > l.label()/2) {
        swap_starts.push_back(l.label());
        swap_end.push_back(m(l.label()/2)*2);
        swap_extra.push_back(m(l.label()/2)*2 + 1);
      }
    }
    if (debug_enabled) std::cout << "extra levels [ ";
    for(typename Policy::label_type e : swap_extra) {if (debug_enabled) std::cout << e << ",";}
    if (debug_enabled) std::cout << "]";

    typename std::vector<typename Policy::label_type>::iterator s_begin = swap_starts.begin(), s_end = swap_starts.end();
    typename std::vector<typename Policy::label_type>::iterator t_begin = swap_end.begin(), t_end = swap_end.end();
    typename std::vector<typename Policy::label_type>::iterator e_begin = swap_extra.begin(), e_end = swap_extra.end();
    generator<typename Policy::label_type> level_gen = make_generator(s_begin, s_end);
    generator<typename Policy::label_type> end_gen = make_generator(t_begin, t_end);
    generator<typename Policy::label_type> extra_gen = make_generator(e_begin, e_end);
    optional<typename Policy::label_type> next_swap = level_gen();
    optional<typename Policy::label_type> next_target = end_gen();

    //setup PQs
    PQ1 pq1({dd_shifted, extra_gen}, pq1_mem , max_pq1_size, stats_replace.lpq);
    PQ2 pq2(pq2_mem, max_pq2_size);

    //init request
    cor_req_t<0> init_req;
    if(v.uid().label() == next_swap){
      //push 2-ary to children
      if(debug_enabled) std::cout << "root is part of a swap\n";
      init_req = {{v.low(), v.high()},{},{ptr_uint64::nil(),next_target.value() +1}};
    } else {
      //just push 1-ary
      if(debug_enabled) std::cout << "root is NOT part of a swap\n";
      init_req = {{v.uid(), ptr_uint64::nil()},{},{ptr_uint64::nil()}};
    }
    pq1.push(init_req);
    
    while(!pq1.empty()){ 
      pq1.setup_next_level();
      typename Policy::label_type label = pq1.current_level();
      typename Policy::label_type id = -1;
      if(debug_enabled) std::cout << "starting work for level" << label <<"\n";

      if(label == next_swap){
      /////////////////////////////////////////// xi level //////////////////////////////////////////////////////
         if (debug_enabled) std::cout << "found top of next adjacent swap: " << label << "\n";
        //we just pushing 2 ary reqs
         while(!pq1.empty_level()){
          cor_req_t<0> req = pq1.pull();
          if (debug_enabled) std::cout << "found req " << req << "\n";
          const ptr_uint64 t_uid(req.data.level, 0);
          ptr_uint64 tseek = std::min(req.target.first(), t_uid);
          while (v.uid() < tseek && in.can_pull()) { v = in.pull(); }
          cor_req_t<0> n_req = {{v.low(), v.high()},{},{req.data.source, next_target.value() +1 }};
          if (debug_enabled) std::cout << "pushing req to PQ1 " << n_req << "\n";
          pq1.push(n_req);
        }
      } else if (label == next_target){
       /////////////////////////////////////////// xj level //////////////////////////////////////////////////////
        if (debug_enabled) std::cout << "found target of next adjacent swap: " << label << "\n";
        // TODO: evetually - could specialize single layer correctify to do these things -> would need a new policy prob..
        //correctify but 
        // (0) ignore correct layer case..
        // (1) out_uid has label (next_swap)/2
        // (2) pushes to PQ1 are given level = label+1 (aka next_extra) for non-copy case
        
        //just copy-paste single layer correctify for now..
        while((!pq1.empty_level()) || pq2.has_top()){
        cor_req_t<1> r = getNext(pq1, pq2);
        const ptr_uint64 t_uid(r.data.level, 0);
        ptr_uint64 tseek = (r.empty_carry()) ? std::min(r.target.first(), t_uid) : r.target.second(); 
        while (v.uid() < tseek && in.can_pull()) { v = in.pull(); }

        
        //CASE should push to PQ2
        //big copy-paste from prod + small changes 
        if (r.empty_carry() && r.target[0].is_node() && r.target[1].is_node()
              && r.target[0].label() == r.target[1].label()
              && r.target[0].id() != r.target[1].id()) {
            if(debug_enabled) std::cout << "enters pq2 push if-statement! r is" << r << "\n";
            if(debug_enabled) std::cout << "top of pq1 is" << pq1.top() << "\n";
            const typename Policy::children_type children = v.children();
            while (pq1.has_top() && pq1.top().target == r.target) {
              if(debug_enabled) std::cout << "enters pq2 while\n";
              cor_req_t<1> nr = {r.target, { children }, pq1.top().data};
              pq2.push(nr);
              pq1.pop();
            }
            continue;
        }

          //CASE wrong layer
          if (debug_enabled) std::cout << "wrong layer case \n";
          id = (label == tseek.label()) ? id+1 : 0; 
          label = tseek.label() ;
          if(debug_enabled) std::cout << "label, id: " << label << "," << id << "\n";

          tuple<tuple<typename Policy::pointer_type>> reqs = reqFor<Policy>(r.target, v, r.node_carry[0][0], r.node_carry[0][1]);
          tuple<typename Policy::pointer_type> rlow = reqs[0]; 
          tuple<typename Policy::pointer_type> rhigh = reqs[1]; 
          
          //forward outgoing
          const node::uid_type out_uid(next_swap.value()/2, id); //
          pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(false), rlow, r.data.level);
          pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(true), rhigh, r.data.level);

          // forward incoming
          internal_pusher<Policy, PQ1, 0>(pq1, aw, out_uid, r.target,  r.data.level);
          internal_pusher<Policy, PQ2, 1>(pq2, aw, out_uid, r.target,  r.data.level);
        }
        if (id >= 0) {aw.push(level_info(next_swap.value()/2, id+1));}

      } else if (next_target.has_value() && label == next_target.value() + 1){
      /////////////////////////////////////////// extra level //////////////////////////////////////////////////////
      if (debug_enabled) std::cout << "found extra level: " << label << "\n";
        //correctify but
        // (0) we always in correct layer case 
        // (1) push to out with label-1 /2 (aka next_target)
        while(!pq1.empty_level()) {
          cor_req_t<0> r = pq1.top();
          if(debug_enabled) std::cout << "found req " << r << "\n";
          const ptr_uint64 t_uid(r.data.level, 0);
          ptr_uint64 tseek = (r.empty_carry()) ? std::min(r.target.first(), t_uid) : r.target.second(); 
          while (v.uid() < tseek && in.can_pull()) { v = in.pull(); }
          
          //SUBCASE suppresible node case - push one req
          if (r.target[0] == r.target[1]) {
            if (debug_enabled)std::cout << "skip surpressible node! \n";
            pq1.pop(); //remove req without pushing internal
            //(source -> (target[0], nil))
            cor_req_t<0> r1 = {{r.target[0],node::pointer_type::nil()}, {}, {r.data.source}};
            if (debug_enabled)std::cout << "pushes req " << r1 << "\n";
            pq1.push(r1);
            
          } else {
            //SUBCASE not surpressible
            id = (label == tseek.label()) ? id+1 : 0; 
            label = tseek.label() ;
            if(debug_enabled) std::cout << "label, id: " << label << "," << id << "\n";

            //push copy reqs
            const node::uid_type out_uid(next_target.value()/2, id); //x_label,id
            typename Policy::label_type nil_lbl = node::pointer_type::nil().level();
            tuple<typename Policy::pointer_type> tl = {r.target[0],node::pointer_type::nil()};
            tuple<typename Policy::pointer_type> th = {r.target[1],node::pointer_type::nil()};
            pusher<Policy>(pq1,aw,out_uid.as_ptr(false),tl,nil_lbl);
            pusher<Policy>(pq1,aw,out_uid.as_ptr(true), th,nil_lbl);

            // forward incoming
            internal_pusher<Policy, PQ1, 0>(pq1, aw, out_uid, r.target,  r.data.level);
          }
        }
          //push the level
          if (id >= 0) {aw.push(level_info(next_target.value()/2, id+1));}
          //now update all the values!
          next_swap = level_gen();
          next_target = end_gen();

      } else {
      /////////////////////////////////////////// copy level //////////////////////////////////////////////////////
        //we're not doing adj swaps - just copy like always.. 
        if (debug_enabled) std::cout << "found non-involved level: " << label << "\n";
        correctify_single_level<Policy>(in, aw, pq1, pq2,  v, label_indicator::SHIFT_BACK);
      }

    }
    if (debug_enabled) std::cout << "finished all levels :D \n";
    return typename Policy::__dd_type(out_arcs,ep); 
}

//------------------------------------------------- JUMP UP special case -------------------------------------------------
//jump_up -> bottom-up sweep like reduce, but doubling edges to remeber both jump-up variable's sub-trees in parents

/////reduce types extended with payload
struct jump_up_mapping{
  node::uid_type old_uid;
  node::pointer_type new_uid;
  assignment payload = assignment::None; 

  std::string 
  to_string() const {
    std::stringstream stream;
    const std::string payload = (this->payload == assignment::None) ? "NONE" : ((this->payload == assignment::True) ? "T" : "⊥");
    stream << "( " << old_uid << ", " << new_uid << ", " << payload << ")";
     return stream.str();
  }
};

struct  jump_up_arc : public arc {
  // fields
    assignment payload;
    arc::label_type xi;
  public:
    //constructors... i need like a million?
    jump_up_arc(const arc& a, const assignment p , const arc::label_type xi) : 
    arc(a), payload(p), xi(xi) {}

    //defaults
    jump_up_arc() = default;
    jump_up_arc(const arc& a): arc(a) {payload = assignment::None; xi = 0;}
    jump_up_arc(const jump_up_arc&) = default;
    jump_up_arc& operator=(const jump_up_arc& a) = default;
  
  //level for pq
  arc::label_type level() const {
    //if source above jump target (xi), let level be xi st. it's handled at level xi
    return std::max(source().label(), xi);
  }
  //printing to include payload..?
  std::string
    to_string() const
    {
      std::stringstream stream;
      const std::string arrow =
        !this->source().is_node() || this->source().out_idx() ? " ---> " : " - -> ";
      const std::string payload = (this->payload == assignment::None) ? "NONE" : ((this->payload == assignment::True) ? "T" : "⊥");
      stream << this->source() << arrow << this->target() << ", p: " << payload ;
      return stream.str();
    }
};

struct jump_up_node : public node {
  assignment _payload = assignment::None;

  //defaults
  jump_up_node() = default;
  jump_up_node(const jump_up_node& n) = default;
  jump_up_node(const node& n, const assignment payload )
    : node(n), _payload(payload)
  {}
  jump_up_node&
  operator=(const jump_up_node& n) = default;
};

//specifically for keeping canonicity when making nodes on jump target level 
struct jump_up_node_ws : public node {
  assignment _payload = assignment::None;
  node::pointer_type _source;

  //defaults
  jump_up_node_ws() = default;
  jump_up_node_ws(const jump_up_node_ws& n) = default;
  jump_up_node_ws(const node& n, const assignment payload, const node::pointer_type source )
    : node(n), _payload(payload), _source(source)
  {}
  jump_up_node_ws&
  operator=(const jump_up_node_ws& n) = default;
};

/////Comparators for new types
struct jump_up_queue_lt //for pq
  {
    bool
    operator()(const jump_up_arc& a, const jump_up_arc& b)
    {
      // We want: sort by source then payload then low/high child
      // should this take into account the weird xi stuff? no right? thats just for pq
      if (a.source().level() >  b.source().level()) {return true;} //if one source is greater it should be first
      if (a.source().level() <  b.source().level()) {return false;} //if one source is greater it should be first
      if (a.source().id() > b.source().id()) {return true;}
      if (a.source().id() < b.source().id()) {return false;}
      //if we get to here the sources must have same uids..
      //so now- we decide: no payload < false payload < true payload (follows ternary type ints)
      if (a.payload < b.payload ) {return true;}
      if (a.payload > b.payload ) {return false;}
      //if we get here thay also have same payload..
      //sort on arc type
      return a.out_idx() > b.out_idx();
    }
  };

 
  struct jump_reduce_uid_lt
  {
    bool
    operator()(const jump_up_mapping& a, const jump_up_mapping& b)
    {
      //grouping payloads
      if (a.old_uid == b.old_uid) {return a.payload < b.payload;} //we want false first
      return a.old_uid > b.old_uid;
    }
  };
////helpers
inline jump_up_node
j_node_of(const jump_up_arc& low, const jump_up_arc& high){
  //this is just big copy paste of regular node_of except it packs payload
  //also checks that the two arcs have the same payload (or one is none)
  adiar_assert(essential(low.source()) == essential(high.source()), "Source are the same origin");

  adiar_assert(low.out_idx() == 0u, "Out-index is correct on low arc");
  adiar_assert(high.out_idx() == 1u, "Out-index is correct on high arc");

  adiar_assert(!low.target().is_node() || low.target().out_idx() == 0u,
                "Out-index is empty in low target");
  adiar_assert(!high.target().is_node() || high.target().out_idx() == 0u,
                "Out-index is empty in high target");

  adiar_assert(low.source().is_flagged() == false, "Source is not flagged on low arc");
  adiar_assert(high.source().is_flagged() == false, "Source is not flagged on high arc");

  adiar_assert(low.payload == assignment::None || high.payload == assignment::None || low.payload == high.payload, "the arcs have same payload");
  adiar_assert(essential(low.source()) == low.source()
                && essential(high.source()) == low.source());
  node res = node(node::uid_type(low.source()), low.target(), high.target());
  assignment act_payload = (low.payload == assignment::None) ? high.payload : low.payload;
  return jump_up_node(res, act_payload);
}

template <typename pq_t, typename arc_ifstream_t>
inline jump_up_arc
  _jump_get_next(pq_t& reduce_pq, arc_ifstream_t& arcs){
    if (!reduce_pq.can_pull()
        || (arcs.can_pull_terminal() && arcs.peek_terminal().source() > reduce_pq.top().source())) {
      return jump_up_arc(arcs.pull_terminal(), assignment::None, 0); //maybe dangerous..
    } else {
      return reduce_pq.pull();
    }
  }



template <typename Policy, typename pq_t, template <typename, typename> typename sorter_t>
typename Policy::dd_type 
replace_jump_up_sweep(const shared_levelized_file<arc>& dd,
                      replace_func<Policy> m,
                      [[maybe_unused]] exec_policy ep, 
                      size_t pq_mem, 
                      size_t max_pq_size,
                      size_t sorters_mem){
  if (debug_enabled) std::cout << "entered jump_up special case\n";
  //setting up input
  arc_ifstream<> arcs(dd);
  level_info_ifstream<> levels(dd); 
  
  //setting up output
  shared_levelized_file<typename Policy::node_type> out_file = __reduce_init_output<Policy>();
  node_ofstream out(out_file);

  //finding jump_starts and targets
  std::vector<typename Policy::label_type> jump_starts, jump_targets;
  {
    level_info_ifstream<> levels1(dd);
    while(levels1.can_pull()){
      level_info li = levels1.pull();
      if(li.level() > m(li.level())) {
        jump_starts.push_back(li.level());
        jump_targets.push_back(m(li.level()));
      }
    }
  }
  //making generators..
  generator<typename Policy::label_type> level_gen = make_generator(jump_starts.begin(), jump_starts.end());
  generator<typename Policy::label_type> target_gen_for_pq = make_generator(jump_targets.begin(), jump_targets.end());
  generator<typename Policy::label_type> target_gen_for_me = make_generator(jump_targets.begin(), jump_targets.end());
  optional<typename Policy::label_type> xj = level_gen();
  optional<typename Policy::label_type> xi = target_gen_for_me();


  //setting up pq
  pq_t pq({dd, target_gen_for_pq}, pq_mem, max_pq_size, stats_replace.lpq);

  while(!pq.empty() || arcs.can_pull_terminal()){
    if (debug_enabled) std::cout << "still more arcs to process!\n";
    //find next level (max seen in pq or arc file)
    typename Policy::label_type level;
    if(!arcs.can_pull_terminal() || (!pq.empty() && pq.has_current_level() && pq.current_level() >= arcs.peek_terminal().source().level())){
      //max is from pq
      level = pq.current_level();
      if (debug_enabled) std::cout << "chose level " << level << "from pq\n";
    } else {
      level = arcs.peek_terminal().source().level();
      if (debug_enabled) std::cout << "chose level " << level << "from arc file\n";
    }

    //case distinction on level
    if (level > xj) {
      //////////////////////////////////////// non-involved level ////////////////////////////////////////
      //just perform regular reduce work..
      //TODO: could potentially just be fast reduce?
      if (debug_enabled) std::cout << "found regular level " << level << "\n";
      const size_t unreduced_width = levels.pull().width() *2; //TODO: figure out why this fails if no *2
      __reduce_level<Policy, sorter_t, pq_t>(arcs, level, level, pq, out, sorters_mem, unreduced_width);
    } else if (level == xj) {
      //////////////////////////////////////// jump start level ////////////////////////////////////////
      //reduce but
      //(1) output no nodes
      //(2) make 2 F2 mappings, one for each payload
      //(3) when forwarding, for each incoming push 2 arcs, one for each payload for that source
      //(4) in resulting bdd -> no nodes on this level so update cuts accordingly..
      if (debug_enabled) std::cout << "found jump level " << level << "\n";

      //temp files
      iofstream<jump_up_mapping> red1_mapping;
      size_t unreduced_width = (level == xi) ? max_pq_size : levels.pull().width() *2; 
      if (debug_enabled) std::cout << "width of current layer " << unreduced_width << "\n";
      sorter_t<jump_up_node, reduce_node_children_lt> child_grouping(sorters_mem, unreduced_width, 2);
      sorter_t<jump_up_mapping, jump_reduce_uid_lt> red2_mapping(sorters_mem, unreduced_width, 2);

      while ((arcs.can_pull_terminal() && arcs.peek_terminal().source().label() == level)
              || pq.can_pull()) {
        const jump_up_arc e_high = _jump_get_next(pq, arcs);
        const jump_up_arc e_low  = _jump_get_next(pq, arcs);
        const jump_up_node n = j_node_of(e_low, e_high);
        if (debug_enabled) std::cout << "pulled arcs: " << e_high.to_string() << ", " << e_low.to_string() << ", to build " << n << "\n";

        //red rule 1 (kill suppresible)
        if (unflag(n.low()) == unflag(n.high())){
          if (!red1_mapping.is_open()) { red1_mapping.open(); }
          red1_mapping.write({ n.uid(), n.low() });
        } else {
          child_grouping.push(n);
        }
      }

      // Count number of arcs that cross this level
      cuts_t local_1level_cut   = { { 0u, 0u, 0u, 0u } };
      cuts_t tainted_1level_cut = { { 0u, 0u, 0u, 0u } };

      __reduce_cut_add(local_1level_cut,
                      pq.size_without_terminals(),
                      pq.terminals(false) + arcs.unread_terminals(false),
                      pq.terminals(true) + arcs.unread_terminals(true));

      //red rule 2 (merge duplicates)
      child_grouping.sort(); //group duplicates 
      node seen_node = node(node::uid_type(), ptr_uint64::nil(), ptr_uint64::nil());
      while (child_grouping.can_pull()){
        //for each non-dupe pulled we want to push two mappings - one for \bot, one for \top
        //DOUBLE CHECK: should we also do it for dupe nodes??
        const node next_node = child_grouping.pull();
        if (seen_node.low() != unflag(next_node.low()) || seen_node.high() != unflag(next_node.high())) {
          seen_node = next_node;
          red2_mapping.push({ next_node.uid(), next_node.low(), assignment::False });
          red2_mapping.push({ next_node.uid(), next_node.high(), assignment::True });
          if (debug_enabled) std::cout << "pushed F2 mapping: (" << next_node.uid() << "-->" << next_node.low() << ", p: ⊥)\n";
          if (debug_enabled) std::cout << "pushed F2 mapping: (" << next_node.uid() << "-->" << next_node.high() << ", p: T)\n";
          //notice: outputting no nodes, so no updated to cuts
        }
      }

      //forwarding
      red2_mapping.sort();

      //handling F1 specially...
      jump_up_mapping next_red1  = { node::uid_type(), node::uid_type() }; // <-- dummy value
      bool has_next_red1 = red1_mapping.is_open() && red1_mapping.size() > 0;
      if (has_next_red1) {
        red1_mapping.seek_begin();
        next_red1 = red1_mapping.next();
      }

      while (has_next_red1 || red2_mapping.can_pull()) {
        const bool is_red1_current = !(red2_mapping.can_pull()) || (has_next_red1 && next_red1.old_uid > red2_mapping.top().old_uid);
        if (is_red1_current) {
          //then handle like normal -> we skipping xi node
          const jump_up_mapping current_map = next_red1;
          while (arcs.can_pull_internal() && current_map.old_uid == arcs.peek_internal().target()) {
               const ptr_uint64 s = arcs.pull_internal().source();
               const ptr_uint64 t = flag(current_map.new_uid);
               jump_up_arc n_req = {jump_up_arc({s, t}, assignment::None, xi.value())};
               if (debug_enabled) std::cout << "pushing req " << n_req << "\n";
               pq.push(n_req);
            }
        } else {
          // for each ingoing : pull 2 mappings and push 2 reqs
          const jump_up_mapping map1 = red2_mapping.pull();
          const jump_up_mapping map2 = red2_mapping.pull();
          if (debug_enabled) std::cout << "found mappings: " << map1.to_string()  << ", "<< map2.to_string() <<"\n";

          adiar_assert(map1.old_uid == map2.old_uid, "pulled mappings not for same old uid!");
          
          while (arcs.can_pull_internal() && map1.old_uid == arcs.peek_internal().target()) {
              const ptr_uint64 s = arcs.pull_internal().source();
              if (debug_enabled) std::cout << "found matching req starting at " << s << "\n";
              const jump_up_arc n1 = {{s,map1.new_uid}, map1.payload, xi.value()};
              const jump_up_arc n2 = {{s,map2.new_uid}, map2.payload, xi.value()};
              if (debug_enabled) std::cout << "pushing requests: " << n1.to_string() << " and " << n2.to_string() << "\n";
              pq.push(n1);
              pq.push(n2);
            }
        }
        //update next_red1 if any..
        if(is_red1_current) {
          has_next_red1 = red1_mapping.has_next(); 
          if (has_next_red1) {next_red1 = red1_mapping.next();}
        }
      }
      red1_mapping.close();
      //updating cuts?? (think no update happens?)
      out.unsafe_max_1level_cut(local_1level_cut);
      out.unsafe_inc_1level_cut(tainted_1level_cut);
      //epilogue - does a bunch of checks + setup of levelized pq
      const bool terminal_value = next_red1.new_uid.is_terminal() && next_red1.new_uid.value();
      __reduce_level__epilogue<>(arcs, pq, out, terminal_value);

    } else if (level > xi) {
      //////////////////////////////////////// in-between level ////////////////////////////////////////
      //reduce but
      //(1) build nodes from payload pairs, if non-payload arc pulled -> use it twice
      //(2) when forwarding, for each incoming push 2 arcs, one for each payload for that source
      if (debug_enabled) std::cout << "found in-between level " << level << "\n";

      //temp files
      iofstream<jump_up_mapping> red1_mapping;
      size_t unreduced_width = (level == xi) ? max_pq_size : levels.pull().width() *2; 
      if (debug_enabled) std::cout << "width of current layer " << unreduced_width << "\n";
      sorter_t<jump_up_node, reduce_node_children_lt> child_grouping(sorters_mem, unreduced_width, 2);
      sorter_t<jump_up_mapping, jump_reduce_uid_lt> red2_mapping(sorters_mem, unreduced_width, 2);

      while ((arcs.can_pull_terminal() && arcs.peek_terminal().source().label() == level)
              || pq.can_pull()) {
        const jump_up_arc e_1 = _jump_get_next(pq, arcs);
        const jump_up_arc e_2 = _jump_get_next(pq, arcs);
        if (e_1.payload == e_2.payload) {
          //we chilling build node and push if non supressible
          if (debug_enabled) std::cout << "pulled arcs: " << e_1.to_string() << ", " << e_2.to_string();
          const jump_up_node n = j_node_of(e_2, e_1);
          if (debug_enabled) std::cout << ", to build " << n << "\n" ;
          //check red 1
          if (unflag(n.low()) == unflag(n.high())){
            if (!red1_mapping.is_open()) { red1_mapping.open(); }
            red1_mapping.write({ n.uid(), n.low(), e_1.payload });
            if (debug_enabled) std::cout << "new F1 mapping" << n << "\n";
          } else {
            child_grouping.push(n);
            if (debug_enabled) std::cout << "pushed node" << n << "\n";
          }
        } else {
          //diff payloads means either one of them is none, or there is an additional none to pull
          const jump_up_arc e_3 = _jump_get_next(pq, arcs);
          //case distinction to figure out what is low and high...
          adiar_assert(e_1.out_idx(), "somehow first pulled is not a high edge??");
          if (debug_enabled) std::cout << "pulled arcs: " << e_1.to_string() << ", " << e_2.to_string() << ", " << e_3.to_string() << "\n" ;
          jump_up_node n1, n2;
          if (e_2.out_idx()) {
            //then third pulled must be none
            if (debug_enabled) std::cout << "we conclude that third is none and thus low edge\n";
            n1 = j_node_of(e_3, e_1);
            n2 = j_node_of(e_3, e_2);
          } else {
            //first pulled must be none?
            if (debug_enabled) std::cout << "we conclude that first is none and thus second and third low edge\n";
            n1 = j_node_of(e_2, e_1);
            n2 = j_node_of(e_3, e_1);
          }
          if (debug_enabled) std::cout << "built nodes: " << n1 << ", " << n2 << "\n" ;
          //check red 1
          if (unflag(n1.low()) == unflag(n1.high())){
            if (!red1_mapping.is_open()) { red1_mapping.open(); }
            red1_mapping.write({ n1.uid(), n1.low() , n1._payload});
            if (debug_enabled) std::cout << "new F1 mapping" << n1 << "\n";
          } else {
            child_grouping.push(n1);
            if (debug_enabled) std::cout << "pushed node" << n1 << "\n";
          }
          if (unflag(n2.low()) == unflag(n2.high())){
            if (!red1_mapping.is_open()) { red1_mapping.open(); }
            red1_mapping.write({ n2.uid(), n2.low(), n2._payload });
            if (debug_enabled) std::cout << "new F1 mapping" << n2 << "\n";
          } else {
            child_grouping.push(n2);
            if (debug_enabled) std::cout << "pushed node" << n2 << "\n";
          }
        }
      }

      // cut stuff:
      cuts_t local_1level_cut   = { { 0u, 0u, 0u, 0u } };
      cuts_t tainted_1level_cut = { { 0u, 0u, 0u, 0u } };
      __reduce_cut_add(local_1level_cut,
                     pq.size_without_terminals(),
                     pq.terminals(false) + arcs.unread_terminals(false),
                     pq.terminals(true) + arcs.unread_terminals(true));
      
      //red rule 2 (merge duplicates)
      child_grouping.sort();
      typename Policy::id_type out_id = Policy::max_id;
      node seen_node = node(node::uid_type(), ptr_uint64::nil(), ptr_uint64::nil());
      while (child_grouping.can_pull()) {
        const jump_up_node next_node = child_grouping.pull();
        if (debug_enabled) std::cout << "found node " << next_node << "\n";
        if (seen_node.low() != unflag(next_node.low()) || seen_node.high() != unflag(next_node.high())) {
          adiar_assert(0 <= out_id, "Should still have more ids left");
          seen_node = node(level, out_id--, unflag(next_node.low()), unflag(next_node.high()));
          if (debug_enabled) std::cout << "pushing node to out " << seen_node << "\n";
          out.unsafe_push(seen_node); //needs to be unsafe donno why
          
          //now that we adding nodes - update cuts
          __reduce_cut_add(next_node.low().is_flagged() ? tainted_1level_cut : local_1level_cut,
                        seen_node.low());
          __reduce_cut_add(next_node.high().is_flagged() ? tainted_1level_cut : local_1level_cut,
                        seen_node.high());

        } 
        if (debug_enabled) std::cout << "new F2 mapping: " << next_node.uid() << " -> " << seen_node.uid() << "\n";
        red2_mapping.push({ next_node.uid(), seen_node.uid(), next_node._payload });
      }

      //update level info:
      const size_t reduced_width = Policy::max_id - out_id;
      if (reduced_width > 0) { out.unsafe_push(level_info(level, reduced_width)); }

      //forwarding
      //very like xj level but we dont know that pairs will be in F2 so slightly more ugly..
      //actually: we might pull non-pair involved mapping if a node is merged at this level?
      red2_mapping.sort();

      
      jump_up_mapping next_red1 = { node::uid_type(), node::uid_type() };
      bool has_next_red1 = red1_mapping.is_open() && red1_mapping.size() > 0;
      if (has_next_red1) {red1_mapping.seek_begin(); next_red1 = red1_mapping.next();}


      //attempt 2..
      while (has_next_red1 || red2_mapping.can_pull()) {
        bool first_is_f1, second_is_f1;
        //pull a mapping, if it has payload none - then it must be from a supressible node?
        // and then -> do we know for a fact that it wont have a partner, cus then just handle like normal..
        bool is_red1_current = !red2_mapping.can_pull() || (has_next_red1 && next_red1.old_uid > red2_mapping.top().old_uid);
        first_is_f1 = is_red1_current ;
        const jump_up_mapping map1 = is_red1_current ? next_red1 : red2_mapping.pull();
        if (map1.payload == assignment::None){
          //then just one req per incoming trust me bro
          //so handle like regular reduce -> pull arcs push one req
          while(arcs.can_pull_internal() && map1.old_uid == arcs.peek_internal().target()){
            const ptr_uint64 s = arcs.pull_internal().source();
            const ptr_uint64 t = is_red1_current ? flag(map1.new_uid) : static_cast<ptr_uint64>(map1.new_uid);
            pq.push(jump_up_arc({s, t}, map1.payload, xi.value()));
          }
        } else {
          //what we did before
          //pull the partner mapping and push twice for each arc
          if (is_red1_current){
            has_next_red1 = red1_mapping.has_next();
            if (has_next_red1) {next_red1 = red1_mapping.next();}
          }
          is_red1_current = !red2_mapping.can_pull() || (has_next_red1 && next_red1.old_uid > red2_mapping.top().old_uid);
          second_is_f1 = is_red1_current;
          const jump_up_mapping map2 = is_red1_current ? next_red1 : red2_mapping.pull();
          if (debug_enabled) std::cout << "found mappings: " << map1.to_string() << ", " << map2.to_string() << "\n";
          adiar_assert(map1.old_uid == map2.old_uid, "pulled pair uids dont match!");

          while (arcs.can_pull_internal() && map1.old_uid == arcs.peek_internal().target()) {
            const ptr_uint64 s = arcs.pull_internal().source();
            if (debug_enabled) std::cout << "found matching req starting in :" << s << "\n";
            const jump_up_arc n1 = {{s,(first_is_f1) ? flag(map1.new_uid) : map1.new_uid}, map1.payload, xi.value()};
            const jump_up_arc n2 = {{s,(second_is_f1) ? flag(map2.new_uid) : map2.new_uid}, map2.payload, xi.value()};
            if (debug_enabled) std::cout << "pushing requests: " << n1.to_string() << " and " << n2.to_string() << "\n";
            pq.push(n1);
            pq.push(n2);
          }
          //handle special case -> something jumping up to be new root...
          typename Policy::id_type test = Policy::max_id;
          if(xi.has_value()  && !pq.has_next_level()) {
            //this looks suspect but should only happen once so id's are ok?
            if (debug_enabled) std::cout << "detected that we're in weird case, pushing nil arcs for new top level";
            const typename Policy::uid_type s(xi.value(), test--);
            const ptr_uint64 s1 = s.as_ptr(map1.payload == assignment::True);
            const jump_up_arc n1 = {{s1,((first_is_f1) ? flag(map1.new_uid) : map1.new_uid)}, map1.payload, xi.value()};
            const jump_up_arc n2 = {{s1,((first_is_f1) ? flag(map2.new_uid) : map2.new_uid)}, map2.payload, xi.value()};
            if (debug_enabled) std::cout << "built reqs:" << n1.to_string() << ", " << n2.to_string() << "\n";
            pq.push(n1);
            pq.push(n2);
          }
        }
        //updating red1 things
        if (is_red1_current) {
          has_next_red1 = red1_mapping.has_next();
          if (has_next_red1) { next_red1 = red1_mapping.next(); }
        }
      }
      red1_mapping.close();

      //cuts
      out.unsafe_max_1level_cut(local_1level_cut);
      out.unsafe_inc_1level_cut(tainted_1level_cut);

      //epilogue
      const bool terminal_value = next_red1.new_uid.is_terminal() && next_red1.new_uid.value();
      __reduce_level__epilogue<>(arcs, pq, out, terminal_value);

    } else {
      //////////////////////////////////////// jump target level ////////////////////////////////////////
      //Not really reduce stuff here
      //should:
      //(0) i think? only pull arcs from pq (leaf arcs will be handled when we actually reach their source)
      //(1) pull an arc: if it has no payload just pass it on
      //(2) if first arc has payload pull another: if payload doesn't match -> build a new output node
      //(3) if the payload matched then pull 2 more arcs -> output 2 new nodes
      //(4) after all arcs processed update to next xj, xi
      if (debug_enabled) std::cout << "found xi level " << level << "\n";
      const typename Policy::label_type cur_xi = xi.value();
      typename Policy::id_type out_id = Policy::max_id;
      xi = target_gen_for_me(); //update xi
      if (debug_enabled) std::cout <<  "updated xi to be " << xi.value_or(5000) << "\n";

      //temp files
      size_t unreduced_width = max_pq_size; 
      if (debug_enabled) std::cout << "width of current layer " << unreduced_width << "\n";
      sorter_t<jump_up_node_ws, reduce_node_children_lt> child_grouping(sorters_mem, unreduced_width, 1);

      //temp files
      while(pq.can_pull()){
        //special case: moving to root layer..
        const bool jump_to_root = pq.top().source().level() == cur_xi;
        const jump_up_arc r1 = pq.pull();
        if (r1.payload == assignment::None) {
          if (debug_enabled) std::cout << "found jump-crossing arc " << r1.to_string() <<" we just pass on as req\n";
          if (!jump_to_root){pq.push(jump_up_arc({r1.source(), r1.target()}, assignment::None, (xi.has_value() ? xi.value() : 0)));}}
          
        else {
          const jump_up_arc r2 = pq.pull();
          if (debug_enabled) std::cout << "found arcs " << r1.to_string() << ", " << r2.to_string();
          if (r1.payload != r2.payload) {
            //push one node and req case (if not reducible?)
            if(unflag(r1.target()) == unflag(r2.target())) {
              //node supressible so just push along req
              if (debug_enabled) std::cout << " reducible so just pass on req to target\n ";
              if (!jump_to_root){pq.push(jump_up_arc({r1.source(), r1.target()}, assignment::None, (xi.has_value() ? xi.value() : 0)));}
            } else {
              //push node and req
              const node::uid_type out_uid(cur_xi, out_id--);
              const jump_up_node_ws res_node = {{out_uid, unflag(r1.target()), unflag(r2.target())}, assignment::None,r1.source()};
              if (debug_enabled) std::cout << " to build " << res_node << /*", and req" << n_req.to_string() <<*/ "\n";
              child_grouping.push(res_node);
            }
          } else {
            //payloads match mean there are 2 more matching reqs with same source, so we pull these
            const jump_up_arc r3 = pq.pull();
            const jump_up_arc r4 = pq.pull();
            if (debug_enabled) std::cout << ", " << r3.to_string() << ", " << r4.to_string() << "\n";
            //asserting pairwise not same payload
            adiar_assert(r1.payload != r3.payload, "r1 and r3 match :c");
            adiar_assert(r2.payload != r4.payload, "r2 and r4 match :c");
            //handling first pair..
            if(unflag(r1.target()) == unflag(r3.target())) { 
              if (debug_enabled) std::cout << "  first supressible.. just pushing req \n";
              //node supressible so just push along req
               if (!jump_to_root){pq.push(jump_up_arc({r1.source(), r1.target()}, assignment::None, (xi.has_value() ? xi.value() : 0)));}
            } else {
              const node::uid_type out_uid(cur_xi, out_id--);
              const jump_up_node_ws res_node = {{out_uid, unflag(r1.target()), unflag(r3.target())}, assignment::None, r1.source()};
              if (debug_enabled) std::cout << "  build first: " << res_node << /*", and req" << n_req << */"\n";
              child_grouping.push(res_node);
            }
            //handling second pair..
            if(unflag(r2.target()) == unflag(r4.target())) { 
              //node supressible so just push along req
              if (debug_enabled) std::cout << "  second supressible.. just pushing req \n";
              if (!jump_to_root){pq.push(jump_up_arc({r2.source(), r2.target()}, assignment::None, (xi.has_value() ? xi.value() : 0)));}
            } else {
              const node::uid_type out_uid(cur_xi, out_id--);
              const jump_up_node_ws res_node = {{out_uid, unflag(r2.target()), unflag(r4.target())}, assignment::None, r2.source()};
              if (debug_enabled) std::cout << "  build second: " << res_node <</* ", and req" << n_req << */"\n";
              child_grouping.push(res_node);
            }
          }
        }
      }
      // Count number of arcs that cross this level
      //NOTE: i have no idea is this is right...
      cuts_t local_1level_cut   = { { 0u, 0u, 0u, 0u } };
      cuts_t tainted_1level_cut = { { 0u, 0u, 0u, 0u } };

      __reduce_cut_add(local_1level_cut,
                      pq.size_without_terminals(),
                      pq.terminals(false) + arcs.unread_terminals(false),
                      pq.terminals(true) + arcs.unread_terminals(true));

      // red rule 2
      child_grouping.sort();
      node seen_node = node(node::uid_type(), ptr_uint64::nil(), ptr_uint64::nil());
      auto id = Policy::max_id;
      typename Policy::uid_type seen_uid;
      
      while (child_grouping.can_pull()) {
        jump_up_node_ws next_node = child_grouping.pull();
        if(seen_node.low() != unflag(next_node.low()) || seen_node.high() != unflag(next_node.high())) {
          seen_node = next_node;
          seen_uid = typename Policy::uid_type(next_node.uid().level(), id--);
          //not dupliate! we output
          out.unsafe_push({seen_uid, next_node.low(), next_node.high()});
          //update cut
          __reduce_cut_add(next_node.low().is_flagged() ? tainted_1level_cut : local_1level_cut,
                         seen_node.low());
          __reduce_cut_add(next_node.high().is_flagged() ? tainted_1level_cut : local_1level_cut,
                         seen_node.high());
        }
        if (!(next_node._source.level() == cur_xi)) {pq.push({{next_node._source, seen_uid}, assignment::None, (xi.has_value() ? xi.value() : 0)});}
      }
      //we dont need to forward since we already did while building nodes...

      //end stuff
      // Update with new possible maximum 1-level cut (the one below the current level)
      out.unsafe_max_1level_cut(local_1level_cut);
      // Add the tainted edges
      out.unsafe_inc_1level_cut(tainted_1level_cut);

      const bool terminal_value = false; //NOTE: again no clue
      __reduce_level__epilogue<>(arcs, pq, out, terminal_value);

      //update xj
      xj = level_gen();
      if (debug_enabled) std::cout <<  "updated xj to be " << xj.value_or(5000) << "\n";
      //update lvl info
      const size_t reduced_width = Policy::max_id - out_id;
      if (reduced_width > 0) { out.unsafe_push(level_info(level, reduced_width)); }
    }
  }
  if (debug_enabled) std::cout << "exited big loop";
  return typename Policy::dd_type(out_file);
}


//--------------------- setup of PQs for Non-monotone single sweeps (not nested sweeping stuff...) ------------------------------


//calculating memory stuff for the PQs and sorters used in the special cases 
//idea
// shared entry-point takes replace type and depending on it runs right special case after setting up PQs
// NOTE: could potentially be cleaner to do this in the public entry pont for replace
// but since only non-monotone cases need PQs we do it here for now
template <typename Policy, typename Cut, size_t ConstSizeInc, typename In>
  size_t
  __cor_ilevel_upper_bound(const In& in)
  {
    const safe_size_t max_cut_all = Cut::get(in,cut::type::All);
    return to_size(max_cut_all * max_cut_all + ConstSizeInc);
  }

template<typename Policy>
typename Policy::__dd_type //special cases can return arc format
replace(typename Policy::dd_type dd, 
        replace_func<Policy> m,
        replace_type t, 
        exec_policy ep){
  //should setup PQs used for special cases 
  // jump-down -> 2 pqs one levelized, one not (same as are used for correctify)
  // adj_swap -> 2 pqs one levelized one not + one sorter for extra levels
  // jump-up -> uses just 1 levelized pq, requests are diff shapes as well... and expects arc format input.. 
  
  //things that dont depend on case
  const bool internal_only = ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
  const bool external_only = ep.template get<exec_policy::memory>() == exec_policy::memory::External;

  switch (t) {
    case replace_type::Jump_Down : {
    //------------------------------------------- jump down -------------------------------------------------
      //one levelized, one not
      //memory left after opening all the streams we need 
      // I THINK: dotn need ot take level_info part into account here since we open it to calculate stuff, then close it again before making PQs and so on..
      const size_t aux_available_memory = memory_available()- node_ifstream<>::memory_usage() - arc_ofstream::memory_usage();
      constexpr size_t data_structures_in_pq_1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,2u>::data_structures;
      constexpr size_t data_structures_in_pq_2 = cor_priority_queue_2_t<memory_mode::Internal>::data_structures;

      const size_t pq_1_internal_memory = (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
      * data_structures_in_pq_1;
      const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

      const size_t pq_1_memory_fits =
        cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,2u>::memory_fits(pq_1_internal_memory);
      const size_t pq_2_memory_fits =
        cor_priority_queue_2_t<memory_mode::Internal>::memory_fits(pq_2_internal_memory);

      const size_t pq_1_bound =  __cor_ilevel_upper_bound<Policy, get_2level_cut, 2u>(dd);
      const size_t pq_2_bound =  __cor_ilevel_upper_bound<Policy, get_1level_cut, 0u>(dd);

      const size_t max_pq_1_size = internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;
      const size_t max_pq_2_size = internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

      if (!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_replace.lpq.unbucketed += 1u;
#endif
        //internal, no lookahead
        using PQ1 = cor_lvl_priority_queue_t<0, memory_mode::Internal,2u>;
        using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      } 
      else if (!external_only && max_pq_1_size <= pq_1_memory_fits && max_pq_2_size <= pq_2_memory_fits) {
        //internal, with lookahead
#ifdef ADIAR_STATS
      stats_replace.lpq.internal += 1u;
#endif
        using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,2u>;
        using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      }
      else {
        //external
#ifdef ADIAR_STATS
      stats_replace.lpq.external += 1u;
#endif
        using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External,2u>;
        using PQ2 = cor_priority_queue_2_t<memory_mode::External>;
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      }
    }

    case replace_type::Swap_Adjacent : {
      //------------------------------------------- adj_swap  -------------------------------------------------
      //one levelized, one not and a sorter
      const size_t aux_available_memory = memory_available()- node_ifstream<>::memory_usage() - arc_ofstream::memory_usage();
      using pq1_t = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,1u>;
      using default_sorter_t = nested_sweeping::outer::roots_sorter<memory_mode::Internal, cor_req_t<0>, request_data_first_lt<cor_req_t<0>>>;
      
      constexpr size_t data_structures_in_pq1 = pq1_t::data_structures;
      constexpr size_t data_structures_in_sorter = default_sorter_t::data_structures;
      constexpr size_t data_structures_in_pq_2 = cor_priority_queue_2_t<memory_mode::Internal>::data_structures;

      const size_t pq1_memory = aux_available_memory / (data_structures_in_pq1 + data_structures_in_sorter + data_structures_in_pq_2) * data_structures_in_pq1;
      const size_t sorter_memory = (aux_available_memory - pq1_memory) / (data_structures_in_sorter + data_structures_in_pq_2) * data_structures_in_sorter;
      const size_t pq2_memory = aux_available_memory - pq1_memory - sorter_memory;
      
      const size_t pq1_memory_fits = pq1_t::memory_fits(pq1_memory );
      const size_t pq2_memory_fits = cor_priority_queue_2_t<memory_mode::Internal>::memory_fits(pq1_memory );
      const size_t sorter_memory_fits = default_sorter_t::memory_fits(sorter_memory);

      const size_t pq_1_bound =  __cor_ilevel_upper_bound<Policy, get_2level_cut, 2u>(dd);
      const size_t pq_2_bound =  __cor_ilevel_upper_bound<Policy, get_1level_cut, 0u>(dd);
      const size_t sorter_bound = pq_1_bound ; //this might be too high?

      const size_t max_pq_1_size = internal_only ? std::min(pq1_memory_fits, pq_1_bound) : pq_1_bound;
      const size_t max_pq_2_size = internal_only ? std::min(pq2_memory_fits, pq_2_bound) : pq_2_bound;
      const size_t sorter_max =  internal_only ? std::min({ sorter_memory_fits, sorter_bound }) : sorter_bound;

      if (!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
  #ifdef ADIAR_STATS
        stats_replace.lpq.unbucketed += 1u;
  #endif
          //internal, no lookahead
          using PQ1 = cor_lvl_priority_queue_t<0, memory_mode::Internal,1u>;
          using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
          using sorter_t = nested_sweeping::outer::roots_sorter<memory_mode::Internal, cor_req_t<0>, request_first_lt<cor_req_t<0>>>;
          return replace_adj_swap_sweep<Policy, PQ1, PQ2, sorter_t>(dd, m, ep, 
            pq1_memory, max_pq_1_size, pq2_memory, max_pq_2_size, sorter_memory, sorter_max);
      } 
      else if (!external_only && max_pq_1_size <= pq1_memory_fits 
                              && sorter_max <= sorter_memory_fits
                              && max_pq_2_size <= pq2_memory_fits) {
        //internal with lookahead
  #ifdef ADIAR_STATS
        stats_replace.lpq.internal += 1u;
  #endif
        using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,1u>;
        using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
        using sorter_t = nested_sweeping::outer::roots_sorter<memory_mode::Internal, cor_req_t<0>, request_first_lt<cor_req_t<0>>>;
        return replace_adj_swap_sweep<Policy, PQ1, PQ2, sorter_t>(dd, m, ep, 
            pq1_memory, max_pq_1_size, pq2_memory, max_pq_2_size, sorter_memory, sorter_max);
      } else {
        //external case
        using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External,1u>;
        using PQ2 = cor_priority_queue_2_t<memory_mode::External>;
        using sorter_t = nested_sweeping::outer::roots_sorter<memory_mode::External, cor_req_t<0>, request_first_lt<cor_req_t<0>>>;
        return replace_adj_swap_sweep<Policy, PQ1, PQ2, sorter_t>(dd, m, ep, 
            pq1_memory, max_pq_1_size, pq2_memory, max_pq_2_size, sorter_memory, sorter_max);
      }
      //break;
    }
    case replace_type::Jump_Up : {
      //------------------------------------------- jump up  -------------------------------------------------
      //has one pq and some sorters
      const size_t aux_available_memory = memory_available()
      // Input streams
      - arc_ifstream<>::memory_usage()
      - level_info_ifstream<>::memory_usage()
      // Output streams
      - node_ofstream::memory_usage();

      //transpose dd
      const shared_levelized_file<arc>& t_dd = transpose(dd);

      const size_t pq_memory = aux_available_memory / 2;
      const size_t sorters_memory = aux_available_memory - pq_memory - iofstream<jump_up_mapping>::memory_usage();
      if(debug_enabled) std::cout << "sorters have memory " << sorters_memory << "\n";

      const size_t pq_memory_fits = reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal, jump_up_arc, jump_up_queue_lt, 2>::memory_fits(pq_memory);
      const size_t pq_bound = (t_dd->max_1level_cut) *2u;

      const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

      if (!external_only && max_pq_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      stats_replace.lpq.unbucketed += 1u;
#endif        
      using PQ = reduce_priority_queue<0, memory_mode::Internal, jump_up_arc, jump_up_queue_lt, 2>;
      return replace_jump_up_sweep<Policy, PQ, internal_sorter>(t_dd, m, ep, pq_memory, max_pq_size, sorters_memory);
      } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_replace.lpq.internal += 1u;
#endif 
        using PQ = reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal, jump_up_arc, jump_up_queue_lt, 2>;
        return replace_jump_up_sweep<Policy, PQ, internal_sorter>(t_dd, m, ep, pq_memory, max_pq_size, sorters_memory);
      } else {
#ifdef ADIAR_STATS
      stats_replace.lpq.external += 1u;
#endif 
        using PQ = reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory_mode::External, jump_up_arc, jump_up_queue_lt, 2>;
        return replace_jump_up_sweep<Policy, PQ, external_sorter>(t_dd, m, ep, pq_memory, max_pq_size, sorters_memory);
      }
    }
    default: //Non-Monotonic,  swap, and all Monotonic cases
      adiar_unreachable();

  }
}



  //TODO: SWAP special case


///-------------------------------------------------NON_MONOTONE--------------------------------------------------------------------



//-------------------------------------------------- full correctify sweep (2 pqs) --------------------------------------------------
  template <typename Policy, typename PQ1, typename PQ2, typename In>
  inline typename Policy::__dd_type
  replace_cor_scan_level(const In& in, 
                         PQ1& pq1,    //we assume that pq1 have been preloaded when given here  
                         PQ2& pq2,
                         exec_policy ep)               
  {
  //repeatedly runs correctify for single levels -> to be used for inner sweeps in nested sweeping
    // Set up input
    node_ifstream<> in_nodes(in);
    node v = in_nodes.pull();

    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);
    out_arcs->max_1level_cut = 0;

    while(!pq1.empty()){
      pq1.setup_next_level();
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq1.size());
      correctify_single_level<Policy>(in_nodes, aw, pq1, pq2, v, label_indicator::NORMAL);
  }
     if(debug_enabled) std::cout << "exited big loop!\n";
     return typename Policy::__dd_type(out_arcs,ep);
}

//-------------------------------------------------- full correctify sweep (ra) --------------------------------------------------
  // random_access version of correctify
  template <typename NodeRandomAccess, typename Policy, typename PQ>
  typename Policy::__dd_type
  cor_ra(const exec_policy& ep, NodeRandomAccess& in_nodes, PQ& pq) {
    // so just like normal correctify but we have all the children via random access
    //assume -> already set up pq with requests and input already set up

    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);
    out_arcs->max_1level_cut = 0;

    while(!pq.empty()){
      pq.setup_next_level();
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq.size());
      typename Policy::label_type label = pq.current_level();
      typename Policy::id_type id             = 0;
      if(debug_enabled) std::cout << "starting level " << label << "\n";
      in_nodes.setup_next_level(label);
      

      // --- Helpers ------------------------------------------------------------
      auto update_label_and_id = [&](const ptr_uint64& tseek) {
        id = (label == tseek.label()) ? (id + 1) : 0; 
        label = tseek.label() ;
        if(debug_enabled) std::cout << "label, id: " << label << "," << id << "\n";
      };

      while(!pq.empty_level()){
        cor_req_t<0> r = pq.top();
        if (debug_enabled)  std::cout << "found req " << r << "\n";
        const ptr_uint64 level_uid(r.data.level, 0);
        const ptr_uint64 tseek = std::min(r.target.first(), level_uid);

        if (r.target.first().level() >= r.data.level){
          if (r.target[0] == r.target[1]) {
            //supressible node
            pq.pop();
            cor_req_t<0> nr = {{r.target[0], node::pointer_type::nil()}, {}, {r.data.source}};
            pq.push(nr);
            continue;
          }
          //forwarding
          update_label_and_id(tseek);
          const typename Policy::uid_type out_uid(label, id);
          pusher<Policy>(pq, aw, out_uid.as_ptr(false), {r.target[0],node::pointer_type::nil()}, node::pointer_type::nil().level());
          pusher<Policy>(pq, aw, out_uid.as_ptr(true),  {r.target[1],node::pointer_type::nil()}, node::pointer_type::nil().level());

          internal_pusher<Policy, PQ, 0>(pq, aw, out_uid, r.target,  r.data.level);
          continue;
        }

        //children
        // if node then actually get children, else just pair with the target twice 
        // unlike prod2u -> reqs can be leaf, nil and leaf,leaf so we have to handle this
        const typename Policy::children_type test1 = {r.target.first(), r.target.first()};
        const typename Policy::children_type test2 = {r.target.second(), r.target.second()};
        //never nil, nil so first must be eitehr leaf or node
        const typename Policy::children_type children_fst = (!r.target.first().is_terminal()) ? in_nodes.at(r.target.first()).children() : test1;
        //second can be nil or node or leaf  
        const typename Policy::children_type children_snd = (r.target.second().is_node() && r.target.second().level() == label) ? in_nodes.at(r.target.second()).children() : test2;
        if(debug_enabled) std::cout << "made children... \n";
        //wrong layer case
        update_label_and_id(tseek);
        //build node of min for req_for
        if (!r.target.first().is_terminal()){
          const typename Policy::node_type v = node(r.target.first().level(), r.target.first().id(), children_fst[0], children_fst[1]);
          tuple<tuple<typename Policy::pointer_type>> reqs = reqFor<Policy>(r.target, v, children_snd[0], children_snd[1]);
          tuple<typename Policy::pointer_type> rlow = reqs[0]; 
          tuple<typename Policy::pointer_type> rhigh = reqs[1];
          const typename Policy::uid_type out_uid(label, id);

          // Forward outgoing
          pusher<Policy, PQ>(pq, aw, out_uid.as_ptr(false), rlow, r.data.level);
          pusher<Policy, PQ>(pq, aw, out_uid.as_ptr(true), rhigh, r.data.level);

          // Forward incoming
          internal_pusher<Policy, PQ, 0>(pq, aw, out_uid, r.target,  r.data.level);
        } else {
          //first is leaf so second is leaf or nil..
          const typename Policy::node_type v = node(0, 0, children_fst[0], children_fst[1]); //dummy wont be used
          tuple<tuple<typename Policy::pointer_type>> reqs = reqFor<Policy>(r.target, v, node::pointer_type::nil(), node::pointer_type::nil());
          tuple<typename Policy::pointer_type> rlow = reqs[0]; 
          tuple<typename Policy::pointer_type> rhigh = reqs[1];
          const typename Policy::uid_type out_uid(label, id);
          pusher<Policy, PQ>(pq, aw, out_uid.as_ptr(false), rlow, r.data.level);
          pusher<Policy, PQ>(pq, aw, out_uid.as_ptr(true), rhigh, r.data.level);

          // Forward incoming
          internal_pusher<Policy, PQ, 0>(pq, aw, out_uid, r.target,  r.data.level);
        }

      }
      if (id >= 0) { aw.push(level_info(label, id+1)); }
    }
    return typename Policy::__dd_type(out_arcs,ep);
  }


  //calculate from BDD levels and m what levels should be sweeped 
  //from the bottom, should sweep in those levels which mapped value is larger than smallest seen
  template<typename Policy>
  std::vector<typename Policy::label_type>
  levels_from_map(const replace_func<Policy>& m, const typename Policy::dd_type& dd)
  {
    level_info_ifstream<true> level_info_file(dd);
    std::vector<typename Policy::label_type> sweep_levels;

    level_info lvl_info = level_info_file.pull();
    bdd::label_type min_seen = m(lvl_info.level());

    while(level_info_file.can_pull()) {
      lvl_info = level_info_file.pull();
      bdd::label_type mapped = m(lvl_info.level());
      if (mapped > min_seen) sweep_levels.push_back(lvl_info.level());
      else min_seen = mapped;
    }
    return sweep_levels;
  }


//Policy for use in nested sweeping 
template <typename Policy>
class nested_sweeping_replace : public Policy
{
private:
    bool use_list;
    int index;
    const vector<memory_mode::Internal, typename Policy::label_type> m_arr;
    const replace_func<Policy> _m;                                
    const generator<typename Policy::label_type> _nesting_levels; // generator for levels to sweep on
    const generator<typename Policy::label_type> _targets;        //target for sweeps - to init pq with
    optional<typename Policy::label_type> _next_level; //next level to sweep on

public:
    //types
    using request_t = cor_req_t<0>;
    using request_pred_t = request_data_first_lt<request_t>;

    template <size_t LookAhead, memory_mode MemoryMode>
    using pq_t = cor_lvl_priority_queue_t<LookAhead, MemoryMode, 2u>; //inner pq expects inputs from 2 files

public:
    nested_sweeping_replace(const replace_func<Policy> m, 
                            generator<typename Policy::label_type> nl,
                            generator<typename Policy::label_type> t) 
    : _m(m), 
      _nesting_levels(nl),
      _targets(t)
    {
      use_list = false;
      _next_level = _nesting_levels();
    };

    nested_sweeping_replace(const vector<memory_mode::Internal, typename Policy::label_type> m_arr, 
                            generator<typename Policy::label_type> nl,
                            generator<typename Policy::label_type> t) 
    : m_arr(m_arr), 
      _nesting_levels(nl),
      _targets(t)
    {
      use_list = true;
      index = 0;
      _next_level = _nesting_levels();
    };

    static size_t
    stream_memory() {return node_ifstream<>::memory_usage() + arc_ofstream::memory_usage();}

    static size_t
    pq_memory(const size_t inner_memory) {
      constexpr size_t data_structures_in_pq_1 = pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;
      constexpr size_t data_structures_in_pq_2 = cor_priority_queue_2_t<memory_mode::Internal>::data_structures;
      return (inner_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))* data_structures_in_pq_1;
    }

    static size_t
    ra_memory(const shared_levelized_file<node>& outer_file) 
    {return node_raccess::memory_usage(outer_file);}

    static size_t
    pq_bound(const shared_levelized_file<node>& outer_file, const size_t /*outer_roots*/) {
      const typename Policy::dd_type outer_wrapper(outer_file);
      return __cor_ilevel_upper_bound<Policy, get_2level_cut, 2u>(outer_wrapper);
    }

    //labels mapped according to given map
    constexpr inline bdd::label_type
    map_level(typename Policy::label_type x) { 
      if (use_list){
        return m_arr[index];
        index = index + 1;
      }
      return _m(x);}

    //heavily based on sweep_pq from the prod2u sweeping policy
    //main differences:
    // - __cor_ilevel_upper_bound is different -> takes all arcs into account for cut??
    template <typename inner_pq_t>
    __bdd sweep_pq([[maybe_unused]]const exec_policy& ep,
                   [[maybe_unused]]const shared_levelized_file<node>& outer_file,
                   inner_pq_t& inner_pq,
                   const size_t inner_remaining_memory){
      if(debug_enabled) std::cout << "START sweep_pq \n";
      //should
      // (1) setup PQ2
      const size_t pq_2_memory_fits =
      cor_priority_queue_2_t<memory_mode::Internal>::memory_fits(inner_remaining_memory);

      const size_t pq_2_bound =__cor_ilevel_upper_bound<Policy, get_1level_cut, 0u>(typename Policy::dd_type(outer_file)) 
        + (inner_pq.size()); // Add crossing arcs

      const size_t max_pq_2_size =
        ep.template get<exec_policy::memory>() == exec_policy::memory::Internal
        ? std::min(pq_2_memory_fits, pq_2_bound)
        : pq_2_bound;

        if (ep.template get<exec_policy::memory>() != exec_policy::memory::External
          && max_pq_2_size <= pq_2_memory_fits) {
        using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
        PQ2 pq2(inner_remaining_memory, max_pq_2_size);
        // (2) run correctify sweep
        return replace_cor_scan_level<Policy, inner_pq_t, PQ2, shared_levelized_file<node>>(outer_file, inner_pq, pq2, ep);
      } else {
        using PQ2 = cor_priority_queue_2_t<memory_mode::External>;
        PQ2 pq2(inner_remaining_memory, max_pq_2_size);
        // (2) run correctify sweep
        return replace_cor_scan_level<Policy, inner_pq_t, PQ2, shared_levelized_file<node>>(outer_file, inner_pq, pq2, ep);
      }
    }

    template <typename inner_pq_t>
    __bdd
    sweep_ra(const exec_policy& ep,
             const shared_levelized_file<node>& outer_file,
             inner_pq_t& pq,
             const size_t /*inner_remaining_memory*/)
    {   
        if (debug_enabled) std::cout << "running ra sweep";
        node_raccess in_nodes(outer_file);
        return cor_ra<node_raccess, Policy, inner_pq_t>(ep, in_nodes, pq);
    }

    bool
    has_sweep(const typename Policy::label_type l)
    {   
        bool res = l == next_level(l);
        if(debug_enabled) std::cout << "testing if " << l << " has sweep -> " << res << "\n";
        return res;
    }

    typename Policy::label_type
    next_level(const typename Policy::label_type l)
    {
      while (_next_level.has_value() && l < _next_level.value()) { 
        _next_level = _nesting_levels(); 
      }
      return _next_level.value_or(Policy::max_label + 1);
    }

    template <typename outer_roots_t>
    __bdd
    sweep(const exec_policy& ep,
          const shared_levelized_file<node>& outer_file,
          outer_roots_t& outer_roots,
          const size_t inner_memory)
    {
        if(debug_enabled) std::cout << "runs sweep \n";
        return nested_sweeping::inner::down__sweep_switch(
            ep, *this, outer_file, outer_roots, inner_memory, stats_replace.lpq);
    }

    inline request_t
    request_from_node(const node& n, const ptr_uint64& parent)
    {
        //only run for nodes where we want to update level
        if(debug_enabled) std::cout << "runs req from node with " << n << "\n";
        typename Policy::label_type new_lvl = (use_list) ? m_arr[index] : _m(n.label());
        return request_t({n.low(), n.high()}, {}, { parent, new_lvl });
    }

    //method for supplying the level_inputs for initializing inner PQ
    template<typename PQT>
    std::array<typename PQT::level_input_type, PQT::lvl_input>
    priority_queue_initializer_list(const typename Policy::shared_node_file_type outer_file) const{
      optional<typename Policy::label_type> dummy = _targets();
      typename Policy::label_type target = (dummy.has_value()) ? dummy.value() : throw invalid_argument("target should exist for sweep");
      std::array<typename PQT::level_input_type , PQT::lvl_input> res = {typename Policy::dd_type(outer_file), make_generator(target)};
      return res;
    }

  static constexpr bool final_canonical = true; //should make canonical before output?
  static constexpr bool fast_reduce     = true; //should not reduce until final
  static constexpr bool skip_term_reqs = false; //terminal-only requests should still be passed to inner sweep!

};


  // nested sweeping entry
  template <typename Policy>
  typename Policy::__dd_type
  replace_nested_sweep(const typename Policy::dd_type& dd,
                       const replace_func<Policy>& m,
                       exec_policy ep) {
    //calculate sweeping levels and their targets from map
    std::vector<typename Policy::label_type> nest_levels = levels_from_map<Policy>(m, dd);

    if(debug_enabled) std::cout << "levels to sweep on: [";
    for(typename Policy::label_type e : nest_levels) {if(debug_enabled) std::cout << e << ", ";}
    if(debug_enabled) std::cout << "]\n";

    std::vector<typename Policy::label_type> targets;
    for (typename Policy::label_type l : nest_levels) {
      targets.push_back(m(l));
    }

    generator<typename Policy::label_type> level_gen = make_generator(nest_levels.begin(),nest_levels.end());
    generator<typename Policy::label_type> targets_gen = make_generator(targets.begin(),targets.end());

    //setup policy
    nested_sweeping_replace<Policy> inner_impl(m, level_gen, targets_gen);

    //run nested sweep
    const bdd res = nested_sweep<>(ep, dd, inner_impl);
    if (debug_enabled) std::cout << "Replace nested-sweeping complete! \n";
    return res;
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // "Public" interface

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  ////////////////////////Process//////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  replace(const exec_policy& ep,
          const typename Policy::dd_type& dd,
          const replace_func<Policy>& m,
          replace_type m_type)
  {
    // Return if nothing needs to be remapped
    if (dd->is_terminal()) {
#ifdef ADIAR_STATS
      stats_replace.terminal_returns += 1u;
#endif
      return dd;
    }

    const replace_type inferred_type =
      m_type == replace_type::Auto ? replace__infer_type<Policy>(dd, m) : m_type;

    // Map internal nodes
    switch (inferred_type) {
      // LCOV_EXCL_START
    case replace_type::Auto:
      adiar_unreachable();
      // LCOV_EXCL_STOP

    case replace_type::Non_Monotone:
#ifdef ADIAR_STATS
      stats_replace.nested_sweeps += 1u;
#endif
      std::cout << "begun replace\n";
      return replace_nested_sweep<Policy>(dd,m,ep);
    
    case replace_type::Non_Monotone_Test: {
      //split the map
      //auto map_lists = map_adj_split(m);
      if(false /*map_lists[0].size() > 1*/){
        //TODO: more sophisticated check here
        return replace_nested_sweep<Policy>(dd,m,ep);
      } else {
        return replace_nested_sweep<Policy>(dd,m,ep);
      }
    }
    case replace_type::Jump_Down:
#ifdef ADIAR_STATS
      stats_replace.jump_down_scans += 1u;
#endif
    return replace<Policy>(dd,m,inferred_type, ep);

    case replace_type::Jump_Up: //NOTE: running from here -> bdd is transposed first
#ifdef ADIAR_STATS
      stats_replace.jump_up_scans += 1u;
#endif
    return replace<Policy>(dd,m,inferred_type, ep);

    case replace_type::Swap_Adjacent:
#ifdef ADIAR_STATS
      stats_replace.adj_swap_scans += 1u;
#endif
      return replace<Policy>(dd,m,inferred_type, ep);

    case replace_type::Monotone:
#ifdef ADIAR_STATS
      stats_replace.monotonic_scans += 1u;
#endif
      return __replace__monotonic_scan<Policy>(dd, m);

    case replace_type::Shift:
#ifdef ADIAR_STATS
      stats_replace.shift_returns += 1u;
#endif
      return __replace__shift_return<Policy>(dd, m);

    case replace_type::Identity:
#ifdef ADIAR_STATS
      stats_replace.identity_returns += 1u;
#endif
      return dd;
    }
    adiar_unreachable(); // LCOV_EXCL_LINE
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  replace(const exec_policy& ep,
          typename Policy::__dd_type&& __dd,
          const replace_func<Policy>& m,
          replace_type m_type)
  {
    // Is it already reduced?
    if (__dd.template has<typename Policy::shared_node_file_type>()) {
      const typename Policy::dd_type dd(
        __dd.template get<typename Policy::shared_node_file_type>(), __dd._negate, __dd._shift);
      return replace<Policy>(ep, dd, m, m_type);
    }

    const replace_type inferred_type =
      m_type == replace_type::Auto ? replace__infer_type<Policy>(__dd, m) : m_type;

    // Otherwise, map while reducing
    switch (inferred_type) {
      // LCOV_EXCL_START
    case replace_type::Auto:
      adiar_unreachable();
      // LCOV_EXCL_STOP
    
    case replace_type::Non_Monotone:
    case replace_type::Swap_Adjacent:
    case replace_type::Jump_Down:
    case replace_type::Jump_Up:
#ifdef ADIAR_STATS
      stats_replace.nested_sweeps += 1u;
#endif
      //NOTE: jump_down , swap_adj not built for input arc files currently
      return replace_nested_sweep<Policy>(std::move(__dd), m,ep);
    
    case replace_type::Monotone:
    case replace_type::Shift:
#ifdef ADIAR_STATS
      stats_replace.monotonic_reduces += 1u;
#endif
      return __replace__monotonic_reduce<Policy>(ep, std::move(__dd), m);

    case replace_type::Identity:
#ifdef ADIAR_STATS
      stats_replace.identity_reduces += 1u;
#endif
      return typename Policy::dd_type(std::move(__dd));
    }
    adiar_unreachable(); // LCOV_EXCL_LINE
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  replace(typename Policy::__dd_type&& __dd, const replace_func<Policy>& m, replace_type m_type)
  {
    const exec_policy ep = __dd._policy;
    return replace<Policy>(ep, std::move(__dd), m, m_type);
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
