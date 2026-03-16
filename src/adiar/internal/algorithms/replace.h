#ifndef ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
#define ADIAR_INTERNAL_ALGORITHMS_REPLACE_H

#include "adiar/bdd.h"
#include "adiar/internal/data_types/ptr.h"
#include "adiar/internal/data_types/request.h"
#include "adiar/internal/data_types/uid.h"
#include "adiar/internal/io/shared_file_ptr.h"
#include <type_traits>

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

    label_type prev_before = Policy::max_label + 1;
    label_type prev_after  = Policy::max_label + 1;

    signed_label_type prev_diff = 0;

    while (ls.can_pull()) {
      const label_type next_before     = ls.pull().level();
      const result_type next_after_opt = m(next_before);

      if constexpr (is_partial_map) {
        if (!next_after_opt.has_value()) { continue; }
      }

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

      prev_before = next_before;
      prev_after  = next_after;
    }

    if (!monotone) { return replace_type::Jump_Down; } //DUMMY!
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
  using cor_priority_queue_1_t = priority_queue<mem_mode, cor_req_t<0>,
                                request_data_first_lt<cor_req_t<0>>>;

  template <memory_mode mem_mode>
  using cor_priority_queue_2_t = priority_queue<mem_mode, cor_req_t<1>,
                                request_data_second_lt<cor_req_t<1>>>;

  template<typename Policy, typename pq_t>
  inline void 
  pusher(pq_t& pq, 
         arc_ofstream& out_stream, 
         ptr_uint64 source, 
         tuple<typename Policy::pointer_type>& target, 
         typename Policy::label_type& level) {

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
     while((!pq.empty()) && pq.top().target == target 
                             && pq.top().data.level == level) {
          if (debug_enabled) std::cout << "goes into while again?\n";
          const cor_req_t<nc> r1 = pq.top(); //when levelized change to pull
          pq.pop();
          if (debug_enabled) std::cout << "has popped: " << r1 << "\n";
          if (r1.data.source != ptr_uint64::nil()) {
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

      auto label_cmp = [](bdd::pointer_type tl, bdd::pointer_type th){
            //th not terminal
        if (tl.is_terminal()) {return false;}
        else {return tl.label() < th.label();}
      };

      if (tl.is_terminal() && th.is_terminal()) {
        if(debug_enabled) std::cout << " \t both terminal case\n";
        return { {tl,tl}, {th,th} };
      }

      if (th.is_terminal() || label_cmp(tl, th)) {
        if(debug_enabled) std::cout << " \t th terminal  or tl less case\n";
        return { {v.low(), th}, {v.high(), th}};
      }

      if (tl.is_terminal() || tl.label() > th.label()) {
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
  
  template <typename Policy>
  inline typename Policy::dd_type
  relabel_all(const typename Policy::dd_type& dd, 
                          const replace_func<Policy>& m) {
    shared_levelized_file<typename Policy::node_type> out;
    {
        node_ofstream nw(out);
        node_ifstream<true> in_nodes(dd); 
        while(in_nodes.can_pull()){
         node n = in_nodes.pull();
         nw.unsafe_push(__replace(n, m));
        }
    }
    return typename Policy::dd_type(out);
  } 

  template <typename Policy>
  inline typename Policy::__dd_type
  replace__cor_scan(const typename Policy::dd_type& dd, 
                          const replace_func<Policy>& m) {
    //TODO: replace with nested sweeping by inputting priority queues or levels if that works?
  
    bdd out = relabel_all<Policy>(dd,m);
    
    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    // Set up input
    node_ifstream<> in_nodes(out);
    node root = in_nodes.pull();

    //prep PQ1 (hardcoded for external + memory is whack)
    const size_t aux_available_memory = memory_available();
    using pq_1_type = cor_priority_queue_1_t<memory_mode::External>;
    const size_t pq_1_memory = aux_available_memory / 2;
    
    //prep PQ2 (same vibes)
    using pq_2_type = cor_priority_queue_2_t<memory_mode::External>;
    pq_2_type pq2(pq_1_memory, aux_available_memory / 10);
    
    //push initial request to PQ1 (based on root)
    pq_1_type pq1(pq_1_memory,  aux_available_memory / 10);
    cor_req_t<0> init_req({root.low(), root.high()}, {}, {ptr_uint64::nil(), root.uid().label()});
    pq1.push(init_req);
    
    //let v be smallest node after root
    node v = (in_nodes.can_pull()) ? in_nodes.pull() : throw invalid_argument("tree is only root?");
    typename Policy::id_type id = -1;
    typename Policy::label_type label = -1; //may make ids weird?
    while ((!pq1.empty()) || (!pq2.empty())) {
      //figure out which queue to pull from!
      //should be pq1 if it's min is less than pq2's second
      cor_req_t<1> r;
      //DUMMY! should also take level into acoount!
      if (pq1.has_top() && 
          (pq2.empty() || pq1.top().target.first() < pq2.top().target.second()))
          {
            if(debug_enabled) std::cout << "takes req from pq1\n";
            r = {pq1.top().target, 
                 { { { node::pointer_type::nil(), node::pointer_type::nil() } } }, 
                 pq1.top().data};
      } else {
            if (debug_enabled)std::cout << "\ntakes req from pq2\n";
            r = pq2.top();
      }

      if (debug_enabled) std::cout << "iter scan " << r << "\n";
      const node::uid_type t_uid(r.data.level, 0); //id here is questionable..
      typename adiar::internal::ptr_uint64 tseek;
      if (r.empty_carry()) { tseek = (r.target.first() < t_uid) ? r.target.first() : t_uid; } 
      else { tseek = r.target.second();}  //TODO fix dummy request fetchign from PQs see if this is still right..
      if(debug_enabled) std::cout << "tseek: " << tseek << "\n";
      while (v.uid() < tseek && in_nodes.can_pull()) { v = in_nodes.pull(); }
      if (debug_enabled) std::cout << "v: " << v << "\n";

      //correct layer check (can probably be more efficient)
      if (r.target[0].is_node() && r.target[1].is_node() &&
          r.data.level < r.target[0].label() && 
          r.data.level < r.target[1].label()) {
        if (debug_enabled) std::cout << "enters copy if \n";
        if (r.target[0] == r.target[1]) {
          if (debug_enabled)std::cout << "skip surpressible node! \n";
          const cor_req_t<0> r1 = pq1.top(); pq1.pop();
          typename Policy::id_type id1 = (label == r.target[0].label()) ? id+1 : 0;
          typename Policy::label_type lab1 = r.target[0].label() ; 
          const node::uid_type t_uid(lab1, id1);
          if (r1.data.source != ptr_uint64::nil()) {
            arc in = {r1.data.source , t_uid};
            if(debug_enabled) std::cout << "has pushed internal: " << in << "\n";
            aw.push_internal(in);
          }
          continue;
        }

        if (label == tseek.label()) id++;
        label = tseek.label() ;
        if(debug_enabled) std::cout << "label, id: " << label << "," << id << "\n";

        //push copy reqs
        const node::uid_type out_uid(label, id); //x_label,id
        pq1.push({{r.target[0],r.target[0]},
                    {},
                    {out_uid.as_ptr(false)}});

        pq1.push({{r.target[1],r.target[1]},
                    {},
                    {out_uid.as_ptr(true)}});

        // forward incoming
        internal_pusher<Policy, pq_1_type, 0>(pq1, aw, out_uid, r.target,  r.data.level);
        continue;
      }
    
      //big copy-paste from prod + small changes 
      if (r.empty_carry() && r.target[0].is_node() && r.target[1].is_node()
            && r.target[0].label() == r.target[1].label()
            && r.target[0].id() != r.target[1].id()) {
          if(debug_enabled) std::cout << "enters pq2 push if-statement!\n";
          const typename Policy::children_type children = v.children();
          while (pq1.has_top() && pq1.top().target == r.target) {
             if(debug_enabled) std::cout << "enters pq2 while\n";
             pq2.push({ r.target, { children }, pq1.top().data });
             pq1.pop();
          }
          continue;
      }

      //Case either on wrong level or at least one of targets is leaf.. 
      id = (label == tseek.label()) ? id+1 : 0; 
      label = tseek.label() ;
      if(debug_enabled) std::cout << "lable, id: " << label << "," << id << "\n";

      //check if we should update level in requests
      typename Policy::label_type new_level = r.data.level;
      if (r.target[0].is_terminal() && r.target[1].is_node()) {
        new_level = (r.data.level > r.target[1].label()) ? r.data.level : r.target[1].label();
      } else {
        if (r.target[1].is_terminal() && r.target[0].is_node()){
          new_level = (r.data.level > r.target[0].label()) ? r.data.level : r.target[0].label();
        }
      }
      if(debug_enabled) std::cout << "new level is: " << new_level << "\n";
     
      tuple<tuple<typename Policy::pointer_type>> reqs = reqFor<Policy>(r.target, v, r.node_carry[0][0], r.node_carry[0][1]);
      tuple<typename Policy::pointer_type> rlow = reqs[0]; 
      tuple<typename Policy::pointer_type> rhigh = reqs[1]; 
      
      const node::uid_type out_uid(label, id); //x_label,id
      pusher<Policy, pq_1_type>(pq1, aw, out_uid.as_ptr(false), rlow, new_level);
      pusher<Policy, pq_1_type>(pq1, aw, out_uid.as_ptr(true), rhigh, new_level);

      
      // forward incoming
      internal_pusher<Policy, pq_1_type, 0>(pq1, aw, out_uid, r.target,  r.data.level);
      internal_pusher<Policy, pq_2_type, 1>(pq2, aw, out_uid, r.target,  r.data.level);
    }
    if(debug_enabled) std::cout << "exited big loop!\n";
     aw.close();  // shouldn't need to do this..
     //in_nodes.close();

     //return typename Policy::__dd_type(out_arcs, exec_policy::access::Auto);
     return typename Policy::__dd_type(out_arcs,exec_policy::access::Auto);
  }


 
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
      return replace__cor_scan<Policy>(dd,m);
    case replace_type::Swap_Adjacent:
      return replace__cor_scan<Policy>(dd,m);
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
