#ifndef ADIAR_INTERNAL_ALGORITHMS_PROD2B_H
#define ADIAR_INTERNAL_ALGORITHMS_PROD2B_H

#include <variant>

#include <adiar/exec_policy.h>

#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/block_size.h>
#include <adiar/internal/bool_op.h>
#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_ofstream.h>
#include <adiar/internal/io/node_ifstream.h>
#include <adiar/internal/io/node_raccess.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  //  2-ary Product Construction
  // ============================
  //
  // Given two Decision Diagrams construct the product of both.
  /*
  //          (a)          (b)               _(a,b)_
  //         /   \    X   /   \     =>      /       \
  //        a0   a1      b1   b2        (a0,b0)   (a1,b1)
  */
  // Examples of uses are `bdd_apply` and `zdd_binop`.
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::prod2b_t stats_prod2b;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Data structures
  template <uint8_t nodes_carried>
  using prod2b_request = request_data<2, with_parent, nodes_carried>;

  template <size_t look_ahead, memory_mode mem_mode>
  using prod_priority_queue_t = levelized_node_priority_queue<prod2b_request<0>,
                                                              request_data_lt<prod2b_request<0>>,
                                                              look_ahead,
                                                              mem_mode,
                                                              2,
                                                              0>;

  template <size_t look_ahead, memory_mode mem_mode>
  using prod_priority_queue_1_t =
    levelized_node_priority_queue<prod2b_request<0>,
                                  request_data_first_lt<prod2b_request<0>>,
                                  look_ahead,
                                  mem_mode,
                                  2,
                                  0>;

  template <memory_mode mem_mode>
  using prod_priority_queue_2_t =
    priority_queue<mem_mode, prod2b_request<1>, request_data_second_lt<prod2b_request<1>>>;

  // TODO: turn into 'tuple<tuple<node::pointer_type>>'
  struct prod2b_rec_output
  {
    tuple<dd::pointer_type> low;
    tuple<dd::pointer_type> high;
  };

  using prod2b_rec_skipto = tuple<dd::pointer_type>;

  using prod2b_rec = std::variant<prod2b_rec_output, prod2b_rec_skipto>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Helper functions

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forward edge from `source` to `target`.
  ///
  /// \details If `target` is two terminals, the result terminal is computed, and the edge output.
  ///          Otherwise the edge is forwarded to be processed later.
  ///
  /// \pre `source` level is strictly before `target`
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename PriorityQueue>
  inline void
  __prod2b_recurse_out(PriorityQueue& pq,
                       arc_ofstream& aw,
                       const Policy& policy,
                       const ptr_uint64& source,
                       const typename PriorityQueue::value_type::target_t& target)
  {
    if (target[0].is_terminal() && target[1].is_terminal()) {
      const arc out_arc = { source, policy(target[0], target[1]) };
      aw.push_terminal(out_arc);
    } else {
      adiar_assert(source.label() < std::min(target[0], target[1]).label(),
                   "should always push recursion for 'later' level");

      pq.push({ target, {}, { source } });
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `request_foreach` for an arc to an internal node.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  struct __prod2b_recurse_in__output_node
  {
  private:
    arc_ofstream& _aw;
    const typename Policy::node_type::uid_type& _out_uid;

  public:
    __prod2b_recurse_in__output_node(arc_ofstream& aw,
                                     const typename Policy::node_type::uid_type& out_uid)
      : _aw(aw)
      , _out_uid(out_uid)
    {}

    template <typename Request>
    inline void
    operator()(const Request& req) const
    {
      // TODO: Use 'Policy::no_skip'
      if (!req.data.source.is_nil()) {
        this->_aw.push_internal({ req.data.source, this->_out_uid });
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `request_foreach` for an arc to a terminal.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Pointer>
  struct __prod2b_recurse_in__output_terminal
  {
  private:
    arc_ofstream& _aw;
    const Pointer& _out_terminal;

  public:
    __prod2b_recurse_in__output_terminal(arc_ofstream& aw, const Pointer& out_terminal)
      : _aw(aw)
      , _out_terminal(out_terminal)
    {}

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
  struct __prod2b_recurse_in__forward
  {
  private:
    PriorityQueue& _pq;
    const Target& _t;

  public:
    __prod2b_recurse_in__forward(PriorityQueue& pq, const Target& t)
      : _pq(pq)
      , _t(t)
    {}

    template <typename Request>
    inline void
    operator()(const Request& req) const
    {
      this->_pq.push({ this->_t, {}, req.data });
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Construct a terminal for the result of two pointers.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline shared_levelized_file<node>
  __prod2b_terminal(const tuple<typename Policy::pointer_type>& rp, const Policy& policy)
  {
    return build_terminal<Policy>(policy(rp[0], rp[1]).value());
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Common logic for reconstructing nodes forwarded across a level.
  ///
  /// \details The policy for `prod2b` should provide a `merge` function which. Inherit from this
  ///          class, if you can guarantee all requests are to pairs of nodes on the same level.
  ///
  /// \pre All requests are assumed to only be for nodes on the same level
  ///
  /// \see prod2b_mixed_level_merger
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy>
  class prod2b_same_level_merger
  {
  public:
    template <typename Request>
    static tuple<typename DdPolicy::children_type>
    merge(const Request& r, const ptr_uint64& t_seek, const node& v0, const node& v1)
    {
      const typename DdPolicy::children_type pair_0 =
        r.target[0] < t_seek ? r.node_carry[0] : v0.children();

      const typename DdPolicy::children_type pair_1 =
        r.target[1] < t_seek ? r.node_carry[0] : v1.children();

      return { pair_0, pair_1 };
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Logic for reconstructing nodes forwarded across a level.
  ///
  /// \details The policy for `prod2b` should provide a `merge` function which. Inherit from this
  ///          class, if you cannot guarantee all requests are to pairs of nodes on the same level.
  ///
  /// \pre Requests may be for nodes on different levels.
  ///
  /// \see prod2b_same_level_merger
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy>
  class prod2b_mixed_level_merger
  {
  public:
    template <typename Request>
    static tuple<typename DdPolicy::children_type>
    merge(const Request& r,
          const typename DdPolicy::pointer_type& t_seek,
          const typename DdPolicy::node_type& v0,
          const typename DdPolicy::node_type& v1)
    {
      if (r.target[0].is_terminal() || r.target[1].is_terminal()
          || r.target[0].label() != r.target[1].label()) {

        adiar_assert(r.target[0] != r.target[1], "Cannot have mismatching levels and be equal");

        // t.target[0].label() < r.target[1].label() || r.target[1].is_terminal() ?
        const typename DdPolicy::children_type pair_0 =
          r.target[0] < r.target[1] ? v0.children() : DdPolicy::reduction_rule_inv(r.target[0]);

        // r.target[1].label() < r.target[0].label() || r.target[0].is_terminal() ?
        const typename DdPolicy::children_type pair_1 =
          r.target[1] < r.target[0] ? v1.children() : DdPolicy::reduction_rule_inv(r.target[1]);

        return { pair_0, pair_1 };
      } else {
        return prod2b_same_level_merger<DdPolicy>::merge(r, t_seek, v0, v1);
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief 2-ary Product Construction Algorithm where Random Access is used to get nodes from the
  ///        one of the two decision diagrams; this removes the need for the secondary priority
  ///        queue in `__prod2b_pq`.
  ///
  /// \pre `in_ra` is the input to random access
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename PriorityQueue_1>
  typename Policy::__dd_type
  __prod2b_ra(const exec_policy& ep,
              const typename Policy::dd_type& in_pq,
              const typename Policy::dd_type& in_ra,
              Policy& policy,
              const size_t pq_memory,
              const size_t max_pq_size)
  {
    constexpr size_t pq_idx = 0;
    constexpr size_t ra_idx = 1;

    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    // Set up input
    node_ifstream<> in_nodes_pq(in_pq);
    node_raccess in_nodes_ra(in_ra);

    node v_pq = in_nodes_pq.pull();

    // Set up cross-level priority queue
    PriorityQueue_1 prod_pq({ in_pq, in_ra }, pq_memory, max_pq_size, stats_prod2b.lpq);
    prod_pq.push({ { v_pq.uid(), in_nodes_ra.root() }, {}, { ptr_uint64::nil() } });
    // TODO: Allow using 'Policy::no_skip' when pushing; the ptr_uint64::nil() above breaks this!

    out_arcs->max_1level_cut = prod_pq.size();

    // Process all requests
    while (!prod_pq.empty()) {
      // Set up level
      prod_pq.setup_next_level();

      typename Policy::label_type out_label = prod_pq.current_level();
      typename Policy::id_type out_id       = 0;

      policy.setup_next_level(out_label);
      in_nodes_ra.setup_next_level(out_label);

      // Update maximum 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, prod_pq.size());

      // Process all requests for this level
      while (!prod_pq.empty_level()) {
        const prod2b_request<0> req = prod_pq.top();

        // Seek request partially in stream
        if (req.target[pq_idx].is_node() && req.target[pq_idx].label() == out_label) {
          while (v_pq.uid() < req.target[pq_idx] && in_nodes_pq.can_pull()) {
            v_pq = in_nodes_pq.pull();
          }

          adiar_assert(v_pq.uid() == req.target[pq_idx], "Must have found correct node in `in_pq`");
        }

        // Recreate/Obtain children of req.target (possibly of suppressed node)
        const typename Policy::children_type children_pq = req.target[pq_idx].level() == out_label
          ? v_pq.children()
          : Policy::reduction_rule_inv(req.target[pq_idx]);

        const typename Policy::children_type children_ra = req.target[ra_idx].level() == out_label
          ? in_nodes_ra.at(req.target[ra_idx]).children()
          : Policy::reduction_rule_inv(req.target[ra_idx]);

        // Create pairing of product children
        const tuple<typename Policy::pointer_type> rec_pair_0 = { children_pq[false],
                                                                  children_ra[false] };

        const tuple<typename Policy::pointer_type> rec_pair_1 = { children_pq[true],
                                                                  children_ra[true] };

        // Obtain new recursion targets
        const prod2b_rec rec_res = policy.resolve_request(rec_pair_0, rec_pair_1);

        // Forward recursion targets
        if (Policy::no_skip || std::holds_alternative<prod2b_rec_output>(rec_res)) {
          const prod2b_rec_output r = std::get<prod2b_rec_output>(rec_res);

          adiar_assert(out_id < Policy::max_id, "Has run out of ids");
          const node::uid_type out_uid(out_label, out_id++);

          __prod2b_recurse_out(prod_pq, aw, policy, out_uid.as_ptr(false), r.low);
          __prod2b_recurse_out(prod_pq, aw, policy, out_uid.as_ptr(true), r.high);

          const __prod2b_recurse_in__output_node<Policy> handler(aw, out_uid);
          request_foreach(prod_pq, req.target, handler);

        } else { // std::holds_alternative<prod2b_rec_skipto>(root_rec)
          const prod2b_rec_skipto r = std::get<prod2b_rec_skipto>(rec_res);
          if (r[0].is_terminal() && r[1].is_terminal()) {
            if (req.data.source.is_nil()) {
              // Skipped in both DAGs all the way from the root until a pair of terminals.
              return __prod2b_terminal<Policy>(r, policy);
            }

            const typename Policy::pointer_type result = policy(r[0], r[1]);
            const __prod2b_recurse_in__output_terminal handler(aw, result);
            request_foreach(prod_pq, req.target, handler);
          } else {
            const __prod2b_recurse_in__forward handler(prod_pq, r);
            request_foreach(prod_pq, req.target, handler);
          }
        }
      }

      // Update meta information
      if (Policy::no_skip || out_id > 0) { aw.push(level_info(out_label, out_id)); }
    }

    // Ensure the edge case, where the in-going edge from nil to the root pair
    // does not dominate the max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                          - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return typename Policy::__dd_type(out_arcs, ep);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief 2-ary Product Construction where nodes are potentially forwarded with a secondary
  ///        priority queue such that they are accessible at the same time.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename PriorityQueue_1, typename PriorityQueue_2>
  typename Policy::__dd_type
  __prod2b_pq(const exec_policy& ep,
              const typename Policy::dd_type& in_0,
              const typename Policy::dd_type& in_1,
              Policy& policy,
              const size_t pq_1_memory,
              const size_t max_pq_1_size,
              const size_t pq_2_memory,
              const size_t max_pq_2_size)
  {
    // Set up output
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    out_arcs->max_1level_cut = 0;

    // Set up input
    node_ifstream<> in_nodes_0(in_0);
    node_ifstream<> in_nodes_1(in_1);

    node v0 = in_nodes_0.pull();
    node v1 = in_nodes_1.pull();

    // Set up cross-level priority queue
    PriorityQueue_1 prod_pq_1({ in_0, in_1 }, pq_1_memory, max_pq_1_size, stats_prod2b.lpq);
    prod_pq_1.push({ { v0.uid(), v1.uid() }, {}, { ptr_uint64::nil() } });
    // TODO: Allow using 'Policy::no_skip' when pushing; the ptr_uint64::nil() above breaks this!

    // Set up per-level priority queue
    PriorityQueue_2 prod_pq_2(pq_2_memory, max_pq_2_size);

    // Process requests in topological order of both BDDs
    while (!prod_pq_1.empty()) {
      // Set up next level
      prod_pq_1.setup_next_level();

      const typename Policy::label_type out_label = prod_pq_1.current_level();
      typename Policy::id_type out_id             = 0;

      policy.setup_next_level(out_label);

      // Update max 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, prod_pq_1.size());

      // Process all requests for this level
      while (!prod_pq_1.empty_level() || !prod_pq_2.empty()) {
        prod2b_request<1> req;

        // Merge requests from prod_pq_1 or prod_pq_2
        if (prod_pq_1.can_pull()
            && (prod_pq_2.empty()
                || prod_pq_1.top().target.first() < prod_pq_2.top().target.second())) {
          req = { prod_pq_1.top().target,
                  { { { node::pointer_type::nil(), node::pointer_type::nil() } } },
                  { prod_pq_1.top().data } };
        } else {
          req = prod_pq_2.top();
        }

        adiar_assert(req.target[0].is_terminal() || out_label <= req.target[0].label(),
                     "Request should never level-wise be behind current position");
        adiar_assert(req.target[1].is_terminal() || out_label <= req.target[1].label(),
                     "Request should never level-wise be behind current position");

        // Seek request partially in stream
        const typename Policy::pointer_type t_seek =
          req.empty_carry() ? req.target.first() : req.target.second();

        while (v0.uid() < t_seek && in_nodes_0.can_pull()) { v0 = in_nodes_0.pull(); }
        while (v1.uid() < t_seek && in_nodes_1.can_pull()) { v1 = in_nodes_1.pull(); }

        // Forward information across the level
        if (req.empty_carry() && req.target[0].is_node() && req.target[1].is_node()
            && req.target[0].label() == req.target[1].label()
            && (v0.uid() != req.target[0] || v1.uid() != req.target[1])) {
          const typename Policy::children_type children =
            (req.target[0] == v0.uid() ? v0 : v1).children();

          while (prod_pq_1.can_pull() && prod_pq_1.top().target == req.target) {
#ifdef ADIAR_STATS
            stats_prod2b.pq.pq_2_elems += 1u;
#endif
            prod_pq_2.push({ req.target, { children }, prod_pq_1.pull().data });
          }
          continue;
        }

        // Recreate children of nodes for req.target
        const tuple<typename Policy::children_type> children = Policy::merge(req, t_seek, v0, v1);

        // Create pairing of product children
        const tuple<typename Policy::pointer_type> rec_pair_0 = { children[0][false],
                                                                  children[1][false] };

        const tuple<typename Policy::pointer_type> rec_pair_1 = { children[0][true],
                                                                  children[1][true] };

        // Obtain new recursion targets
        const prod2b_rec rec_res = policy.resolve_request(rec_pair_0, rec_pair_1);

        // Forward recursion targets
        if (Policy::no_skip || std::holds_alternative<prod2b_rec_output>(rec_res)) {
          const prod2b_rec_output r = std::get<prod2b_rec_output>(rec_res);

          adiar_assert(out_id < Policy::max_id, "Has run out of ids");
          const node::uid_type out_uid(out_label, out_id++);

          __prod2b_recurse_out(prod_pq_1, aw, policy, out_uid.as_ptr(false), r.low);
          __prod2b_recurse_out(prod_pq_1, aw, policy, out_uid.as_ptr(true), r.high);

          const __prod2b_recurse_in__output_node<Policy> handler(aw, out_uid);
          request_foreach(prod_pq_1, prod_pq_2, req.target, handler);

        } else { // std::holds_alternative<prod2b_rec_skipto>(root_rec)
          const prod2b_rec_skipto r = std::get<prod2b_rec_skipto>(rec_res);
          if (r[0].is_terminal() && r[1].is_terminal()) {
            if (req.data.source.is_nil()) {
              // Skipped in both DAGs all the way from the root until a pair of terminals.
              return __prod2b_terminal<Policy>(r, policy);
            }

            const typename Policy::pointer_type result = policy(r[0], r[1]);
            const __prod2b_recurse_in__output_terminal handler(aw, result);
            request_foreach(prod_pq_1, prod_pq_2, req.target, handler);
          } else {
            const __prod2b_recurse_in__forward handler(prod_pq_1, r);
            request_foreach(prod_pq_1, prod_pq_2, req.target, handler);
          }
        }
      }

      // Push meta data about this level
      if (Policy::no_skip || out_id > 0) { aw.push(level_info(out_label, out_id)); }
    }

    // Ensure the edge case, where the in-going edge from nil to the root pair does not dominate the
    // max_1level_cut
    out_arcs->max_1level_cut = std::min(aw.size() - out_arcs->number_of_terminals[false]
                                          - out_arcs->number_of_terminals[true],
                                        out_arcs->max_1level_cut);

    return typename Policy::__dd_type(out_arcs, ep);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut based on the product of the maximum
  /// i-level cut of both inputs.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename cut, size_t const_size_inc, typename Policy>
  size_t
  __prod2b_ilevel_upper_bound(const typename Policy::dd_type& in_0,
                              const typename Policy::dd_type& in_1,
                              const Policy& policy)
  {
    // Cuts for left-hand side
    const safe_size_t left_cut_internal = cut::get(in_0, cut::type::Internal);

    const typename cut::type left_ct     = policy.left_cut();
    const safe_size_t left_cut_terminals = cut::get(in_0, left_ct) - left_cut_internal;

    // Cuts for right-hand side
    const safe_size_t right_cut_internal = cut::get(in_1, cut::type::Internal);

    const typename cut::type right_ct     = policy.right_cut();
    const safe_size_t right_cut_terminals = cut::get(in_1, right_ct) - right_cut_internal;

    // Compute cut, where we make sure not to pair terminals with terminals.
    return to_size(left_cut_internal * right_cut_internal + left_cut_terminals * right_cut_internal
                   + left_cut_internal * right_cut_terminals + const_size_inc);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum 2-level cut based on both using the max 1 and
  /// 2-level cuts and the number of relevant terminals.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  size_t
  __prod2b_2level_upper_bound(const typename Policy::dd_type& in_0,
                              const typename Policy::dd_type& in_1,
                              const Policy& policy)
  {
    // Left-hand side
    const safe_size_t left_2level_cut = in_0.max_2level_cut(cut::Internal);
    const safe_size_t left_1level_cut = in_0.max_1level_cut(cut::Internal);

    const cut left_ct                    = policy.left_cut();
    const safe_size_t left_terminal_vals = left_ct.number_of_terminals();

    const safe_size_t left_terminal_arcs = in_0.max_1level_cut(left_ct) - left_1level_cut;

    // Right-hand side
    const safe_size_t right_2level_cut = in_1.max_2level_cut(cut::Internal);
    const safe_size_t right_1level_cut = in_1.max_1level_cut(cut::Internal);

    const cut right_ct                    = policy.right_cut();
    const safe_size_t right_terminal_vals = right_ct.number_of_terminals();

    const safe_size_t right_terminal_arcs = in_1.max_1level_cut(right_ct) - right_1level_cut;

    // Compute cut, where we count the product, the input-terminal pairings, and the connection from
    // the product to the input-terminal pairings separately.
    return to_size(left_2level_cut * right_2level_cut + (right_1level_cut * left_terminal_arcs)
                   + left_terminal_vals * right_2level_cut + (left_1level_cut * right_terminal_arcs)
                   + right_terminal_vals * left_2level_cut + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Upper bound on i-level cut based on the maximum possible number of nodes in the output.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  size_t
  __prod2b_ilevel_upper_bound(const typename Policy::dd_type& in_0,
                              const typename Policy::dd_type& in_1,
                              const Policy& policy)
  {
    const cut left_ct                    = policy.left_cut();
    const safe_size_t left_terminal_vals = left_ct.number_of_terminals();
    const safe_size_t left_size          = in_0->size();

    const cut right_ct                    = policy.right_cut();
    const safe_size_t right_terminal_vals = right_ct.number_of_terminals();
    const safe_size_t right_size          = in_1->size();

    return to_size((left_size + left_terminal_vals) * (right_size + right_terminal_vals) + 1u + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  __prod2b_ra(const exec_policy& ep,
              const typename Policy::dd_type& in_0,
              const typename Policy::dd_type& in_1,
              Policy& policy)
  {
    adiar_assert(in_0->indexable || in_1->indexable, "At least one input must be indexable");

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_bound =
      std::min({ __prod2b_ilevel_upper_bound<get_2level_cut, 2u>(in_0, in_1, policy),
                 __prod2b_2level_upper_bound(in_0, in_1, policy),
                 __prod2b_ilevel_upper_bound(in_0, in_1, policy) });

    // Possibly flip inputs (and operator) such that 'in_ra' is always the second argument to
    // `__prod2b_ra(...)` algorithm.
    const bool flip_in = in_0->indexable && (!in_1->indexable || in_0.width() <= in_1.width());

    const typename Policy::dd_type& in_pq = flip_in ? in_1 : in_0;
    const typename Policy::dd_type& in_ra = flip_in ? in_0 : in_1;

    // TODO:
    //   Allow the type of the policy's operator to change, e.g. 'imp' becoming 'invimp'?
    if (flip_in) { policy.flip(); }

#ifdef ADIAR_STATS
    stats_prod2b.ra.used_narrowest +=
      static_cast<size_t>(in_ra.width() == std::min(in_pq.width(), in_ra.width()));

    stats_prod2b.ra.acc_width += in_ra.width();
    stats_prod2b.ra.min_width = std::min(stats_prod2b.ra.min_width, in_ra.width());
    stats_prod2b.ra.max_width = std::max(stats_prod2b.ra.max_width, in_ra.width());
#endif

    const size_t pq_available_memory = memory_available()
      // Input stream
      - node_ifstream<>::memory_usage()
      // Random access
      - node_raccess::memory_usage(in_ra)
      // Output stream
      - arc_ofstream::memory_usage();

    const size_t pq_memory_fits =
      prod_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(
        pq_available_memory);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if (!external_only && max_pq_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_prod2b.lpq.unbucketed += 1u;
#endif
      return __prod2b_ra<Policy, prod_priority_queue_t<0, memory_mode::Internal>>(
        ep, in_pq, in_ra, policy, pq_available_memory, max_pq_size);
    } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_prod2b.lpq.internal += 1u;
#endif
      return __prod2b_ra<Policy, prod_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>>(
        ep, in_pq, in_ra, policy, pq_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_prod2b.lpq.external += 1u;
#endif
      return __prod2b_ra<Policy, prod_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>>(
        ep, in_pq, in_ra, policy, pq_available_memory, max_pq_size);
    }
  }

  template <typename Policy>
  typename Policy::__dd_type
  __prod2b_pq(const exec_policy& ep,
              const typename Policy::dd_type& in_0,
              const typename Policy::dd_type& in_1,
              Policy& policy)
  {
    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_1_bound =
      std::min({ __prod2b_ilevel_upper_bound<get_2level_cut, 2u>(in_0, in_1, policy),
                 __prod2b_2level_upper_bound(in_0, in_1, policy),
                 __prod2b_ilevel_upper_bound(in_0, in_1, policy) });

    // Compute amount of memory available for auxiliary data structures after having opened all
    // streams.
    //
    // We then may derive an upper bound on the size of auxiliary data structures and check whether
    // we can run them with a faster internal memory variant.
    const size_t aux_available_memory = memory_available()
      // Input streams
      - 2 * node_ifstream<>::memory_usage()
      // Output stream
      - arc_ofstream::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pq_2 =
      prod_priority_queue_2_t<memory_mode::Internal>::data_structures;

    const size_t pq_1_internal_memory =
      (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
      * data_structures_in_pq_1;

    const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

    const size_t pq_1_memory_fits =
      prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(
        pq_1_internal_memory);

    const size_t pq_2_memory_fits =
      prod_priority_queue_2_t<memory_mode::Internal>::memory_fits(pq_2_internal_memory);

    const size_t max_pq_1_size =
      internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = __prod2b_ilevel_upper_bound<get_1level_cut, 0u>(in_0, in_1, policy);

    const size_t max_pq_2_size =
      internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    if (!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
      stats_prod2b.lpq.unbucketed += 1u;
#endif
      using pq_1_type = prod_priority_queue_1_t<0, memory_mode::Internal>;
      using pq_2_type = prod_priority_queue_2_t<memory_mode::Internal>;

      return __prod2b_pq<Policy, pq_1_type, pq_2_type>(ep,
                                                       in_0,
                                                       in_1,
                                                       policy,
                                                       pq_1_internal_memory,
                                                       max_pq_1_size,
                                                       pq_2_internal_memory,
                                                       max_pq_2_size);
    } else if (!external_only && max_pq_1_size <= pq_1_memory_fits
               && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
      stats_prod2b.lpq.internal += 1u;
#endif
      using pq_1_type = prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>;
      using pq_2_type = prod_priority_queue_2_t<memory_mode::Internal>;

      return __prod2b_pq<Policy, pq_1_type, pq_2_type>(ep,
                                                       in_0,
                                                       in_1,
                                                       policy,
                                                       pq_1_internal_memory,
                                                       max_pq_1_size,
                                                       pq_2_internal_memory,
                                                       max_pq_2_size);
    } else {
#ifdef ADIAR_STATS
      stats_prod2b.lpq.external += 1u;
#endif
      using pq_1_type = prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>;
      const size_t pq_1_memory = aux_available_memory / 2;

      using pq_2_type          = prod_priority_queue_2_t<memory_mode::External>;
      const size_t pq_2_memory = pq_1_memory;

      return __prod2b_pq<Policy, pq_1_type, pq_2_type>(
        ep, in_0, in_1, policy, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  2-ary Product Construction algorithm
  ///
  /// \return A class that inherits from `__dd` and describes the product the two given DAGs.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  prod2b(const exec_policy& ep,
         const typename Policy::dd_type& in_0,
         const typename Policy::dd_type& in_1,
         Policy& policy)
  {
    // ---------------------------------------------------------------------------------------------
    // Case: Same file with a matching shift, i.e. exactly the same DAG.
    if (in_0.file_ptr() == in_1.file_ptr() && in_0.shift() == in_1.shift()) {
#ifdef ADIAR_STATS
      stats_prod2b.trivial_file += 1u;
#endif
      return policy.resolve_same_file(in_0, in_1);
    }

    // ---------------------------------------------------------------------------------------------
    // Case: At least one terminal.
    if (dd_isterminal(in_0) || dd_isterminal(in_1)) {
      typename Policy::__dd_type maybe_resolved = policy.resolve_terminal_root(in_0, in_1);

      if (!(maybe_resolved.template has<no_file>())) {
#ifdef ADIAR_STATS
        stats_prod2b.trivial_terminal += 1u;
#endif
        return maybe_resolved;
      }
    }

    // ---------------------------------------------------------------------------------------------
    // Case: Do the product construction (with random access)
    //
    // Use random access if requested or the width fits into half(ish) of the memory otherwise
    // dedicated to the secondary priority queue.
    //
    // TODO: Optimisation!
    //
    //   We do not need to check on the decision diagram being 'canonical'. It merely suffices, that
    //   it is 'indexable', i.e. the one half of being 'canonical'.
    constexpr size_t data_structures_in_pq_2 =
      prod_priority_queue_2_t<memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pqs = data_structures_in_pq_2
      + prod_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

    const size_t ra_threshold =
      (memory_available() * data_structures_in_pq_2) / 2 * (data_structures_in_pqs);

    const size_t width_0   = in_0->indexable ? in_0.width() : Policy::max_id;
    const size_t width_1   = in_1->indexable ? in_1.width() : Policy::max_id;
    const size_t min_width = std::min(width_0, width_1);

    if ( // Use `__prod2b_ra` if user has forced Random Access
      ep.template get<exec_policy::access>() == exec_policy::access::Random_Access
      || ( // Heuristically, if the narrowest canonical fits
        ep.template get<exec_policy::access>() == exec_policy::access::Auto
        && (in_0->indexable || in_1->indexable)
        && (node_raccess::memory_usage(min_width) <= ra_threshold))) {
#ifdef ADIAR_STATS
      stats_prod2b.ra.runs += 1u;
#endif
      return __prod2b_ra<Policy>(ep, in_0, in_1, policy);
    }

    // ---------------------------------------------------------------------------------------------
    // Case: Do the product construction (with priority queues)
#ifdef ADIAR_STATS
    stats_prod2b.pq.runs += 1u;
#endif
    return __prod2b_pq<Policy>(ep, in_0, in_1, policy);
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_PROD2B_H
