#ifndef ADIAR_INTERNAL_ALGORITHMS_PROD2U_H
#define ADIAR_INTERNAL_ALGORITHMS_PROD2U_H

#include <adiar/functional.h>

#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/io/arc_ofstream.h>
#include <adiar/internal/io/node_ifstream.h>
#include <adiar/internal/io/node_raccess.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  //  2-ary Product Construction on a single DD
  // ===========================================
  //
  // Given a Decision Diagram and one (or more) variables, runs a product construction on the
  // children at the desired levels (removing the level in question).
  //
  // Unlike most other algorithms, this is merely supposed to be used internally. Hence, there is no
  // 'final' 'prod2u' function to be called. You have to wrap it with the outermost function.
  /*
  //             ____ O ____                    O
  //            /           \                 /   \
  //          (a)          (b)               /     \
  //         /   \    X   /   \     =>      /       \
  //        a0   a1      b1   b2        (a0,b0)   (a1,b1)
  */
  // Examples of uses are `internal::quantify`.
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::prod2u_t stats_prod2u;

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Data structures
  template <uint8_t NodesCarried>
  using prod2u_request = request_data<2, with_parent, NodesCarried, 1>;

  /// \brief Type of the primary priority queue for node-based inputs.
  template <size_t LookAhead, memory_mode MemoryMode>
  using prod2u_priority_queue_1_node_t =
    levelized_node_priority_queue<prod2u_request<0>,
                                  request_data_first_lt<prod2u_request<0>>,
                                  LookAhead,
                                  MemoryMode,
                                  1,
                                  0>;

  /// \brief Type of the secondary priority queue to further forward requests across a level.
  template <memory_mode MemoryMode>
  using prod2u_priority_queue_2_t =
    priority_queue<MemoryMode, prod2u_request<1>, request_data_second_lt<prod2u_request<1>>>;

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Common i-level cut computations

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Derives an upper bound on the output's maximum i-level cut given its maximum i-level
  ///        cut.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename Cut, size_t ConstSizeInc, typename In>
  size_t
  __prod2u_ilevel_upper_bound(const In& in)
  {
    const typename Cut::type ct_internal  = cut::type::Internal;
    const typename Cut::type ct_terminals = Policy::cut_with_terminals();

    const safe_size_t max_cut_internal  = Cut::get(in, ct_internal);
    const safe_size_t max_cut_terminals = Cut::get(in, ct_terminals);

    return to_size(max_cut_internal * max_cut_terminals + ConstSizeInc);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut given its size.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename In>
  size_t
  __prod2u_ilevel_upper_bound(const In& in)
  {
    const safe_size_t in_size = in.size();
    return to_size(in_size * in_size + 1u + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Common logic for sweeps

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Zero-indexed value for statistics on the arity of a request.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Request>
  bool
  __prod2u_arity_idx(const Request& req)
  {
    return req.targets() - 1;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Places a request from `source` to `target` in the output if resolved to a terminal.
  ///        Otherwise, it is placed in the priority queue to be resolved later.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename PriorityQueue>
  inline void
  __prod2u_recurse_out(PriorityQueue& pq,
                       arc_ofstream& aw,
                       const typename Policy::pointer_type source,
                       const prod2u_request<0>::target_t& target)
  {
    adiar_assert(!target.first().is_nil(),
                 "pointer_type::nil() should only ever end up being placed in target.second()");

    adiar_assert(source.is_nil() || source.level() < target.first().level(),
                 "Request should be placed further downwards");

    if (target.first().is_terminal()) {
      adiar_assert(target.second().is_nil(), "Operator should already be resolved at this point");
      aw.push({ source, target.first() });
    } else {
      pq.push({ target, {}, { source } });
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `request_foreach` for an arc to an internal node.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename UId>
  struct __prod2u_recurse_in__output_node
  {
  private:
    arc_ofstream& _aw;
    const UId& _out_uid;

  public:
    __prod2u_recurse_in__output_node(arc_ofstream& aw, const UId& out_uid)
      : _aw(aw)
      , _out_uid(out_uid)
    {
      adiar_assert(out_uid.is_node());
    }

    template <typename Request>
    inline void
    operator()(const Request& req) const
    {
#ifdef ADIAR_STATS
      stats_prod2u.requests[__prod2u_arity_idx(req)] += 1u;
#endif
      if (!req.data.source.is_nil()) {
        this->_aw.push_internal({ req.data.source, this->_out_uid });
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `request_foreach` for an arc to a terminal.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Pointer>
  struct __prod2u_recurse_in__output_terminal
  {
  private:
    arc_ofstream& _aw;
    const Pointer& _out_terminal;

  public:
    __prod2u_recurse_in__output_terminal(arc_ofstream& aw, const Pointer& out_terminal)
      : _aw(aw)
      , _out_terminal(out_terminal)
    {
      adiar_assert(out_terminal.is_terminal());
    }

    template <typename Request>
    inline void
    operator()(const Request& req) const
    {
      this->_aw.push_terminal({ req.data.source, this->_out_terminal });
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `request_foreach` for forwarding a request further, i.e. skipping outputting
  ///        an internal node.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename PriorityQueue, typename Target>
  struct __prod2u_recurse_in__forward
  {
  private:
    PriorityQueue& _pq;
    const Target& _t;

  public:
    __prod2u_recurse_in__forward(PriorityQueue& pq, const Target& t)
      : _pq(pq)
      , _t(t)
    {
      adiar_assert(t.first().is_node(), "Forwarding should only be used for internal nodes");
    }

    template <typename Request>
    inline void
    operator()(const Request& req) const
    {
      adiar_assert(req.data.source.is_nil() || req.data.source.level() < this->_t.first().level(),
                   "Request should be forwarded downwards");
#ifdef ADIAR_STATS
      stats_prod2u.requests[__prod2u_arity_idx(req)] += 1u;
#endif
      this->_pq.push({ this->_t, {}, req.data });
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Reduces a request with (up to) `Targets` many values into its canonical form.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline tuple<typename Policy::pointer_type, 2, true>
  __prod2u_resolve_request(const typename Policy::pointer_type& t1,
                           const typename Policy::pointer_type& t2)
  {
    using tuple_t = tuple<typename Policy::pointer_type, 2, true>;

    // Collapse to a single value, if `t1` and `t2` are equal
    if (t1 == t2) { return tuple_t(t1, Policy::pointer_type::nil()); }

    // Sort the two values
    const tuple_t ts = t1 < t2 ? tuple_t(t1, t2) : tuple_t(t2, t1);

    // Skip remainder, if only one value is not nil
    if (ts[1].is_nil()) { return ts; }

    // Prune terminals.
    if (ts[0].is_terminal() && !Policy::keep_terminal(ts[0])) {
      return tuple_t(ts[1], Policy::pointer_type::nil());
    }

    if (ts[1].is_terminal() && !Policy::keep_terminal(ts[1])) {
      return tuple_t(ts[0], Policy::pointer_type::nil());
    }

    // Collapse to terminal. Due to sorting and the above pruning, this value has to be the last.
    if (ts[1].is_terminal() && Policy::collapse_to_terminal(ts[1])) {
      return tuple_t(ts[1], Policy::pointer_type::nil());
    }

    // Are there only terminals left that should be combined with the operator?
    //
    // TODO (optimisation): disable based on policy
    if (ts[0].is_terminal() && ts[1].is_terminal()) {
      return tuple_t(Policy::resolve_terminals(ts[0], ts[1]), Policy::pointer_type::nil());
    }

    // Otherwise, return the sorted tuple.
    return ts;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Sweep logic with random access
  template <typename NodeRandomAccess, typename Policy, typename PriorityQueue>
  typename Policy::__dd_type
  __prod2u_ra(const exec_policy& ep, NodeRandomAccess& in_nodes, Policy& policy, PriorityQueue& pq)
  {
    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    // Process requests in topological order of both BDDs
    while (!pq.empty()) {
      // Set up level
      pq.setup_next_level();
      const typename Policy::label_type out_label = pq.current_level();
      typename Policy::id_type out_id             = 0;

      in_nodes.setup_next_level(out_label);

      const bool split = policy.split(out_label);

      while (!pq.empty_level()) {
        prod2u_request<0> req = pq.top();

#ifdef ADIAR_STATS
        stats_prod2u.requests_unique[__prod2u_arity_idx(req)] += 1u;
#endif

        // Obtain of first to-be seen node
        adiar_assert(req.target.first().level() == out_label,
                     "Level of requests always ought to match the one currently processed");

        const typename Policy::children_type children_fst =
          in_nodes.at(req.target.first()).children();

        const typename Policy::children_type children_snd = req.target.second().level() == out_label
          ? in_nodes.at(req.target.second()).children()
          : Policy::reduction_rule_inv(req.target.second());

        // -----------------------------------------------------------------------------------------
        // CASE: Split node into binary recursion request
        if (split) {
          const tuple<typename Policy::pointer_type, 2, true> rec_all =
            __prod2u_resolve_request<Policy>(children_fst[false], children_fst[true]);

          // Collapsed to a terminal?
          if (req.data.source.is_nil() && rec_all[0].is_terminal()) {
            adiar_assert(rec_all[1] == Policy::pointer_type::nil(),
                         "Operator should already be applied");

            return typename Policy::dd_type(rec_all[0].value());
          }

          prod2u_request<0>::target_t rec(rec_all[0], rec_all[1]);

          if (rec[0].is_terminal()) {
            const __prod2u_recurse_in__output_terminal handler(aw, rec[0]);
            request_foreach(pq, req.target, handler);
          } else {
            const __prod2u_recurse_in__forward handler(pq, rec);
            request_foreach(pq, req.target, handler);
          }

          continue;
        }

        // -----------------------------------------------------------------------------------------
        // CASE: Regular Level
        //   The variable should stay: proceed as in the Product Construction by simulating both
        //   possibilities in parallel.

        const node::uid_type out_uid(out_label, out_id++);

        prod2u_request<0>::target_t rec0 =
          __prod2u_resolve_request<Policy>(children_fst[false], children_snd[false]);

        __prod2u_recurse_out<Policy>(pq, aw, out_uid.as_ptr(false), rec0);

        prod2u_request<0>::target_t rec1 =
          __prod2u_resolve_request<Policy>(children_fst[true], children_snd[true]);
        __prod2u_recurse_out<Policy>(pq, aw, out_uid.as_ptr(true), rec1);

        const __prod2u_recurse_in__output_node handler(aw, out_uid);
        request_foreach(pq, req.target, handler);
      }

      // Update meta information
      if (out_id > 0) { aw.push(level_info(out_label, out_id)); }

      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq.size());
    }

    // Ensure the edge case, where the in-going edge from nil to the root pair
    // does not dominate the max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                          - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return typename Policy::__dd_type(out_arcs, ep);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Sweep logic with a secondary priority queue

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Execution of a single quantification sweep with two priority queues.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename NodeStream,
            typename PriorityQueue_1,
            typename PriorityQueue_2,
            typename Policy,
            typename In>
  typename Policy::__dd_type
  __prod2u_pq(const exec_policy& ep,
              const In& in,
              Policy& policy,
              PriorityQueue_1& pq_1,
              PriorityQueue_2& pq_2)
  {
    // Set up input

    // TODO (optimisation):
    //   Use '.seek(...)' with 'in_nodes' instead such that it only needs to be
    //   opened once in the caller of this function.
    NodeStream in_nodes(in);
    typename Policy::node_type v = in_nodes.pull();

    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    // Process requests in topological order of both BDDs
    while (!pq_1.empty()) {
      adiar_assert(pq_2.empty(),
                   "Secondary priority queue is only non-empty while processing each level");

      // Set up level
      pq_1.setup_next_level();
      const typename Policy::label_type out_label = pq_1.current_level();
      typename Policy::id_type out_id             = 0;

      const bool split = policy.split(out_label);

      while (!pq_1.empty_level() || !pq_2.empty()) {
        // Merge requests from pq_1 and pq_2
        prod2u_request<1> req;

        if (pq_1.can_pull()
            && (pq_2.empty() || pq_1.top().target.first() < pq_2.top().target.second())) {
          req = { pq_1.top().target,
                  { { { node::pointer_type::nil(), node::pointer_type::nil() } } },
                  pq_1.top().data };
        } else {
          req = pq_2.top();
        }

        // Seek element from request in stream
        const ptr_uint64 t_seek = req.empty_carry() ? req.target.first() : req.target.second();

        while (v.uid() < t_seek) { v = in_nodes.pull(); }

        // Forward information of node t1 across the level if needed
        if (req.empty_carry() && req.target.second().is_node()
            && req.target.first().label() == req.target.second().label()) {
          do {
#ifdef ADIAR_STATS
            stats_prod2u.pq.pq_2_elems += 1u;
#endif
            pq_2.push({ req.target, { v.children() }, pq_1.pull().data });
          } while (pq_1.can_pull() && pq_1.top().target == req.target);
          continue;
        }

        adiar_assert(req.target.first().label() == out_label,
                     "Level of requests always ought to match the one currently processed");

#ifdef ADIAR_STATS
        stats_prod2u.requests_unique[__prod2u_arity_idx(req)] += 1u;
#endif

        // Recreate children of the two targeted nodes (or possibly the
        // suppressed node for target.second()).
        const node::children_type children_fst =
          req.empty_carry() ? v.children() : req.node_carry[0];

        const node::children_type children_snd = req.target.second().level() == out_label
          ? v.children()
          : Policy::reduction_rule_inv(req.target.second());

        adiar_assert(out_id < Policy::max_id, "Has run out of ids");

        // -----------------------------------------------------------------------------------------
        // CASE: Split node into binary recursion request
        if (split) {
          const tuple<typename Policy::pointer_type, 2, true> rec_all =
            __prod2u_resolve_request<Policy>(children_fst[false], children_fst[true]);

          // Collapsed to a terminal?
          if (req.data.source.is_nil() && rec_all[0].is_terminal()) {
            adiar_assert(rec_all[1] == Policy::pointer_type::nil(),
                         "Operator should already be applied");

            return typename Policy::dd_type(rec_all[0].value());
          }

          prod2u_request<0>::target_t rec(rec_all[0], rec_all[1]);

          if (rec[0].is_terminal()) {
            const __prod2u_recurse_in__output_terminal handler(aw, rec[0]);
            request_foreach(pq_1, pq_2, req.target, handler);
          } else {
            const __prod2u_recurse_in__forward handler(pq_1, rec);
            request_foreach(pq_1, pq_2, req.target, handler);
          }

          continue;
        }

        // -----------------------------------------------------------------------------------------
        // CASE: Regular Level
        //   The variable should stay: proceed as in the Product Construction by simulating both
        //   possibilities in parallel.

        const node::uid_type out_uid(out_label, out_id++);

        prod2u_request<0>::target_t rec0 =
          __prod2u_resolve_request<Policy>(children_fst[false], children_snd[false]);

        __prod2u_recurse_out<Policy>(pq_1, aw, out_uid.as_ptr(false), rec0);

        prod2u_request<0>::target_t rec1 =
          __prod2u_resolve_request<Policy>(children_fst[true], children_snd[true]);
        __prod2u_recurse_out<Policy>(pq_1, aw, out_uid.as_ptr(true), rec1);

        const __prod2u_recurse_in__output_node handler(aw, out_uid);
        request_foreach(pq_1, pq_2, req.target, handler);
      }

      // Update meta information
      if (out_id > 0) { aw.push(level_info(out_label, out_id)); }

      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq_1.size());
    }

    // Ensure the edge case, where the in-going edge from nil to the root pair
    // does not dominate the max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                          - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return typename Policy::__dd_type(out_arcs, ep);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Common logic for a full single sweep.

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Set up of priority queue for a single top-down sweep with random access.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename NodeRandomAccess, typename PriorityQueue, typename In, typename Policy>
  typename Policy::__dd_type
  __prod2u_ra(const exec_policy& ep,
              const In& in,
              Policy& policy,
              const size_t pq_memory,
              const size_t max_pq_size)
  {
    NodeRandomAccess in_nodes(in);

    // Set up cross-level priority queue with a request for the root
    PriorityQueue pq({ in }, pq_memory, max_pq_size, stats_prod2u.lpq);
    pq.push({ { in_nodes.root(), ptr_uint64::nil() }, {}, { ptr_uint64::nil() } });

    return __prod2u_ra(ep, in_nodes, policy, pq);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Memory computations to decide types of priority queue for a single sweep with random
  ///        access.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename NodeRandomAccess,
            template <size_t, memory_mode> typename PriorityQueueTemplate,
            typename In,
            typename Policy>
  typename Policy::__dd_type
  __prod2u_ra(const exec_policy& ep, const In& in, Policy& policy)
  {
    // Compute amount of memory available for auxiliary data structures after having opened all
    // streams.
    //
    // We then may derive an upper bound on the size of auxiliary data structures and check whether
    // we can run them with a faster internal memory variant.
    const size_t pq_memory = memory_available()
      // Input stream
      - NodeRandomAccess::memory_usage(in)
      // Output stream
      - arc_ofstream::memory_usage();

    const size_t pq_memory_fits =
      PriorityQueueTemplate<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(pq_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_bound = std::min({ __prod2u_ilevel_upper_bound<Policy, get_2level_cut, 2u>(in),
                                       __prod2u_ilevel_upper_bound(in) });

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if (!external_only && max_pq_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_prod2u.lpq.unbucketed += 1u;
#endif
      using PriorityQueue = PriorityQueueTemplate<0, memory_mode::Internal>;

      return __prod2u_ra<NodeRandomAccess, PriorityQueue>(ep, in, policy, pq_memory, max_pq_size);
    } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_prod2u.lpq.internal += 1u;
#endif
      using PriorityQueue = PriorityQueueTemplate<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>;

      return __prod2u_ra<NodeRandomAccess, PriorityQueue>(ep, in, policy, pq_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_prod2u.lpq.external += 1u;
#endif
      using PriorityQueue = PriorityQueueTemplate<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>;

      return __prod2u_ra<NodeRandomAccess, PriorityQueue>(ep, in, policy, pq_memory, max_pq_size);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Set up of priority queues for a single top-down sweep with two priority queues.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename NodeStream,
            typename PriorityQueue_1,
            typename PriorityQueue_2,
            typename Policy,
            typename In>
  typename Policy::__dd_type
  __prod2u_pq(const exec_policy& ep,
              const In& in,
              Policy& policy,
              const size_t pq_1_memory,
              const size_t max_pq_1_size,
              const size_t pq_2_memory,
              const size_t max_pq_2_size)
  {
    // Check for trivial terminal-only return on shortcutting the root
    typename Policy::pointer_type root;

    { // Detach and garbage collect node_ifstream<>
      NodeStream in_nodes(in);
      root = in_nodes.pull().uid();
    }

    // Set up cross-level priority queue
    PriorityQueue_1 pq_1({ in }, pq_1_memory, max_pq_1_size, stats_prod2u.lpq);
    pq_1.push({ { root, ptr_uint64::nil() }, {}, { ptr_uint64::nil() } });

    // Set up per-level priority queue
    PriorityQueue_2 pq_2(pq_2_memory, max_pq_2_size);

    return __prod2u_pq<NodeStream, PriorityQueue_1, PriorityQueue_2>(ep, in, policy, pq_1, pq_2);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Memory computations to decide types of both priority queues for a single sweep.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename NodeStream,
            template <size_t, memory_mode> typename PriorityQueue_1_Template,
            typename Policy,
            typename In>
  typename Policy::__dd_type
  __prod2u_pq(const exec_policy& ep, const In& in, Policy& policy)
  {
    // Compute amount of memory available for auxiliary data structures after having opened all
    // streams.
    //
    // We then may derive an upper bound on the size of auxiliary data structures and check whether
    // we can run them with a faster internal memory variant.
    const size_t aux_available_memory = memory_available()
      // Input stream
      - NodeStream::memory_usage()
      // Output stream
      - arc_ofstream::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      PriorityQueue_1_Template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pq_2 =
      prod2u_priority_queue_2_t<memory_mode::Internal>::data_structures;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
      * data_structures_in_pq_1;

    const size_t pq_1_memory_fits =
      PriorityQueue_1_Template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(
        pq_1_internal_memory);

    const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

    const size_t pq_2_memory_fits =
      prod2u_priority_queue_2_t<memory_mode::Internal>::memory_fits(pq_2_internal_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_1_bound =
      std::min({ __prod2u_ilevel_upper_bound<Policy, get_2level_cut, 2u>(in),
                 __prod2u_ilevel_upper_bound(in) });

    const size_t max_pq_1_size =
      internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = __prod2u_ilevel_upper_bound<Policy, get_1level_cut, 0u>(in);

    const size_t max_pq_2_size =
      internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    if (!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_prod2u.lpq.unbucketed += 1u;
#endif
      using PriorityQueue_1 = PriorityQueue_1_Template<0, memory_mode::Internal>;
      using PriorityQueue_2 = prod2u_priority_queue_2_t<memory_mode::Internal>;

      return __prod2u_pq<NodeStream, PriorityQueue_1, PriorityQueue_2>(
        ep, in, policy, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else if (!external_only && max_pq_1_size <= pq_1_memory_fits
               && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_prod2u.lpq.internal += 1u;
#endif
      using PriorityQueue_1 = PriorityQueue_1_Template<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>;
      using PriorityQueue_2 = prod2u_priority_queue_2_t<memory_mode::Internal>;

      return __prod2u_pq<NodeStream, PriorityQueue_1, PriorityQueue_2>(
        ep, in, policy, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
    } else {
#ifdef ADIAR_STATS
      stats_prod2u.lpq.external += 1u;
#endif
      using PriorityQueue_1 = PriorityQueue_1_Template<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>;
      using PriorityQueue_2 = prod2u_priority_queue_2_t<memory_mode::External>;

      const size_t pq_1_memory = aux_available_memory / 2;
      const size_t pq_2_memory = pq_1_memory;

      return __prod2u_pq<NodeStream, PriorityQueue_1, PriorityQueue_2>(
        ep, in, policy, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Single-top quantification sweep on node-based inputs.
  ///
  /// \pre `in.is_terminal() == false`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  __prod2u(const exec_policy& ep, const typename Policy::dd_type& in, Policy& policy)
  {
    // ---------------------------------------------------------------------------------------------
    // Case: Terminal
    adiar_assert(!in->is_terminal());

    // ---------------------------------------------------------------------------------------------
    // Case: Do the product construction (with random access)
    //
    // Use random access if requested or the width fits half(ish) of the memory otherwise dedicated
    // to the secondary priority queue.

    constexpr size_t data_structures_in_pq_2 =
      prod2u_priority_queue_2_t<memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pqs = data_structures_in_pq_2
      + prod2u_priority_queue_1_node_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

    const size_t ra_threshold =
      (memory_available() * data_structures_in_pq_2) / 2 * (data_structures_in_pqs);

    if ( // If user has forced Random Access
      ep.template get<exec_policy::access>() == exec_policy::access::Random_Access
      || ( // Heuristically, if it is indexable and it fits
        ep.template get<exec_policy::access>() == exec_policy::access::Auto && in->indexable
        && node_raccess::memory_usage(in->width) <= ra_threshold)) {
#ifdef ADIAR_STATS
      stats_prod2u.ra.runs += 1u;
#endif
      return __prod2u_ra<node_raccess, prod2u_priority_queue_1_node_t>(ep, in, policy);
    }

    // ---------------------------------------------------------------------------------------------
    // Case: Do the product construction (with priority queues)
#ifdef ADIAR_STATS
    stats_prod2u.pq.runs += 1u;
#endif
    return __prod2u_pq<node_ifstream<>, prod2u_priority_queue_1_node_t>(ep, in, policy);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Common logic for using `prod2u` as part of a nested sweep.

  template <typename Policy>
  class prod2u_nested_policy : public Policy
  {
  public:
    using request_t      = prod2u_request<0>;
    using request_pred_t = request_data_first_lt<request_t>;

    template <size_t LookAhead, memory_mode MemoryMode>
    using pq_t = prod2u_priority_queue_1_node_t<LookAhead, MemoryMode>;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static size_t
    stream_memory()
    {
      return node_ifstream<>::memory_usage() + arc_ofstream::memory_usage();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    static size_t
    pq_memory(const size_t inner_memory)
    {
      constexpr size_t data_structures_in_pq_1 =
        prod2u_priority_queue_1_node_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

      constexpr size_t data_structures_in_pq_2 =
        prod2u_priority_queue_2_t<memory_mode::Internal>::data_structures;

      return (inner_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
        * data_structures_in_pq_1;
    }

    static size_t
    ra_memory(const shared_levelized_file<node>& outer_file)
    {
      return node_raccess::memory_usage(outer_file);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    pq_bound(const typename Policy::shared_node_file_type& outer_file,
             const size_t /*outer_roots*/) const
    {
      const typename Policy::dd_type outer_wrapper(outer_file);
      return std::min(__prod2u_ilevel_upper_bound<Policy, get_2level_cut, 2u>(outer_wrapper),
                      __prod2u_ilevel_upper_bound(outer_wrapper));
    }

  public:
    prod2u_nested_policy()
    {}

  public:
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Whether an inner nested sweep needs to split on a given level.
    ///
    /// \details As an invariant, the Inner Sweep only ever touches levels beneath the deepest yet
    ///          to-be split level. Hence, we can provide an always-false predicate that can be
    ///          optimized by the compiler.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    constexpr bool split(typename Policy::label_type /*level*/) const
    {
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Entry Point for Nested Sweeping framework to start an Inner Sweep with Random Access.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename PriorityQueue>
    typename Policy::__dd_type
    sweep_ra(const exec_policy& ep,
             const shared_levelized_file<node>& outer_file,
             PriorityQueue& pq,
             [[maybe_unused]] const size_t inner_remaining_memory) const
    {
      adiar_assert(ep.template get<exec_policy::access>() != exec_policy::access::Priority_Queue);
      adiar_assert(node_raccess::memory_usage(outer_file) <= inner_remaining_memory);

      node_raccess in_nodes(outer_file);
      return __prod2u_ra(ep, in_nodes, *this, pq);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Entry Point for Nested Sweeping framework to start an Inner Sweep with multiple
    ///        priority queues.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename PriorityQueue_1>
    typename Policy::__dd_type
    sweep_pq(const exec_policy& ep,
             const shared_levelized_file<node>& outer_file,
             PriorityQueue_1& pq_1,
             const size_t inner_remaining_memory) const
    {
      adiar_assert(ep.template get<exec_policy::access>() != exec_policy::access::Random_Access);

      const size_t pq_2_memory_fits =
        prod2u_priority_queue_2_t<memory_mode::Internal>::memory_fits(inner_remaining_memory);

      const size_t pq_2_bound =
        // Obtain 1-level cut from subset
        __prod2u_ilevel_upper_bound<Policy, get_1level_cut, 0u>(
          typename Policy::dd_type(outer_file))
        // Add crossing arcs
        + (pq_1.size());

      const size_t max_pq_2_size =
        ep.template get<exec_policy::memory>() == exec_policy::memory::Internal
        ? std::min(pq_2_memory_fits, pq_2_bound)
        : pq_2_bound;

      if (ep.template get<exec_policy::memory>() != exec_policy::memory::External
          && max_pq_2_size <= pq_2_memory_fits) {
        using PriorityQueue_2 = prod2u_priority_queue_2_t<memory_mode::Internal>;
        PriorityQueue_2 pq_2(inner_remaining_memory, max_pq_2_size);

        return __prod2u_pq<node_ifstream<>, PriorityQueue_1, PriorityQueue_2>(
          ep, outer_file, *this, pq_1, pq_2);
      } else {
        using PriorityQueue_2 = prod2u_priority_queue_2_t<memory_mode::External>;
        PriorityQueue_2 pq_2(inner_remaining_memory, max_pq_2_size);

        return __prod2u_pq<node_ifstream<>, PriorityQueue_1, PriorityQueue_2>(
          ep, outer_file, *this, pq_1, pq_2);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Pick the type of a Priority Queue and algorithm.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename OuterRoots>
    typename Policy::__dd_type
    sweep(const exec_policy& ep,
          const shared_levelized_file<node>& outer_file,
          OuterRoots& outer_roots,
          const size_t inner_memory) const
    {
      return nested_sweeping::inner::down__sweep_switch(
        ep, *this, outer_file, outer_roots, inner_memory, stats_prod2u.lpq);
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_INTERNAL_ALGORITHMS_PROD2U_H
