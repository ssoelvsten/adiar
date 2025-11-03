#include "pred.h"

#include <adiar/internal/algorithms/prod2b.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/levelized_ifstream.h>

namespace adiar::internal
{
  statistics::equality_t stats_equality;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Slow O(sort(N)) I/Os comparison by traversing the product construction and comparing each
  // related pair of nodes.

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Check whether more requests were processed on this level than allowed. An isomorphic DAG would
  // not create more requests than the original number of nodes.
  template <bool tv>
  class input_bound_levels
  {
  private:
    level_info_ifstream<> in_meta_1;

    size_t curr_level_size      = 0;
    size_t curr_level_processed = 0;

  public:
    static size_t
    pq1_upper_bound(const dd& a, const dd& b)
    {
      return std::max(a->max_2level_cut[cut::Internal], b->max_2level_cut[cut::Internal]);
    }

    static size_t
    pq2_upper_bound(const dd& a, const dd& b)
    {
      return std::max(a->max_1level_cut[cut::Internal], b->max_1level_cut[cut::Internal]);
    }

    static constexpr size_t
    memory_usage()
    {
      return level_info_ifstream<>::memory_usage();
    }

  public:
    input_bound_levels(const dd& a, const dd& /*b*/)
      : in_meta_1(a)
    {}

    void
    next_level(ptr_uint64::label_type /* level */)
    { // Ignore input, since only used with the isomorphism_policy below.
      curr_level_size      = in_meta_1.pull().width();
      curr_level_processed = 0;
    }

    bool
    on_step()
    {
      curr_level_processed++;
      const bool ret_value = curr_level_size < curr_level_processed;
#ifdef ADIAR_STATS
      if (ret_value) { stats_equality.slow_check.exit_on_processed_on_level += 1u; }
#endif
      return ret_value;
    }

    static constexpr bool termination_value = tv;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for isomorphism checking with `comparison_check`.
  ///
  /// \pre To use this operation, the following should be satisfied.
  ///  - The number of nodes are the same
  ///  - The number of levels are the same
  ///  - The label and size of each level are the same
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO (Decision Diagrams with other kinds of pointers):
  // template<class dd_policy>
  class isomorphism_policy
    : public dd_policy<dd, __dd>
    , public prod2b_same_level_merger<dd_policy<dd, __dd>>
  {
  public:
    using level_check_t = input_bound_levels<false>;

  public:
    static constexpr size_t
    lookahead_bound()
    {
      return 2u;
    }

  public:
    static bool
    resolve_terminals(const dd::node_type& v1, const dd::node_type& v2, bool& ret_value)
    {
      ret_value = v1.is_terminal() && v2.is_terminal() && v1.value() == v2.value();
#ifdef ADIAR_STATS
      stats_equality.slow_check.exit_on_root += 1u;
#endif
      return true;
    }

  public:
    static bool
    resolve_singletons(const dd::node_type& v1, const dd::node_type& v2)
    {
#ifdef ADIAR_STATS
      stats_equality.slow_check.exit_on_root += 1u;
#endif
      adiar_assert(v1.label() == v2.label(), "Levels match per the precondition");
      return v1.low() == v2.low() && v1.high() == v2.high();
    }

  public:
    template <typename pq_1_t>
    static bool
    resolve_request(pq_1_t& pq, const tuple<dd::pointer_type>& rp)
    {
      // Are they both a terminal (and the same terminal)?
      if (rp[0].is_terminal() || rp[1].is_terminal()) {
        if (rp[0].is_terminal() && rp[1].is_terminal() && rp[0].value() == rp[1].value()) {
          return false;
        } else {
#ifdef ADIAR_STATS
          stats_equality.slow_check.exit_on_children += 1u;
#endif
          return true;
        }
      }

      // Do they NOT point to a node with the same level?
      if (rp[0].label() != rp[1].label()) {
#ifdef ADIAR_STATS
        stats_equality.slow_check.exit_on_children += 1u;
#endif
        return true;
      }

      // No violation, so recurse
      pq.push({ rp, {} });
      return false;
    }

  public:
    static constexpr bool early_return_value    = false;
    static constexpr bool no_early_return_value = true;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Fast 2N/B I/Os comparison by comparing the i'th nodes numerically. This requires, that the
  /// shared_levelized_file<node> is 'canonical' in the following sense:
  ///
  /// - For each level, the ids are decreasing from max_id in increments of one.
  /// - There are no duplicate nodes.
  /// - Nodes within each level are sorted by the children (e.g. ordered first on 'high', secondly
  ///   on 'low').
  ///
  /// \remark See Section 3.3 in 'Efficient Binary Decision Diagram Manipulation in External Memory'
  ///         on arXiv (v2 or newer) for an induction proof this is a valid comparison.
  ///
  /// \pre The following are satisfied:
  /// (1) The number of nodes are the same (to simplify the 'while' condition)
  /// (2) Both shared_levelized_file<node>s are 'canonical'.
  /// (3) The negation flags given to both shared_levelized_file<node>s agree
  ///     (breaks canonicity)
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  fast_isomorphism_check(const dd& a, const dd& b)
  {
    node_ifstream<> in_nodes_a(a);
    node_ifstream<> in_nodes_b(b);

    while (in_nodes_a.can_pull()) {
      adiar_assert(in_nodes_b.can_pull(), "The number of nodes should coincide");
      if (in_nodes_a.pull() != in_nodes_b.pull()) {
#ifdef ADIAR_STATS
        stats_equality.fast_check.exit_on_mismatch += 1u;
#endif
        return false;
      }
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  is_isomorphic(const exec_policy& ep, const dd& a, const dd& b)
  {
    const bool a_negated = a.is_negated();
    const bool b_negated = b.is_negated();

    // Are they literally referring to the same underlying file?
    if (a.file_ptr() == b.file_ptr()) {
#ifdef ADIAR_STATS
      stats_equality.exit_on_same_file += 1u;
#endif
      return a_negated == b_negated && a.shift() == b.shift();
    }

    // Are they trivially not the same, since they have different number of nodes?
    if (a->size() != b->size()) {
#ifdef ADIAR_STATS
      stats_equality.exit_on_nodecount += 1u;
#endif
      return false;
    }

    // Are they trivially not the same, since their width is different?
    if (a->width != b->width) {
#ifdef ADIAR_STATS
      stats_equality.exit_on_width += 1u;
#endif
      return false;
    }

    // Are they trivially not the same, since they have different number of terminal arcs?
    if (a->number_of_terminals[a_negated] != b->number_of_terminals[b_negated]
        || a->number_of_terminals[!a_negated] != b->number_of_terminals[!b_negated]) {
#ifdef ADIAR_STATS
      stats_equality.exit_on_terminalcount += 1u;
#endif
      return false;
    }

    // Are they trivially not the same, since they have different number of levels?
    if (a->levels() != b->levels()) {
#ifdef ADIAR_STATS
      stats_equality.exit_on_varcount += 1u;
#endif
      return false;
    }

    // Are they trivially not the same, since the labels or the size of each level does not match?
    { // Create new scope to garbage collect the two meta_ifstreams early
      level_info_ifstream<> in_meta_a(a);
      level_info_ifstream<> in_meta_b(b);

      while (in_meta_a.can_pull()) {
        adiar_assert(in_meta_b.can_pull(), "level_info files are same size");
        if (in_meta_a.pull() != in_meta_b.pull()) {
#ifdef ADIAR_STATS
          stats_equality.exit_on_levels_mismatch += 1u;
#endif
          return false;
        }
      }
    }

    // TODO: Use 'fast_isomorphism_check' when there is only one node per level. In this case, we
    // can just ignore the id (and only focus on the label and terminal values).

    // Compare their content to discern whether there exists an isomorphism between them.
    if (a->is_canonical() && b->is_canonical() && a_negated == b_negated) {
#ifdef ADIAR_STATS
      stats_equality.fast_check.runs += 1u;
#endif
      return fast_isomorphism_check(a, b);
    } else {
#ifdef ADIAR_STATS
      stats_equality.slow_check.runs += 1u;
#endif
      return comparison_check<isomorphism_policy>(ep, a, b);
    }
  }
}
