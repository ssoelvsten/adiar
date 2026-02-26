#include <adiar/bdd.h>
#include <adiar/exec_policy.h>
#include <adiar/statistics.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_ofstream.h>
#include <adiar/internal/io/levelized_ifstream.h>
#include <adiar/internal/io/levelized_ofstream.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_ifstream.h>
#include <adiar/internal/io/node_ofstream.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/util.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Struct to hold statistics
  statistics::prod3_t stats_prod3;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Data structures
  //
  // To make the merge simpler we will make sure that 'data_1_x' and 'data_2_x' are ordered based on
  // the order of the BDDs are given, i.e. the elements of the if-BDD would only every be forwarded
  // in 'data_1_x'. The then-elements are forwarded only in 'data_2_x' if the if-BDD is also
  // forwarded. Finally, the placement of the else-BDD on whether any element from the if-BDD or
  // then-BDD has been forwarded.

  template <uint8_t nodes_carried>
  using ite_request = internal::request_data<3, internal::with_parent, nodes_carried>;

  template <size_t look_ahead, internal::memory_mode mem_mode>
  using ite_priority_queue_1_t =
    internal::levelized_node_priority_queue<ite_request<0>,
                                            internal::request_data_first_lt<ite_request<0>>,
                                            look_ahead,
                                            mem_mode,
                                            3>;

  template <internal::memory_mode mem_mode>
  using ite_priority_queue_2_t = internal::
    priority_queue<mem_mode, ite_request<1>, internal::request_data_second_lt<ite_request<1>>>;

  template <internal::memory_mode mem_mode>
  using ite_priority_queue_3_t = internal::
    priority_queue<mem_mode, ite_request<2>, internal::request_data_third_lt<ite_request<2>>>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Helper functions
  internal::shared_levelized_file<bdd::node_type>
  __ite_zip_bdds(const bdd& bdd_if, const bdd& bdd_then, const bdd& bdd_else)
  {
    // TODO: What is the performance of '<<' rather than 'unsafe_push'? If there is a major
    // difference, then we may want to "inline" the '<<' with its _canonical check here.
    internal::node::pointer_type root_then = internal::node::pointer_type::nil();
    internal::node::pointer_type root_else = internal::node::pointer_type::nil();

    internal::shared_levelized_file<bdd::node_type> out_nodes;
    internal::node_ofstream nw(out_nodes);

    // zip 'then' and 'else' cases
    internal::node_ifstream<true> in_nodes_then(bdd_then);
    internal::node_ifstream<true> in_nodes_else(bdd_else);

    while (in_nodes_then.can_pull() || in_nodes_else.can_pull()) {
      bool from_then = in_nodes_then.can_pull()
        && (!in_nodes_else.can_pull() || in_nodes_then.peek() > in_nodes_else.peek());

      internal::node n = from_then ? in_nodes_then.pull() : in_nodes_else.pull();

      if (from_then && !in_nodes_then.can_pull()) { root_then = n.uid(); }
      if (!from_then && !in_nodes_else.can_pull()) { root_else = n.uid(); }

      nw << n;
    }

    // push all nodes from 'if' conditional and remap its terminals
    adiar_assert(!root_then.is_nil(), "Did not obtain root from then stream");
    adiar_assert(!root_else.is_nil(), "Did not obtain root from else stream");

    internal::node_ifstream<true> in_nodes_if(bdd_if);

    while (in_nodes_if.can_pull()) {
      const internal::node n = in_nodes_if.pull();

      const internal::node::pointer_type low =
        n.low().is_terminal() ? (n.low().value() ? root_then : root_else) : n.low();

      const internal::node::pointer_type high =
        n.high().is_terminal() ? (n.high().value() ? root_then : root_else) : n.high();

      nw << internal::node(n.uid(), low, high);
    }

    for (size_t ct = 0u; ct < internal::cut::size; ct++) {
      out_nodes->max_1level_cut[ct] =
        std::max(bdd_if->max_1level_cut[internal::cut::All],
                 bdd_then->max_1level_cut[ct] + bdd_else->max_1level_cut[ct]);
      out_nodes->max_2level_cut[ct] =
        std::max(bdd_if->max_2level_cut[internal::cut::All],
                 bdd_then->max_2level_cut[ct] + bdd_else->max_2level_cut[ct]);
    }

    return out_nodes;
  }

  inline bool
  ite_must_forward(internal::node v,
                   internal::node::pointer_type t,
                   internal::node::label_type out_label,
                   internal::node::pointer_type t_seek)
  {
    return
      // is it a node at this level?
      t.is_node()
      && t.label() == out_label
      // and we should be seeing it later
      && t_seek < t
      // and we haven't by accident just run into it anyway
      && v.uid() != t;
  }

  inline void
  ite_init_request(internal::node_ifstream<>& in_nodes,
                   internal::node& v,
                   const internal::node::label_type out_label,
                   internal::node::pointer_type& low,
                   internal::node::pointer_type& high)
  {
    if (v.label() == out_label) {
      low  = v.low();
      high = v.high();

      if (in_nodes.can_pull()) { v = in_nodes.pull(); }
    } else {
      low = high = v.uid();
    }
  }

  template <typename PriorityQueue_1>
  inline void
  __ite_resolve_request(PriorityQueue_1& ite_pq_1,
                        internal::arc_ofstream& aw,
                        const internal::node::pointer_type source,
                        internal::node::pointer_type r_if,
                        internal::node::pointer_type r_then,
                        internal::node::pointer_type r_else)
  {
    // Early shortcut an ite, if the terminals of both cases have collapsed to the
    // same anyway
    if (r_then.is_terminal() && r_else.is_terminal() && r_then.value() == r_else.value()) {

      aw.push_terminal(internal::arc{ source, r_then });
      return;
    }

    // Remove irrelevant parts of a request to prune requests similar to
    // shortcutting the operator in bdd_apply.
    r_then = r_if.is_false() ? internal::node::pointer_type::nil() : r_then;
    r_else = r_if.is_true() ? internal::node::pointer_type::nil() : r_else;

    if (r_if.is_terminal() && r_then.is_terminal()) {
      // => ~internal::node::pointer_type::nil() => r_if is a terminal with the 'true' value
      aw.push_terminal(internal::arc(source, r_then));
    } else if (r_if.is_terminal() && r_else.is_terminal()) {
      // => ~internal::node::pointer_type::nil() => r_if is a terminal with the 'false' value
      aw.push_terminal(internal::arc(source, r_else));
    } else {
      ite_pq_1.push({ { r_if, r_then, r_else }, {}, { source } });
    }
  }

  template <typename PriorityQueue_1, typename PriorityQueue_2, typename PriorityQueue_3>
  __bdd
  __bdd_ite(const exec_policy& ep,
            const bdd& bdd_if,
            const bdd& bdd_then,
            const bdd& bdd_else,
            const size_t pq_1_memory,
            const size_t max_pq_1_size,
            const size_t pq_2_memory,
            const size_t max_pq_2_size,
            const size_t pq_3_memory,
            const size_t max_pq_3_size)
  {
    internal::node_ifstream<> in_nodes_if(bdd_if);
    bdd::node_type v_if = in_nodes_if.pull();

    internal::node_ifstream<> in_nodes_then(bdd_then);
    bdd::node_type v_then = in_nodes_then.pull();

    internal::node_ifstream<> in_nodes_else(bdd_else);
    bdd::node_type v_else = in_nodes_else.pull();

    // Set up output
    internal::shared_levelized_file<internal::arc> out_arcs;
    internal::arc_ofstream aw(out_arcs);

    out_arcs->max_1level_cut = 0;

    // Set up cross-level priority queue
    PriorityQueue_1 pq_1(
      { bdd_if, bdd_then, bdd_else }, pq_1_memory, max_pq_1_size, stats_prod3.lpq);
    // TODO: Add support for nil parents and replace loop unrolling below with:
    //
    /* pq_1.push({ { v_if.uid(), v_then.uid(), v_else.uid() }, {}, { ptr_uint64::nil() } }); */

    // Set up per-level priority queues
    PriorityQueue_2 pq_2(pq_2_memory, max_pq_2_size);
    PriorityQueue_3 pq_3(pq_3_memory, max_pq_3_size);

    // Process root and create initial recursion requests
    {
      const bdd::label_type out_label = first(v_if.uid(), v_then.uid(), v_else.uid()).label();

      bdd::pointer_type low_if, low_then, low_else, high_if, high_then, high_else;
      ite_init_request(in_nodes_if, v_if, out_label, low_if, high_if);
      ite_init_request(in_nodes_then, v_then, out_label, low_then, high_then);
      ite_init_request(in_nodes_else, v_else, out_label, low_else, high_else);

      const bdd::node_type::uid_type out_uid(out_label, 0);
      __ite_resolve_request(pq_1, aw, out_uid.as_ptr(false), low_if, low_then, low_else);
      __ite_resolve_request(pq_1, aw, out_uid.as_ptr(true), high_if, high_then, high_else);

      aw.push(internal::level_info(out_label, 1));
    }

    // Process all nodes in topological order of both BDDs
    while (!pq_1.empty()) {
      // Set up next level
      pq_1.setup_next_level();

      const bdd::label_type out_label = pq_1.current_level();
      bdd::id_type out_id             = 0;

      // Update max 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq_1.size());

      // Process all requests for this level
      while (!pq_1.empty_level() || !pq_2.empty() || !pq_3.empty()) {
        ite_request<2> req;
        bool with_data_1 = false, with_data_2 = false;

        // Merge requests from priority queues
        if (pq_1.can_pull()
            && (pq_2.empty() || pq_1.top().target.first() < pq_2.top().target.second())
            && (pq_3.empty() || pq_1.top().target.first() < pq_3.top().target.third())) {
          req = { pq_1.top().target,
                  { { { bdd::pointer_type::nil(), bdd::pointer_type::nil() },
                      { bdd::pointer_type::nil(), bdd::pointer_type::nil() } } },
                  pq_1.top().data };

          pq_1.pop();
        } else if (!pq_2.empty()
                   && (pq_3.empty() || pq_2.top().target.second() < pq_3.top().target.third())) {
          with_data_1 = true;

          req = { pq_2.top().target,
                  { pq_2.top().node_carry[0],
                    { bdd::pointer_type::nil(), bdd::pointer_type::nil() } },
                  pq_2.top().data };

          pq_2.pop();
        } else {
          with_data_1 = true;
          with_data_2 = true;

          req = pq_3.top();
          pq_3.pop();
        }

        // Seek request partially in stream
        bdd::pointer_type t_first  = req.target.first();
        bdd::pointer_type t_second = req.target.second();
        bdd::pointer_type t_third  = req.target.third();

        bdd::pointer_type t_seek = with_data_2 ? t_third : with_data_1 ? t_second : t_first;

        while (v_if.uid() < t_seek && in_nodes_if.can_pull()) { v_if = in_nodes_if.pull(); }
        while (v_then.uid() < t_seek && in_nodes_then.can_pull()) { v_then = in_nodes_then.pull(); }
        while (v_else.uid() < t_seek && in_nodes_else.can_pull()) { v_else = in_nodes_else.pull(); }

        // Forward information across the level
        if (ite_must_forward(v_if, req.target[0], out_label, t_seek)
            || ite_must_forward(v_then, req.target[1], out_label, t_seek)
            || ite_must_forward(v_else, req.target[2], out_label, t_seek)) {
          // An element should be forwarded, if it was not already forwarded (t_seek <= t_x), if it
          // isn't the last one to seek (t_x < t_third), and if we actually are holding it.
          const bool forward_if =
            (t_seek <= req.target[0]) && (req.target[0] < t_third) && (v_if.uid() == req.target[0]);

          const bool forward_then = (t_seek == req.target[1]) && (req.target[1] < t_third)
            && (v_then.uid() == req.target[1]);

          const bool forward_else = (t_seek == req.target[2]) && (req.target[2] < t_third)
            && (v_else.uid() == req.target[2]);

          const int number_of_elements_to_forward =
            ((int)forward_if) + ((int)forward_then) + ((int)forward_else);

          if (with_data_1 || number_of_elements_to_forward == 2) {
            adiar_assert(!with_data_1 || t_seek != t_first,
                         "cannot have data and still seek the first element");
            adiar_assert(!(with_data_1 && (number_of_elements_to_forward == 2)),
                         "cannot have forwarded an element, hold two unforwarded items, and still "
                         "need to forward for something");

            bdd::node_type::children_type children_1;
            bdd::node_type::children_type children_2;
            if (with_data_1) {
              if (req.target[0] < t_seek || forward_else) {
                children_1 = req.node_carry[0];

                bdd::node_type v2 = forward_else ? v_else : v_then;
                children_2        = v2.children();
              } else { // if (forward_if || req.target[2] < t_seek)
                bdd::node_type v1 = forward_if ? v_if : v_then;
                children_1        = v1.children();

                children_2 = req.node_carry[0];
              }
            } else {
              bdd::node_type v1 = forward_if ? v_if : v_then;
              bdd::node_type v2 = forward_else ? v_else : v_then;

              children_1 = v1.children();
              children_2 = v2.children();
            }

            pq_3.push({ req.target, { children_1, children_2 }, req.data });

            while (pq_1.can_pull() && pq_1.top().target == req.target) {
              pq_3.push({ req.target, { children_1, children_2 }, pq_1.pull().data });
            }
          } else {
            // got no data and the stream only gave us a single item to forward.
            const bdd::node_type v1 = forward_if ? v_if : forward_then ? v_then : v_else;

            const bdd::node_type::children_type v1_children = v1.children();
            pq_2.push({ req.target, { v1_children }, req.data });

            while (pq_1.can_pull() && pq_1.top().target == req.target) {
              pq_2.push({ req.target, { v1_children }, pq_1.pull().data });
            }
          }
          continue;
        }

        // Recreate nodes from priority queue carries
        bdd::pointer_type low_if, low_then, low_else, high_if, high_then, high_else;

        if (req.target[0].is_terminal() || out_label < req.target[0].label()) {
          low_if = high_if = req.target[0];
        } else {
          low_if  = req.target[0] == v_if.uid() ? v_if.low() : req.node_carry[0][false];
          high_if = req.target[0] == v_if.uid() ? v_if.high() : req.node_carry[0][true];
        }

        if (req.target[1].is_nil() || req.target[1].is_terminal()
            || out_label < req.target[1].label()) {
          low_then = high_then = req.target[1];
        } else if (req.target[1] == v_then.uid()) {
          low_then  = v_then.low();
          high_then = v_then.high();
        } else if (t_seek <= req.target[0]) {
          low_then  = req.node_carry[0][false];
          high_then = req.node_carry[0][true];
        } else {
          low_then  = req.node_carry[1][false];
          high_then = req.node_carry[1][true];
        }

        if (req.target[2].is_nil() || req.target[2].is_terminal()
            || out_label < req.target[2].label()) {
          low_else = high_else = req.target[2];
        } else if (req.target[2] == v_else.uid()) {
          low_else  = v_else.low();
          high_else = v_else.high();
        } else if (t_seek <= req.target[0] && t_seek <= req.target[1]) {
          low_else  = req.node_carry[0][false];
          high_else = req.node_carry[0][true];
        } else {
          low_else  = req.node_carry[1][false];
          high_else = req.node_carry[1][true];
        }

        // Resolve request
        adiar_assert(out_id < bdd::max_id, "Has run out of ids");
        const bdd::node_type::uid_type out_uid(out_label, out_id++);

        __ite_resolve_request(pq_1, aw, out_uid.as_ptr(false), low_if, low_then, low_else);
        __ite_resolve_request(pq_1, aw, out_uid.as_ptr(true), high_if, high_then, high_else);

        // Output ingoing arcs
        bdd::pointer_type source = req.data.source;

        while (true) {
          internal::arc out_arc = { source, out_uid };
          aw.push_internal(out_arc);

          if (pq_1.can_pull() && pq_1.top().target == req.target) {
            source = pq_1.pull().data.source;
          } else if (!pq_2.empty() && pq_2.top().target == req.target) {
            source = pq_2.top().data.source;
            pq_2.pop();
          } else if (!pq_3.empty() && pq_3.top().target == req.target) {
            source = pq_3.top().data.source;
            pq_3.pop();
          } else {
            break;
          }
        }
      }

      // Push meta data about this level
      aw.push(internal::level_info(out_label, out_id));
    }

    return __bdd(out_arcs, ep);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut based on the product of the maximum
  /// i-level cut of all three inputs.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename cut, size_t const_size_inc>
  size_t
  __ite_ilevel_upper_bound(const internal::dd& in_if,
                           const internal::dd& in_then,
                           const internal::dd& in_else)
  {
    // 2-level cuts for 'if', where we split the false and true arcs away.
    const internal::safe_size_t if_cut_internal = cut::get(in_if, internal::cut::Internal);
    const internal::safe_size_t if_cut_falses =
      cut::get(in_if, internal::cut::Internal_False) - if_cut_internal;
    const internal::safe_size_t if_cut_trues =
      cut::get(in_if, internal::cut::Internal_True) - if_cut_internal;

    // 2-level cuts for 'then'
    const internal::safe_size_t then_cut_internal = cut::get(in_then, internal::cut::Internal);
    const internal::safe_size_t then_cut_falses =
      cut::get(in_then, internal::cut::Internal_False) - then_cut_internal;
    const internal::safe_size_t then_cut_trues =
      cut::get(in_then, internal::cut::Internal_True) - then_cut_internal;
    const internal::safe_size_t then_cut_all = cut::get(in_then, internal::cut::All);

    // 2-level cuts for 'else'
    const internal::safe_size_t else_cut_internal = cut::get(in_else, internal::cut::Internal);
    const internal::safe_size_t else_cut_falses =
      cut::get(in_else, internal::cut::Internal_False) - else_cut_internal;
    const internal::safe_size_t else_cut_trues =
      cut::get(in_else, internal::cut::Internal_True) - else_cut_internal;
    const internal::safe_size_t else_cut_all = cut::get(in_else, internal::cut::All);

    // Compute 2-level cut where irrelevant pairs of terminals are not paired
    return internal::to_size(
      (if_cut_internal
       * (then_cut_all * else_cut_internal + then_cut_internal * else_cut_all
          + then_cut_falses * else_cut_trues + then_cut_trues * else_cut_falses))
      + if_cut_trues * then_cut_internal + if_cut_falses * else_cut_internal + const_size_inc);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Derives an upper bound on the output's maximum i-level cut given its size.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  size_t
  __ite_ilevel_upper_bound(const internal::dd& in_if,
                           const internal::dd& in_then,
                           const internal::dd& in_else)
  {
    const internal::safe_size_t if_size   = in_if->size();
    const internal::safe_size_t then_size = in_then->size();
    const internal::safe_size_t else_size = in_else->size();

    // Compute the number of triples (t_if, t_then, t_else) where t_if is an internal node and
    // t_then and t_else are nodes or (mismatching) terminals. Then also count the copies of in_then
    // and in_else for when in_if hits a terminal early.
    return internal::to_size(if_size * ((then_size + 2u) * (else_size + 2u) - 2u) + then_size
                             + else_size + 1u + 2u);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_ite(const exec_policy& ep, const bdd& f, const bdd& g, const bdd& h)
  {
    // ---------------------------------------------------------------------------------------------
    // There are multiple cases, where this boils down to an Apply rather than an If-Then-Else. The
    // bdd_apply uses tuples rather than triples and only two priority queues, so it will run
    // considerably faster.
    //
    // The translations into Apply can be found in Figure 1 of "Efficient Implementation of a BDD
    // Package" of Karl S. Brace, Richard L. Rudell, and Randal E. Bryant.

    // Resolve being given the same underlying file in both cases
    if (g.file_ptr() == h.file_ptr() && g.shift() == h.shift()) {
      return g.is_negated() == h.is_negated() ? __bdd(g) : bdd_xnor(f, g);
    }

    // Resolve being given the same underlying file for conditional and a case
    if (f.file_ptr() == g.file_ptr() && f.shift() == g.shift()) {
      return f.is_negated() == g.is_negated() ? bdd_or(f, h) : bdd_less(f, h);
    }
    if (f.file_ptr() == h.file_ptr() && f.shift() == h.shift()) {
      return f.is_negated() == h.is_negated() ? bdd_and(f, g) : bdd_imp(f, g);
    }

    // Resolve being given a terminal conditional
    if (bdd_isterminal(f)) { return dd_valueof(f) ? g : h; }

    // Resolve being given a terminal in one of the cases
    if (bdd_isterminal(g)) { return dd_valueof(g) ? bdd_or(f, h) : bdd_less(f, h); }
    if (bdd_isterminal(h)) { return dd_valueof(h) ? bdd_imp(f, g) : bdd_and(f, g); }

    // ---------------------------------------------------------------------------------------------
    // If the levels of 'then' and 'else' are disjoint and the 'if' BDD is above the two others,
    // then we can merely zip the 'then' and 'else' BDDs. This is only O((N1+N2+N3)/B) I/Os!
    if (bdd_maxvar(f) < bdd_topvar(g) && bdd_maxvar(f) < bdd_topvar(h)
        && internal::disjoint_levels(g, h)) {
      return __ite_zip_bdds(f, g, h);
    }

    // ---------------------------------------------------------------------------------------------
    // From here on forward, we have to do the 3-ary product construction...

    // Compute amount of memory available for auxiliary data structures after having opened all
    // streams.
    //
    // We then may derive an upper bound on the size of auxiliary data structures and check whether
    // we can run them with a faster internal memory variant.
    const size_t aux_available_memory = internal::memory_available()
      // Input streams
      - 3 * internal::node_ifstream<>::memory_usage()
      // Output stream
      - internal::arc_ofstream::memory_usage();

    constexpr size_t data_structures_in_pq_1 =
      ite_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, internal::memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pq_2 =
      ite_priority_queue_2_t<internal::memory_mode::Internal>::data_structures;

    constexpr size_t data_structures_in_pq_3 =
      ite_priority_queue_3_t<internal::memory_mode::Internal>::data_structures;

    const size_t pq_1_internal_memory =
      (aux_available_memory
       / (data_structures_in_pq_1 + data_structures_in_pq_2 + data_structures_in_pq_3))
      * data_structures_in_pq_1;

    const size_t pq_2_internal_memory = ((aux_available_memory - pq_1_internal_memory)
                                         / (data_structures_in_pq_2 + data_structures_in_pq_3))
      * data_structures_in_pq_2;

    const size_t pq_3_internal_memory =
      aux_available_memory - pq_1_internal_memory - pq_2_internal_memory;

    const size_t pq_1_memory_fits =
      ite_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, internal::memory_mode::Internal>::memory_fits(
        pq_1_internal_memory);

    const size_t pq_2_memory_fits =
      ite_priority_queue_2_t<internal::memory_mode::Internal>::memory_fits(pq_2_internal_memory);

    const size_t pq_3_memory_fits =
      ite_priority_queue_3_t<internal::memory_mode::Internal>::memory_fits(pq_3_internal_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_1_bound =
      std::min({ __ite_ilevel_upper_bound<internal::get_2level_cut, 2u>(f, g, h),
                 __ite_ilevel_upper_bound(f, g, h) });

    const size_t max_pq_1_size =
      internal_only ? std::min(pq_1_memory_fits, pq_1_bound) : pq_1_bound;

    const size_t pq_2_bound = __ite_ilevel_upper_bound<internal::get_1level_cut, 0u>(f, g, h);

    const size_t max_pq_2_size =
      internal_only ? std::min(pq_2_memory_fits, pq_2_bound) : pq_2_bound;

    const size_t pq_3_bound = pq_2_bound;

    const size_t max_pq_3_size =
      internal_only ? std::min(pq_3_memory_fits, pq_3_bound) : pq_3_bound;

    if (!external_only && max_pq_1_size <= internal::no_lookahead_bound(3)) {
#ifdef ADIAR_STATS
      stats_prod3.lpq.unbucketed += 1u;
#endif
      using PriorityQueue_1 = ite_priority_queue_1_t<0, internal::memory_mode::Internal>;
      using PriorityQueue_2 = ite_priority_queue_2_t<internal::memory_mode::Internal>;
      using PriorityQueue_3 = ite_priority_queue_3_t<internal::memory_mode::Internal>;

      return __bdd_ite<PriorityQueue_1, PriorityQueue_2, PriorityQueue_3>(ep,
                                                                          f,
                                                                          g,
                                                                          h,
                                                                          pq_1_internal_memory,
                                                                          max_pq_1_size,
                                                                          pq_2_internal_memory,
                                                                          max_pq_2_size,
                                                                          pq_3_internal_memory,
                                                                          max_pq_3_size);

    } else if (!external_only && max_pq_1_size <= pq_1_memory_fits
               && max_pq_2_size <= pq_2_memory_fits && max_pq_3_size <= pq_3_memory_fits) {
#ifdef ADIAR_STATS
      stats_prod3.lpq.internal += 1u;
#endif
      using PriorityQueue_1 =
        ite_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, internal::memory_mode::Internal>;
      using PriorityQueue_2 = ite_priority_queue_2_t<internal::memory_mode::Internal>;
      using PriorityQueue_3 = ite_priority_queue_3_t<internal::memory_mode::Internal>;

      return __bdd_ite<PriorityQueue_1, PriorityQueue_2, PriorityQueue_3>(ep,
                                                                          f,
                                                                          g,
                                                                          h,
                                                                          pq_1_internal_memory,
                                                                          max_pq_1_size,
                                                                          pq_2_internal_memory,
                                                                          max_pq_2_size,
                                                                          pq_3_internal_memory,
                                                                          max_pq_3_size);

    } else {
#ifdef ADIAR_STATS
      stats_prod3.lpq.external += 1u;
#endif
      using PriorityQueue_1 =
        ite_priority_queue_1_t<ADIAR_LPQ_LOOKAHEAD, internal::memory_mode::External>;
      using PriorityQueue_2 = ite_priority_queue_2_t<internal::memory_mode::External>;
      using PriorityQueue_3 = ite_priority_queue_3_t<internal::memory_mode::External>;

      const size_t pq_1_memory = aux_available_memory / 3;
      const size_t pq_2_memory = pq_1_memory;
      const size_t pq_3_memory = pq_1_memory;

      return __bdd_ite<PriorityQueue_1, PriorityQueue_2, PriorityQueue_3>(ep,
                                                                          f,
                                                                          g,
                                                                          h,
                                                                          pq_1_memory,
                                                                          max_pq_1_size,
                                                                          pq_2_memory,
                                                                          max_pq_2_size,
                                                                          pq_3_memory,
                                                                          max_pq_3_size);
    }
  }

  __bdd
  bdd_ite(const bdd& f, const bdd& g, const bdd& h)
  {
    return bdd_ite(exec_policy(), f, g, h);
  }
}
