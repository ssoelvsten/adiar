#ifndef ADIAR_INTERNAL_ALGORITHMS_SELECT_H
#define ADIAR_INTERNAL_ALGORITHMS_SELECT_H

#include <variant>

#include <adiar/exec_policy.h>
#include <adiar/types.h>

#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_ofstream.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/node_ifstream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  //  Select Algorithm
  // ==================
  //
  // Given a Decision Diagram, a subset of the levels are removed and bridged over in either the
  // 'high' or the 'low' direction.
  /*
  //          _( )_     ---- xi                _( )_        ---- xi
  //         /     \                          /     \
  //        ( )   ( )   ---- xj     =>        \      \
  //        / \   / \                          \      \
  //        a b   c d                          b      d
  */
  // Examples of uses are `bdd_restrict` and `zdd_onset` / `zdd_offset`
  //
  // As such, this is merely a niche-case of the Intercut algorithm, where the *onset* levels are
  // removed and bridged over in a uniform direction (and the edges crossing said level only have
  // suppressed nodes added). Yet compared to the Intercut algorithm, this implementation is
  // optimised both in terms of time and space.
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::select_t stats_select;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Data Structures
  using select_request = request_data<1, with_parent>;

  template <size_t look_ahead, memory_mode mem_mode>
  using select_priority_queue_t =
    levelized_node_priority_queue<select_request,
                                  request_data_first_lt<select_request>,
                                  look_ahead,
                                  mem_mode,
                                  1u,
                                  0u>;

  /// Return a desired `node` to be output or a `node::pointer_type` to skip to one of its children.
  using select_rec = std::variant<node, node::pointer_type>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template <typename PriorityQueue>
  inline void
  __select_recurse_out(PriorityQueue& pq,
                       arc_ofstream& aw,
                       const ptr_uint64& source,
                       const ptr_uint64& target)
  {
    if (target.is_terminal()) {
      aw.push_terminal({ source, target });
    } else {
      pq.push({ { target }, {}, { source } });
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename PriorityQueue>
  typename Policy::__dd_type
  __select(const exec_policy& ep,
           const typename Policy::dd_type& dd,
           Policy& policy,
           const size_t pq_memory,
           const size_t pq_max_size)
  {
    // Set up outputs
    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    out_arcs->max_1level_cut = 0;
    bool output_changes      = false;

    // Set up inputs
    node_ifstream<> ns(dd);

    // Set up priority queue with initial request to the root
    PriorityQueue pq({ dd }, pq_memory, pq_max_size, stats_select.lpq);
    {
      const node root = ns.peek();
      pq.push({ { root.uid() }, {}, { Policy::pointer_type::nil() } });
    }

    // Process all to-be-visited nodes in topological order
    while (!pq.empty()) {
      // Set up next level
      pq.setup_next_level();

      const typename Policy::level_type level = pq.current_level();
      typename Policy::id_type level_size     = 0;

      policy.setup_level(level);

      // Update max 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, pq.size());

      // Process entire level
      while (!pq.empty_level()) {
        // Seek requested node
        const node n = ns.seek(pq.top().target[0]);

        adiar_assert(pq.top().target == n.uid());
        adiar_assert(n.uid().level() == level);

        const select_rec rec = policy.process(n);

        // Output/Forward resulting node
        if (std::holds_alternative<typename Policy::node_type>(rec)) {
          const node rec_node = std::get<typename Policy::node_type>(rec);

          if constexpr (Policy::skip_reduce) { output_changes |= rec_node != n; }

          // Output/Forward outgoing arcs
          __select_recurse_out(pq, aw, n.uid().as_ptr(false), rec_node.low());
          __select_recurse_out(pq, aw, n.uid().as_ptr(true), rec_node.high());

          // Output ingoing arcs
          while (pq.can_pull() && pq.top().target == rec_node.uid()) {
            const select_request request_pq = pq.pull();

            if (!request_pq.data.source.is_nil()) {
              aw.push_internal({ request_pq.data.source, rec_node.uid() });
            }
          }

          level_size++;
        } else { // std::holds_alternative<node::pointer_type>(rec_res)
          const typename Policy::pointer_type rec_target =
            std::get<typename Policy::pointer_type>(rec);

          if constexpr (Policy::skip_reduce) { output_changes = true; }

          // Output/Forward extension of arc
          while (pq.can_pull() && pq.top().target == n.uid()) {
            const typename Policy::pointer_type source = pq.pull().data.source;

            if (rec_target.is_terminal() && source.is_nil()) {
              // Edge-case: restriction to a terminal
              return policy.terminal(rec_target.value());
            }

            __select_recurse_out(pq, aw, source, rec_target);
          }
        }
      }

      // Push meta data about this level
      if (level_size > 0) { aw.push(level_info(level, level_size)); }
    }

    if constexpr (Policy::skip_reduce) {
      if (!output_changes) { return dd; }
    }

    return typename Policy::__dd_type(out_arcs, ep);
  }

  template <typename Policy>
  size_t
  __select_2level_upper_bound(const typename Policy::dd_type& dd)
  {
    const safe_size_t max_2level_cut = dd.max_2level_cut(cut::Internal);
    return to_size(max_2level_cut + 2u);
  }

  template <typename Policy>
  typename Policy::__dd_type
  select(const exec_policy& ep, const typename Policy::dd_type& dd, Policy& policy)
  {
    // Compute amount of memory available for auxiliary data structures after having opened all
    // streams.
    //
    // We then may derive an upper bound on the size of auxiliary data structures and check whether
    // we can run them with a faster internal memory variant.
    const size_t aux_available_memory =
      memory_available() - node_ifstream<>::memory_usage() - arc_ofstream::memory_usage();

    const size_t pq_memory_fits =
      select_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(
        aux_available_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_bound = __select_2level_upper_bound<Policy>(dd);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if (!external_only && max_pq_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      stats_select.lpq.unbucketed += 1u;
#endif
      return __select<Policy, select_priority_queue_t<0, memory_mode::Internal>>(
        ep, dd, policy, aux_available_memory, max_pq_size);
    } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_select.lpq.internal += 1u;
#endif
      return __select<Policy, select_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>>(
        ep, dd, policy, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_select.lpq.external += 1u;
#endif
      return __select<Policy, select_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>>(
        ep, dd, policy, aux_available_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_SELECT_H
