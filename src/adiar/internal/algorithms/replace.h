#ifndef ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
#define ADIAR_INTERNAL_ALGORITHMS_REPLACE_H

#include "adiar/bdd.h"
#include "adiar/exec_policy.h"
#include "adiar/internal/data_types/level_info.h"
#include "adiar/internal/data_types/ptr.h"
#include "adiar/internal/data_types/request.h"
#include "adiar/internal/data_types/uid.h"
#include "adiar/internal/io/arc_ofstream.h"
#include "adiar/internal/io/shared_file_ptr.h"
#include "adiar/internal/memory.h"
#include <functional>
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

    bool identity = true;
    bool shift    = true;
    bool monotone = true;
    bool jump_down = true;

    bool only_down = true;   //if things ever move up this false
    bool movers_preserve_order = true; //if jumps corss layers false
    typename Policy::label_type last_jump = 0;
    
    label_type prev_before = Policy::max_label + 1;
    label_type prev_after  = Policy::max_label + 1;

    signed_label_type prev_diff = 0;

    while (ls.can_pull()) {
      const label_type next_before     = ls.pull().level();
      const result_type next_after_opt = m(next_before);

      //seems unnecessary??
      /*if constexpr (is_partial_map) {
        if (!next_after_opt.has_value()) { continue; }
      }*/

      label_type next_after;
      if constexpr (is_partial_map) {
        if (!next_after_opt.has_value()) { continue; }
        next_after = *next_after_opt;
      } else {
        next_after = next_after_opt;
      }

      if (shift) {
        const signed_label_type next_diff =
          static_cast<signed_label_type>(next_before) - static_cast<signed_label_type>(next_after);

        shift &= Policy::max_label < prev_before || prev_diff == next_diff;
        prev_diff = next_diff;
      }

      identity &= next_before == next_after;
      monotone &= Policy::max_label < prev_before || prev_after < next_after;

      //JUMP_DOWN checks
      if(next_after != next_before ){ //level is moved check
        only_down &= (next_after > next_before); //level moved down / static?
        movers_preserve_order &= (last_jump < next_after); //seems wrong
        last_jump = next_after;
      }

      prev_before = next_before;
      prev_after  = next_after;
    }
    jump_down = only_down && movers_preserve_order;

    if (!monotone) { 
      if (jump_down) {std::cout << "\n detected jump_down\n" ;
         return replace_type::Jump_Down;}
      return replace_type::Jump_Down; } //DUMMY!
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
  using cor_priority_queue_1_t = priority_queue<mem_mode, cor_req_t<0>,
                                request_data_first_lt<cor_req_t<0>>>;

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

          
  template<typename Policy, typename pq_t>
  inline void 
  pusher(pq_t& pq, 
         arc_ofstream& out_stream, 
         ptr_uint64 source, 
         tuple<typename Policy::pointer_type>& target, 
         typename Policy::label_type& level) {
    std::cout << "enter pusher here??\n";
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
        if (debug_enabled) std::cout<< "(from pusher) pushing req to pq1: " << lreq << "\n";
        pq.push(lreq);
        std::cout << "we reach here??? \n";
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
          if (debug_enabled) std::cout << "goes into while again?\n";
          const cor_req_t<nc> r1 = pq.top(); pq.pop(); //non-levelized has no pull
          if (debug_enabled) std::cout << "has popped: " << r1 << "\n";
          //r1.data.source.level() != ptr_uint64::nil().level()
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
  reqFor(tuple<typename Policy::pointer_type> t, 
         node v , 
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
  std::vector<int>
  levels_from_map(replace_func<Policy> m, internal::level_info_ifstream<false>& level_info_file){
    std::vector<int> vec_to_fill;
    level_info init = level_info_file.pull();
    bdd::label_type min_seen = m(init.level());
    while(level_info_file.can_pull()){
      level_info l = level_info_file.pull();
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
          if(debug_enabled) std::cout << "takes req from pq1\n";
          if(pq2.empty() || min_pq1 < pq2.top().target.second()) {
            r = {pq1.top().target, 
                  { { { node::pointer_type::nil(), node::pointer_type::nil() } } }, 
                  pq1.top().data};
          } else {
            if (debug_enabled)std::cout << "\ntakes req from pq2\n";
            r = pq2.top();
          }
    } else {
          if (debug_enabled)std::cout << "\ntakes req from pq2\n";
          r = pq2.top();
    }
    return r;
  }

  template <typename Policy, typename In, typename Out, typename PQ1, typename PQ2>
  void
  correctify_single_level(In& in, Out& aw, PQ1& pq1, PQ2& pq2, typename Policy::node_type& v){
    //does all the correctify stuff for single level - for use both in general non-monotone replace and jump-down special case
    
    //vars
    typename Policy::label_type label = pq1.current_level();
    typename Policy::label_type id = -1;

     while(!pq1.empty_level() || pq2.has_top()) {
        cor_req_t<1> r = getNext(pq1, pq2);
        if(debug_enabled) std::cout << "found req is" << r << "\n";

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
            const node::uid_type out_uid(label, id); //x_label,id
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
        const node::uid_type out_uid(label, id); //x_label,id
        pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(false), rlow, r.data.level);
        pusher<Policy, PQ1>(pq1, aw, out_uid.as_ptr(true), rhigh, r.data.level);

        // forward incoming
        internal_pusher<Policy, PQ1, 0>(pq1, aw, out_uid, r.target,  r.data.level);
        internal_pusher<Policy, PQ2, 1>(pq2, aw, out_uid, r.target,  r.data.level);
        }
        if(debug_enabled) std::cout << "finished work for level "  << label << "\n";
        if (id >= 0) { aw.push(level_info(label, id+1)); }
  }

  template <typename Policy, typename PQ1, typename PQ2, typename In>
  inline typename Policy::__dd_type
  replace_cor_scan_level(const In& in, 
                         PQ1& pq1,
                         PQ2& pq2)
    //we assume that pq has been preloaded when given here - as done by nested sweeping                    
  {
     // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    // Set up input
    node_ifstream<> in_nodes(in);
    node v = in_nodes.pull();

    while(!pq1.empty()){
      pq1.setup_next_level();

      correctify_single_level<Policy>(in_nodes, aw, pq1, pq2, v);
  }
     if(debug_enabled) std::cout << "exited big loop!\n";
     aw.close();  // shouldn't need to do this..
     return typename Policy::__dd_type(out_arcs,exec_policy::access::Auto);
}

  template <typename Policy>
  inline typename Policy::__dd_type
  replace__cor_scan_fancy(const typename Policy::dd_type& dd, 
                          const replace_func<Policy>& m) {
    // Set up input
    node root;
    {node_ifstream<> in_nodes(dd);
     root = in_nodes.pull();
    }
    //setup PQs
    const size_t aux_available_memory = memory_available();
    const size_t pq1_memory = aux_available_memory / 2;
    const size_t max_pq_1_size = aux_available_memory / 10;
    statistics::levelized_priority_queue_t test;
    using PQ1 = cor_lvl_priority_queue_t<1, memory_mode::External,2>;
    PQ1 pq1({dd,make_generator(m(root.label()))}, pq1_memory , max_pq_1_size, test);

    using PQ2 = cor_priority_queue_2_t<memory_mode::External>;
    PQ2 pq2(memory_available()/2, memory_available() / 10);

    //init req
    cor_req_t<0> r = {{root.low(), root.high()},{},{ptr_uint64::nil(), m(root.label())}};
    pq1.push(r);

    //run sweep
    typename Policy::__dd_type res = replace_cor_scan_level<Policy,PQ1,PQ2,typename Policy::dd_type>(dd, pq1, pq2);
    return res;
  }

  ///JUMP_DOWN special case

  //func for creating generators from dd_info and m
  //cost an extra sweep of the info file - these could have been made when detecting jump-down...
  template <typename Policy>
  std::tuple<generator<typename Policy::label_type>,generator<typename Policy::label_type>>
  genenrator_generator(const typename Policy::dd_type& dd, 
                       replace_func<Policy> m){
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
    
    if (debug_enabled) {
      std::cout << "found levels: [";
      for(typename Policy::label_type e : jump_starts){
        std::cout << e << ", ";
      }
      std::cout << "]\n";
      std::cout << "found targets: [";
      for(typename Policy::label_type e : jump_targets){
        std::cout << e << ", ";
      }
      std::cout << "]\n";
    }
    
    //build generators
    typename std::vector<typename Policy::label_type>::iterator s_begin = jump_starts.begin();
    typename std::vector<typename Policy::label_type>::iterator s_end = jump_starts.end();
    typename std::vector<typename Policy::label_type>::iterator t_begin = jump_targets.begin();
    typename std::vector<typename Policy::label_type>::iterator t_end = jump_targets.end();
    generator<typename Policy::label_type> test_level = make_generator(s_begin, s_end);
    generator<typename Policy::label_type> test_target = make_generator(t_begin, t_end);

    std::tuple<generator<typename Policy::label_type>,generator<typename Policy::label_type>> test(test_level, test_target);
    return test ;
  }

  template <typename Policy>
  inline typename Policy::__dd_type
  replace_jump_down_sweep(const typename Policy::dd_type& dd, 
                          replace_func<Policy> m) {
    if (debug_enabled) std::cout << "start jump_down special case! \n";
    //setup input
    node_ifstream<> in(dd);
    node root = in.pull();
    
    //setup output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);
    
    //finding jump_down levels and targets
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
    
    if (debug_enabled) {
      std::cout << "found levels: [";
      for(typename Policy::label_type e : jump_starts){
        std::cout << e << ", ";
      }
      std::cout << "]\n";
      std::cout << "found targets: [";
      for(typename Policy::label_type e : jump_targets){
        std::cout << e << ", ";
      }
      std::cout << "]\n";
    }
    
    //build generators
    typename std::vector<typename Policy::label_type>::iterator s_begin = jump_starts.begin();
    typename std::vector<typename Policy::label_type>::iterator s_end = jump_starts.end();
    typename std::vector<typename Policy::label_type>::iterator t_begin = jump_targets.begin();
    typename std::vector<typename Policy::label_type>::iterator t_end = jump_targets.end();
    generator<typename Policy::label_type> level_gen = make_generator(s_begin, s_end);
    generator<typename Policy::label_type> target_gen = make_generator(t_begin, t_end);
    optional<typename Policy::label_type> next_jump_down = level_gen();

    //setup PQs
    const size_t aux_available_memory = memory_available();
    const size_t pq1_memory = aux_available_memory / 2;
    const size_t max_pq_1_size = aux_available_memory / 10;
    statistics::levelized_priority_queue_t test;
    using PQ1 = cor_lvl_priority_queue_t<1, memory_mode::External,2>;
    PQ1 pq1({dd,target_gen}, pq1_memory , max_pq_1_size, test);
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
       init_r = {{root.uid(),node::pointer_type::nil() },{},{ptr_uint64::nil(), node::pointer_type::nil().level()}};
    }
    if (debug_enabled) std::cout << "init jump_down req: " << init_r << "\n";
    pq1.push(init_r); 

    using PQ2 = cor_priority_queue_2_t<memory_mode::External>;
    PQ2 pq2(memory_available()/2, memory_available() / 10);
    
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
          pq1.push(n_req);
        }
        //update next_jump_down
        next_jump_down = level_gen();

      } else {
        //do normal cor stuff for this level
        correctify_single_level<Policy>(in, aw, pq1, pq2,  v);
      }
    }
    return typename Policy::__dd_type(out_arcs,exec_policy::access::Auto); 
  }
  ///JUMP_UP special case

  ///ADJ_SWAP special case
  //TBA
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // "Public" interface

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  replace(const exec_policy& /*ep*/,
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
      throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");
    case replace_type::Jump_Down:
      return replace_jump_down_sweep<Policy>(dd,m);
    case replace_type::Swap_Adjacent:
      return replace__cor_scan_fancy<Policy>(dd,m);
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
      throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");

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