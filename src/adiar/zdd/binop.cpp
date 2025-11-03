#include <array>
#include <limits>

#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/prod2b.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/bool_op.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/dd_func.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // ZDD product construction policy
  template <typename BinaryOp = internal::binary_op<predicate<bool, bool>>>
  class zdd_prod2b_policy
    : public zdd_policy
    , public internal::prod2b_mixed_level_merger<zdd_policy>
  {
  private:
    /// \brief Operator
    BinaryOp _op;

    // TODO: replace `_can_..._shortcut` with a `zdd::pointer_type` that is either an (essential)
    //       pointer or nil. This way, one can remove the conditional on '.is_terminal()'.

    /// \brief Pre-computed ZDD-specific variant of 'can left shourtcut'.
    std::array<bool, 2> _can_left_shortcut;

    /// \brief Pre-computed ZDD-specific variant of 'can right shourtcut'.
    std::array<bool, 2> _can_right_shortcut;

    /// \brief Computes an entry in `_can_left_shortcut`
    static bool
    can_left_shortcut(const BinaryOp& op, const bool terminal)
    {
      return // Does it shortcut on this level?
        op(terminal, false) == false
        && op(terminal, true) == false
        // Does it shortcut on all other levels below?
        && op(false, false) == false && op(false, true) == false;
    }

    static std::array<bool, 2>
    can_left_shortcut(const BinaryOp& op)
    {
      return { zdd_prod2b_policy::can_left_shortcut(op, false),
               zdd_prod2b_policy::can_left_shortcut(op, true) };
    }

    /// \brief Computes an entry in `_can_right_shortcut`
    static bool
    can_right_shortcut(const BinaryOp& op, const bool terminal)
    {
      return // Does it shortcut on this level?
        op(false, terminal) == false
        && op(true, terminal) == false
        // Does it shortcut on all other levels below?
        && op(false, false) == false && op(true, false) == false;
    }

    static std::array<bool, 2>
    can_right_shortcut(const BinaryOp& op)
    {
      return { zdd_prod2b_policy::can_right_shortcut(op, false),
               zdd_prod2b_policy::can_right_shortcut(op, true) };
    }

  public:
    zdd_prod2b_policy()
      : _op()
      , _can_left_shortcut{ zdd_prod2b_policy::can_left_shortcut(this->_op) }
      , _can_right_shortcut{ zdd_prod2b_policy::can_right_shortcut(this->_op) }
    {}

    zdd_prod2b_policy(const BinaryOp& op)
      : _op(op)
      , _can_left_shortcut{ zdd_prod2b_policy::can_left_shortcut(op) }
      , _can_right_shortcut{ zdd_prod2b_policy::can_right_shortcut(op) }
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

      const std::array<bool, 2> tmp = this->_can_right_shortcut;
      this->_can_right_shortcut     = std::move(this->_can_left_shortcut);
      this->_can_left_shortcut      = std::move(tmp);
    }

  public:
    /// \brief Hook for case of two BDDs with the same node file.
    __zdd
    resolve_same_file(const zdd& zdd_1, const zdd& /*zdd_2*/) const
    {
      // Compute the results on all children.
      const bool op_F = this->_op(false, false);
      const bool op_T = this->_op(true, true);

      // Does it collapse to a terminal?
      if (op_F == op_T) { return zdd_terminal(op_F); }

      return zdd_1;
    }

    /// \brief Hook for either of the two BDDs being a terminal.
    __zdd
    resolve_terminal_root(const zdd& zdd_1, const zdd& zdd_2) const
    {
      adiar_assert(zdd_isterminal(zdd_1) || zdd_isterminal(zdd_2));

      if (zdd_isterminal(zdd_1) && zdd_isterminal(zdd_2)) {
        const bool p1 = dd_valueof(zdd_1);
        const bool p2 = dd_valueof(zdd_2);

        return zdd_terminal(this->_op(p1, p2));
      } else if (zdd_isterminal(zdd_1)) {
        const bool p1 = dd_valueof(zdd_1);

        if (this->_can_left_shortcut[p1]) {
          // Shortcuts the left-most path to {Ø} and all others to Ø
          return zdd_terminal(false);
        } else if (this->_op.is_left_idempotent(p1) && this->_op.is_left_idempotent(false)) {
          // Has no change to left-most path to {Ø} and neither any others
          return zdd_2;
        }
      } else { // if (is_terminal(zdd_2)) {
        const bool p2 = dd_valueof(zdd_2);

        if (this->_can_right_shortcut[p2]) {
          // Shortcuts the left-most path to {Ø} and all others to Ø
          return zdd_terminal(false);
        } else if (this->_op.is_right_idempotent(p2) && this->_op.is_right_idempotent(false)) {
          // Has no change to left-most path to {Ø} and neither any others
          return zdd_1;
        }
      }
      return __zdd(); // return with no_file, i.e. proceed with the product construction.
    }

  private:
    /// \brief Applies shortcutting on a recursion target, if possible.
    internal::tuple<zdd::pointer_type>
    __resolve_request(const internal::tuple<zdd::pointer_type>& r) const
    {
      if (r[0].is_terminal() && this->_can_left_shortcut[r[0].value()]) {
        return { r[0], zdd::pointer_type(true) };
      }
      if (r[1].is_terminal() && this->_can_right_shortcut[r[1].value()]) {
        return { zdd::pointer_type(true), r[1] };
      }
      return r;
    }

  public:
    /// \brief Hook for changing the targets of a new node's children.
    internal::prod2b_rec
    resolve_request(const internal::tuple<zdd::pointer_type>& r_low,
                    const internal::tuple<zdd::pointer_type>& r_high) const
    {

      /// Whether a potential ZDD node should be skipped.
      const bool skippable = (r_high[0].is_terminal() && r_high[1].is_terminal()
                              && this->_op(r_high[0], r_high[1]) == zdd::pointer_type(false))
        || (r_high[0].is_terminal() && this->_can_left_shortcut[r_high[0].value()])
        || (r_high[1].is_terminal() && this->_can_right_shortcut[r_high[1].value()]);

      // If r_high surely suppresses the node during the ZDD Reduce, then skip
      // creating it in the first place and just forward the edge to r_low.
      if (skippable) {
        // TODO: 'r_low' => '__resolve_request(r_low)' ?
        return internal::prod2b_rec_skipto(r_low);
      }

      // Otherwise, create a node with children r_low and r_high
      return internal::prod2b_rec_output{ __resolve_request(r_low), __resolve_request(r_high) };
    }

    /// \brief Hook for applying an operator to a pair of terminals.
    zdd::pointer_type
    operator()(const zdd::pointer_type& a, const zdd::pointer_type& b) const
    {
      return this->_op(a, b);
    }

  public:
    /// \brief Hook for deriving the cut type of the left-hand-side.
    internal::cut
    left_cut() const
    {
      const bool incl_false = !this->_can_left_shortcut[false];
      const bool incl_true  = !this->_can_left_shortcut[true];

      return internal::cut(incl_false, incl_true);
    }

    /// \brief Hook for deriving the cut type of the right-hand-side.
    internal::cut
    right_cut() const
    {
      const bool incl_false = !this->_can_right_shortcut[false];
      const bool incl_true  = !this->_can_right_shortcut[true];

      return internal::cut(incl_false, incl_true);
    }

    /// \brief Whether this policy may introduce skipping of nodes.
    ///
    /// \detail This variable can be used at compile-time to prune conditional statements.
    static constexpr bool no_skip = false;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  __zdd
  zdd_binop(const exec_policy& ep, const zdd& A, const zdd& B, const predicate<bool, bool>& op)
  {
    zdd_prod2b_policy<internal::binary_op<predicate<bool, bool>>> policy(op);
    return internal::prod2b(ep, A, B, policy);
  }

  __zdd
  zdd_binop(const zdd& A, const zdd& B, const predicate<bool, bool>& op)
  {
    return zdd_binop(exec_policy(), A, B, op);
  }

  __zdd
  zdd_union(const exec_policy& ep, const zdd& A, const zdd& B)
  {
    zdd_prod2b_policy<internal::or_op> policy;
    return internal::prod2b(ep, A, B, policy);
  }

  __zdd
  zdd_union(const zdd& A, const zdd& B)
  {
    return zdd_union(exec_policy(), A, B);
  }

  __zdd
  zdd_intsec(const exec_policy& ep, const zdd& A, const zdd& B)
  {
    zdd_prod2b_policy<internal::and_op> policy;
    return internal::prod2b(ep, A, B, policy);
  }

  __zdd
  zdd_intsec(const zdd& A, const zdd& B)
  {
    return zdd_intsec(exec_policy(), A, B);
  }

  __zdd
  zdd_diff(const exec_policy& ep, const zdd& A, const zdd& B)
  {
    return zdd_binop(ep, A, B, diff_op);
  }

  __zdd
  zdd_diff(const zdd& A, const zdd& B)
  {
    return zdd_diff(exec_policy(), A, B);
  }
}
