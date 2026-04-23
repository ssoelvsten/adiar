#ifndef ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
#define ADIAR_INTERNAL_ALGORITHMS_REPLACE_H

#include "adiar/bdd.h"
#include "adiar/bdd/bdd.h"
#include "adiar/exec_policy.h"
#include "adiar/internal/algorithms/nested_sweeping.h"
#include "adiar/internal/data_structures/levelized_priority_queue.h"
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
//#include <adiar/internal/algorithms/reorder.h>

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
    bool adj_swap  = true;

    typename Policy::label_type last_jump = 0;
    typename Policy::label_type adj_node  = 0;

    label_type prev_before = Policy::max_label + 1;
    label_type prev_after  = Policy::max_label + 1;

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
        jump_down &= (last_jump <= next_before); // Maybe should allow overlaps?
        //Jump_Up check
        //jump_up &= (next_before > next_after ); //levels are only moved up
        //jump_up &= (last_jump >= next_after);
        last_jump = next_after;

        //Adjacent swap checks - currently only detects when both adjacent variables
        //are to be swapped -- might however work when adjacent level is empty...
        const int32_t next_diff32 =
          static_cast<int32_t>(next_after) - static_cast<int32_t>(next_before);
        if(next_diff32 == 1) {
          adj_node = next_before;
        } else if (next_diff32 == -1){
            adj_swap &= adj_node == next_after;
        } else {
          adj_swap = false;
        }
        // Todo: swap 
      }

      prev_before = next_before;
      prev_after  = next_after;
    }

    if (!monotone) {
      if (jump_down) {//std::cout << "\n detected jump_down\n" ;
         return replace_type::Jump_Down;}
      if (adj_swap) {//std::cout << "\n detected adjacent swap\n" ;
         return replace_type::Swap_Adjacent;}
      return replace_type::Non_Monotone; } //DUMMY! - missing handling of adj_swap, swap and jump_up
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
      //if (debug_enabled) std::cout<< "pushing req to pq1: " << lreq << "\n";
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

    //does all the correctify stuff for single level - for use both in general non-monotone replace and special cases
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
    node v = in.pull(); // NOTE: is it always non-empty?

    //setup output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);
    out_arcs->max_1level_cut = 0;

    //finding jump_down levels and targets
    //TODO move to seperate function pls.. NOTE: neccesary?
    if (debug_enabled) std::cout << "generating generators for jump down\n";

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

        const label_t mapped = m(cur_label);
        //push reqs
        while(!pq1.empty_level()) {
          const cor_req_t<0> req = pq1.pull();
          const ptr_uint64 t_uid(req.data.level, 0);
          const ptr_uint64 tseek = std::min(req.target.first(), t_uid);
          while (v.uid() < tseek && in.can_pull()) { v = in.pull(); }
          const cor_req_t<0> n_req = {{v.low(), v.high()},{},{req.data.source, mapped}};
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

//decorator for pq1 such that it pushes/pulls to pq3 instead sometimes
//inspired from decorators in nested sweeping
template <typename PQ1>
class adj_swap_pq_decorator{
  //decorator/strategy for swappign between two different pqs - for use in adj swap
  //allows us to use correctify_single_level without too much modification
  //some types
  public:
    using value_type = typename PQ1::value_type;
    using value_comp_type = typename PQ1::value_comp_type;
    static constexpr memory_mode mem_mode = PQ1::mem_mode;
    using level_type = typename value_type::pointer_type::label_type;
    static constexpr level_type no_label = PQ1::no_label;
  private:
    PQ1& _pq1; //ref to pq1
    PQ1& _pq3; //ref to pq3
    bool active_pq; //active pq -> true = pq1 , false = pq3

  //constructor
  public:
    adj_swap_pq_decorator(PQ1& pq1, PQ1& pq3) :
    _pq1(pq1), 
    _pq3(pq3)
    {active_pq = true;}
  
    //TODO think about this... - should these just access the active one? 
    //since size used directly by empty it seems dangerous to have both?
  size_t
  terminals(const bool terminal_value) const
  {
    return _pq1.terminals(terminal_value) + _pq3.terminals(terminal_value);
  }
  size_t size() const {return _pq1.size() + _pq3.size();}

  size_t size_without_terminals() const
    {
      return size() - terminals(false) - terminals(true);
    }

  bool empty_level() const       { return (active_pq) ? _pq1.empty_level() : _pq3.empty_level();}
  bool can_pull() const          { return (active_pq) ? _pq1.can_pull() : _pq3.can_pull();}
  cor_req_t<0> pull()            { return (active_pq) ? _pq1.pull() : _pq3.pull(); }
  bool has_top() const           { return (active_pq) ? _pq1.has_top() : _pq3.has_top(); }
  cor_req_t<0> top()             { return (active_pq) ? _pq1.top() : _pq3.top(); }
  void pop()                     { return (active_pq) ? _pq1.pop() : _pq3.pop(); }
  bool has_current_level() const { return (active_pq) ? _pq1.has_current_level() : _pq3.has_current_level(); }
  level_type current_level() const { return (active_pq) ? _pq1.current_level() : _pq3.current_level(); }
  bool empty() const { return size() == 0u;}

  void setup_next_level(level_type stop_level = no_label)
        { (active_pq) ? _pq1.setup_next_level(stop_level) : _pq3.setup_next_level(stop_level);}

  //pushing
  void push(const cor_req_t<0> r){
    if (r.data.level <= r.target.first().level()){
      if(debug_enabled) std::cout << "pushing to pq3: " << r << "\n";
      _pq3.push(r);
    } else {
      if(debug_enabled) std::cout << "pushing to pq1: " << r << "\n";
      _pq1.push(r);
    }
  }

  //setting a pq to active
  //deffo not the pretties way to do this..
  void set_active(const bool flag){
    if (debug_enabled) std::cout << "setting " << ((flag) ? "pq1 " : "pq3 ") << "as active! \n";
    active_pq = flag;
  }

};

//new version - using an extra PQ
// instead of duplicating layers have another pq that holds stuff for the extra layer 
// i think - really we only need one "bucket" as it will be entirely emptied at the end of each adj swap??"
// dont know if there is data structure stuff to specify that tho?
// mempry stuff will be a bit annoying got this as well i guess? - cus only this special case needs an extra PQ?
//TBA

template <typename Policy, typename PQ1, typename PQ2>
inline typename Policy::__dd_type
replace_adj_swap_sweep_v2(const typename Policy::dd_type& dd, 
                          replace_func<Policy> m,
                          exec_policy ep,
                          size_t pq1_mem,
                          size_t max_pq1_size,
                          size_t pq2_mem,
                          size_t max_pq2_size) {
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
    std::vector<typename Policy::label_type> swap_starts, swap_end, swap_extra; 
    while (info_in.can_pull()){
      level_info l = info_in.pull();
      if (debug_enabled) std::cout << "found level " << l << "\n";
      if (m(l.label()) > l.label()) {
        swap_starts.push_back(l.label());
        swap_end.push_back(m(l.label()));
        swap_extra.push_back(m(l.label()) + 1);
      }
    }
    if (debug_enabled) std::cout << "extra levels [ ";
    for(typename Policy::label_type e : swap_extra) {if (debug_enabled) std::cout << e << ",";}
    if (debug_enabled) std::cout << "]";

    const generator<typename Policy::label_type> level_gen = make_generator(swap_starts.begin(), swap_starts.end());
    const generator<typename Policy::label_type> end_gen = make_generator(swap_end.begin(), swap_end.end());
    const generator<typename Policy::label_type> extra_gen = make_generator(swap_extra.begin(), swap_extra.end());
    optional<typename Policy::label_type> next_swap = level_gen();
    optional<typename Policy::label_type> next_target = end_gen();

    //setup PQs
    //TODO - calc mem before and pass
    //DUMMY! should probably be set up pq1, pq3 in a good way -> potentially have the memory allocator thingy do it 
    PQ1 pq1({dd}, (pq1_mem/4)*3 , (max_pq1_size/4)*3, stats_replace.lpq); //give pq1 3/4 of memory available for it..
    //sorter istedet 
    PQ1 pq3({extra_gen}, (pq1_mem/4) , (max_pq1_size/4), stats_replace.lpq); //give pq3 1/4 of memory available for pq1
    PQ2 pq2(pq2_mem, max_pq2_size);

    using pq_swap_t = adj_swap_pq_decorator<PQ1>;
    pq_swap_t apq(pq1,pq3);

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
    apq.push(init_req);

    while(!apq.empty()){
      apq.set_active(true); //moving active to pq1
      if (debug_enabled) std::cout << "about to setup level from pq1 \n";
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
        //(2) handle the extra level as correctify but
        //    (a) always in correct layer case!
        //    (b) pulls requests from pq3
        //    (c) out_label should be current level -1 (handled by label_indicator)
        apq.set_active(false);  //move active to pq3
        if (!apq.empty()){
          if(debug_enabled) std::cout << "entered extra level case\n";
          apq.setup_next_level();
          out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, apq.size());
          correctify_single_level<Policy>(in, aw, apq, pq2, v,  label_indicator::DEC);
        }
          //now update all the values!
          next_swap = level_gen();
          next_target = end_gen();

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
  inline typename Policy::__dd_type
  replace_adj_swap_sweep(const typename Policy::dd_type& dd, 
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

//--------------------- setup of PQs for Non-monotone single sweeps (not nested sweeping stuff...) ------------------------------
//aka setting up PQ1 and PQ2 types for the special cases... 
//so goal is we just pass PQs along to special case funcs

//idea
// shared entry-point takes replace type and depending on it runs right special case after setting up PQs
// NOTE: could potentially be cleaner to do this in the replace func that initially delegates 
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
  // jump-up -> 2 pqs one levelized, one not (same as are used for correctify)
  // adj_swap -> 3 pqs two levelized one not. the 3'rd used just as extra bucket for additional level.. 
  // jump-up -> uses just 1 levelized pq, requests are diff shapes as well... and expects arc format input.. 

  //below is based on memory stuff done in prod2u..

  //All of the memory variables 
  //free mem after input/output stuff
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

  const bool internal_only = ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
  const bool external_only = ep.template get<exec_policy::memory>() == exec_policy::memory::External;

  const size_t pq_1_bound =  __cor_ilevel_upper_bound<Policy, get_2level_cut, 2u>(dd);
  const size_t pq_2_bound =  __cor_ilevel_upper_bound<Policy, get_1level_cut, 0u>(dd);

  const size_t max_pq_1_size = internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;
  const size_t max_pq_2_size = internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

  //(2) switch on replace-type and run correct thing from there...
  //TODO: the code duplication here is horrible 
  if(!external_only && max_pq_1_size <= no_lookahead_bound(2)){ //internal mem no lookahead case
    using PQ1 = cor_lvl_priority_queue_t<0, memory_mode::Internal,2u>;
    using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
#ifdef ADIAR_STATS
      stats_replace.lpq.unbucketed += 1u;
#endif
    switch (t) {
      case replace_type::Jump_Down: 
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      case replace_type::Swap_Adjacent:
      //uses extra pq for acting like it has more levels.. (so only needs one initializer)
        using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,1u>;
        using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
        return replace_adj_swap_sweep_v2<Policy, PQ1, PQ2>(dd, m, ep,
           pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      default: //Non-Monotonic, jump-up, swap, and all Monotonic cases
        adiar_unreachable();
    }

  } else if (!external_only && max_pq_1_size <= pq_1_memory_fits && max_pq_2_size <= pq_2_memory_fits) { //internal mem with lookahead case
#ifdef ADIAR_STATS
      stats_replace.lpq.internal += 1u;
#endif  
    if(debug_enabled) std::cout << "picks internal PQ case..?\n";
    using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,2u>;
    using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;

    switch (t) {
      case replace_type::Jump_Down: 
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      case replace_type::Swap_Adjacent:
        using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,1u>;
        using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
        return replace_adj_swap_sweep_v2<Policy, PQ1, PQ2>(dd, m, ep,
           pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);

      default: //Non-Monotonic, jump-up, swap, and all Monotonic cases
        adiar_unreachable();
    }

  } else { // PQs dont fit in internal so external
#ifdef ADIAR_STATS
      stats_replace.lpq.external += 1u;
#endif   
    using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External,2u>;
    using PQ2 = cor_priority_queue_2_t<memory_mode::External>;

    switch (t) {
      case replace_type::Jump_Down: 
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      case replace_type::Swap_Adjacent:
        using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,1u>;
        using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;
        return replace_adj_swap_sweep_v2<Policy, PQ1, PQ2>(dd, m, ep,
           pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      default: //Non-Monotonic, jump-up, swap, and all Monotonic cases
        adiar_unreachable();
    }
  }
}

//-------------------------------------------------- full correctify sweep --------------------------------------------------
  template <typename Policy, typename PQ1, typename PQ2, typename In>
  inline typename Policy::__dd_type
  replace_cor_scan_level(const In& in, 
                         PQ1& pq1,    //we assume that pqs have been preloaded when given here  
                         PQ2& pq2,
                         exec_policy ep)               
  {
     // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);
    out_arcs->max_1level_cut = 0;

    // Set up input
    node_ifstream<> in_nodes(in);
    node v = in_nodes.pull();

    
    while(!pq1.empty()){
      pq1.setup_next_level();
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq1.size());
      correctify_single_level<Policy>(in_nodes, aw, pq1, pq2, v, label_indicator::NORMAL);
  }
     if(debug_enabled) std::cout << "exited big loop!\n";
     return typename Policy::__dd_type(out_arcs,ep);
}


  ///JUMP_UP special case
  //TBA
  ///SWAP special case
  //TBA

///-------------------------------------------------NON_MONOTONE--------------------------------------------------------------------

  //calculate from BDD levels and m what levels should be sweeped - maybe shouldn't be in this file??
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
      //std::cout << "levels from map order" << l << "\n";
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
    const replace_func<Policy> _m;
    const generator<typename Policy::label_type> _nesting_levels;
    const generator<typename Policy::label_type> _targets;  //target for sweeps
    optional<typename Policy::label_type> _next_level; //next level to sweep on

public:
    //types
    using request_t = cor_req_t<0>;
    using request_pred_t = request_data_first_lt<request_t>;

    template <size_t LookAhead, memory_mode MemoryMode>
    using pq_t = cor_lvl_priority_queue_t<LookAhead, MemoryMode, 2u>; //inner pq expects inputs from 2 files

public:
    nested_sweeping_replace(const internal::replace_func<Policy> m, 
                            generator<typename Policy::label_type> nl,
                            generator<typename Policy::label_type> t) 
    : _m(m), 
      _nesting_levels(nl),
      _targets(t)
    {
      _next_level = _nesting_levels();
    };

    static size_t
    stream_memory() {return node_ifstream<>::memory_usage() + arc_ofstream::memory_usage();}

    static size_t
    pq_memory(const size_t inner_memory) {
      constexpr size_t data_structures_in_pq_1 =
        pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

      constexpr size_t data_structures_in_pq_2 =
        cor_priority_queue_2_t<memory_mode::Internal>::data_structures;

      return (inner_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
        * data_structures_in_pq_1;
    }

    //DUMMY - RA not supported so just give max
    static size_t
    ra_memory(const shared_levelized_file<node>& /*outer_file*/) {return std::numeric_limits<size_t>::max();}

    static size_t
    pq_bound(const shared_levelized_file<node>& outer_file, const size_t /*outer_roots*/) {
      const typename Policy::dd_type outer_wrapper(outer_file);
      return __cor_ilevel_upper_bound<Policy, get_2level_cut, 2u>(outer_wrapper);
    }

    //labels mapped according to given map
    constexpr inline bdd::label_type
    map_level(bdd::label_type x) const {return _m(x);}

    //heavily based on sweep_pq from the prod2u sweeping policy
    //main differences:
    // - pq2_bound based on 2-level-cut instead of one 
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

      const size_t pq_2_bound =__cor_ilevel_upper_bound<Policy, get_2level_cut, 0u>(typename Policy::dd_type(outer_file)) //should this be 2 level?
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
        return replace_cor_scan_level<Policy, inner_pq_t, PQ2, shared_levelized_file<node>>(outer_file, inner_pq, pq2,ep);
      } else {
        using PQ2 = cor_priority_queue_2_t<memory_mode::External>;
        PQ2 pq2(inner_remaining_memory, max_pq_2_size);
        // (2) run correctify sweep
        return replace_cor_scan_level<Policy, inner_pq_t, PQ2, shared_levelized_file<node>>(outer_file, inner_pq, pq2,ep);
      }
    }

    template <typename inner_pq_t>
    __bdd
    sweep_ra(const exec_policy& /*ep*/,
             const shared_levelized_file<node>& /*outer_file*/,
             inner_pq_t& /*inner_pq*/,
             const size_t /*inner_remaining_memory*/)
    {   
        throw invalid_argument("Non-monotonic variable replacement does not support random access");
    }

    bool
    has_sweep(const typename Policy::label_type l)
    {   
        //this may be wrong..
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
        return request_t({n.low(), n.high()}, {}, { parent, _m(n.label()) });
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

  static constexpr bool final_canonical = true;
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
      return replace_nested_sweep<Policy>(dd,m,ep);
    case replace_type::Jump_Down:
#ifdef ADIAR_STATS
      stats_replace.jump_down_scans += 1u;
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
