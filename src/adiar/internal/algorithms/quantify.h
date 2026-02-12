#ifndef ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
#define ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H

#include <algorithm>
#include <limits>
#include <queue>
#include <variant>

#include <adiar/exec_policy.h>
#include <adiar/functional.h>

#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/algorithms/prod2u.h>
#include <adiar/internal/algorithms/select.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/block_size.h>
#include <adiar/internal/bool_op.h>
#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_ofstream.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/node_ifstream.h>
#include <adiar/internal/io/node_raccess.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/unreachable.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  //  Quantification
  // ================
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::quantify_t stats_quantify;

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Single-variable Quantification

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator for quantifying a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class single_quantify_policy : public prod2u_single_policy<Policy, /* SortTargets = */ true>
  {
  private:
    const typename Policy::label_type _level;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    single_quantify_policy(typename Policy::label_type level)
      : _level(level)
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Start product construction at the desired level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool
    split(typename Policy::label_type level) const
    {
      return this->_level == level;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Entry Point for Quantification algorithm for a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  quantify(const exec_policy& ep,
           const typename Policy::dd_type& in,
           const typename Policy::label_type label)
  {
#ifdef ADIAR_STATS
    stats_quantify.runs += 1u;
#endif

    // -------------------------------------------------------------------------
    // Case: Terminal / Disjunct Levels
    if (dd_isterminal(in) || !has_level(in, label)) {
#ifdef ADIAR_STATS
      stats_quantify.skipped += 1u;
#endif
      return in;
    }

    // -------------------------------------------------------------------------
    // Case: Do the product construction
#ifdef ADIAR_STATS
    stats_quantify.singleton_sweeps += 1u;
#endif

    single_quantify_policy<Policy> policy(label);
    return __prod2u(ep, in, policy);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Multi-variable (common)

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator to lift it for Nested Sweeping.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class multi_quantify_policy : public prod2u_nested_policy<Policy, /* SortTargets = */ true>
  {
    using base = prod2u_nested_policy<Policy, true>;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    multi_quantify_policy()
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief What the labels should be mapped to (themselves).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    constexpr inline typename Policy::label_type
    map_level(typename Policy::label_type x) const
    {
      return x;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Convert a node from the outer sweep on a to-be quantified level into a request.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    inline typename base::request_t
    request_from_node(const typename Policy::node_type& n,
                      const typename Policy::pointer_type& parent) const
    {
      using request_t = typename base::request_t;
      using target_t  = typename request_t::target_t;

      // Shortcutting or Irrelevant terminal?
      const typename Policy::pointer_type result = Policy::resolve_root(n);

      const bool shortcut = result != n.uid();

      target_t tgt = shortcut
        // If able to shortcut, preserve result.
        ? target_t{ result, Policy::pointer_type::nil() }
        // Otherwise, create product of children
        : target_t{ first(n.low(), n.high()), second(n.low(), n.high()) };

#ifdef ADIAR_STATS
      stats_quantify.nested_policy.shortcut_terminal +=
        static_cast<int>(shortcut && result.is_terminal());
      stats_quantify.nested_policy.shortcut_node += static_cast<int>(shortcut && result.is_node());
      stats_quantify.nested_policy.products += static_cast<int>(!shortcut);
#endif

      return request_t(tgt, {}, { parent });
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The final result from Nested Sweeping should be canonical.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr bool final_canonical = true;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Nested Sweeping should **not** use 'fast reduce'
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr bool fast_reduce = false;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Multi-variable (predicate)

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator for Pruning Quantification.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class pruning_quantify_policy : public Policy
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using pred_t = predicate<typename Policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const pred_t& _pred;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Stores result of `_pred` to save on computation time.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool _pred_result;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    pruning_quantify_policy(const pred_t& pred)
      : _pred(pred)
      , _pred_result(false)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    setup_level(typename Policy::label_type level)
    {
      _pred_result = _pred(level) == Policy::quantify_onset;
    }

    internal::select_rec
    process(const typename Policy::node_type& n)
    {
      // -------------------------------------------------------------------------------------------
      // CASE: Not to-be quantified node.
      if (!_pred_result) { return n; }

      // -------------------------------------------------------------------------------------------
      // CASE: Prune low()
      //
      // TODO (ZDD): Remove 'Policy::keep_terminal' depending on semantics in Policy
      if (Policy::collapse_to_terminal(n.low())) { return n.low(); }
      if (n.low().is_terminal() && !Policy::keep_terminal(n.low())) { return n.high(); }

      // -------------------------------------------------------------------------------------------
      // CASE: Prune high()
      if (Policy::collapse_to_terminal(n.high())) { return n.high(); }
      if (n.high().is_terminal() && !Policy::keep_terminal(n.high())) { return n.low(); }

      // -------------------------------------------------------------------------------------------
      // No pruning possible. Do nothing.
      return n;
    }

    typename Policy::dd_type
    terminal(bool terminal_val)
    {
      return typename Policy::dd_type(terminal_val);
    }

    static constexpr bool skip_reduce = false;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator for Nested Sweeping with a Predicate.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class multi_quantify_policy__pred : public multi_quantify_policy<Policy>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using pred_t = predicate<typename Policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Predicate for whether a level should be swept on (or not).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const pred_t& _pred;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    multi_quantify_policy__pred(const pred_t& pred)
      : _pred(pred)
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the predicate wants to sweep on the given level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_sweep(const typename Policy::label_type x)
    {
      return _pred(x) == Policy::quantify_onset;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  struct quantify__pred_profile
  {
  private:
  public:
    /// \brief Total number of nodes in the diagram.
    size_t dd_size;

    /// \brief Width of the diagram.
    size_t dd_width;

    /// \brief Total number of variables in the diagram.
    size_t dd_vars;

    /// \brief Number of nodes of *all* to-be quantified levels.
    size_t quant_all_size = 0u;

    /// \brief Number of *all* to-be quantified levels.
    size_t quant_all_vars = 0u;

    /// \brief Number of *deep* to-be quantified levels, i.e. the last N/3 nodes.
    size_t quant_deep_vars = 0u;

    /// \brief Number of *shallow* to-be quantified levels, i.e. the first N/3 nodes.
    size_t quant_shallow_vars = 0u;

    struct var_data
    {
      /// \brief The to-be quantified variable.
      typename Policy::label_type level;

      /// \brief Number of nodes below this level (not inclusive)
      size_t nodes_below;

      /// \brief Number of nodes on said level
      typename Policy::id_type width;
    };

    /// \brief The *deepest* to-be quantified level.
    var_data deepest_var{ 0, std::numeric_limits<size_t>::max(), Policy::max_id + 1 };

    /// \brief The *shallowest* to-be quantified level.
    var_data shallowest_var{ Policy::max_label + 1,
                             std::numeric_limits<size_t>::max(),
                             Policy::max_id + 1 };

    /// \brief The *widest* to-be quantified level.
    var_data widest_var{
      Policy::max_label + 1,
      std::numeric_limits<size_t>::max(),
      0,
    };

    /// \brief The *narrowest* to-be quantified level.
    var_data narrowest_var{ Policy::max_label + 1,
                            std::numeric_limits<size_t>::max(),
                            Policy::max_id + 1 };
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain statistics on the variables in a Decision Diagram satisfying the predicate.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline quantify__pred_profile<Policy>
  __quantify__pred_profile(const typename Policy::dd_type& dd,
                           const predicate<typename Policy::label_type>& pred)
  {
    // TODO: tighten 'shallow' to only be above shallowest widest level (inclusive)
    // TODO: tighten 'deep' to not be 'shallow'

    quantify__pred_profile<Policy> res;
    res.dd_size  = dd_nodecount(dd);
    res.dd_width = dd_width(dd);
    res.dd_vars  = dd_varcount(dd);

    level_info_ifstream<false /* top-down */> lis(dd);

    size_t shallow_threshold = res.dd_size / 3;

    size_t nodes_above = 0u;
    size_t nodes_below = res.dd_size;

    while (lis.can_pull()) {
      const level_info li = lis.pull();

      nodes_below -= li.width();

      if (pred(li.label()) == Policy::quantify_onset) {
        res.quant_all_vars += 1u;
        res.quant_all_size += li.width();
        res.quant_deep_vars += nodes_below < shallow_threshold;
        res.quant_shallow_vars += nodes_above <= shallow_threshold;

        { // Deepest variable (always updated due to top-down direction).
          res.deepest_var.level       = li.level();
          res.deepest_var.nodes_below = nodes_below;
          res.deepest_var.width       = li.width();
        }
        // Shallowest variable
        if (res.shallowest_var.level < li.level()) { res.shallowest_var = res.deepest_var; }
        // Widest variable
        if (res.widest_var.width < li.width()) { res.widest_var = res.deepest_var; }
        // Narrowest variable
        if (li.width() < res.narrowest_var.width) { res.narrowest_var = res.deepest_var; }
      }

      if (li.width() == res.dd_width) {
        shallow_threshold = std::min(shallow_threshold, nodes_above);
      }
      nodes_above += li.width();
    }
    return res;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the deepest level that satisfies (or not) the requested level.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: optimisations
  //       - initial cheap check on is_terminal.
  //       - initial '__quantify__get_deepest' should not terminate early but
  //         determine whether any variable may "survive".
  template <typename Policy>
  inline typename Policy::label_type
  __quantify__get_deepest(const typename Policy::dd_type& dd,
                          const predicate<typename Policy::label_type>& pred)
  {
    level_info_ifstream<true /* bottom-up */> lis(dd);

    while (lis.can_pull()) {
      const typename Policy::label_type l = lis.pull().label();
      if (pred(l) == Policy::quantify_onset) { return l; }
    }
    return Policy::max_label + 1;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Entry Point for Multi-variable Quantification with a Predicate.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  quantify(const exec_policy& ep,
           typename Policy::dd_type dd,
           const predicate<typename Policy::label_type>& pred)
  {
#ifdef ADIAR_STATS
    stats_quantify.runs += 1u;
#endif

    using unreduced_t = typename Policy::__dd_type;
    // TODO: check for missing std::move(...)

    const quantify__pred_profile<Policy> pred_profile = __quantify__pred_profile<Policy>(dd, pred);

    // ---------------------------------------------------------------------------------------------
    // Case: Nothing to do
    if (pred_profile.quant_all_vars == 0u) {
#ifdef ADIAR_STATS
      stats_quantify.skipped += 1u;
#endif
      return dd;
    }

    // ---------------------------------------------------------------------------------------------
    // Case: Only one variable to quantify
    //
    // TODO: pred_profile.quant_all_vars == 1u

    switch (ep.template get<exec_policy::quantify::algorithm>()) {
    case exec_policy::quantify::Singleton: {
      // -------------------------------------------------------------------------------------------
      // Case: Repeated single variable quantification
      typename Policy::label_type label = pred_profile.deepest_var.level;

      while (label <= Policy::max_label) {
        dd = quantify<Policy>(ep, dd, label);
#ifdef ADIAR_STATS
        // HACK: Undo the += 1 in the nested call
        stats_quantify.runs -= 1u;
#endif
        if (dd_isterminal(dd)) { return dd; }

        label = __quantify__get_deepest<Policy>(dd, pred);
      }
      return dd;
    }

    case exec_policy::quantify::Nested: {
      // -------------------------------------------------------------------------------------------
      // Case: Nested Sweeping
#ifdef ADIAR_STATS
      stats_quantify.nested_transposition.pruning += 1u;
#endif
      pruning_quantify_policy<Policy> pruning_impl(pred);
      const unreduced_t transposed = select(ep, std::move(dd), pruning_impl);

#ifdef ADIAR_STATS
      stats_quantify.nested_sweeps += 1u;
#endif
      multi_quantify_policy__pred<Policy> inner_impl(pred);
      return nested_sweep<>(ep, std::move(transposed), inner_impl);
    }

      // LCOV_EXCL_START
    default:
      // -------------------------------------------------------------------------------------------
      adiar_unreachable();
      // LCOV_EXCL_STOP
    }
  }

  template <typename Policy>
  typename Policy::__dd_type
  quantify(const exec_policy& ep,
           typename Policy::__dd_type&& __dd,
           const predicate<typename Policy::label_type>& pred)
  {
    switch (ep.template get<exec_policy::quantify::algorithm>()) {
    case exec_policy::quantify::Singleton: {
      // -------------------------------------------------------------------------------------------
      // Case: Repeated single variable quantification
      return quantify<Policy>(ep, typename Policy::dd_type(std::move(__dd)), pred);
    }

    case exec_policy::quantify::Nested: {
      // -------------------------------------------------------------------------------------------
      // Case: Nested Sweeping
      if (__dd.template has<typename Policy::shared_node_file_type>()) {
        return quantify<Policy>(ep, typename Policy::dd_type(std::move(__dd)), pred);
      }

#ifdef ADIAR_STATS
      stats_quantify.runs += 1u;
      stats_quantify.nested_transposition.none += 1u;
      stats_quantify.nested_sweeps += 1u;
#endif
      multi_quantify_policy__pred<Policy> inner_impl(pred);
      return nested_sweep<>(ep, std::move(__dd), inner_impl);
    }

      // LCOV_EXCL_START
    default:
      // -------------------------------------------------------------------------------------------
      adiar_unreachable();
      // LCOV_EXCL_STOP
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Multi-variable (descending generator)

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Policy Decorator for Nested Sweeping with a Generator
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class multi_quantify_policy__generator : public multi_quantify_policy<Policy>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Generator of the levels to sweep on (or not to sweep on) in descending order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using generator_t = generator<typename Policy::label_type>;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Generator of levels to sweep on (or not to sweep on) in descending order.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const generator_t& _lvls;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer for to hold onto the generated next level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    optional<typename Policy::label_type> _next_level;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    multi_quantify_policy__generator(const generator_t& g)
      : _lvls(g)
    {
      _next_level = _lvls();
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the generator wants to do a Nested Sweep on the given level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_sweep(const typename Policy::label_type x)
    {
      return x == next_level(x) ? Policy::quantify_onset : !Policy::quantify_onset;
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are more levels for the Nested Sweping framework.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_next_level() const
    {
      return _next_level.has_value();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The next level to start a Nested Sweep.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    typename Policy::label_type
    next_level(const typename Policy::label_type l)
    {
      while (_next_level.has_value() && l < _next_level.value()) { _next_level = _lvls(); }
      return _next_level.value_or(Policy::max_label + 1);
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the deepest variable between `bot_level` and `top_level`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: optimisations
  //       - initial cheap check on is_terminal.
  //       - initial '__quantify__get_deepest' should not terminate early but
  //         determine whether any variable may "survive".
  //       clean up
  //       - Make return type 'optional' rather than larger than 'max_label'
  template <typename Policy>
  inline typename Policy::label_type
  __quantify__get_deepest(const typename Policy::dd_type& dd,
                          const typename Policy::label_type bot_level,
                          const optional<typename Policy::label_type> top_level)
  {
    level_info_ifstream<true /* bottom-up */> lis(dd);

    while (lis.can_pull()) {
      const typename Policy::label_type l = lis.pull().label();
      if ((!top_level || top_level.value() < l) && l < bot_level) { return l; }
    }
    return Policy::max_label + 1;
  }

  template <typename Policy>
  typename Policy::__dd_type
  quantify(const exec_policy& ep,
           typename Policy::dd_type dd,
           const typename multi_quantify_policy__generator<Policy>::generator_t& lvls)
  {
#ifdef ADIAR_STATS
    stats_quantify.runs += 1u;
#endif

    switch (ep.template get<exec_policy::quantify::algorithm>()) {
    case exec_policy::quantify::Singleton: {
      // -------------------------------------------------------------------------------------------
      // Case: Repeated single variable quantification
      // TODO: correctly handle Policy::quantify_onset
      optional<typename Policy::label_type> on_level = lvls();

      if (Policy::quantify_onset) {
        if (!on_level) {
#ifdef ADIAR_STATS
          stats_quantify.skipped += 1u;
#endif
          return dd;
        }

        // Quantify all but the last 'on_level'. Hence, look one ahead with
        // 'next_on_level' to see whether it is the last one.
        optional<typename Policy::label_type> next_on_level = lvls();
        while (next_on_level) {
          dd = quantify<Policy>(ep, dd, on_level.value());
#ifdef ADIAR_STATS
          // HACK: Undo the += 1 in the nested call
          stats_quantify.runs -= 1u;
#endif
          if (dd_isterminal(dd)) { return dd; }

          on_level      = next_on_level;
          next_on_level = lvls();
        }
        const typename Policy::__dd_type out = quantify<Policy>(ep, dd, on_level.value());
#ifdef ADIAR_STATS
        // HACK: Undo the += 1 in the nested call
        stats_quantify.runs -= 1u;
#endif
        return out;
      } else { // !Policy::quantify_onset
        // TODO: only designed for 'OR' at this point in time
        if (!on_level) { return typename Policy::dd_type(dd->number_of_terminals[true] > 0); }

        // Quantify everything below 'label'
        for (;;) {
          const typename Policy::label_type off_level =
            __quantify__get_deepest<Policy>(dd, Policy::max_label, on_level.value());

          if (Policy::max_label < off_level) { break; }

          dd = quantify<Policy>(ep, dd, off_level);
#ifdef ADIAR_STATS
          // HACK: Undo the += 1 in the nested call
          stats_quantify.runs -= 1u;
#endif
          if (dd_isterminal(dd)) { return dd; }
        }

        // Quantify everything strictly in between 'bot_level' and 'top_level'
        optional<typename Policy::label_type> bot_level = on_level;
        optional<typename Policy::label_type> top_level = lvls();

        while (bot_level) {
          for (;;) {
            const typename Policy::label_type off_level =
              __quantify__get_deepest<Policy>(dd, bot_level.value(), top_level);

            if (Policy::max_label < off_level) { break; }

            dd = quantify<Policy>(ep, dd, off_level);
#ifdef ADIAR_STATS
            // HACK: Undo the += 1 in the nested call
            stats_quantify.runs -= 1u;
#endif
            if (dd_isterminal(dd)) { return dd; }
          }

          bot_level = top_level;
          top_level = lvls();
        }
        return dd;
      }
    }

    case exec_policy::quantify::Nested: {
      // -------------------------------------------------------------------------------------------
      // Case: Nested Sweeping

      if constexpr (Policy::quantify_onset) {
        // Obtain the bottom-most onset level that exists in the diagram.
        // TODO: Move into helper function.

        optional<typename Policy::label_type> transposition_level = lvls();
        if (!transposition_level) {
#ifdef ADIAR_STATS
          stats_quantify.skipped += 1u;
#endif
          return dd;
        }

        {
          level_info_ifstream<true> in_meta(dd);
          typename Policy::label_type dd_level = in_meta.pull().level();

          for (;;) {
            // Go forward in the diagram's levels, until we are at or above
            // the current candidate
            while (in_meta.can_pull() && transposition_level.value() < dd_level) {
              dd_level = in_meta.pull().level();
            }
            // There is no onset level in the diagram? If so, then nothing is
            // going to change and we may just return the input.
            if (!in_meta.can_pull() && transposition_level.value() < dd_level) {
#ifdef ADIAR_STATS
              stats_quantify.skipped += 1u;
#endif
              return dd;
            }

            adiar_assert(dd_level <= transposition_level.value(),
                         "Must be at or above candidate level");

            // Did we find the current candidate or skipped past it?
            if (dd_level == transposition_level.value()) {
              break;
            } else { // dd_level < transposition_level
              transposition_level = lvls();

              // Did we run out of 'onset' levels?
              if (!transposition_level) {
#ifdef ADIAR_STATS
                stats_quantify.skipped += 1u;
#endif
                return dd;
              }
            }
          }
        }
        adiar_assert(transposition_level.has_value());

        // Quantify the 'transposition_level' as part of the initial transposition step
        typename Policy::__dd_type transposed =
          quantify<Policy>(ep, dd, transposition_level.value());

#ifdef ADIAR_STATS
        stats_quantify.nested_transposition.singleton += 1u;
        stats_quantify.nested_sweeps += 1u;
        // HACK: Undo the two += 1 in the nested call
        stats_quantify.runs -= 1u;
        stats_quantify.singleton_sweeps -= 1u;
#endif
        multi_quantify_policy__generator<Policy> inner_impl(lvls);
        return nested_sweep<>(ep, std::move(transposed), inner_impl);
      } else { // !Policy::quantify_onset
#ifdef ADIAR_STATS
        stats_quantify.nested_transposition.simple += 1u;
        stats_quantify.nested_sweeps += 1u;
#endif
        multi_quantify_policy__generator<Policy> inner_impl(lvls);
        return nested_sweep<>(ep, dd, inner_impl);
      }
    }

      // LCOV_EXCL_START
    default:
      // -------------------------------------------------------------------------------------------
      adiar_unreachable();
      // LCOV_EXCL_STOP
    }
  }

  template <typename Policy>
  typename Policy::__dd_type
  quantify(const exec_policy& ep,
           typename Policy::__dd_type&& __dd,
           const typename multi_quantify_policy__generator<Policy>::generator_t& lvls)
  {
    switch (ep.template get<exec_policy::quantify::algorithm>()) {
    case exec_policy::quantify::Singleton: {
      // -------------------------------------------------------------------------------------------
      // Case: Repeated single variable quantification
      return quantify<Policy>(ep, typename Policy::dd_type(std::move(__dd)), lvls);
    }
    case exec_policy::quantify::Nested: {
      // -------------------------------------------------------------------------------------------
      // Case: Nested Sweeping
      if (__dd.template has<typename Policy::shared_node_file_type>()) {
        return quantify<Policy>(ep, typename Policy::dd_type(std::move(__dd)), lvls);
      }

#ifdef ADIAR_STATS
      stats_quantify.runs += 1u;
      stats_quantify.nested_transposition.none += 1u;
      stats_quantify.nested_sweeps += 1u;
#endif
      multi_quantify_policy__generator<Policy> inner_impl(lvls);
      return nested_sweep<>(ep, std::move(__dd), inner_impl);
    }

      // LCOV_EXCL_START
    default:
      // -------------------------------------------------------------------------------------------
      adiar_unreachable();
      // LCOV_EXCL_STOP
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_QUANTIFY_H
