#include "statistics.h"

#include "adiar/internal/algorithms/optmin.h"
#include <iomanip>

#include <adiar/bdd/if_then_else.h>

#include <adiar/internal/algorithms/count.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/algorithms/pred.h>
#include <adiar/internal/algorithms/prod2b.h>
#include <adiar/internal/algorithms/prod2u.h>
#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/algorithms/replace.h>
#include <adiar/internal/algorithms/select.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/node_file.h>

namespace adiar
{
  // TODO: Move ownership of statistics object into `statistics.cpp`. This way,
  //       statistics do not need to be recompiled every time an algorithm is
  //       changed.

  statistics
  statistics_get()
  {
#ifndef ADIAR_STATS
    std::cerr << "Statistics not gathered. Please compile with 'ADIAR_STATS'" << std::endl;
#endif

    return { // i/o
             internal::stats_arc_file,
             internal::stats_node_file,

             // data structures
             internal::stats_levelized_priority_queue,

             // top-down sweeps
             internal::stats_count,
             internal::stats_equality,
             internal::stats_intercut,
             internal::stats_optmin,
             internal::stats_prod2b,
             internal::stats_prod2u,
             stats_prod3,
             internal::stats_quantify,
             internal::stats_select,

             // bottom-up sweeps
             internal::stats_reduce,

             // other algorithms
             internal::stats_replace,
             internal::nested_sweeping::stats
    };
  }

  void
  statistics_reset()
  {
    // i/o
    internal::stats_arc_file  = {};
    internal::stats_node_file = {};

    // data structures
    internal::stats_levelized_priority_queue = {};

    // top-down sweeps
    internal::stats_count    = {};
    internal::stats_equality = {};
    internal::stats_intercut = {};
    internal::stats_optmin   = {};
    internal::stats_prod2b   = {};
    stats_prod3              = {};
    internal::stats_quantify = {};
    internal::stats_select   = {};

    // bottom-up sweeps
    internal::stats_reduce = {};

    // other algorithms
    internal::stats_replace          = {};
    internal::nested_sweeping::stats = {};
  }

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions for pretty printing (UNIX)
  int indent_level = 0;

  constexpr int FLOAT_PRECISION = 2;

  const std::string bold_on  = "\033[1m";
  const std::string bold_off = "\033[0m";
  const std::string percent  = "%";

  inline std::ostream&
  indent(std::ostream& os)
  {
    return os << "| " << std::left << std::setw(2 * indent_level) << "";
  }

  inline std::ostream&
  label(std::ostream& os)
  {
    constexpr int cell1_indent_level = 38;
    return os << std::left << std::setw(cell1_indent_level - 2 * indent_level);
  }

  inline std::ostream&
  endl(std::ostream& os)
  {
    return os << '\n';
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Convert a wide integer to a string and push it to an output stream.
  ///
  /// \details This should be moved to `adiar::internal` in
  ///          `<adiar/internal/cnl.h>` but doing so creates an ambiguity for
  ///          the `ptr_uint64` class. Hence, we have to keep this an function
  ///          of only this file until we have added `to_string()` functions to
  ///          all basic data types.
  //////////////////////////////////////////////////////////////////////////////
  inline std::ostream&
  operator<<(std::ostream& os, const uintwide& s)
  {
    return os << internal::to_string(s);
  }

  void
  __printstat_arc_file(std::ostream& o)
  {
    o << indent << bold_on << "Arc Files" << bold_off << endl;

    indent_level++;

    uintwide total_pushes = internal::stats_arc_file.push_internal
      + internal::stats_arc_file.push_in_order + internal::stats_arc_file.push_out_of_order;

    if (total_pushes == 0u) {
      o << indent << "No writes" << endl;
      indent_level--;
      return;
    }

    o << indent << bold_on << label << "unsafe_push(arc ...)" << bold_off << total_pushes << endl;

    indent_level++;
    o << indent << label << "internal" << internal::stats_arc_file.push_internal << " = "
      << internal::percent_frac(internal::stats_arc_file.push_internal, total_pushes) << percent
      << endl;
    o << indent << label << "terminals (in-order)" << internal::stats_arc_file.push_in_order
      << " = " << internal::percent_frac(internal::stats_arc_file.push_in_order, total_pushes)
      << percent << endl;
    o << indent << label << "terminals (out-of-order)" << internal::stats_arc_file.push_out_of_order
      << " = " << internal::percent_frac(internal::stats_arc_file.push_out_of_order, total_pushes)
      << percent << endl;
    indent_level--;

    o << indent << bold_on << label << "push(level_info ...)" << bold_off
      << internal::stats_arc_file.push_level << endl;

    o << indent << bold_on << label << "out-of-order sortings" << bold_off
      << internal::stats_arc_file.sort_out_of_order << endl;

    indent_level--;
  }

  void
  __printstat_node_file(std::ostream& o)
  {
    o << indent << bold_on << "Node Files" << bold_off << endl;

    indent_level++;

    if (internal::stats_node_file.push_node == 0u) {
      o << indent << "No writes" << endl;
      indent_level--;
      return;
    }

    o << indent << bold_on << label << "unsafe_push(...)" << bold_off << endl;

    indent_level++;
    o << indent << label << "node" << internal::stats_node_file.push_node << endl;
    o << indent << label << "level_info" << internal::stats_node_file.push_level << endl;
    indent_level--;

    indent_level--;
  }

  void
  __printstat_lpq(std::ostream& o, const statistics::levelized_priority_queue_t& stats)
  {
    if (indent_level == 0) {
      o << indent << bold_on << "Levelized Priority Queue" << bold_off << endl;
    }

    indent_level++;

    if (stats.sum_destructors == 0) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    uintwide total_pushes = stats.push_bucket + stats.push_overflow;
    o << indent << bold_on << label << "push(...)" << bold_off << total_pushes << endl;

    indent_level++;
    o << indent << label << "hit bucket" << stats.push_bucket << " = "
      << internal::percent_frac(stats.push_bucket, total_pushes) << percent << endl;
    o << indent << label << "hit overflow" << stats.push_overflow << " = "
      << internal::percent_frac(stats.push_overflow, total_pushes) << percent << endl;
    indent_level--;

    o << indent << endl;

    o << indent << bold_on << "prediction precision ratio" << bold_off << endl;

    indent_level++;
    o << indent << label << "unweighted" << stats.sum_max_size_ratio << " / "
      << stats.sum_destructors << " = " << 100.0 * stats.sum_max_size_ratio / stats.sum_destructors
      << percent << endl;

    o << indent << label << "weighted" << stats.sum_actual_max_size << " / "
      << stats.sum_predicted_max_size << " = "
      << internal::percent_frac(stats.sum_actual_max_size, stats.sum_predicted_max_size) << percent
      << endl;
    indent_level -= 2;
  }

  void
  __printstat_alg_base(std::ostream& o, const statistics::__alg_base& stats)
  {
    o << indent << bold_on << "levelized priority queue" << bold_off << endl;

    const uintwide total_lpqs          = stats.lpq.total();
    const uintwide total_internal_lpqs = stats.lpq.unbucketed + stats.lpq.internal;

    indent_level++;

    o << indent << label << "external memory" << stats.lpq.external << " = "
      << internal::percent_frac(stats.lpq.external, total_lpqs) << percent << endl;

    o << indent << label << "internal memory" << total_internal_lpqs << " = "
      << internal::percent_frac(total_internal_lpqs, total_lpqs) << percent << endl;

    o << indent << label << "  unbucketed" << stats.lpq.unbucketed << " = "
      << internal::percent_frac(stats.lpq.unbucketed, total_internal_lpqs) << percent << endl;

    indent_level--;

    o << indent << endl;

    __printstat_lpq(o, stats.lpq);
  }

  void
  __printstat_count(std::ostream& o)
  {
    uintwide total_runs = internal::stats_count.lpq.total();
    o << indent << bold_on << label << "Count" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_count);
    indent_level--;
  }

  void
  __printstat_comparison_check(std::ostream& o)
  {
    uintwide total_runs = internal::stats_equality.lpq.total();
    o << indent << bold_on << label << "Comparison Check" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_equality);
    indent_level--;
  }

  void
  __printstat_intercut(std::ostream& o)
  {
    uintwide total_runs = internal::stats_intercut.lpq.total();
    o << indent << bold_on << label << "Intercut" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_intercut);
    indent_level--;
  }

  void
  __printstat_isomorphism(std::ostream& o)
  {
    const uintwide total_runs = internal::stats_equality.exit_on_same_file
      + internal::stats_equality.exit_on_nodecount + internal::stats_equality.exit_on_varcount
      + internal::stats_equality.exit_on_width + internal::stats_equality.exit_on_terminalcount
      + internal::stats_equality.exit_on_levels_mismatch + internal::stats_equality.slow_check.runs
      + internal::stats_equality.fast_check.runs;

    o << indent << bold_on << label << "Isomorphism Check" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    o << indent << bold_on << "execution trace" << bold_off << endl;

    indent_level++;
    o << indent << "O(1) termination cases" << endl;
    indent_level++;
    o << indent << label << "same file" << internal::stats_equality.exit_on_same_file << endl;
    o << indent << label << "node count mismatch" << internal::stats_equality.exit_on_nodecount
      << endl;
    o << indent << label << "var count mismatch" << internal::stats_equality.exit_on_varcount
      << endl;
    o << indent << label << "width mismatch" << internal::stats_equality.exit_on_width << endl;
    o << indent << label << "terminal count mismatch"
      << internal::stats_equality.exit_on_terminalcount << endl;
    indent_level--;

    o << indent << endl;

    o << indent << "O(L/B) termination cases" << endl;
    indent_level++;
    o << indent << label << "level info mismatch"
      << internal::stats_equality.exit_on_levels_mismatch << endl;
    indent_level--;

    o << indent << endl;

    o << indent << label << "O(sort(N)) algorithm" << internal::stats_equality.slow_check.runs
      << endl;
    indent_level++;
    o << indent << label << "local violation (root)"
      << internal::stats_equality.slow_check.exit_on_root << endl;
    o << indent << label << "local violation (other)"
      << internal::stats_equality.slow_check.exit_on_children << endl;
    o << indent << label << "too many requests"
      << internal::stats_equality.slow_check.exit_on_processed_on_level << endl;
    indent_level--;

    o << indent << endl;

    o << indent << label << "O(N/B) algorithm" << internal::stats_equality.fast_check.runs << endl;
    indent_level++;
    o << indent << label << "node mismatch" << internal::stats_equality.fast_check.exit_on_mismatch
      << endl;

    indent_level -= 2;

    o << indent << endl;

    o << indent << bold_on << "levelized priority queue" << bold_off << endl;
    indent_level++;
    o << indent << "see 'Comparison Check'" << endl;

    indent_level -= 2;
  }

  void
  __printstat_optmin(std::ostream& o)
  {
    uintwide total_runs = internal::stats_optmin.lpq.total();
    o << indent << bold_on << label << "Optmin" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_optmin);
    indent_level--;
  }

  void
  __printstat_prod2b(std::ostream& o)
  {
    uintwide total_runs = internal::stats_prod2b.trivial_file
      + internal::stats_prod2b.trivial_terminal + internal::stats_prod2b.ra.runs
      + internal::stats_prod2b.pq.runs;

    o << indent << bold_on << label << "Product Construction (2-ary) <binary>" << bold_off
      << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    o << indent << bold_on << label << "case [same file]" << bold_off
      << internal::stats_prod2b.trivial_file << " = "
      << internal::percent_frac(internal::stats_prod2b.trivial_file, total_runs) << percent << endl;

    o << indent << endl;

    o << indent << bold_on << label << "case [terminal]" << bold_off
      << internal::stats_prod2b.trivial_terminal << " = "
      << internal::percent_frac(internal::stats_prod2b.trivial_terminal, total_runs) << percent
      << endl;

    o << indent << endl;

    o << indent << bold_on << label << "case [random access]" << bold_off
      << internal::stats_prod2b.ra.runs << " = "
      << internal::percent_frac(internal::stats_prod2b.ra.runs, total_runs) << percent << endl;
    if (internal::stats_prod2b.ra.runs > 0u) {
      indent_level++;
      o << indent << label << "used narrowest:" << internal::stats_prod2b.ra.used_narrowest << " = "
        << internal::percent_frac(internal::stats_prod2b.ra.used_narrowest,
                                  internal::stats_prod2b.ra.runs)
        << percent << endl;

      o << indent << endl;

      o << indent << bold_on << label << "width:" << bold_off << endl;
      indent_level++;

      o << indent << label << "minimum:" << internal::stats_prod2b.ra.min_width << endl;

      o << indent << label << "maximum:" << internal::stats_prod2b.ra.max_width << endl;

      o << indent << label << "accumulated:" << internal::stats_prod2b.ra.acc_width << " (avg = "
        << internal::frac(internal::stats_prod2b.ra.acc_width, internal::stats_prod2b.ra.runs)
        << ")" << endl;

      indent_level -= 2;
    }

    o << indent << endl;

    o << indent << bold_on << label << "case [priority queue]" << bold_off
      << internal::stats_prod2b.pq.runs << " = "
      << internal::percent_frac(internal::stats_prod2b.pq.runs, total_runs) << percent << endl;
    if (internal::stats_prod2b.pq.runs > 0u) {
      indent_level++;
      o << indent << label << "pq2 elements:" << internal::stats_prod2b.pq.pq_2_elems << endl;

      indent_level--;
    }

    o << indent << endl;
    __printstat_alg_base(o, internal::stats_prod2b);
    indent_level--;
  }

  void
  __printstat_prod2u(std::ostream& o)
  {
    uintwide total_runs = internal::stats_prod2u.ra.runs + internal::stats_prod2u.pq.runs;
    o << indent << bold_on << label << "Product Construction (2-ary) <unary>" << bold_off
      << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    o << indent << bold_on << label << "case [random access]" << bold_off
      << internal::stats_prod2u.ra.runs << " = "
      << internal::percent_frac(internal::stats_prod2u.ra.runs, total_runs) << percent << endl;
    if (internal::stats_prod2u.ra.runs > 0u) {
      indent_level++;
      o << indent << label << "used narrowest:" << internal::stats_prod2u.ra.used_narrowest << " = "
        << internal::percent_frac(internal::stats_prod2u.ra.used_narrowest,
                                  internal::stats_prod2u.ra.runs)
        << percent << endl;

      o << indent << endl;

      o << indent << bold_on << label << "width:" << bold_off << endl;
      indent_level++;

      o << indent << label << "minimum:" << internal::stats_prod2u.ra.min_width << endl;

      o << indent << label << "maximum:" << internal::stats_prod2u.ra.max_width << endl;

      o << indent << label << "accumulated:" << internal::stats_prod2u.ra.acc_width << " (avg = "
        << internal::frac(internal::stats_prod2u.ra.acc_width, internal::stats_prod2u.ra.runs)
        << ")" << endl;

      indent_level -= 2;
    }

    o << indent << endl;

    o << indent << bold_on << label << "case [priority queue]" << bold_off
      << internal::stats_prod2u.pq.runs << " = "
      << internal::percent_frac(internal::stats_prod2u.pq.runs, total_runs) << percent << endl;
    if (internal::stats_prod2u.pq.runs > 0u) {
      indent_level++;
      o << indent << label << "pq2 elements:" << internal::stats_prod2u.pq.pq_2_elems << endl;

      indent_level--;
    }

    o << indent << endl;
    __printstat_alg_base(o, internal::stats_prod2u);
    indent_level--;

    o << indent << endl;
    {
      o << indent << bold_on << label << "requests (excluding roots)" << bold_off << endl;

      indent_level++;
      {
        const uintwide total_requests =
          internal::stats_prod2u.requests[0] + internal::stats_prod2u.requests[1];

        o << indent << label << "incl. duplicates" << total_requests << endl;

        indent_level++;
        for (int arity_idx = 0; arity_idx < 2; ++arity_idx) {
          std::string t;
          t += static_cast<char>('1' + arity_idx);
          t += "-ary";

          o << indent << label << t << internal::stats_prod2u.requests[arity_idx] << " = "
            << internal::percent_frac(internal::stats_prod2u.requests[arity_idx], total_requests)
            << percent << endl;
        }
        indent_level--;
      }

      o << indent << endl;

      {
        const uintwide total_unique =
          internal::stats_prod2u.requests_unique[0] + internal::stats_prod2u.requests_unique[1];

        o << indent << label << "excl. duplicates" << total_unique << endl;

        indent_level++;
        for (int arity_idx = 0; arity_idx < 2; ++arity_idx) {
          std::string t;
          t += static_cast<char>('1' + arity_idx);
          t += "-ary";

          o << indent << label << t << internal::stats_prod2u.requests_unique[arity_idx] << " = "
            << internal::percent_frac(internal::stats_prod2u.requests_unique[arity_idx],
                                      total_unique)
            << percent << endl;
        }
        indent_level--;
      }
      indent_level--;
    }
  }

  void
  __printstat_prod3(std::ostream& o)
  {
    uintwide total_runs = stats_prod3.lpq.total();
    o << indent << bold_on << label << "Product Construction (3-ary)" << bold_off << total_runs
      << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, stats_prod3);
    indent_level--;
  }

  void
  __printstat_quantify(std::ostream& o)
  {
    const uintwide total_runs = internal::stats_quantify.runs;

    o << indent << bold_on << label << "Quantification" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    {
      o << indent << bold_on << label << "case [skipped]" << bold_off
        << internal::stats_quantify.skipped << " = "
        << internal::percent_frac(internal::stats_quantify.skipped, total_runs) << percent << endl;
    }

    {
      o << indent << endl;

      o << indent << bold_on << label << "case [singleton sweep]" << bold_off
        << internal::stats_quantify.singleton_sweeps << endl;
    }

    {
      o << indent << endl;

      o << indent << bold_on << label << "case [nested sweep]" << bold_off
        << internal::stats_quantify.nested_sweeps << " = "
        << internal::percent_frac(internal::stats_quantify.nested_sweeps, total_runs) << percent
        << endl;

      indent_level++;

      {
        o << indent << bold_on << label << "transposition" << bold_off << endl;

        indent_level++;
        o << indent << label << "none" << internal::stats_quantify.nested_transposition.none
          << endl;

        o << indent << label << "simple" << internal::stats_quantify.nested_transposition.simple
          << endl;

        o << indent << label << "singleton quantification"
          << internal::stats_quantify.nested_transposition.singleton << endl;

        o << indent << label << "pruning" << internal::stats_quantify.nested_transposition.pruning
          << endl;

        indent_level--;
      }
      o << indent << endl;

      const uintwide total_roots = internal::stats_quantify.nested_policy.shortcut_terminal
        + internal::stats_quantify.nested_policy.shortcut_node
        + internal::stats_quantify.nested_policy.products;

      o << indent << label << "root requests" << total_roots << endl;

      indent_level++;

      o << indent << label << "shortcut (terminal)"
        << internal::stats_quantify.nested_policy.shortcut_terminal << " = "
        << internal::percent_frac(internal::stats_quantify.nested_policy.shortcut_terminal,
                                  total_roots)
        << percent << endl;

      o << indent << label << "shortcut (node)"
        << internal::stats_quantify.nested_policy.shortcut_node << " = "
        << internal::percent_frac(internal::stats_quantify.nested_policy.shortcut_node, total_roots)
        << percent << endl;

      o << indent << label << "products" << internal::stats_quantify.nested_policy.products << " = "
        << internal::percent_frac(internal::stats_quantify.nested_policy.products, total_roots)
        << percent << endl;

      indent_level -= 2;
    }
    indent_level--;
  }

  void
  __printstat_select(std::ostream& o)
  {
    uintwide total_runs = internal::stats_select.lpq.total();
    o << indent << bold_on << label << "Selection" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    __printstat_alg_base(o, internal::stats_select);
    indent_level--;
  }

  void
  __printstat_reduce(std::ostream& o, statistics::reduce_t& stats_struct = internal::stats_reduce)
  {
    const bool is_outer = indent_level == 0;
    if (is_outer) {
      uintwide total_runs = stats_struct.lpq.total();
      o << indent << bold_on << label << "Reduce" << bold_off << total_runs << endl;

      indent_level++;

      if (total_runs == 0u) {
        o << indent << "Not used" << endl;
        indent_level--;
        return;
      }
    }

    uintwide total_arcs = stats_struct.sum_node_arcs + stats_struct.sum_terminal_arcs;
    o << indent << bold_on << label << "inputs size" << bold_off << total_arcs
      << " arcs = " << total_arcs / 2 << " nodes" << endl;

    indent_level++;
    o << indent << label << "node arcs:" << stats_struct.sum_node_arcs << " = "
      << internal::percent_frac(stats_struct.sum_node_arcs, total_arcs) << percent << endl;

    o << indent << label << "terminal arcs:" << stats_struct.sum_terminal_arcs << " = "
      << internal::percent_frac(stats_struct.sum_terminal_arcs, total_arcs) << percent << endl;
    indent_level--;

    o << indent << endl;
    uintwide total_removed = stats_struct.removed_by_rule_1 + stats_struct.removed_by_rule_2;
    o << indent << bold_on << label << "nodes removed" << bold_off;
    if (total_removed > 0u) {
      o << total_removed << " = " << internal::percent_frac(total_removed, total_arcs) << percent
        << endl;
      indent_level++;
      o << indent << label << "rule 1:" << stats_struct.removed_by_rule_1 << " = "
        << internal::percent_frac(stats_struct.removed_by_rule_1, total_removed) << percent << endl;

      o << indent << label << "rule 2:" << stats_struct.removed_by_rule_2 << " = "
        << internal::percent_frac(stats_struct.removed_by_rule_2, total_removed) << percent << endl;
      indent_level--;
    } else {
      o << "none" << endl;
    }

    o << indent << endl;
    __printstat_alg_base(o, stats_struct);

    if (is_outer) { indent_level--; }
  }

  void
  __printstat_replace(std::ostream& o)
  {
    const uintwide total_runs = internal::stats_replace.terminal_returns
      + internal::stats_replace.identity_returns + internal::stats_replace.identity_reduces
      + internal::stats_replace.monotonic_scans + internal::stats_replace.monotonic_reduces
      + internal::stats_replace.nested_sweeps;

    o << indent << bold_on << label << "Replace" << bold_off << total_runs << endl;

    indent_level++;
    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      indent_level--;
      return;
    }

    const uintwide const_runs =
      internal::stats_replace.terminal_returns + internal::stats_replace.identity_returns;

    o << indent << bold_on << label << "case O(1)" << bold_off << const_runs << " = "
      << internal::percent_frac(const_runs, total_runs) << percent << endl;

    indent_level++;
    o << indent << label << "terminal" << internal::stats_replace.terminal_returns << " = "
      << internal::percent_frac(internal::stats_replace.terminal_returns, total_runs) << percent
      << endl;

    o << indent << label << "identity" << internal::stats_replace.identity_returns << " = "
      << internal::percent_frac(internal::stats_replace.identity_returns, total_runs) << percent
      << endl;
    indent_level--;

    o << indent << endl;

    o << indent << bold_on << label << "case O(N/B)" << bold_off << endl;

    indent_level++;
    o << indent << label << "monotonic" << internal::stats_replace.monotonic_scans << " = "
      << internal::percent_frac(internal::stats_replace.monotonic_scans, total_runs) << percent
      << endl;
    indent_level--;

    o << indent << endl;

    const uintwide reduce_runs =
      internal::stats_replace.identity_reduces + internal::stats_replace.monotonic_reduces;

    o << indent << bold_on << label << "case O(sort(N))" << bold_off << reduce_runs << " = "
      << internal::percent_frac(reduce_runs, total_runs) << percent << endl;

    indent_level++;
    o << indent << label << "identity" << internal::stats_replace.identity_reduces << " = "
      << internal::percent_frac(internal::stats_replace.identity_reduces, total_runs) << percent
      << endl;

    o << indent << label << "monotonic" << internal::stats_replace.monotonic_reduces << " = "
      << internal::percent_frac(internal::stats_replace.monotonic_reduces, total_runs) << percent
      << endl;
    indent_level--;

    o << indent << endl;

    o << indent << bold_on << label << "case O(N sort(T))" << bold_off << endl;

    indent_level++;
    o << indent << label << "non-monotonic" << internal::stats_replace.nested_sweeps << " = "
      << internal::percent_frac(internal::stats_replace.nested_sweeps, total_runs) << percent
      << endl;
    indent_level--;

    indent_level--;
  }

  void
  __printstat_nested_sweeping(std::ostream& o)
  {
    const uintwide total_runs =
      internal::nested_sweeping::stats.skips + internal::nested_sweeping::stats.runs;

    if (total_runs == 0u) {
      o << indent << "Not used" << endl;
      return;
    }

    o << indent << label << "skips" << internal::nested_sweeping::stats.skips << endl
      << indent << label << "runs" << internal::nested_sweeping::stats.runs << endl;

    o << indent << endl;

    {
      o << indent << bold_on << "outer up sweep" << bold_off << endl;

      indent_level++;

      const uintwide total_nested = internal::nested_sweeping::stats.outer_up.nested_levels
        + internal::nested_sweeping::stats.outer_up.skipped_nested_levels;

      o << indent << bold_on << label << "nested levels" << bold_off << total_nested << endl;

      indent_level++;
      o << indent << label << "executed" << internal::nested_sweeping::stats.outer_up.nested_levels
        << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.outer_up.nested_levels,
                                  total_nested)
        << percent << endl;

      const uintwide unpruned_skipped =
        internal::nested_sweeping::stats.outer_up.skipped_nested_levels
        - internal::nested_sweeping::stats.outer_up.skipped_nested_levels__prune;

      o << indent << label << "skipped (non-pruning)" << unpruned_skipped << " = "
        << internal::percent_frac(unpruned_skipped, total_nested) << percent << endl;

      o << indent << label << "skipped (pruning)"
        << internal::nested_sweeping::stats.outer_up.skipped_nested_levels__prune << " = "
        << internal::percent_frac(
             internal::nested_sweeping::stats.outer_up.skipped_nested_levels__prune, total_nested)
        << percent << endl;

      indent_level--;

      o << indent << endl;
      o << indent << bold_on << label << "collapse to terminal" << bold_off
        << internal::nested_sweeping::stats.outer_up.collapse_to_terminal << endl;

      o << indent << endl;

      o << indent << bold_on << label << "reduced levels" << bold_off
        << internal::nested_sweeping::stats.outer_up.reduced_levels << endl;

      indent_level++;

      const uintwide canonical_levels = internal::nested_sweeping::stats.outer_up.reduced_levels
        - internal::nested_sweeping::stats.outer_up.reduced_levels__fast;

      o << indent << label << "canonical" << canonical_levels << " = "
        << internal::percent_frac(canonical_levels,
                                  internal::nested_sweeping::stats.outer_up.reduced_levels)
        << percent << endl;

      o << indent << label << "fast"
        << internal::nested_sweeping::stats.outer_up.reduced_levels__fast << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.outer_up.reduced_levels__fast,
                                  internal::nested_sweeping::stats.outer_up.reduced_levels)
        << percent << endl;
      indent_level--;

      o << indent << endl;

      __printstat_reduce(o, internal::nested_sweeping::stats.outer_up);

      indent_level--;
    }

    o << indent << endl;

    {
      const uintwide total_sweeps = internal::nested_sweeping::stats.inner_down.ra_runs
        + internal::nested_sweeping::stats.inner_down.pq_runs;

      o << indent << bold_on << label << "inner down sweep" << bold_off << total_sweeps << endl;
      indent_level++;

      o << indent << bold_on << label << "case [random access]" << bold_off
        << internal::nested_sweeping::stats.inner_down.ra_runs << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_down.ra_runs, total_sweeps)
        << percent << endl;

      o << indent << bold_on << label << "case [priority queues]" << bold_off
        << internal::nested_sweeping::stats.inner_down.pq_runs << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_down.pq_runs, total_sweeps)
        << percent << endl;

      o << indent << endl;

      o << indent << bold_on << "inputs" << bold_off << endl;

      indent_level++;

      o << indent << "size" << endl;

      indent_level++;
      o << indent << label << "accumulated"
        << internal::nested_sweeping::stats.inner_down.inputs.acc_size << endl;
      o << indent << label << "maximum"
        << internal::nested_sweeping::stats.inner_down.inputs.max_size << endl;
      indent_level--;

      o << indent << "width" << endl;

      indent_level++;
      o << indent << label << "accumulated"
        << internal::nested_sweeping::stats.inner_down.inputs.acc_width << endl;
      o << indent << label << "maximum"
        << internal::nested_sweeping::stats.inner_down.inputs.max_width << endl;
      indent_level--;

      o << indent << "levels" << endl;

      indent_level++;
      o << indent << label << "accumulated"
        << internal::nested_sweeping::stats.inner_down.inputs.acc_levels << endl;
      o << indent << label << "maximum"
        << internal::nested_sweeping::stats.inner_down.inputs.max_levels << endl;
      indent_level--;

      indent_level--;

      o << indent << endl;

      const uintwide total_arcs = internal::nested_sweeping::stats.inner_up.outer_arcs
        + internal::nested_sweeping::stats.inner_up.inner_arcs;

      o << indent << bold_on << label << "output origin (arcs)" << bold_off << total_arcs << endl;

      indent_level++;

      o << indent << label << "outer sweep" << internal::nested_sweeping::stats.inner_up.outer_arcs
        << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_up.outer_arcs, total_arcs)
        << percent << endl;

      o << indent << label << "inner sweep" << internal::nested_sweeping::stats.inner_up.inner_arcs
        << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_up.inner_arcs, total_arcs)
        << percent << endl;

      indent_level--;

      o << indent << endl;

      const uintwide total_requests = internal::nested_sweeping::stats.inner_down.requests.terminals
        + internal::nested_sweeping::stats.inner_down.requests.preserving
        + internal::nested_sweeping::stats.inner_down.requests.modifying;

      o << indent << bold_on << label << "root requests" << bold_off << total_requests << endl;

      indent_level++;
      o << indent << label << "reduce rule 1"
        << internal::nested_sweeping::stats.inner_down.removed_by_rule_1 << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_down.removed_by_rule_1,
                                  total_requests)
        << percent << endl;

      o << indent << endl;

      o << indent << label << "terminals"
        << internal::nested_sweeping::stats.inner_down.requests.terminals << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_down.requests.terminals,
                                  total_requests)
        << percent << endl;

      o << indent << label << "modifying"
        << internal::nested_sweeping::stats.inner_down.requests.modifying << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_down.requests.modifying,
                                  total_requests)
        << percent << endl;

      o << indent << label << "preserving"
        << internal::nested_sweeping::stats.inner_down.requests.preserving << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_down.requests.preserving,
                                  total_requests)
        << percent << endl;

      indent_level -= 2;
    }

    o << indent << endl;

    {
      o << indent << bold_on << "inner up sweep" << bold_off << endl;

      indent_level++;

      o << indent << bold_on << label << "reduced levels" << bold_off
        << internal::nested_sweeping::stats.inner_up.reduced_levels << endl;

      indent_level++;

      const uintwide canonical_levels = internal::nested_sweeping::stats.inner_up.reduced_levels
        - internal::nested_sweeping::stats.inner_up.reduced_levels__fast;

      o << indent << label << "canonical" << canonical_levels << " = "
        << internal::percent_frac(canonical_levels,
                                  internal::nested_sweeping::stats.inner_up.reduced_levels)
        << percent << endl;

      o << indent << label << "fast"
        << internal::nested_sweeping::stats.inner_up.reduced_levels__fast << " = "
        << internal::percent_frac(internal::nested_sweeping::stats.inner_up.reduced_levels__fast,
                                  internal::nested_sweeping::stats.inner_up.reduced_levels)
        << percent << endl;

      indent_level--;

      o << indent << endl;

      __printstat_reduce(o, internal::nested_sweeping::stats.inner_up);

      indent_level--;
    }
  }

  void
  statistics_print(std::ostream& o)
  {
    o << bold_on << "Adiar statistics" << bold_off << endl;
    o << endl;
#ifndef ADIAR_STATS
    o << indent << "Not gathered; please compile with 'ADIAR_STATS'." << endl;
#else
    o << std::fixed << std::setprecision(FLOAT_PRECISION);

    o << bold_on << "--== I/O ==--" << bold_off << endl << endl;

    __printstat_arc_file(o);
    o << endl;

    __printstat_node_file(o);
    o << endl;

    o << bold_on << "--== Data Structures ==--" << bold_off << endl << endl;

    __printstat_lpq(o, internal::stats_levelized_priority_queue);
    o << endl;

    o << bold_on << "--== Nested Sweeping Framework ==--" << bold_off << endl << endl;

    __printstat_nested_sweeping(o);
    o << endl;

    o << bold_on << "--== Top-Down Sweep Algorithms ==--" << bold_off << endl << endl;

    __printstat_count(o);
    o << endl;

    __printstat_comparison_check(o);
    o << endl;

    __printstat_intercut(o);
    o << endl;

    __printstat_isomorphism(o);
    o << endl;

    __printstat_optmin(o);
    o << endl;

    __printstat_prod2b(o);
    o << endl;

    __printstat_prod2u(o);
    o << endl;

    __printstat_prod3(o);
    o << endl;

    __printstat_select(o);
    o << endl;

    o << bold_on << "--== Bottom-Up Sweep Algorithms ==--" << bold_off << endl << endl;

    __printstat_reduce(o);
    o << endl;

    o << bold_on << "--== Mixed Sweep Algorithms ==--" << bold_off << endl << endl;

    __printstat_quantify(o);
    o << endl;

    __printstat_replace(o);
#endif
  }
}
