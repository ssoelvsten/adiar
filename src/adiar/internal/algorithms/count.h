#ifndef ADIAR_INTERNAL_ALGORITHMS_COUNT_H
#define ADIAR_INTERNAL_ALGORITHMS_COUNT_H

#include <stdint.h>

#include <adiar/exec_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/node_ifstream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  //  Count Algorithm
  // =================
  //
  // Traverses a Decision Diagram and accumulates a numeric count along each of
  // its paths.
  //
  // Examples of uses are `bdd_pathcount` and `bdd_satcount`.
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::count_t stats_count;

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  template <typename Data>
  using count_request = request_data<1, Data>;

  template <typename Data, size_t look_ahead, memory_mode mem_mode>
  using count_priority_queue_t =
    levelized_node_priority_queue<count_request<Data>,
                                  request_data_first_lt<count_request<Data>>,
                                  look_ahead,
                                  mem_mode,
                                  1u,
                                  0u>;

  //////////////////////////////////////////////////////////////////////////////
  // PathCount Policy

  //////////////////////////////////////////////////////////////////////////////
  /// Auxiliary data for the Priority Queue in PathCount algorithm.
  //////////////////////////////////////////////////////////////////////////////
  struct path_data
  {
    static constexpr bool sort_on_tiebreak = false;
    static constexpr bool has_level        = false;

    /// Sum of paths from parent
    uint64_t sum;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Policy with logic for specializing Count algorithm into PathCount.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy>
  class path_count_policy : public DdPolicy
  {
  public:
    using data_type = path_data;

    static constexpr data_type init_data = { 1u };

    static constexpr uint64_t
    resolve_false(const data_type& /*d*/, const typename DdPolicy::label_type /*varcount*/)
    {
      return 0u;
    }

    static inline uint64_t
    resolve_true(const data_type& d, const typename DdPolicy::label_type /*varcount*/)
    {
      return d.sum;
    }

    static inline data_type
    merge(const data_type&& acc, const data_type& next)
    {
      adiar_assert(acc.sum > 0u && next.sum > 0u, "No request should have an 'empty' set of paths");

      return { acc.sum + next.sum };
    }

    static inline data_type
    merge_end(const data_type&& acc)
    {
      return acc;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename PriorityQueue>
  inline uint64_t
  __count_resolve(PriorityQueue& count_pq,
                  const typename Policy::pointer_type& target,
                  const typename Policy::data_type& data,
                  const typename Policy::label_type varcount)
  {
    if (target.is_false()) { return Policy::resolve_false(data, varcount); }
    if (target.is_true()) { return Policy::resolve_true(data, varcount); }
    count_pq.push({ { target }, {}, data });
    return 0u;
  }

  template <typename Policy, typename PriorityQueue>
  uint64_t
  __count(const typename Policy::dd_type& dd,
          const typename Policy::label_type varcount,
          const size_t pq_max_memory,
          const size_t pq_max_size)
  {
    adiar_assert(!dd->is_terminal(), "Count Algorithm does not support terminal case");

    // Set up output
    uint64_t result = 0u;

    // Set up input
    node_ifstream<> ns(dd);

    // Set up cross-level priority queue with a request for the root
    PriorityQueue count_pq({ dd }, pq_max_memory, pq_max_size, stats_count.lpq);
    {
      const node root = ns.peek();

      count_pq.push({ { root.uid() }, {}, Policy::init_data });
    }

    // Take out the rest of the nodes and process them one by one
    while (!count_pq.empty()) {
      count_pq.setup_next_level();

      while (!count_pq.empty_level()) {
        // Assuming there are no dead nodes, we should visit every node of dd
        const node n = ns.pull();

        adiar_assert(count_pq.top().target == n.uid(), "Decision Diagram includes dead nodes");

        // Merge requests for 'n'
        const typename PriorityQueue::value_type request = count_pq.pull();
        typename Policy::data_type data                  = request.data;

        while (count_pq.can_pull() && count_pq.top().target == n.uid()) {
          data = Policy::merge(std::move(data), count_pq.pull().data);
        }

        data = Policy::merge_end(std::move(data));

        // Forward requests for children of 'n'
        result += __count_resolve<Policy>(count_pq, n.low(), data, varcount);
        result += __count_resolve<Policy>(count_pq, n.high(), data, varcount);
      }
    }

    return result;
  }

  template <typename Policy>
  uint64_t
  count(const exec_policy& ep,
        const typename Policy::dd_type& dd,
        const typename Policy::label_type varcount)
  {
    adiar_assert(!dd_isterminal(dd), "Count algorithm does not work on terminal-only edge case");

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.

    const size_t aux_available_memory = memory_available() - node_ifstream<>::memory_usage();

    const size_t pq_memory_fits =
      count_priority_queue_t<typename Policy::data_type,
                             ADIAR_LPQ_LOOKAHEAD,
                             memory_mode::Internal>::memory_fits(aux_available_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_bound = dd.max_2level_cut(cut::Internal);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if (!external_only && max_pq_size <= no_lookahead_bound()) {
#ifdef ADIAR_STATS
      stats_count.lpq.unbucketed += 1u;
#endif
      return __count<Policy,
                     count_priority_queue_t<typename Policy::data_type, 0, memory_mode::Internal>>(
        dd, varcount, aux_available_memory, max_pq_size);
    } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_count.lpq.internal += 1u;
#endif
      return __count<Policy,
                     count_priority_queue_t<typename Policy::data_type,
                                            ADIAR_LPQ_LOOKAHEAD,
                                            memory_mode::Internal>>(
        dd, varcount, aux_available_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_count.lpq.external += 1u;
#endif
      return __count<Policy,
                     count_priority_queue_t<typename Policy::data_type,
                                            ADIAR_LPQ_LOOKAHEAD,
                                            memory_mode::External>>(
        dd, varcount, aux_available_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_COUNT_H
