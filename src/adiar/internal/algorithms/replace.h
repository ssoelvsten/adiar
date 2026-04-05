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
#include <initializer_list>
#include <optional>
#include <type_traits>
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
  constexpr bool debug_enabled = true;

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
         tuple<typename Policy::pointer_type>& target, 
         typename Policy::label_type& level) {
    if (target[0].is_nil() && target[1].is_terminal()){
        arc a =  {source, target[1]};
        if (debug_enabled) std::cout << "pushing term arc: " << a << "\n";
        out_stream.push_terminal(a);
        return;
    }

    if (target[1].is_nil() && target[0].is_terminal()){
        arc a =  {source, target[0]};
        if (debug_enabled) std::cout << "pushing term arc: " << a << "\n";
        out_stream.push_terminal(a);
        return;
    }

    if (target[0].is_terminal() && target[1].is_terminal() && target[0] == target[1]) {
        //push leaf arc from current
        arc alow =  {source, target[0]};
        if (debug_enabled) std::cout << "pushing term arc: " << alow << "\n";
        out_stream.push_terminal(alow);
      } else {
        //push request from current
        cor_req_t<0> lreq({target[0],target[1]},{},{source, level});
        if (debug_enabled) std::cout<< "pushing req to pq1: " << lreq << "\n";
        pq.push(lreq);
      }
  }

  template<typename Policy, typename pq_t, uint8_t nc>
  inline void 
  internal_pusher(pq_t& pq, 
                  arc_ofstream& aw , 
                  typename Policy::uid_type out_uid, 
                  tuple<typename Policy::pointer_type> target,
                  typename Policy::label_type level){
     while(pq.has_top() && pq.top().target == target && pq.top().data.level == level) {
          const cor_req_t<nc> r1 = pq.top(); pq.pop(); //non-levelized has no pull
          if (debug_enabled) std::cout << "has popped: " << r1 << "\n";
          if (r1.data.source.level() != ptr_uint64::nil().level()) {
            //push to out!
            arc in = {r1.data.source , out_uid};
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
      bdd::pointer_type tl = t[0];
      bdd::pointer_type th = t[1];

      if (tl.is_terminal() && th.is_terminal()) {
        if(debug_enabled) std::cout << " \t both terminal case\n";
        return { {tl,tl}, {th,th} };
      }

      if (th.is_terminal() || tl.level() < th.level()) {
        if(debug_enabled) std::cout << " \t th terminal  or tl less case\n";
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

      else {
          throw invalid_argument("Unexpected case missing!");
      }

  }
  
  //calculate from BDD levels and m what levels should be sweeped - maybe shouldn't be in this file??
  template<typename Policy>
  std::vector<typename Policy::label_type>
  levels_from_map(const replace_func<Policy>& m, const typename Policy::dd_type& dd){
    level_info_ifstream<true> level_info_file(dd);
    std::vector<typename Policy::label_type> vec_to_fill;
    level_info init = level_info_file.pull();
    bdd::label_type min_seen = m(init.level());
    while(level_info_file.can_pull()){
      level_info l = level_info_file.pull();
      //std::cout << "levels from map order" << l << "\n";
      if (m(l.level()) > min_seen) {vec_to_fill.push_back(l.level()); continue;}
      min_seen = m(l.level());
    }
    return vec_to_fill;
  }

  //find top req from given PQs
  template<typename PQ1, typename PQ2>
  cor_req_t<1>
  getNext(PQ1& pq1 , PQ2& pq2){
    cor_req_t<1> r;
    if (pq1.can_pull()) {
          ptr_uint64 l_uid(pq1.top().data.level, 0);  //for treating level like uid for comp
          ptr_uint64 min_pq1 = std::min(pq1.top().target.first() , l_uid);
          if(pq2.empty() || min_pq1 < pq2.top().target.second()) {
            r = {pq1.top().target, 
                  { { { node::pointer_type::nil(), node::pointer_type::nil() } } }, 
                  pq1.top().data};
            if(debug_enabled) std::cout << "takes req " << r << " from pq1\n";
          } else {
            r = pq2.top();
            if(debug_enabled) std::cout << "takes req " << r << " from pq2\n";
          }
    } else {
          r = pq2.top();
          if(debug_enabled) std::cout << "takes req " << r << " from pq2\n";
    }
    return r;
  }

  //------------------------------------- correctify logic for single level ------------------------------------------
  template <typename Policy, typename In, typename Out, typename PQ1, typename PQ2>
  void
  correctify_single_level(In& in, Out& aw, PQ1& pq1, PQ2& pq2, typename Policy::node_type& v, const bool shift_back){
    //does all the correctify stuff for single level - for use both in general non-monotone replace and jump-down special case
    //TODO: flag is kinda dummy currently - added such that adj_swap can do shift-back on the fly

    //vars
    typename Policy::label_type label = pq1.current_level();
    typename Policy::label_type id = -1; 

     while(!pq1.empty_level() || pq2.has_top()) {
        cor_req_t<1> r = getNext(pq1, pq2);

        //updating tseek, v
        const ptr_uint64 t_uid(r.data.level, 0); //id here is questionable..
        ptr_uint64 tseek = (r.empty_carry()) ? std::min(r.target.first(), t_uid) : r.target.second(); 
        while (v.uid() < tseek && in.can_pull()) { v = in.pull(); }
        
        //CASE found correct layer!
        if (r.target.first().level() > r.data.level) {
          if (debug_enabled) std::cout << "enters copy case \n";
          
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
            //typename Policy::uid_type test = (label/2, id);
            node::uid_type out_uid = (shift_back) ?  node::uid_type(label/2, id) : node::uid_type(label,id);
            typename Policy::label_type nil_lbl = node::pointer_type::nil().level();
            tuple<typename Policy::pointer_type> tl = {r.target[0],node::pointer_type::nil()};
            tuple<typename Policy::pointer_type> th = {r.target[1],node::pointer_type::nil()};
            pusher<Policy>(pq1,aw,out_uid.as_ptr(false),tl,nil_lbl);
            pusher<Policy>(pq1,aw,out_uid.as_ptr(true), th,nil_lbl);

            // forward incoming
            internal_pusher<Policy, PQ1, 0>(pq1, aw, out_uid, r.target,  r.data.level);
            
          }
          continue;
        }
      
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
        node::uid_type out_uid = (shift_back) ?  node::uid_type(label/2, id) : node::uid_type(label,id);
        pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(false), rlow, r.data.level);
        pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(true), rhigh, r.data.level);

        // forward incoming
        internal_pusher<Policy, PQ1, 0>(pq1, aw, out_uid, r.target,  r.data.level);
        internal_pusher<Policy, PQ2, 1>(pq2, aw, out_uid, r.target,  r.data.level);
        }
        if(debug_enabled) std::cout << "finished work for level "  << label << "\n";
        const typename Policy::label_type level_to_push = (shift_back) ? label/2 : label;
        if (id >= 0) { aw.push(level_info(level_to_push, id+1)); }
  }

  //--------------------------JUMP_DOWN special case-----------------------------------

  template <typename Policy, typename PQ1, typename PQ2>
  inline typename Policy::__dd_type
  replace_jump_down_sweep(const typename Policy::dd_type& dd, 
                          replace_func<Policy> m,
                          exec_policy ep,
                          size_t pq1_mem,
                          size_t max_pq1_size,
                          size_t pq2_mem,
                          size_t max_pq2_size) {
    if (debug_enabled) std::cout << "start jump_down special case! \n";
    //setup input
    node_ifstream<> in(dd);
    node root = in.pull();
    
    //setup output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);
    
    //finding jump_down levels and targets
    //TODO move to seperate function pls
    if (debug_enabled) std::cout << "generating generators for jump down\n";
    //open info file
    level_info_ifstream<> info_in(dd);
    //vecs to fill
    std::vector<typename Policy::label_type> jump_starts;
    std::vector<typename Policy::label_type> jump_targets;
    while (info_in.can_pull()){
      level_info l = info_in.pull();
      if (debug_enabled) std::cout << "found level " << l << "\n";
      if (m(l.label()) > l.label()) {
        jump_starts.push_back(l.label());
        jump_targets.push_back(m(l.label()));
      }
    }
    
    //build generators
    typename std::vector<typename Policy::label_type>::iterator s_begin = jump_starts.begin(), s_end = jump_starts.end();
    typename std::vector<typename Policy::label_type>::iterator t_begin = jump_targets.begin(), t_end = jump_targets.end() ;
    generator<typename Policy::label_type> level_gen = make_generator(s_begin, s_end);
    generator<typename Policy::label_type> target_gen = make_generator(t_begin, t_end);
    optional<typename Policy::label_type> next_jump_down = level_gen();

    //setup PQs
    statistics::levelized_priority_queue_t test;
    PQ1 pq1({dd,target_gen}, pq1_mem , max_pq1_size, test);
    PQ2 pq2(pq2_mem, max_pq2_size);

    //init req
    cor_req_t<0> init_r;
    if (root.uid().level() == next_jump_down) {
      if (debug_enabled) {std::cout << "CASE first level moves down\n";}
      //push proper req
       init_r = {{root.low(), root.high()},{},{ptr_uint64::nil(), m(root.uid().level())}};
       next_jump_down = level_gen();
    } else {
      if (debug_enabled) {std::cout << "CASE first level doesn't moves down\n";}
      //just init copy req
       init_r = {{root.uid(),node::pointer_type::nil() },
                 {},{ptr_uint64::nil(), node::pointer_type::nil().level()}};
    }
    if (debug_enabled) std::cout << "init jump_down req: " << init_r << "\n";
    
    pq1.push(init_r); 

    node v = in.pull();
    while(!pq1.empty()){
      pq1.setup_next_level();
      typename Policy::label_type lable = pq1.current_level();
      //typename Policy::label_type id = -1;

      if (lable == next_jump_down) {
        if (debug_enabled) std::cout << "found jump down level " << lable << "\n";
        //push reqs
        while(!pq1.empty_level()){
          cor_req_t<0> req = pq1.pull();
          const ptr_uint64 t_uid(req.data.level, 0);
          ptr_uint64 tseek = std::min(req.target.first(), t_uid);
          while (v.uid() < tseek && in.can_pull()) { v = in.pull(); }
          cor_req_t<0> n_req = {{v.low(), v.high()},{},{req.data.source, m(lable)}};
          if (debug_enabled) std::cout << "pushing req to PQ1 " << n_req << "\n";
          pq1.push(n_req);
        }
        //update next_jump_down
        next_jump_down = level_gen();
        //no level update since this level no longer exists

      } else {
        //do normal cor stuff for this level
        correctify_single_level<Policy>(in, aw, pq1, pq2,  v, false);
      }
    }
    return typename Policy::__dd_type(out_arcs,ep); 
  }



// -------------------------- Adj Swap special case -------------------------------------
//NOTE TO SELF
//all levels are multiplied by 2 to ensure that the extra level we work with for each swap is free
//the initial doubling means that we do an extra sweep but i dont see how this can be avoided
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
    //could maybe do this in the PQ init thing? eeeh..
    replace_func<Policy> shift_forward = [](int x){ return x*2;};
    replace_func<Policy> shift_back = [](int x){ return x / 2;};
    const typename Policy::dd_type dd_shifted = bdd_replace(dd, shift_forward,replace_type::Monotone); //expeeensive                       

    //setup input
    node_ifstream<> in(dd_shifted);
    node root = in.pull();
    
    //setup output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);   

    //should
    // (1) find all adj swaps -> starts and insert level (which can now safely be start+1)
    // (2) for levels above first swap - just copy reqs                      
    // (3) when meet first swap level, output nothing push 2-ary request s -> x_low , x_high 
    // (4) then handling first swap target level - spicy:
    //    (a) requests that dont have levels - handle like normal but push arcs with label xi, requests contain level xj
    //    (b) requests with levels will be at correct level when they met -> should be pushed with their new level
    // so this is almost jump down sweep but with weird extra stuff happening.. for now we code duplicate maybe to be cleaned later..
    
    
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
    std::cout << "extra levels [ ";
    for(typename Policy::label_type e : swap_extra) {std::cout << e << ",";}
    std::cout << "]";

    typename std::vector<typename Policy::label_type>::iterator s_begin = swap_starts.begin(), s_end = swap_starts.end();
    typename std::vector<typename Policy::label_type>::iterator t_begin = swap_end.begin(), t_end = swap_end.end();
    typename std::vector<typename Policy::label_type>::iterator e_begin = swap_extra.begin(), e_end = swap_extra.end();
    generator<typename Policy::label_type> level_gen = make_generator(s_begin, s_end);
    generator<typename Policy::label_type> end_gen = make_generator(t_begin, t_end);
    generator<typename Policy::label_type> extra_gen = make_generator(e_begin, e_end);
    optional<typename Policy::label_type> next_swap = level_gen();
    optional<typename Policy::label_type> next_target = end_gen();

    //setup PQs
    statistics::levelized_priority_queue_t test;
    PQ1 pq1({dd_shifted, extra_gen}, pq1_mem , max_pq1_size, test);
    PQ2 pq2(pq2_mem, max_pq2_size);

    //init request
    cor_req_t<0> init_req;
    if(root.uid().label() == next_swap){
      //push 2-ary to children
      if(debug_enabled) std::cout << "root is part of a swap\n";
      init_req = {{root.low(), root.high()},{},{ptr_uint64::nil()}};
    } else {
      //just push 1-ary
      if(debug_enabled) std::cout << "root is NOT part of a swap\n";
      init_req = {{root.uid(), ptr_uint64::nil()},{},{ptr_uint64::nil()}};
    }
    pq1.push(init_req);
    
    node v = in.pull();
    while(!pq1.empty()){ 
      pq1.setup_next_level();
      typename Policy::label_type label = pq1.current_level();
      typename Policy::label_type id = -1;
      if(debug_enabled) std::cout << "starting work for level" << label <<"\n";

      if(label == next_swap){
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
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
       //////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (debug_enabled) std::cout << "found target of next adjacent swap: " << label << "\n";
        //correctify but
        // (0) we never in correct layer case cus nothign has level diff from nil at this point 
        // (1) out_uid has label (next_swap)
        // (2) pushes to PQ1 are given level = label+1 (aka next_extra)

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
          typename Policy::label_type extra_level = next_target.value() + 1;
          pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(false), rlow, extra_level);
          pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(true), rhigh, extra_level);

          // forward incoming
          internal_pusher<Policy, PQ1, 0>(pq1, aw, out_uid, r.target,  r.data.level);
          internal_pusher<Policy, PQ2, 1>(pq2, aw, out_uid, r.target,  r.data.level);
        }
        if (id >= 0) {aw.push(level_info(next_swap.value()/2, id+1));}

      } else if (next_target.has_value() && label == next_target.value() + 1){
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      if (debug_enabled) std::cout << "found extra level: " << label << "\n";
        //correctify but
        // (0) we always in correct layer case
        // (1) push to out with label-1 (aka next_target)
        while(!pq1.empty_level()) {
          cor_req_t<0> r = pq1.top();
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
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
        //we're not doing adj swaps - just copy like always.. 
        if (debug_enabled) std::cout << "found non-involved level: " << label << "\n";
        correctify_single_level<Policy>(in, aw, pq1, pq2,  v, true);
      }

    }
    if (debug_enabled) std::cout << "finished all levels :D \n";
    return typename Policy::__dd_type(out_arcs,ep); 
}

//--------------------- setup of PQs for Non-monotone single sweeps (not nested sweeping stuff...) ------------------------------
//aka setting up PQ1 and PQ2 types for the special cases... 
//so goal is we just pass PQs along to special case funcs?

//so idea
// shared entry-point takes replace type and depending on it runs right special case after setting up PQs
// NOTE: could potentially be cleaner to do this in the replace func that initially delegates 
// but since only non-monotone cases need PQs we do it here for now
template <typename Policy, typename Cut, size_t ConstSizeInc, typename In>
  size_t
  __cor_ilevel_upper_bound(const In& in)
  {
    const safe_size_t max_cut_all = Cut::get(in,cut::type::All);
    return to_size(max_cut_all * max_cut_all + 2);
  }

template<typename Policy>
typename Policy::__dd_type //special cases return arc format
replace(typename Policy::dd_type dd, 
        replace_func<Policy> m,
        replace_type t, 
        exec_policy ep){
  //should
  //(1) setup PQs
  //PQ setup based on similar in prod2u..
  //All of the memory variables 
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
  const size_t pq_2_bound =  __cor_ilevel_upper_bound<Policy, get_1level_cut, 0u>(dd); //should this be 2 level cut??

  const size_t max_pq_1_size = internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;
  const size_t max_pq_2_size = internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;
  //(2) switch on replace-type and run correct thing from there...
  
  //TODO: the code duplication here is horrible 
  if(!external_only && max_pq_1_size <= no_lookahead_bound(2)){ //internal mem no lookahead case
    using PQ1 = cor_lvl_priority_queue_t<0, memory_mode::Internal,2u>;
    using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;

    switch (t) {
      case replace_type::Jump_Down: 
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      case replace_type::Swap_Adjacent:
        return replace_adj_swap_sweep<Policy, PQ1, PQ2>(dd, m, ep,
           pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      default: //Non-Monotonic, jump-up, swap, and all Monotonic cases
        adiar_unreachable();
    }

  } else if (!external_only && max_pq_1_size <= pq_1_memory_fits && max_pq_2_size <= pq_2_memory_fits) { //internal mem with lookahead case
    using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal,2u>;
    using PQ2 = cor_priority_queue_2_t<memory_mode::Internal>;

    switch (t) {
      case replace_type::Jump_Down: 
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      case replace_type::Swap_Adjacent:
        return replace_adj_swap_sweep<Policy, PQ1, PQ2>(dd, m, ep,
           pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);

      default: //Non-Monotonic, jump-up, swap, and all Monotonic cases
        adiar_unreachable();
    }

  } else { // PQs dont fit in internal so external
    using PQ1 = cor_lvl_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External,2u>;
    using PQ2 = cor_priority_queue_2_t<memory_mode::External>;

    switch (t) {
      case replace_type::Jump_Down: 
        return replace_jump_down_sweep<Policy,PQ1,PQ2>(dd,  m,  ep, 
        pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      case replace_type::Swap_Adjacent:
        return replace_adj_swap_sweep<Policy, PQ1, PQ2>(dd, m, ep,
           pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      default: //Non-Monotonic, jump-up, swap, and all Monotonic cases
        adiar_unreachable();
    }
  }
}

//----------------------------- full correctify sweep ---------------------------------
  template <typename Policy, typename PQ1, typename PQ2, typename In>
  inline typename Policy::__dd_type
  replace_cor_scan_level(const In& in, 
                         PQ1& pq1,
                         PQ2& pq2,
                         exec_policy ep)
    //we assume that pqs have been preloaded when given here - as done by nested sweeping                    
  {
     // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    // Set up input
    node_ifstream<> in_nodes(in);
    node v = in_nodes.pull();

    while(!pq1.empty()){
      pq1.setup_next_level();

      correctify_single_level<Policy>(in_nodes, aw, pq1, pq2, v, false);
  }
     if(debug_enabled) std::cout << "exited big loop!\n";
     //aw.close();  // shouldn't need to do this..
     return typename Policy::__dd_type(out_arcs,ep);
}


  ///JUMP_UP special case
  //TBA
  ///ADJ_SWAP special case
  //TBA

  ///NON_MONOTONE

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

    static size_t pq_pull() {return 2u;}

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
    // - __cor_ilevel_upper_bound is different -> takes all arcs int oaccount for cut??
    template <typename inner_pq_t>
    __bdd sweep_pq([[maybe_unused]]const exec_policy& ep,
                   [[maybe_unused]]const shared_levelized_file<node>& outer_file,
                   inner_pq_t& inner_pq,
                   const size_t inner_remaining_memory){
      std::cout << "START sweep_pq \n";
      //should
      // (1) setup PQ2
      const size_t pq_2_memory_fits =
      cor_priority_queue_2_t<memory_mode::Internal>::memory_fits(inner_remaining_memory);

      const size_t pq_2_bound =__cor_ilevel_upper_bound<Policy, get_2level_cut, 0u>(typename Policy::dd_type(outer_file))
        // Add crossing arcs
        + (inner_pq.size());
      
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
        //return sweep_pq(ep, outer_file, inner_pq, inner_remaining_memory);
    }

    bool
    has_sweep(const typename Policy::label_type l)
    {   
        std::cout << "trying to check has_sweep... \n";
        //this may be wrong..
        bool res = l == next_level(l);
        std::cout << "testing if " << l << " has sweep -> " << res << "\n";
        return res;
    }

    typename Policy::label_type
    next_level(const typename Policy::label_type l)
    {
      std::cout << "trying to find next level... \n";
      while (_next_level.has_value() && l < _next_level.value()) { 
        std::cout << "check1 \n";
        _next_level = _nesting_levels(); 
        std::cout << "check2 \n"; 
      }
      //std::cout << "updated next_level to " << _next_level.value() << "\n";
      return _next_level.value_or(Policy::max_label + 1);
    }

    template <typename outer_roots_t>
    __bdd
    sweep(const exec_policy& ep,
          const shared_levelized_file<node>& outer_file,
          outer_roots_t& outer_roots,
          const size_t inner_memory)
    {
        std::cout << "runs sweep \n";
        adiar::statistics::__alg_base::__lpq_t test;
        return nested_sweeping::inner::down__sweep_switch(
            ep, *this, outer_file, outer_roots, inner_memory, test);
    }

    inline request_t
    request_from_node(const node& n, const ptr_uint64& parent)
    {
        //only run for nodes where we want to update level?
        std::cout << "runs req from node with " << n << "\n";
        typename Policy::label_type new_lvl = _m(n.label());
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
    
  static constexpr bool final_canonical = true;
  static constexpr bool fast_reduce     = true; //should not reduce

};

  // nested sweeping entry
  template <typename Policy>
  typename Policy::__dd_type
  replace_nested_sweep(const typename Policy::dd_type& dd,
                       const replace_func<Policy>& m,
                       exec_policy ep) {
    
    //setup policy
    std::vector<typename Policy::label_type> nest_levels = levels_from_map<Policy>(m, dd);
    
    if(debug_enabled) std::cout << "levels to sweep on: [";
    for(typename Policy::label_type e : nest_levels) {if(debug_enabled) std::cout << e << ", ";}
    if(debug_enabled) std::cout << "]\n";
    auto begin = nest_levels.begin();
    auto end = nest_levels.end();
    generator<typename Policy::label_type> level_gen = make_generator(begin,end);
    
    std::vector<typename Policy::label_type> targets;
    for (typename Policy::label_type l : nest_levels) {
      targets.push_back(m(l));
    }
    auto t_begin = targets.begin();
    auto t_end = targets.end();
    generator<typename Policy::label_type> targets_gen = make_generator(t_begin,t_end);

    nested_sweeping_replace<Policy> test_inner_impl(m, level_gen, targets_gen);

    //run nested sweep
    bdd res = nested_sweep<>(ep, dd, test_inner_impl);
    std::cout << "Replace nested-sweeping complete! \n";

    return res;
    }
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // "Public" interface

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
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
      //throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");
    case replace_type::Jump_Down:
      return replace<Policy>(dd,m,inferred_type, ep);
      //return replace_nested_sweep<Policy>(dd,m,ep);
    case replace_type::Swap_Adjacent:
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
      //throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");

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
