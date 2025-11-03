#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/types.h>

#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/algorithms/prod2.h>
#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/algorithms/replace.h>
#include <adiar/internal/bool_op.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Product Construction policy for the BDD Relational Product.
  ///
  /// \details To improve performance, the `bdd_and` can be replaced with a special version that
  ///          simultaneously prunes its subtrees based on the quantification predicate.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename LevelPredicate>
  class relprod_prod2_policy
    : public bdd_policy
    , public internal::and_op
    , public internal::prod2_mixed_level_merger<bdd_policy>
  {
  private:
    const LevelPredicate& _pred;
    bool _prune_level = false;

  public:
    relprod_prod2_policy(const LevelPredicate& pred)
      : _pred(pred)
    {}

  public:
    void
    setup_next_level(const bdd::label_type next_level)
    {
      using result_type           = typename LevelPredicate::result_type;
      constexpr bool is_total_map = is_convertible<result_type, label_type>;

      if constexpr (is_total_map) {
        this->_prune_level = this->_pred(next_level);
      } else {
        this->_prune_level = !this->_pred(next_level).has_value();
      }
    }

  public:
    /// \brief Flip internal copy of operator
    void
    flip()
    {
      adiar_assert(internal::and_op::is_commutative());
      /* Do nothing...  */
    }

  public:
    /// \brief Hook for case of two BDDs with the same node file.
    __bdd
    resolve_same_file(const bdd& bdd_1, const bdd& bdd_2) const
    {
      return bdd_1.is_negated() == bdd_2.is_negated() ? bdd_1 : bdd_terminal(false);
    }

    /// \brief Hook for either of the two BDDs being a terminal.
    __bdd
    resolve_terminal_root(const bdd& bdd_1, const bdd& bdd_2) const
    {
      adiar_assert(bdd_isterminal(bdd_1) || bdd_isterminal(bdd_2));

      if (bdd_isterminal(bdd_1) && bdd_isterminal(bdd_2)) {
        return bdd_terminal(internal::and_op::operator()(dd_valueof(bdd_1), dd_valueof(bdd_2)));
      } else if (bdd_isterminal(bdd_1) && dd_valueof(bdd_1)) {
        adiar_assert(internal::and_op::is_left_idempotent(dd_valueof(bdd_1)));
        return bdd_2;
      } else if (bdd_isterminal(bdd_2) && dd_valueof(bdd_2)) {
        adiar_assert(internal::and_op::is_right_idempotent(dd_valueof(bdd_2)));
        return bdd_1;
      }
      // and_op::can_left_shortcut(bdd_1) || and_op::can_right_shortcut(bdd_2)
      return bdd_terminal(false);
    }

  private:
    /// \brief Applies shortcutting on a recursion target, if possible.
    pair<internal::tuple<bdd::pointer_type>, bool>
    __resolve_request(const internal::tuple<bdd::pointer_type>& r) const
    {
      if (internal::and_op::can_left_shortcut(r[0]) || internal::and_op::can_right_shortcut(r[1])) {
        return { { bdd::pointer_type(false), bdd::pointer_type(false) }, true };
      }
      return { r, false };
    }

    bool
    __is_true(const internal::tuple<bdd::pointer_type>& t) const
    {
      return (t[0] == bdd::pointer_type(true) && t[1] == bdd::pointer_type(true));
    }

  public:
    /// \brief Hook for changing the targets of a new node's children.
    internal::prod2_rec
    resolve_request(const internal::tuple<bdd::pointer_type>& r_low,
                    const internal::tuple<bdd::pointer_type>& r_high) const
    {
      const auto [low, low_false]   = this->__resolve_request(r_low);
      const auto [high, high_false] = this->__resolve_request(r_high);

      // Prune subtree(s) if either child is 'false' and is quantified later.
      if (this->_prune_level && (low_false || high_false)) {
        return internal::prod2_rec_skipto{ low_false ? high : low };
      }
      // Prune subtree(s) if either child is 'true' and is quantified later.
      if (this->_prune_level && (this->__is_true(low) || this->__is_true(high))) {
        return internal::prod2_rec_skipto{ bdd::pointer_type(true) };
      }
      // Skip node with duplicated children requests
      //
      // NOTE: One could think to also prune 'redundant' nodes. Yet, these cannot be created as the
      //       tuples `(f1,f2)` and `(g1,g2)` could only have `(f1,g1) = (f2,g2)` if `f1 = f2` and
      //       `g1 = g2`.
      //
      //       The only exception would be if `f1`, `f2`, `g1`, and `g2` are terminals. Yet, with
      //       the would 'and' operation, this can only happen if they are shortcutting to the
      //       'false' terminal. Yet, this would only happen at the very bottom. With most variable
      //       orderings, this would be the next-state variable at the very bottom.
      //
      //       In practice, it seems very unlikely these few edges are not cheaper to handle, than
      //       having this additional branch in the code (even if it is very predictable).
      /*
      if (low == high) { return internal::prod2_rec_skipto{ low }; }
      */

      return internal::prod2_rec_output{ low, high };
    }

    /// \brief Hook for applying an operator to a pair of terminals.
    bdd::pointer_type
    operator()(const bdd::pointer_type& a, const bdd::pointer_type& b) const
    {
      return internal::and_op::operator()(a, b);
    }

  public:
    /// \brief Hook for deriving the cut type of the left-hand-side.
    internal::cut
    left_cut() const
    {
      return internal::cut(!internal::and_op::can_left_shortcut(false),
                           !internal::and_op::can_left_shortcut(true));
    }

    /// \brief Hook for deriving the cut type of the right-hand-side.
    internal::cut
    right_cut() const
    {
      return internal::cut(!internal::and_op::can_right_shortcut(false),
                           !internal::and_op::can_right_shortcut(true));
    }

    /// \brief Due to pruning, this prod2 policy may introduce skipping of nodes.
    static constexpr bool no_skip = false;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename LevelPredicate>
  __bdd
  bdd_relprod__and(const exec_policy& ep,
                   const bdd& states,
                   const bdd& relation,
                   const LevelPredicate& pred)
  {
    relprod_prod2_policy<LevelPredicate> policy(pred);
    return internal::prod2(ep, states, relation, policy);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprod(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const predicate<bdd::label_type>& pred)
  {
    return bdd_exists(ep, bdd_relprod__and(ep, states, relation, pred), pred);
  }

  bdd
  bdd_relprod(const bdd& states, const bdd& relation, const predicate<bdd::label_type>& m)
  {
    return bdd_relprod(exec_policy(), states, relation, m);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Existential Quantification policy for BDD Relational Product.
  ///
  /// \details This is a one-to-one copy of the 'bdd_quantify_policy' in 'adiar/bdd/quantify.cpp'.
  ///          We ought to reuse it instead!
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class relnext_quantify_policy : public bdd_policy
  {
  public:
    static inline bdd::pointer_type
    resolve_root(const bdd::node_type& r)
    {
      // TODO: should all but the last case not have a 'suppression taint'?

      // Return shortcutting terminal (including its tainting flag).
      if (essential(r.low()) == bdd::pointer_type(true)) { return r.low(); }
      if (essential(r.high()) == bdd::pointer_type(true)) { return r.high(); }

      // Return other child (including its tainting flag) for irrelevant terminals.
      if (essential(r.low()) == bdd::pointer_type(false)) { return r.high(); }
      if (essential(r.high()) == bdd::pointer_type(false)) { return r.low(); }

      // Otherwise return 'nothing'
      return r.uid();
    }

  public:
    static inline bool
    keep_terminal(const bdd::pointer_type& p)
    {
      return essential(p) != bdd::pointer_type(false);
    }

    static inline bool
    collapse_to_terminal(const bdd::pointer_type& p)
    {
      return essential(p) == bdd::pointer_type(true);
    }

    // LCOV_EXCL_START
    static inline bdd::pointer_type
    resolve_terminals(const bdd::pointer_type& /*a*/, const bdd::pointer_type& /*b*/)
    {
      // Since only a single terminal terminal survives, this piece of code is never executed.
      adiar_unreachable();
    }

    // LCOV_EXCL_STOP

  public:
    static inline internal::cut
    cut_with_terminals()
    {
      return internal::cut(false, true);
    }

  public:
    static constexpr bool quantify_onset = true;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Existential Quantification and Variable Renaming for the Next-state variant of the
  ///          BDD Relational Product.
  ///
  /// \details To improve performance of the `bdd_relnext`, we can merge the `bdd_exists` and
  ///          `bdd_replace` operations.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class relnext_quantify_replace_policy
    : public internal::multi_quantify_policy<relnext_quantify_policy>
  {
  private:
    const function<optional<bdd::label_type>(bdd::label_type)>& _m;

  public:
    relnext_quantify_replace_policy(const function<optional<bdd::label_type>(bdd::label_type)>& m)
      : _m(m)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Variable remapping of.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline bdd::label_type
    map_level(bdd::label_type x) const
    {
      const optional<bdd::label_type> new_x = this->_m(x);
      return new_x.has_value() ? new_x.value() : bdd::max_label + 1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the generator wants to sweep on the given level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool
    has_sweep(const bdd::label_type x) const
    {
      return !this->_m(x).has_value();
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type)
  {
    __bdd tmp_1 = bdd_relprod__and(ep, states, relation, m);

    switch (m_type) {
    case replace_type::Non_Monotone:
#ifdef ADIAR_STATS
      internal::stats_replace.nested_sweeps += 1u;
#endif
      throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");

    case replace_type::Monotone:
    case replace_type::Shift:
    case replace_type::Identity:
#ifdef ADIAR_STATS
      internal::stats_replace.monotonic_reduces += 1u;
      internal::stats_quantify.runs += 1u;
      internal::stats_quantify.nested_sweeps += 1u;
      internal::stats_quantify.nested_transposition.none += 1;
#endif
      relnext_quantify_replace_policy quantify_replace_policy(m);
      return internal::nested_sweep<>(ep, std::move(tmp_1), quantify_replace_policy);
    }
    adiar_unreachable(); // LCOV_EXCL_LINE
  }

  bdd
  bdd_relnext(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type)
  {
    return bdd_relnext(exec_policy(), states, relation, m, m_type);
  }

  bdd
  bdd_relnext(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m)
  {
    replace_type m_type;
    {
      internal::level_info_ifstream<false> ls(relation);
      m_type = internal::__replace__infer_type<bdd_policy>(ls, m);
    }
    return bdd_relnext(ep, states, relation, m, m_type);
  }

  bdd
  bdd_relnext(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m)
  {
    return bdd_relnext(exec_policy(), states, relation, m);
  }

  bdd
  bdd_relnext(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const bdd::label_type varcount)
  {
    const auto map = [=](const bdd::label_type x) -> adiar::optional<int> {
      return x < varcount ? adiar::make_optional<int>() : adiar::make_optional<int>(x - varcount);
    };
    return bdd_relnext(ep, states, relation, map, replace_type::Shift);
  }

  bdd
  bdd_relnext(const bdd& states, const bdd& relation, const bdd::label_type varcount)
  {
    return bdd_relnext(exec_policy(), states, relation, varcount);
  }

  bdd
  bdd_relnext(const exec_policy& ep, const bdd& states, const bdd& relation)
  {
    const auto map = [](const bdd::label_type x) -> adiar::optional<int> {
      return (x % 2) == 0 ? adiar::make_optional<int>() : adiar::make_optional<int>(x - 1);
    };
    return bdd_relnext(ep, states, relation, map, replace_type::Shift);
  }

  bdd
  bdd_relnext(const bdd& states, const bdd& relation)
  {
    return bdd_relnext(exec_policy(), states, relation);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type)
  {
    const bdd tmp_1 =
      bdd_replace(ep, states, [&m](bdd::label_type x) { return m(x).value(); }, m_type);

    __bdd tmp_2 = bdd_relprod__and(ep, std::move(tmp_1), relation, m);

    const bdd tmp_3 =
      bdd_exists(ep, std::move(tmp_2), [&m](bdd::label_type x) { return !m(x).has_value(); });

    return tmp_3;
  }

  bdd
  bdd_relprev(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type)
  {
    return bdd_relprev(exec_policy(), states, relation, m, m_type);
  }

  bdd
  bdd_relprev(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m)
  {
    replace_type m_type;
    {
      internal::level_info_ifstream<false> ls(states);
      m_type = internal::__replace__infer_type<bdd_policy>(ls, m);
    }

    return bdd_relprev(ep, states, relation, m, m_type);
  }

  bdd
  bdd_relprev(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m)
  {
    return bdd_relprev(exec_policy(), states, relation, m);
  }

  bdd
  bdd_relprev(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const bdd::label_type varcount)
  {
    const auto map = [=](const bdd::label_type x) -> adiar::optional<int> {
      return varcount <= x ? adiar::make_optional<int>() : adiar::make_optional<int>(x + varcount);
    };
    return bdd_relprev(ep, states, relation, map, replace_type::Shift);
  }

  bdd
  bdd_relprev(const bdd& states, const bdd& relation, const bdd::label_type varcount)
  {
    return bdd_relprev(exec_policy(), states, relation, varcount);
  }

  bdd
  bdd_relprev(const exec_policy& ep, const bdd& states, const bdd& relation)
  {
    const auto map = [](const bdd::label_type x) -> adiar::optional<int> {
      return (x % 2) == 1 ? adiar::make_optional<int>() : adiar::make_optional<int>(x + 1);
    };
    return bdd_relprev(ep, states, relation, map, replace_type::Shift);
  }

  bdd
  bdd_relprev(const bdd& states, const bdd& relation)
  {
    return bdd_relprev(exec_policy(), states, relation);
  }
}
