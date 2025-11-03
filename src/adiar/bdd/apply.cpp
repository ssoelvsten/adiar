#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/algorithms/prod2b.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/bool_op.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/unreachable.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // BDD product construction policy
  template <typename BinaryOp = internal::binary_op<predicate<bool, bool>>>
  class apply_prod2b_policy
    : public bdd_policy
    , public internal::prod2b_mixed_level_merger<bdd_policy>
  {
  private:
    /// \brief Operator
    BinaryOp _op;

  public:
    /// \brief Constructor for binary operators known at compile-time.
    apply_prod2b_policy() = default;

    /// \brief Constructor for wrapping an unknown binary operator.
    apply_prod2b_policy(const predicate<bool, bool>& op)
      : _op(op)
    {}

  public:
    void
    setup_next_level(const bdd::label_type /*next_level*/) const
    {}

  public:
    /// \brief Flip internal copy of operator
    void
    flip()
    {
      this->_op = this->_op.flip();
    }

  public:
    /// \brief Hook for case of two BDDs with the same node file.
    __bdd
    resolve_same_file(const bdd& bdd_1, const bdd& bdd_2) const
    {
      // Compute the results on all children.
      const bool op_F = this->_op(bdd_1.is_negated(), bdd_2.is_negated());
      const bool op_T = this->_op(!bdd_1.is_negated(), !bdd_2.is_negated());

      // Does it collapse to a terminal?
      if (op_F == op_T) { return bdd_terminal(op_F); }

      return op_F == bdd_1.is_negated() ? bdd_1 : ~bdd_1;
    }

    /// \brief Hook for either of the two BDDs being a terminal.
    __bdd
    resolve_terminal_root(const bdd& bdd_1, const bdd& bdd_2) const
    {
      adiar_assert(bdd_isterminal(bdd_1) || bdd_isterminal(bdd_2));

      if (bdd_isterminal(bdd_1) && bdd_isterminal(bdd_2)) {
        const bool p1 = dd_valueof(bdd_1);
        const bool p2 = dd_valueof(bdd_2);

        return bdd_terminal(this->_op(p1, p2));
      } else if (bdd_isterminal(bdd_1)) {
        const bool p1 = dd_valueof(bdd_1);

        if (this->_op.can_left_shortcut(p1)) { return bdd_terminal(this->_op(p1, false)); }
        if (this->_op.is_left_idempotent(p1)) { return bdd_2; }
        // if (is_left_negating(op, p1))
        return bdd_not(bdd_2);
      } else { // if (bdd_isterminal(bdd_2)) {
        const bool p2 = dd_valueof(bdd_2);

        if (this->_op.can_right_shortcut(p2)) { return bdd_terminal(this->_op(false, p2)); }
        if (this->_op.is_right_idempotent(p2)) { return bdd_1; }
        // if (is_right_negating(op, p2))
        return bdd_not(bdd_1);
      }
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

  private:
    /// \brief Applies shortcutting on a recursion target, if possible.
    internal::tuple<bdd::pointer_type>
    __resolve_request(const internal::tuple<bdd::pointer_type>& r) const
    {
      if (this->_op.can_left_shortcut(r[0])) { return { r[0], bdd::pointer_type(true) }; }
      if (this->_op.can_right_shortcut(r[1])) { return { bdd::pointer_type(true), r[1] }; }
      return r;
    }

  public:
    /// \brief Hook for changing the targets of a new node's children.
    internal::prod2b_rec
    resolve_request(const internal::tuple<bdd::pointer_type>& r_low,
                    const internal::tuple<bdd::pointer_type>& r_high) const
    {
      return internal::prod2b_rec_output{ __resolve_request(r_low), __resolve_request(r_high) };
    }

    /// \brief Hook for applying an operator to a pair of terminals.
    bdd::pointer_type
    operator()(const bdd::pointer_type& a, const bdd::pointer_type& b) const
    {
      return this->_op(a, b);
    }

  public:
    /// \brief Hook for deriving the cut type of the left-hand-side.
    internal::cut
    left_cut() const
    {
      const bool incl_false = !this->_op.can_left_shortcut(false);
      const bool incl_true  = !this->_op.can_left_shortcut(true);

      return internal::cut(incl_false, incl_true);
    }

    /// \brief Hook for deriving the cut type of the right-hand-side.
    internal::cut
    right_cut() const
    {
      const bool incl_false = !this->_op.can_right_shortcut(false);
      const bool incl_true  = !this->_op.can_right_shortcut(true);

      return internal::cut(incl_false, incl_true);
    }

    /// \brief Whether this policy may introduce skipping of nodes.
    ///
    /// \detail This variable can be used at compile-time to prune conditional statements.
    static constexpr bool no_skip = true;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_apply(const exec_policy& ep, const bdd& f, const bdd& g, const predicate<bool, bool>& op)
  {
    apply_prod2b_policy<internal::binary_op<predicate<bool, bool>>> policy(op);
    return internal::prod2b(ep, f, g, policy);
  }

  __bdd
  bdd_apply(const bdd& f, const bdd& g, const predicate<bool, bool>& op)
  {
    return bdd_apply(exec_policy(), f, g, op);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_and(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    apply_prod2b_policy<internal::and_op> policy;
    return internal::prod2b(ep, f, g, policy);
  }

  __bdd
  bdd_and(const bdd& f, const bdd& g)
  {
    return bdd_and(exec_policy(), f, g);
  }

  __bdd
  bdd_nand(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    apply_prod2b_policy<internal::nand_op> policy;
    return internal::prod2b(ep, f, g, policy);
  }

  __bdd
  bdd_nand(const bdd& f, const bdd& g)
  {
    return bdd_nand(exec_policy(), f, g);
  }

  __bdd
  bdd_or(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    apply_prod2b_policy<internal::or_op> policy;
    return internal::prod2b(ep, f, g, policy);
  }

  __bdd
  bdd_or(const bdd& f, const bdd& g)
  {
    return bdd_or(exec_policy(), f, g);
  }

  __bdd
  bdd_nor(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    apply_prod2b_policy<internal::nor_op> policy;
    return internal::prod2b(ep, f, g, policy);
  }

  __bdd
  bdd_nor(const bdd& f, const bdd& g)
  {
    return bdd_nor(exec_policy(), f, g);
  }

  __bdd
  bdd_xor(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    apply_prod2b_policy<internal::xor_op> policy;
    return internal::prod2b(ep, f, g, policy);
  }

  __bdd
  bdd_xor(const bdd& f, const bdd& g)
  {
    return bdd_xor(exec_policy(), f, g);
  }

  __bdd
  bdd_xnor(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    apply_prod2b_policy<internal::xnor_op> policy;
    return internal::prod2b(ep, f, g, policy);
  }

  __bdd
  bdd_xnor(const bdd& f, const bdd& g)
  {
    return bdd_xnor(exec_policy(), f, g);
  }

  __bdd
  bdd_imp(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, imp_op);
  }

  __bdd
  bdd_imp(const bdd& f, const bdd& g)
  {
    return bdd_imp(exec_policy(), f, g);
  }

  __bdd
  bdd_invimp(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, invimp_op);
  }

  __bdd
  bdd_invimp(const bdd& f, const bdd& g)
  {
    return bdd_invimp(exec_policy(), f, g);
  }

  __bdd
  bdd_equiv(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    apply_prod2b_policy<internal::equiv_op> policy;
    return internal::prod2b(ep, f, g, policy);
  }

  __bdd
  bdd_equiv(const bdd& f, const bdd& g)
  {
    return bdd_equiv(exec_policy(), f, g);
  }

  __bdd
  bdd_diff(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, diff_op);
  }

  __bdd
  bdd_diff(const bdd& f, const bdd& g)
  {
    return bdd_diff(exec_policy(), f, g);
  }

  __bdd
  bdd_less(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return bdd_apply(ep, f, g, less_op);
  }

  __bdd
  bdd_less(const bdd& f, const bdd& g)
  {
    return bdd_less(exec_policy(), f, g);
  }
};
