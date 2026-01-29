#ifndef ADIAR_INTERNAL_ALGORITHMS_REDUCE_H
#define ADIAR_INTERNAL_ALGORITHMS_REDUCE_H

#include <adiar/exec_policy.h>
#include <adiar/statistics.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_structures/sorter.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/convert.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_ifstream.h>
#include <adiar/internal/io/iofstream.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_ofstream.h>
#include <adiar/internal/memory.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::reduce_t stats_reduce;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Data structures
  struct mapping
  {
    node::uid_type old_uid;
    node::pointer_type new_uid;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Priority queue elements
  //
  // TODO (Attributed Edges):
  //   First add an 'uint8_t' value with reduce specific flags, e.g. the red1_taint used to place
  //   something in the local og global 1-level cut.
  struct reduce_arc : public arc
  {
    reduce_arc() = default;

    reduce_arc(const reduce_arc&) = default;

    reduce_arc(const arc& a)
      : arc(a)
    {}

    reduce_arc&
    operator=(const reduce_arc& a) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The level at which this nodes source belongs to.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    arc::label_type
    level() const
    {
      return source().label();
    }
  };

  struct reduce_queue_lt
  {
    bool
    operator()(const arc& a, const arc& b)
    {
      // We want the high arc first, but that is already placed on the
      // least-significant bit on the source variable.
      return a.source() > b.source();
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Decorator on the levelized priority queue to also keep track of the number of arcs to
  ///        each terminal.
  ////////////////////////////////////////////////////////////////////////////////////////////////
  template <size_t look_ahead, memory_mode mem_mode>
  class reduce_priority_queue
    : public levelized_arc_priority_queue<reduce_arc, reduce_queue_lt, look_ahead, mem_mode>
  {
  private:
    using inner_lpq =
      levelized_arc_priority_queue<reduce_arc, reduce_queue_lt, look_ahead, mem_mode>;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of terminals (of each type) placed within the priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t _terminals[2] = { 0u, 0u };

  public:
    reduce_priority_queue(std::array<typename inner_lpq::level_input_type, 1>&& files,
                          size_t memory_given,
                          size_t max_size,
                          statistics::levelized_priority_queue_t& stats)
      : inner_lpq(std::move(files), memory_given, max_size, stats)
    {}

    reduce_priority_queue(std::array<typename inner_lpq::level_input_type, 1>&& files,
                          size_t memory_given,
                          size_t max_size)
      : reduce_priority_queue(std::move(files), memory_given, max_size, stats_reduce.lpq)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Push an arc into the priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push(const arc& a)
    {
      _terminals[false] += a.target().is_false();
      _terminals[true] += a.target().is_true();

      inner_lpq::push(a);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top arc on the current level and remove it.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    arc
    pull()
    {
      arc a = inner_lpq::pull();

      _terminals[false] -= a.target().is_false();
      _terminals[true] -= a.target().is_true();

      return a;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Remove the top arc on the current level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    pop()
    {
      pull();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of terminals (of each type) placed within the priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t&
    terminals(const bool terminal_value) const
    {
      return _terminals[terminal_value];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of arcs (across all levels) ignoring terminals.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    size_without_terminals() const
    {
      return inner_lpq::size() - _terminals[false] - _terminals[true];
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Reduction Rule 2 sorting (and back again)
  struct reduce_node_children_lt
  {
    bool
    operator()(const node& a, const node& b)
    {
      // TODO (Attributed Edges):
      //     Use the 'flag' bit on children to mark attributed edges. Currently,
      //     we use this flag to mark whether Reduction Rule 1 was applied to
      //     some node across some arc.
      const ptr_uint64 a_high = unflag(a.high());
      const ptr_uint64 a_low  = unflag(a.low());

      const ptr_uint64 b_high = unflag(b.high());
      const ptr_uint64 b_low  = unflag(b.low());

      return a_high > b_high || (a_high == b_high && a_low > b_low)
#ifndef NDEBUG
        || (a_high == b_high && a_low == b_low && a.uid() > b.uid())
#endif
        ;
    }
  };

  struct reduce_uid_lt
  {
    bool
    operator()(const mapping& a, const mapping& b)
    {
      return a.old_uid > b.old_uid;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Helper functions

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create the output file with correct initial meta data already set.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename dd_policy>
  shared_levelized_file<typename dd_policy::node_type>
  __reduce_init_output()
  {
    shared_levelized_file<typename dd_policy::node_type> out_file;
    out_file->sorted    = true;
    out_file->indexable = true;

    out_file->max_1level_cut[cut::Internal]       = 0u;
    out_file->max_1level_cut[cut::Internal_False] = 0u;
    out_file->max_1level_cut[cut::Internal_True]  = 0u;
    out_file->max_1level_cut[cut::All]            = 0u;

    return out_file;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Merging priority queue with terminal_arc stream.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename pq_t, typename arc_ifstream_t>
  inline arc
  __reduce_get_next(pq_t& reduce_pq, arc_ifstream_t& arcs)
  {
    if (!reduce_pq.can_pull()
        || (arcs.can_pull_terminal() && arcs.peek_terminal().source() > reduce_pq.top().source())) {
      return arcs.pull_terminal();
    } else {
      return reduce_pq.pull();
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Update a cut size with some number of arcs.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline void
  __reduce_cut_add(cuts_t& cut,
                   const cut::size_type internal_arcs,
                   const cut::size_type false_arcs,
                   const cut::size_type true_arcs)
  {
    cut[cut::Internal] += internal_arcs;
    cut[cut::Internal_False] += internal_arcs + false_arcs;
    cut[cut::Internal_True] += internal_arcs + true_arcs;
    cut[cut::All] += internal_arcs + false_arcs + true_arcs;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline void
  __reduce_cut_add(cuts_t& cut, const ptr_uint64 target)
  {
    cut[cut::Internal] += target.is_node();
    cut[cut::Internal_False] += target.is_node() + target.is_false();
    cut[cut::Internal_True] += target.is_node() + target.is_true();
    cut[cut::All] += 1u;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Algorithm functions

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Epilogue of `__reduce_level` setting the priority queue up for the
  ///        next level (or outputting a terminal).
  ///
  /// \see __reduce_level
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename arc_ifstream_t, typename pq_t>
  inline void
  __reduce_level__epilogue(arc_ifstream_t& arcs,
                           pq_t& reduce_pq,
                           node_ofstream& out,
                           const bool terminal_val);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a single level (while also mapping it to a new label).
  ///
  /// \returns width of output level
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy,
            template <typename, typename> typename sorter_t,
            typename pq_t,
            typename arc_ifstream_t>
  size_t
  __reduce_level(arc_ifstream_t& arcs,
                 const typename Policy::label_type in_label,
                 const typename Policy::label_type out_label,
                 pq_t& reduce_pq,
                 node_ofstream& out,
                 const size_t sorters_memory,
                 const size_t unreduced_width,
                 [[maybe_unused]] statistics::reduce_t& stats = stats_reduce)
  {
    // Temporary file for Reduction Rule 1 mappings (opened later if need be)
    iofstream<mapping> red1_mapping;

    // Sorters to find Reduction Rule 2 mappings
    sorter_t<node, reduce_node_children_lt> child_grouping(sorters_memory, unreduced_width, 2);
    sorter_t<mapping, reduce_uid_lt> red2_mapping(sorters_memory, unreduced_width, 2);

    // Pull out all nodes from reduce_pq and terminal_arcs for this level
    while ((arcs.can_pull_terminal() && arcs.peek_terminal().source().label() == in_label)
           || reduce_pq.can_pull()) {
      // TODO (MDD):
      // TODO (QMDD):
      //   Use __reduce_get_next node_type::outdegree times to create a
      //   node_type::children_type.
      const arc e_high = __reduce_get_next(reduce_pq, arcs);
      const arc e_low  = __reduce_get_next(reduce_pq, arcs);

      const node n = node_of(e_low, e_high);
      adiar_assert(n.label() == in_label, "Label is for desired level");

      // Apply Reduction rule 1
      const typename Policy::pointer_type reduction_rule_ret = Policy::reduction_rule(n);
      if (reduction_rule_ret != n.uid()) {
        // Open red1_mapping first (and create file on disk) when at least one
        // element is written to it.
        if (!red1_mapping.is_open()) { red1_mapping.open(); }
#ifdef ADIAR_STATS
        stats.removed_by_rule_1 += 1u;
#endif
        red1_mapping.write({ n.uid(), reduction_rule_ret });
      } else {
        child_grouping.push(n);
      }
    }

    // Count number of arcs that cross this level
    cuts_t local_1level_cut   = { { 0u, 0u, 0u, 0u } };
    cuts_t tainted_1level_cut = { { 0u, 0u, 0u, 0u } };

    __reduce_cut_add(local_1level_cut,
                     reduce_pq.size_without_terminals(),
                     reduce_pq.terminals(false) + arcs.unread_terminals(false),
                     reduce_pq.terminals(true) + arcs.unread_terminals(true));

    // Sort and apply Reduction rule 2
    child_grouping.sort();

    typename Policy::id_type out_id = Policy::max_id;
    node out_node                   = node(node::uid_type(), ptr_uint64::nil(), ptr_uint64::nil());

    while (child_grouping.can_pull()) {
      const node next_node = child_grouping.pull();

      if (out_node.low() != unflag(next_node.low())
          || out_node.high() != unflag(next_node.high())) {
        adiar_assert(0 <= out_id, "Should still have more ids left");
        out_node = node(out_label, out_id--, unflag(next_node.low()), unflag(next_node.high()));
        out.unsafe_push(out_node);

        __reduce_cut_add(next_node.low().is_flagged() ? tainted_1level_cut : local_1level_cut,
                         out_node.low());
        __reduce_cut_add(next_node.high().is_flagged() ? tainted_1level_cut : local_1level_cut,
                         out_node.high());
      } else {
#ifdef ADIAR_STATS
        stats.removed_by_rule_2 += 1u;
#endif
      }

      red2_mapping.push({ next_node.uid(), out_node.uid() });
    }

    // Add number of nodes to level information, if any nodes were pushed to the output.
    const size_t reduced_width = Policy::max_id - out_id;
    if (reduced_width > 0) { out.unsafe_push(level_info(out_label, reduced_width)); }

    // Sort mappings for Reduction rule 2 back in order of arcs.internal
    red2_mapping.sort();

    // Merging of red1_mapping and red2_mapping
    mapping next_red1  = { node::uid_type(), node::uid_type() }; // <-- dummy value
    bool has_next_red1 = red1_mapping.is_open() && red1_mapping.size() > 0;
    if (has_next_red1) {
      red1_mapping.seek_begin();
      next_red1 = red1_mapping.next();
    }

    mapping next_red2  = { node::uid_type(), node::uid_type() }; // <-- dummy value
    bool has_next_red2 = red2_mapping.can_pull();
    if (has_next_red2) { next_red2 = red2_mapping.pull(); }

    // Pass all the mappings to Q
    while (has_next_red1 || has_next_red2) {
      // Find the mapping with largest old_uid
      const bool is_red1_current =
        !has_next_red2 || (has_next_red1 && next_red1.old_uid > next_red2.old_uid);

      const mapping current_map = is_red1_current ? next_red1 : next_red2;

      adiar_assert(!arcs.can_pull_internal()
                     || current_map.old_uid == arcs.peek_internal().target(),
                   "Mapping forwarded in sync with internal arcs");

      // Find all arcs that have the target that match the current mapping's old_uid
      while (arcs.can_pull_internal() && current_map.old_uid == arcs.peek_internal().target()) {
        // The out_idx is included in arc.source() pulled from the internal arcs.
        const ptr_uint64 s = arcs.pull_internal().source();

        // If Reduction Rule 1 was used, then tell the parents to add to the global cut.
        const ptr_uint64 t = is_red1_current ? flag(current_map.new_uid)
                                             : static_cast<ptr_uint64>(current_map.new_uid);

        adiar_assert(t.is_terminal() || t.out_idx() == false, "Created target is without an index");
        reduce_pq.push(arc(s, t));
      }

      // Update the mapping that was used
      if (is_red1_current) {
        has_next_red1 = red1_mapping.has_next();
        if (has_next_red1) { next_red1 = red1_mapping.next(); }
      } else {
        has_next_red2 = red2_mapping.can_pull();
        if (has_next_red2) { next_red2 = red2_mapping.pull(); }
      }
    }

    // Move on to the next level
    red1_mapping.close();

    // Update with new possible maximum 1-level cut (the one below the current level)
    out.unsafe_max_1level_cut(local_1level_cut);

    // Add the tainted edges
    out.unsafe_inc_1level_cut(tainted_1level_cut);

    const bool terminal_value = next_red1.new_uid.is_terminal() && next_red1.new_uid.value();
    __reduce_level__epilogue<>(arcs, reduce_pq, out, terminal_value);

    adiar_assert(reduced_width <= unreduced_width, "Reduction should only ever remove nodes");

    return reduced_width;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a single level (without mapping it to a new label).
  ///
  /// \returns width of output level
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy,
            template <typename, typename> typename sorter_t,
            typename pq_t,
            typename arc_ifstream_t>
  //[[deprecated("Use '__reduce_label' with a separate 'in_label' and 'out_label'")]]
  size_t
  __reduce_level(arc_ifstream_t& arcs,
                 const typename Policy::label_type label,
                 pq_t& reduce_pq,
                 node_ofstream& out,
                 const size_t sorters_memory,
                 const size_t unreduced_width,
                 statistics::reduce_t& stats = stats_reduce)
  {
    return __reduce_level<Policy, sorter_t, pq_t, arc_ifstream_t>(
      arcs, label, label, reduce_pq, out, sorters_memory, unreduced_width, stats);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename arc_ifstream_t, typename pq_t>
  inline void
  __reduce_level__epilogue(arc_ifstream_t& arcs,
                           pq_t& reduce_pq,
                           node_ofstream& out,
                           const bool terminal_val)
  {

    adiar_assert(reduce_pq.empty_level(),
                 "All forwarded arcs to the current level should be processed");

    if (!reduce_pq.empty()) {
      adiar_assert(!arcs.can_pull_terminal() || !reduce_pq.has_current_level()
                     || arcs.peek_terminal().source().label() < reduce_pq.current_level(),
                   "All terminal arcs for 'current_level' should be processed");

      adiar_assert(!arcs.can_pull_internal() || !reduce_pq.has_current_level()
                     || arcs.peek_internal().target().label() < reduce_pq.current_level(),
                   "All internal arcs for 'current_level' should be processed");

      if (arcs.can_pull_terminal()) {
        reduce_pq.setup_next_level(arcs.peek_terminal().source().label());
      } else {
        reduce_pq.setup_next_level();
      }
    } else if (reduce_pq.size() > 0) {
      // NOTE: Being 'empty()' does not imply 'size() == 0', due to the decorators for Nested
      //       Sweeping may claim the priority queue is empty() but the size still includes arcs
      //       stored in another priority queue.

      adiar_assert(!arcs.can_pull_terminal() || !reduce_pq.has_current_level()
                     || arcs.peek_terminal().source().label() < reduce_pq.current_level(),
                   "All terminal arcs for 'current_level' should be processed");

      adiar_assert(!arcs.can_pull_internal() || !reduce_pq.has_current_level()
                     || arcs.peek_internal().target().label() < reduce_pq.current_level(),
                   "All internal arcs for 'current_level' should be processed");

    } else if (!out.has_pushed()) {
      adiar_assert(!arcs.can_pull_internal() && !arcs.can_pull_terminal(),
                   "All nodes should be processed at this point");

      adiar_assert(
        reduce_pq.size() == 0 && reduce_pq.empty(),
        "'reduce_pq.size() == 0' -> 'reduce_pq.empty()', i.e. no parents have been forwarded to'");

      // adiar_assert(next_red1.new_uid.is_terminal(),
      //             "A node must have been suppressed in favour of a terminal");

      out.unsafe_push(node(terminal_val));
      out.unsafe_set_number_of_terminals(!terminal_val, terminal_val);

      // NOTE: We do not need to update the cuts, since this is taken care of in
      //       `~out() = out.close()`.
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce an entire decision diagram bottom-up.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename pq_t>
  shared_levelized_file<typename Policy::node_type>
  __reduce(Policy& policy,
           const shared_levelized_file<arc>& in_file,
           const size_t lpq_memory,
           const size_t sorters_memory)
  {
#ifdef ADIAR_STATS
    stats_reduce.sum_node_arcs += in_file->size(0);
    stats_reduce.sum_terminal_arcs += in_file->size(1) + in_file->size(2);
#endif

    arc_ifstream<> arcs(in_file);
    level_info_ifstream<> levels(in_file);

    // Set up output
    shared_levelized_file<typename Policy::node_type> out_file = __reduce_init_output<Policy>();

    node_ofstream out(out_file);

    // Trivial single-node case
    if (!arcs.can_pull_internal()) {
      // TODO: move into its own helper function
      const arc e_high = arcs.pull_terminal();
      const arc e_low  = arcs.pull_terminal();

      // Apply reduction rule 1, if applicable
      const ptr_uint64 reduction_rule_ret = Policy::reduction_rule(node_of(e_low, e_high));
      if (reduction_rule_ret != e_low.source()) {
#ifdef ADIAR_STATS
        stats_reduce.removed_by_rule_1 += 1u;
#endif
        const bool terminal_val = reduction_rule_ret.value();
        const node out_node     = node(terminal_val);
        out.unsafe_push(out_node);

        out.unsafe_set_number_of_terminals(!terminal_val, terminal_val);
        __reduce_cut_add(out_file->max_1level_cut, 0u, !terminal_val, terminal_val);
      } else {
        const typename Policy::label_type out_level = policy.map_level(e_low.source().level());

        out.unsafe_push(node(out_level, Policy::max_id, e_low.target(), e_high.target()));

        out.unsafe_push(level_info(out_level, 1u));

        out_file->max_1level_cut[cut::Internal] = 1u;

        out_file->max_1level_cut[cut::Internal_False] =
          std::max(!e_low.target().value() + !e_high.target().value(), 1);

        out_file->max_1level_cut[cut::Internal_True] =
          std::max(e_low.target().value() + e_high.target().value(), 1);

        out_file->max_1level_cut[cut::All] = 2u;
      }

      // Copy over 1-level cut to 2-level cut.
      for (size_t ct = 0u; ct < cut::size; ct++) {
        out_file->max_2level_cut[ct] = out_file->max_1level_cut[ct];
      }

      return out_file;
    }

    // Initialize (levelized) priority queue and run Reduce algorithm
    pq_t reduce_pq({ in_file }, lpq_memory, in_file->max_1level_cut);

    const size_t internal_sorter_can_fit = internal_sorter<node>::memory_fits(sorters_memory / 2);

    // Process bottom-up each level
    while (levels.can_pull()) {
      adiar_assert(arcs.can_pull_terminal() || !reduce_pq.empty(),
                   "If there is a level, then there should also be something for it.");
      const level_info current_level_info         = levels.pull();
      const typename Policy::label_type in_level  = current_level_info.level();
      const typename Policy::label_type out_level = policy.map_level(in_level);

      adiar_assert(!reduce_pq.has_current_level() || in_level == reduce_pq.current_level(),
                   "level and priority queue should be in sync");

      const size_t unreduced_width = current_level_info.width();
      if (unreduced_width <= internal_sorter_can_fit) {
        __reduce_level<Policy, internal_sorter>(
          arcs, in_level, out_level, reduce_pq, out, sorters_memory, unreduced_width);
      } else {
        __reduce_level<Policy, external_sorter>(
          arcs, in_level, out_level, reduce_pq, out, sorters_memory, unreduced_width);
      }
    }

    return out_file;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given edge-based decision diagram.
  ///
  /// \param Policy Which includes the types and the reduction rule
  /// \param input  The (possibly unreduced) decision diagram.
  ///
  /// \return The reduced decision diagram in a node-based representation
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::dd_type
  reduce(const exec_policy& ep, Policy& policy, const typename Policy::__dd_type& input)
  {
    adiar_assert(!input.empty(), "Input for Reduce should always be non-empty");

    // Is it already reduced?
    if (input.template has<typename Policy::shared_node_file_type>()) {
      return typename Policy::dd_type(
        input.template get<typename Policy::shared_node_file_type>(), input._negate, input._shift);
    }

    // Get unreduced input
    const typename Policy::shared_arc_file_type in_file =
      input.template get<typename Policy::shared_arc_file_type>();

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory_available()
      // Input streams
      - arc_ifstream<>::memory_usage()
      - level_info_ifstream<>::memory_usage()
      // Output streams
      - node_ofstream::memory_usage();

    const size_t pq_memory = aux_available_memory / 2;
    const size_t sorters_memory =
      aux_available_memory - pq_memory - iofstream<mapping>::memory_usage();

    const size_t pq_memory_fits =
      reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(pq_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_bound = in_file->max_1level_cut;

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if (!external_only && max_pq_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      stats_reduce.lpq.unbucketed += 1u;
#endif
      return __reduce<Policy, reduce_priority_queue<0, memory_mode::Internal>>(
        policy, in_file, pq_memory, sorters_memory);
    } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_reduce.lpq.internal += 1u;
#endif
      return __reduce<Policy, reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>>(
        policy, in_file, pq_memory, sorters_memory);
    } else {
#ifdef ADIAR_STATS
      stats_reduce.lpq.external += 1u;
#endif
      return __reduce<Policy, reduce_priority_queue<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>>(
        policy, in_file, pq_memory, sorters_memory);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////w
  /// \brief Default policy for Reduce without any variable remapping.
  //////////////////////////////////////////////////////////////////////////////////////////////////w
  template <typename DdPolicy>
  class default_reduce_policy : public DdPolicy
  {
  public:
    constexpr inline typename DdPolicy::label_type
    map_level(typename DdPolicy::label_type x) const
    {
      return x;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Reduce a given edge-based decision diagram.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy>
  typename DdPolicy::dd_type
  reduce(const typename DdPolicy::__dd_type& input)
  {
    const exec_policy ep = input._policy;
    default_reduce_policy<DdPolicy> policy;
    return reduce(ep, policy, input);
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_REDUCE_H
