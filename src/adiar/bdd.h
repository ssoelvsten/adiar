#ifndef ADIAR_BDD_H
#define ADIAR_BDD_H

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__bdd Binary Decision Diagrams
///
/// \brief A Binary Decision Diagram (BDD) represents a boolean function \f$ \{ 0, 1 \}^n
/// \rightarrow \{ 0, 1 \} \f$ over a finite domain of \f$ n \f$ boolean input variables.
///
/// The \ref bdd class takes care of reference counting and optimal garbage collection of the
/// underlying files. To ensure the most disk-space is available, try to release your \ref bdd
/// objects as quickly as possible and/or minimise the number of lvalues of said type.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <type_traits>

#include <adiar/bdd/bdd.h>
#include <adiar/bool_op.h>
#include <adiar/exec_policy.h>
#include <adiar/functional.h>
#include <adiar/types.h>
#include <adiar/zdd/zdd.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__bdd
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Basic Constructors
  ///
  /// To construct a more complex but well-structured \ref bdd by hand, please
  /// use the \ref bdd_builder (see \ref builder) instead.
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the given constant value.
  ///
  /// \param value
  ///    The constant boolean value
  ///
  /// \see bdd_false bdd_true
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_const(bool value);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief       The BDD representing the given constant value.
  ///
  /// \param value
  ///    The constant boolean (terminal) value
  ///
  /// \see bdd_const bdd_false bdd_true
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_terminal(bool value);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the constant `false`.
  ///
  /// \returns \f$ \bot \f$
  ///
  /// \see bdd_bot
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_false();

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The bottom of the powerset lattice.
  ///
  /// \returns \f$ \bot \f$
  ///
  /// \see bdd_false
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bdd
  bdd_bot()
  {
    return bdd_false();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the constant `true`.
  ///
  /// \returns \f$ \top \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_true();

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The top of the powerset lattice.
  ///
  /// \returns \f$ \top \f$
  ///
  /// \see bdd_true
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline bdd
  bdd_top()
  {
    return bdd_true();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the i'th variable.
  ///
  /// \param var
  ///    The label of the desired variable. This value must be smaller or equals to
  ///    `bdd::max_label`.
  ///
  /// \returns \f$ x_{var} \f$
  ///
  /// \throws invalid_argument If `var` is a too large value.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_ithvar(bdd::label_type var);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the negation of the i'th variable.
  ///
  /// \param var
  ///    The label of the desired variable. This value must be smaller or equals to /
  ///    `bdd::max_label`.
  ///
  /// \returns \f$ \neg x_{var} \f$
  ///
  /// \throws invalid_argument If `var` is a too large value.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_nithvar(bdd::label_type var);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   The BDD representing the logical 'and' of all the given variables, i.e. a *term* of
  ///          variables.
  ///
  /// \details Any negative labels provided by the generator are interpreted as the negation of
  ///          said variable.
  ///
  /// \param vars
  ///    Generator of labels of variables in \em descending order. These values can at most be
  ///    `bdd::max_label`.
  ///
  /// \returns \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_and(const generator<int>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the logical 'and' of all the given variables, i.e. a *term* of
  ///        variables.
  ///
  /// \param vars
  ///    Generator of pairs (label, negated) in \em descending order. These values can at most be
  ///    `bdd::max_label`.
  ///
  /// \returns \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_and(const generator<pair<bdd::label_type, bool>>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   The BDD representing the logical 'and' of all the given variables, i.e. a *term* of
  ///          variables.
  ///
  /// \details Any negative labels provided by the generator are interpreted as the negation of said
  ///          variable.
  ///
  /// \param begin
  ///    Single-pass forward iterator that provides the variables in \em descending order. All its
  ///    values should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns \f$ \bigwedge_{x \in \mathit{begin} \dots \mathit{end}} x \f$
  ///
  /// \throws invalid_argument If the iterator does not provide values in descending order.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt, typename = enable_if<!is_convertible<ForwardIt, bdd>>>
  bdd
  bdd_and(ForwardIt begin, ForwardIt end)
  {
    return bdd_and(make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   The BDD representing the logical 'or' of all the given variables, i.e. a *clause* of
  ///          variables.
  ///
  /// \details Any negative labels provided by the generator are interpreted as the negation of said
  ///          variable.
  ///
  /// \param vars
  ///    Generator of labels of variables in \em descending order. When These values can at most be
  ///    `bdd::max_label`.
  ///
  /// \returns \f$ \bigvee_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_or(const generator<int>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the logical 'or' of all the given variables, i.e. a *clause*
  ///             of variables.
  ///
  /// \param vars
  ///    Generator of pairs (label, negated) in \em descending order. These values can at most be
  ///    `bdd::max_label`.
  ///
  /// \returns    \f$ \bigvee_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_or(const generator<pair<bdd::label_type, bool>>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   The BDD representing the logical 'or' of all the given variables, i.e. a *clause* of
  ///          variables.
  ///
  /// \details Any negative labels provided by the generator are interpreted as the negation of said
  ///          variable.
  ///
  /// \param begin
  ///    Single-pass forward iterator that provides the variables in \em descending order. All its
  ///    values should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns \f$ \bigwedge_{x \in \mathit{begin} \dots \mathit{end}} x \f$
  ///
  /// \throws invalid_argument If the iterator does not provide values in descending order.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt, typename = enable_if<!is_convertible<ForwardIt, bdd>>>
  bdd
  bdd_or(ForwardIt begin, ForwardIt end)
  {
    return bdd_or(make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief      The BDD representing the cube of all the given variables.
  ///
  /// \details    Any negative labels provided by the generator are interpreted as the negation of
  ///             said variable.
  ///
  /// \param vars
  ///    Generator of labels of variables in \em descending order. These values can at most be
  ///    `bdd::max_label`.
  ///
  /// \returns \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  ///
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_cube(const generator<int>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD representing the cube of all the given variables.
  ///
  /// \param vars
  ///    Generator of pairs (label, negated) in \em descending order. These values can at most be
  ///    `bdd::max_label`.
  ///
  /// \returns \f$ \bigwedge_{x \in \mathit{vars}} x \f$
  ///
  /// \throws invalid_argument If `vars` are not in descending order.
  ///
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_cube(const generator<pair<bdd::label_type, bool>>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   The BDD representing the cube of all the given variables.
  ///
  /// \details Any negative labels provided by the generator are interpreted as the negation of said
  ///          variable.
  ///
  /// \param begin
  ///    Single-pass forward iterator that provides the variables in \em descending order.
  ///    All its values should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns \f$ \bigwedge_{x \in \mathit{begin} \dots \mathit{end}} x \f$
  ///
  /// \throws invalid_argument If the iterator does not provide values in descending order.
  ///
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  bdd
  bdd_cube(ForwardIt begin, ForwardIt end)
  {
    return bdd_cube(make_generator(begin, end));
  }

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Basic Operations
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Negation of a BDD.
  ///
  /// \details Flips the negation flag such that reading nodes with a `node_stream` within Adiar's
  ///          algorithms will on-the-fly change the `false` terminals into the `true` terminals and
  ///          vice versa.
  ///
  /// \returns \f$ \neg f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_not(const bdd& f);

  /// \cond
  bdd
  bdd_not(bdd&& f);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_not
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  operator~(const bdd& f);

  /// \cond
  bdd
  operator~(bdd&& f);
  bdd
  operator~(__bdd&& f);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Apply a binary operator between two BDDs.
  ///
  /// \param f
  ///    BDD for the left-hand-side of the operator
  ///
  /// \param g
  ///    BDD for the right-hand-side of the operator
  ///
  /// \param op
  ///    Binary predicate on `bool` to-be applied.
  ///
  /// \returns \f$ f \mathbin{\mathit{op}} g\f$
  ///
  /// \see bool_op
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_apply(const bdd& f, const bdd& g, const predicate<bool, bool>& op);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Apply a binary operator between two BDDs.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_apply(const exec_policy& ep, const bdd& f, const bdd& g, const predicate<bool, bool>& op);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'and' operator.
  ///
  /// \returns \f$ f \land g \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_and(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'and' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_and(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  operator&(const bdd& lhs, const bdd& rhs);

  /// \cond
  __bdd
  operator&(const bdd&, __bdd&&);
  __bdd
  operator&(__bdd&&, const bdd&);
  __bdd
  operator&(__bdd&&, __bdd&&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_and
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  operator*(const bdd& lhs, const bdd& rhs);

  /// \cond
  __bdd
  operator*(const bdd&, __bdd&&);
  __bdd
  operator*(__bdd&&, const bdd&);
  __bdd
  operator*(__bdd&&, __bdd&&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'nand' operator.
  ///
  /// \returns \f$ \neg (f \land g) \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_nand(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'nand' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_nand(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'or' operator.
  ///
  /// \returns \f$ f \lor g \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_or(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'or' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_or(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_or
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  operator|(const bdd& lhs, const bdd& rhs);

  /// \cond
  __bdd
  operator|(const bdd&, __bdd&&);
  __bdd
  operator|(__bdd&&, const bdd&);
  __bdd
  operator|(__bdd&&, __bdd&&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_or
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  operator+(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_or
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \cond
  __bdd
  operator+(__bdd&& f);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_or
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  operator+(const bdd& lhs, const bdd& rhs);

  /// \cond
  __bdd
  operator+(const bdd&, __bdd&&);
  __bdd
  operator+(__bdd&&, const bdd&);
  __bdd
  operator+(__bdd&&, __bdd&&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'nor' operator.
  ///
  /// \returns \f$ \neg (f \lor g) \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_nor(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'nor' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_nor(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xor' operator.
  ///
  /// \returns \f$ f \oplus g \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_xor(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xor' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_xor(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_xor
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  operator^(const bdd& lhs, const bdd& rhs);

  /// \cond
  __bdd
  operator^(const bdd&, __bdd&&);
  __bdd
  operator^(__bdd&&, const bdd&);
  __bdd
  operator^(__bdd&&, __bdd&&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'xnor' operator.
  ///
  /// \returns \f$ \neg (f \oplus g) \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_xnor(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Logical 'xnor' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_xnor(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'implication' operator.
  ///
  /// \returns \f$ f \rightarrow g \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_imp(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'implication' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_imp(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'inverse implication' operator.
  ///
  /// \returns \f$ f \leftarrow g \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_invimp(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'inverse implication' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_invimp(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'equivalence' operator.
  ///
  /// \returns \f$ f = g \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_equiv(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'equivalence' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_equiv(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'difference'  operator.
  ///
  /// \returns \f$ f \setminus g \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_diff(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'difference'  operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_diff(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \remark Unary difference subtracts from \f$ \top \f$ value, making its equivalent to negation.
  ///
  /// \see bdd_diff, bdd_not
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  operator-(const bdd& f);

  /// \cond
  __bdd
  operator-(__bdd&& f);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_diff
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  operator-(const bdd& lhs, const bdd& rhs);

  /// \cond
  __bdd
  operator-(const bdd&, __bdd&&);
  __bdd
  operator-(__bdd&&, const bdd&);
  __bdd
  operator-(__bdd&&, __bdd&&);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'less than' operator.
  ///
  /// \returns \f$ f < g \f$
  ///
  /// \see bdd_apply
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_less(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Logical 'less than' operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_less(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   If-Then-Else operator.
  ///
  /// \details Computes the BDD expressing \f$ f ? g : h \f$ more efficient than computing \f$ (f
  ///          \land g) \lor (\neg f \land h) \f$ with `bdd_apply`.
  ///
  /// \param f
  ///    BDD for the if conditional
  ///
  /// \param g
  ///    BDD for the then case
  ///
  /// \param h
  ///    BDD for the else case
  ///
  /// \returns \f$ f ? g : h \f$
  ///
  /// \remark In other BDD packages, this function is good for manually constructing a BDD
  ///         bottom-up. But, here you should use the \ref bdd_builder class (see \ref builder)
  ///         instead
  ///
  /// \see    bdd_apply builder bdd_builder
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_ite(const bdd& f, const bdd& g, const bdd& h);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief If-Then-Else operator.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_ite(const exec_policy& ep, const bdd& f, const bdd& g, const bdd& h);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Restrict a single variable to a constant value.
  ///
  /// \details The variable `i` is restricted to the value `v`.
  ///
  /// \param f
  ///    BDD to restrict.
  ///
  /// \param var
  ///    Variable to assign
  ///
  /// \param val
  ///    Value assigned
  ///
  /// \returns \f$ f|_{x_i = v} \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_restrict(const bdd& f, bdd::label_type var, bool val);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a single variable to a constant value.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_restrict(const exec_policy& ep, const bdd& f, bdd::label_type var, bool val);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Restrict a subset of variables to constant values.
  ///
  /// \details For each tuple (i,v) in the assignment `xs`, the variable with label i is set to the
  ///          constant value v. This binds the scope of the variables in `xs`, i.e. any later
  ///          mention of a variable i is not the same as variable i in `xs`.
  ///
  /// \param f
  ///    BDD to restrict.
  ///
  /// \param xs
  ///    Assignments (i,v) to variables in (in ascending order).
  ///
  /// \returns  \f$ f|_{(i,v) \in xs : x_i = v} \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_restrict(const bdd& f, const generator<pair<bdd::label_type, bool>>& xs);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a subset of variables to constant values.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_restrict(const exec_policy& ep,
               const bdd& f,
               const generator<pair<bdd::label_type, bool>>& xs);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Restrict a subset of variables to constant values.
  ///
  /// \details For each tuple (i,v) provided by the iterator, the variable with label i is set to
  ///          the constant value v.
  ///
  /// \param f
  ///    BDD to restrict
  ///
  /// \param begin
  ///    Single-pass forward iterator that provides the to-be restricted variables in \em ascending
  ///    order. All variables should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns \f$ f|_{(i,v) \in xs : x_i = v} \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_restrict(const bdd& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_restrict(f, make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict a subset of variables to constant values.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_restrict(const exec_policy& ep, const bdd& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_restrict(ep, f, make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Restrict the root to `false`, i.e. follow its low edge.
  ///
  /// \remark In other BDD packages, this function is good for traversing a BDD. But, here this is
  ///         not a constant-time operation but constructs an entire new BDD of up-to linear size
  ///         (requires a full traversal).
  ///
  /// \throws invalid_argument If `f` is a terminal.
  ///
  /// \see bdd_restrict
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_low(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict the root to `false`, i.e. follow its low edge.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_low(const exec_policy& ep, const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Restrict the root to `true`, i.e. follow its high edge.
  ///
  /// \remark In other BDD packages, this function is good for traversing a BDD. But, here this is
  ///         not a constant-time operation but constructs an entire new BDD of up-to linear size
  ///         (requires a full traversal).
  ///
  /// \throws invalid_argument If `f` is a terminal.
  ///
  /// \see bdd_restrict
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_high(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Restrict the root to `true`, i.e. follow its high edge.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_high(const exec_policy& ep, const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Existential quantification of a single variable.
  ///
  /// \details Computes the BDD for \f$ \exists x_{i} : f \f$ faster than computing
  ///          \f$ f|_{x_i = \bot} \lor f|_{x_i = \top} \f$ using `bdd_apply` and `bdd_restrict`.
  ///
  /// \param f
  ///    BDD to be quantified
  ///
  /// \param var
  ///    Variable to quantify in f
  ///
  /// \returns   \f$ \exists x_{var} : f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const bdd& f, bdd::label_type var);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline __bdd
  bdd_exists(bdd&& f, bdd::label_type var)
  {
    return bdd_exists(f, var);
  }

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const exec_policy& ep, const bdd& f, bdd::label_type var);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline __bdd
  bdd_exists(const exec_policy& ep, bdd&& f, bdd::label_type var)
  {
    return bdd_exists(ep, f, var);
  }

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \param f
  ///     BDD to be quantified.
  ///
  /// \param vars
  ///     Predicate to identify the variables to quantify in f. You may abuse the fact, that this
  ///     predicate will only be invoked in ascending/descending order of the levels in `f` (but,
  ///     with retraversals).
  ///
  /// \returns    \f$ \exists x_i \in \texttt{vars} : f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const bdd& f, const predicate<bdd::label_type>& vars);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, const predicate<...>& vars)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(bdd&& f, const predicate<bdd::label_type>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, const predicate<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(__bdd&& f, const predicate<bdd::label_type>& vars);

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const exec_policy& ep, const bdd& f, const predicate<bdd::label_type>& vars);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, const predicate<...>& vars)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const exec_policy& ep, bdd&& f, const predicate<bdd::label_type>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, const predicate<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const exec_policy& ep, __bdd&& f, const predicate<bdd::label_type>& vars);

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief     Existential quantification of multiple variables.
  ///
  /// \param f
  ///     BDD to be quantified.
  ///
  /// \param vars
  ///     Generator function, that produces variables to be quantified in \em descending order.
  ///     These values have to be smaller than or equals to `bdd::max_label`.
  ///
  /// \returns   \f$ \exists x_i \in \texttt{gen()} : f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const bdd& f, const generator<bdd::label_type>& vars);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief     Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, const generator<...>& vars)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(bdd&& f, const generator<bdd::label_type>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief     Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, const generator<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(__bdd&& f, const generator<bdd::label_type>& vars);

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const exec_policy& ep, const bdd& f, const generator<bdd::label_type>& vars);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, const predicate<...>& vars)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const exec_policy& ep, bdd&& f, const generator<bdd::label_type>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, const generator<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const exec_policy& ep, __bdd&& f, const generator<bdd::label_type>& vars);

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \param f
  ///   BDD to be quantified.
  ///
  /// \param begin
  ///    Single-pass forward iterator that provides the to-be quantified variables in \em descending
  ///    order. All variables should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns \f$ \exists x_i \in \texttt{begin} ... \texttt{end} : f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_exists(const bdd& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_exists(f, make_generator(begin, end));
  }

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, ForwardIt, ForwardIt)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_exists(bdd&& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_exists(std::move(f), make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief     Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, ForwardIt, ForwardIt)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_exists(__bdd&& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_exists(std::move(f), make_generator(begin, end));
  }

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_exists(const exec_policy& ep, const bdd& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_exists(ep, f, make_generator(begin, end));
  }

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, ForwardIt, ForwardIt)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_exists(const exec_policy& ep, bdd&& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_exists(ep, std::move(f), make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief     Existential quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_exists(const bdd& f, ForwardIt, ForwardIt)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_exists(const exec_policy& ep, __bdd&& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_exists(ep, std::move(f), make_generator(begin, end));
  }

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Forall quantification of a single variable.
  ///
  /// \details Computes the BDD for \f$ \forall x_{i} : f \f$ faster than computing
  ///          \f$ f|_{x_i = \bot} \land f|_{x_i = \top} \f$ using `bdd_apply` and `bdd_restrict`.
  ///
  /// \param f
  ///    BDD to be quantified.
  ///
  /// \param var
  ///    Variable to quantify in f
  ///
  /// \returns \f$ \forall x_{var} : f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const bdd& f, bdd::label_type var);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline __bdd
  bdd_forall(bdd&& f, bdd::label_type var)
  {
    return bdd_forall(f, var);
  }

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const exec_policy& ep, const bdd& f, bdd::label_type var);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of a single variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline __bdd
  bdd_forall(const exec_policy& ep, bdd&& f, bdd::label_type var)
  {
    return bdd_forall(ep, f, var);
  }

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \param f
  ///    BDD to be quantified.
  ///
  /// \param vars
  ///    Predicate to identify the variables to quantify in f. You may abuse the fact, that this
  ///    predicate will only be invoked in ascending/descending order of the levels in `f` (but,
  ///    with retraversals).
  ///
  /// \returns    \f$ \forall x_i \in \texttt{vars} : f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const bdd& f, const predicate<bdd::label_type>& vars);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const predicate<...>& vars)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(bdd&& f, const predicate<bdd::label_type>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const predicate<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(__bdd&& f, const predicate<bdd::label_type>& vars);

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const exec_policy& ep, const bdd& f, const predicate<bdd::label_type>& vars);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const predicate<...>& vars)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const exec_policy& ep, bdd&& f, const predicate<bdd::label_type>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const predicate<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const exec_policy& ep, __bdd&& f, const predicate<bdd::label_type>& vars);

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief     Forall quantification of multiple variables.
  ///
  /// \param f
  ///    BDD to be quantified.
  ///
  /// \param gen
  ///    Generator function, that produces variables to be quantified in \em descending order. These
  ///    values have to be smaller than or equals to `bdd::max_label`.
  ///
  /// \returns   \f$ \forall x_i \in \texttt{gen()} : f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const bdd& f, const generator<bdd::label_type>& vars);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const predicate<...>& vars)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(bdd&& f, const generator<bdd::label_type>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const generator<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(bdd&& f, const generator<bdd::label_type>& vars);

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const exec_policy& ep, const bdd& f, const generator<bdd::label_type>& vars);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const predicate<...>& vars)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const exec_policy& ep, bdd&& f, const generator<bdd::label_type>& vars);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const generator<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_forall(const exec_policy& ep, __bdd&& f, const generator<bdd::label_type>& vars);

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief       Forall quantification of multiple variables.
  ///
  /// \param f
  ///    BDD to be quantified.
  ///
  /// \param begin
  ///     Single-pass forward iterator that provides the to-be quantified variables in \em
  ///     descending order. All values should be smaller than or equals to `bdd::max_label`.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns     \f$ \forall x_i \in \texttt{begin} ... \texttt{end} : f \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_forall(const bdd& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_forall(f, make_generator(begin, end));
  }

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, ForwardIt, ForwardIt)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_forall(bdd&& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_forall(std::move(f), make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const generator<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_forall(__bdd&& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_forall(std::move(f), make_generator(begin, end));
  }

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_forall(const exec_policy& ep, const bdd& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_forall(ep, f, make_generator(begin, end));
  }

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, ForwardIt, ForwardIt)`, this function moves the
  ///         ownership of `f` into the quantification algorithm. This allows it to garbage collect
  ///         `f` early.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_forall(const exec_policy& ep, bdd&& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_forall(ep, std::move(f), make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forall quantification of multiple variables.
  ///
  /// \remark Unlike `bdd_forall(const bdd& f, const generator<...>& vars)`, this function skips the
  ///         initial transposition of `f` if possible.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_forall(const exec_policy& ep, __bdd&& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_forall(ep, std::move(f), make_generator(begin, end));
  }

  /// \endcond

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Variable Reordering / Substitution
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables in *f* according to the mapping in *m*.
  ///
  /// \param f
  ///    BDD to replace variables within
  ///
  /// \param m
  ///    Function from BDD label to another (or itself).
  ///
  /// \throws invalid_argument if `m` is not a monotonic relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_replace(const bdd& f, const function<bdd::label_type(bdd::label_type)>& m);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables in *f* according to the mapping in *m*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_replace(const exec_policy& ep,
              const bdd& f,
              const function<bdd::label_type(bdd::label_type)>& m);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables in *f* according to the mapping in *m*.
  ///
  /// \param f
  ///    BDD to replace variables within
  ///
  /// \param m
  ///    Function from BDD label to another (or itself).
  ///
  /// \param m_type
  ///    Guarantees on the class of variable relabelling, e.g. whether it is monotonic.
  ///
  /// \throws invalid_argument if `m_type` classifies `m` as not monotonic.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_replace(const bdd& f,
              const function<bdd::label_type(bdd::label_type)>& m,
              replace_type m_type);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables in *f* according to the mapping in *m*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_replace(const exec_policy& ep,
              const bdd& f,
              const function<bdd::label_type(bdd::label_type)>& m,
              replace_type m_type);

  /// \cond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables in *f* according to the mapping in *m*.
  ///
  /// \details Unlike the `bdd_replace(const bdd& f, ...)` variants, this saves *2N/B* I/Os by
  ///          incorporating the renaming into the Reduce algorithm.
  ///
  /// \param f
  ///    Possibly unreduced BDD to replace variables within
  ///
  /// \param m
  ///    Function from BDD label to another (or itself).
  ///
  /// \throws invalid_argument if `m` is not monotonic.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_replace(__bdd&& f, const function<bdd::label_type(bdd::label_type)>& m);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables in *f* according to the mapping in *m*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_replace(const exec_policy& ep,
              __bdd&& f,
              const function<bdd::label_type(bdd::label_type)>& m);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables in *f* according to the mapping in *m*.
  ///
  ///
  /// \param f
  ///    Possibly unreduced BDD to replace variables within
  ///
  /// \param m
  ///    Function from BDD label to another (or itself).
  ///
  /// \param m_type
  ///    Guarantees on the class of variable relabelling, e.g. whether it is monotonic.
  ///
  /// \throws invalid_argument if `m_type` classifies `m` as not monotonic.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_replace(__bdd&& f, const function<bdd::label_type(bdd::label_type)>& m, replace_type m_type);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables in *f* according to the mapping in *m*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_replace(const exec_policy& ep,
              __bdd&& f,
              const function<bdd::label_type(bdd::label_type)>& m,
              replace_type m_type);

  /// \endcond

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Transition System Operations
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Relational Product of *states* and a *relation*.
  ///
  /// \param states
  ///    A symbolic representation of the *current* (or *next*) set of states.
  ///
  /// \param relation
  ///    A relation between *current* and *next* states.
  ///
  /// \param m
  ///    Predicate whether a variable should be existentially quantified.
  ///
  /// \returns \f$ \exists x \in \mathit{pred}(x) : (\mathit{states} \land \mathit{relation}) \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprod(const bdd& states, const bdd& relation, const predicate<bdd::label_type>& pred);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Relational Product of *states* and a *relation*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprod(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const predicate<bdd::label_type>& pred);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forwards step with the Relational Product, including relabelling.
  ///
  /// \param states
  ///    A symbolic representation of the *current* set of states.
  ///
  /// \param relation
  ///    A relation between *current* and *next* states.
  ///
  /// \param m
  ///    A (partial) variable relabelling from *next* to *current*. Variables for which `m` returns
  ///    an empty value are existentially quantified, i.e. the previously *current* state variables.
  ///    For all intends an purposes, this relabelling happens *after* quantification.
  ///
  /// \returns \f$ (\exists x \in \{ x \mid \mathit{m}(x) = \text{None} \}
  ///                        : (\mathit{states} \land \mathit{relation}))[x' \mapsto m(x')] \f$
  ///
  /// \throws invalid_argument if `m` is not a monotonic relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forwards step with the Relational Product, including relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forwards step with the Relational Product, including relabelling.
  ///
  /// \param states
  ///    A symbolic representation of the *current* set of states.
  ///
  /// \param relation
  ///    A relation between *current* and *next* states.
  ///
  /// \param m
  ///    A (partial) variable relabelling from *next* to *current*. Variables for which `m` returns
  ///    an empty value are existentially quantified, i.e. the previously *current* state variables.
  ///    For all intends an purposes, this relabelling happens *after* quantification.
  ///
  /// \param m_type
  ///    Guarantees on the class of variable relabelling, e.g. whether it is monotonic.
  ///
  /// \returns \f$ (\exists x \in \{ x \mid \mathit{m}(x) = \text{None} \}
  ///                        : (\mathit{states} \land \mathit{relation}))[x' \mapsto m(x')] \f$
  ///
  /// \throws invalid_argument if `m` is not a monotonic relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forwards step with the Relational Product, including relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forwards step with the Relational Product for *disjoint* variable orderings,
  ///        including relabelling.
  ///
  /// \param states
  ///    A symbolic representation of the *current* set of states. These are all encoded with the
  ///    variables `0`, `1`, ... `varcount - 1`.
  ///
  /// \param relation
  ///    A relation between *current* and *next* state variables. The *next* state is encoded with
  ///    variables `varcount`, `varcount+1`, ... `2*varcount - 1`.
  ///
  /// \returns \f$ (\exists x \in \{ x \mid x < \mathit{varcount} \}
  ///                        : (\mathit{states} \land \mathit{relation}))
  ///              [x' \mapsto x' - \mathit{varcount}] \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const bdd& states, const bdd& relation, const bdd::label_type varcount);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forwards step with the Relational Product for *disjoint* variable orderings,
  ///        including relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const bdd::label_type varcount);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forwards step with the Relational Product for *interleaved* variable orderings,
  ///        including relabelling.
  ///
  /// \param states
  ///    A symbolic representation of the *current* set of states. These are all encoded with
  ///    *even* variables.
  ///
  /// \param relation
  ///    A relation between *current* (even) and *next* (odd) state variables.
  ///
  /// \returns \f$ (\exists x \in \{ x \mid x \equiv 0 \mod 2 \}
  ///                        : (\mathit{states} \land \mathit{relation}))
  ///              [x' \mapsto x' - 1] \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const bdd& states, const bdd& relation);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Forwards step with the Relational Product for *interleaved* variable orderings,
  ///        including relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relnext(const exec_policy& ep, const bdd& states, const bdd& relation);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Backwards step with the Relational Product, including relabelling.
  ///
  /// \param states
  ///    A symbolic representation of the *current* set of states.
  ///
  /// \param relation
  ///    A relation between *current* and *next* states.
  ///
  /// \param m
  ///    A (partial) variable relabelling from *current* to *next*. Variables for which `m` returns
  ///    an empty value are existentially quantified, i.e. the *next* state variables (the
  ///    previously *current* state variables). For all intends an purposes, this relabelling
  ///    happens *prior* to conjunction.
  ///
  /// \returns \f$ (\exists x' \in \{ x' \mid \mathit{m}(x') = \text{None} \}
  ///                        : (\mathit{states}[x \mapsto m(x)] \land \mathit{relation})) \f$
  ///
  /// \throws invalid_argument if `m` is not a monotonic relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Backwards step with the Relational Product, including relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Backwards step with the Relational Product, including relabelling.
  ///
  /// \param states
  ///    A symbolic representation of the *current* set of states.
  ///
  /// \param relation
  ///    A relation between *current* and *next* states.
  ///
  /// \param m
  ///    A (partial) variable relabelling from *current* to *next*. Variables for which `m` returns
  ///    an empty value are existentially quantified, i.e. the *next* state variables (the
  ///    previously *current* state variables). For all intends and purposes, this relabelling
  ///    happens *prior* to conjunction.
  ///
  /// \param m_type
  ///    Guarantees on the class of variable relabelling, e.g. whether it is monotonic.
  ///
  /// \returns \f$ (\exists x' \in \{ x' \mid \mathit{m}(x') = \text{None} \}
  ///                        : (\mathit{states}[x \mapsto m(x)] \land \mathit{relation})) \f$
  ///
  /// \throws invalid_argument if `m` is not a monotonic relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Backwards step with the Relational Product, including relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const function<optional<bdd::label_type>(bdd::label_type)>& m,
              replace_type m_type);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Backwards step with the Relational Product for *disjoint* variable orderings,
  ///        including relabelling.
  ///
  /// \param states
  ///    A symbolic representation of the *current* set of states. These are all encoded with the
  ///    variables `0`, `1`, ... `varcount - 1`.
  ///
  /// \param relation
  ///    A relation between *current* and *next* state variables. The *next* state is encoded with
  ///    variables `varcount`, `varcount+1`, ... `2*varcount - 1`.
  ///
  /// \returns \f$ (\exists x' \in \{ x' \mid \geq \mathit{varcount} \}
  ///                         : (\mathit{states}[x \mapsto x + \mathit{varcount}]
  ///                            \land \mathit{relation})) \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const bdd& states, const bdd& relation, const bdd::label_type varcount);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Backwards step with the Relational Product for *disjoint* variable orderings,
  ///        including relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const exec_policy& ep,
              const bdd& states,
              const bdd& relation,
              const bdd::label_type varcount);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Backwards step with the Relational Product for *interleaved* variable orderings,
  ///        including relabelling.
  ///
  /// \param states
  ///    A symbolic representation of the *current* set of states. These are all encoded with
  ///    *even* variables.
  ///
  /// \param relation
  ///    A relation between *current* (even) and *next* (odd) state variables.
  ///
  /// \returns \f$ (\exists x' \{ x' \mid x' \equiv 1 \mod 2 \}
  ///                         : (\mathit{states}[x \mapsto x + 1] \land \mathit{relation})) \f$
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const bdd& states, const bdd& relation);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Backwards step with the Relational Product for *interleaved* variable orderings,
  ///        including relabelling.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_relprev(const exec_policy& ep, const bdd& states, const bdd& relation);

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Predicates
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Check whether a BDD is canonical.
  ///
  /// \copydetails adiar::internal::dd_iscanonical
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_iscanonical(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is a constant.
  ///
  /// \see bdd_isfalse bdd_istrue
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_isconst(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is a constant (terminal).
  ///
  /// \see bdd_isfalse bdd_istrue
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_isterminal(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is the constant `false`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_isfalse(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is the constant `true`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_istrue(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is a function dependent on a single variable, i.e. is a literal.
  ///
  /// \details Whether `f` is equivalent to \f$ x \f$ or \f$ \neg x \f$.
  ///
  /// \see bdd_isithvar bdd_isnithvar
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_isvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is the function of a single positive variable.
  ///
  /// \details Whether `f` is equivalent to \f$ x \f$.
  ///
  /// \see bdd_isvar bdd_ithvar
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_isithvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD is the function of a single negative variable.
  ///
  /// \details Whether `f` is equivalent to \f$ \neg x \f$.
  ///
  /// \see bdd_isvar bdd_nithvar
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_isnithvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a BDD represents a cube.
  ///
  /// \see bdd_cube
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_iscube(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two BDDs represent the same function.
  ///
  /// \remark If either `f` or `g` is an unreduced result of an immediate computation as part of
  ///         evaluating `bdd_equal`'s' parameters, one will have to pay the cost of reducing it. If
  ///         you need the result later, please store it in an intermediate variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_equal(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two BDDs represent the same function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_equal(const exec_policy&, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_equal
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  operator==(const bdd& f, const bdd& g);

  /// \cond
  bool
  operator==(__bdd&& f, const bdd& g);
  bool
  operator==(const bdd& f, __bdd&& g);
  bool
  operator==(__bdd&& f, __bdd&& g);
  /// \endcond

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two BDDs represent different functions.
  ///
  /// \remark If either `f` or `g` is an unreduced result of an immediate computation as part of
  ///         evaluating `bdd_unequal`'s' parameters, one will have to pay the cost of reducing it.
  ///         If you need the result later, please store it in an intermediate variable.
  ///
  /// \see bdd_equal
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_unequal(const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether the two BDDs represent different functions.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_unequal(const exec_policy& ep, const bdd& f, const bdd& g);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \see bdd_equal bdd_unequal
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  operator!=(const bdd& f, const bdd& g);

  /// \cond
  bool
  operator!=(const bdd& f, __bdd&& g);
  bool
  operator!=(__bdd&& f, const bdd& g);
  bool
  operator!=(__bdd&& f, __bdd&& g);
  /// \endcond

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Counting Operations
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The number of (internal) nodes used to represent the function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  size_t
  bdd_nodecount(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The number of variables that influence the outcome of the Boolean function, i.e. the
  ///        number of levels present in the BDD.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd::label_type
  bdd_varcount(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths to the true terminal.
  ///
  /// \returns The number of unique paths.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t
  bdd_pathcount(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Count all unique (but not necessarily disjoint) paths to the true terminal.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t
  bdd_pathcount(const exec_policy& ep, const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief          Count the number of assignments x that make f(x) true.
  ///
  /// \param f
  ///    BDD to count within.
  ///
  /// \param varcount
  ///    The number of variables in the domain of the function. This number should be larger than or
  ///    equal to the number of levels in the BDD (\see bdd_varcount())
  ///
  /// \returns        The number of unique assignments.
  ///
  /// \throws invalid_argument If varcount is not larger than the number of levels in the BDD.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t
  bdd_satcount(const bdd& f, bdd::label_type varcount);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Count the number of assignments x that make f(x) true.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t
  bdd_satcount(const exec_policy& ep, const bdd& f, bdd::label_type varcount);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Count the number of assignments x that make f(x) true.
  ///
  /// \details Same as `bdd_satcount(f, varcount)`, with varcount set to be the size of the global
  ///          domain or the number of variables within the given BDD.
  ///
  /// \see domain_set bdd_varcount
  //////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t
  bdd_satcount(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Count the number of assignments *x that make f(x) true.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  uint64_t
  bdd_satcount(const exec_policy& ep, const bdd& f);

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Input Variables
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Get (in \em ascending order) all of the variable labels that occur
  ///        in the BDD.
  ///
  /// \param f
  ///    BDD of interest.
  ///
  /// \param cb
  ///    Callback function that consumes the variable labels.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  void
  bdd_support(const bdd& f, const consumer<bdd::label_type>& cb);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Copy all of the variable labels (in \em ascending order) that occur in the BDD into the
  ///        given container.
  ///
  /// \param f
  ///    BDD of interest.
  ///
  /// \param iter
  ///    Single-pass output iterator for where to place the output.
  ///
  /// \returns The output iterator at its final state.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename OutputIt,
            typename = enable_if<!is_convertible<OutputIt, consumer<bdd::label_type>>>>
  OutputIt
  bdd_support(const bdd& f, OutputIt iter)
  {
    bdd_support(f, make_consumer<bdd::label_type>(iter));
    return iter;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Get the root's variable label.
  ///
  /// \throws invalid_argument If `f` is a terminal.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd::label_type
  bdd_topvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring variable in the function's support.
  ///
  /// \throws invalid_argument If `f` is a terminal.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd::label_type
  bdd_minvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring variable in the function's support.
  ///
  /// \throws invalid_argument If `f` is a terminal.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd::label_type
  bdd_maxvar(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically smallest cube x such that f(x) is true.
  ///
  /// \details Outputs the trace of the low-most path to the true terminal. The resulting assignment
  ///          is lexicographically smallest, where every variable is treated as a digit and
  ///          \f$ x_0 > x_1 > \dots \f$.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the minimal assignment.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_satmin(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically smallest x such that f(x) is true within the given domain.
  ///
  /// \param d
  ///    Generator of domain in ascending order.
  ///
  /// \param d_size
  ///    Upper bound on the number of elements generated by `d`. This value is merely an allocation
  ///    hint for the sake of optimisation.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the minimal assignment.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_satmin(const bdd& f,
             const generator<bdd::label_type>& d,
             const size_t d_size = bdd::max_label + 1);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically smallest x such that f(x) is true.
  ///
  /// \param f
  ///    BDD of interest.
  ///
  /// \param begin
  ///    Single-pass forward iterator of *immutable* variables in *ascending* ordering.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the minimal assignment and
  ///          (at least) includes the variables in *[begin, end)*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt, typename = enable_if<is_const<typename ForwardIt::reference>>>
  bdd
  bdd_satmin(const bdd& f, ForwardIt cbegin, ForwardIt cend)
  {
    return bdd_satmin(f, make_generator(cbegin, cend), std::distance(cbegin, cend));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically smallest x such that f(x) is true.
  ///
  /// \param f
  ///    BDD of interest.
  ///
  /// \param d
  ///    BDD cube of variables that ought to be included.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the minimal assignment.
  ///
  /// \throws domain_error
  ///    If `bdd_iscube(d)` is not satisfied.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_satmin(const bdd& f, const bdd& d);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically smallest x such that f(x) is true.
  ///
  /// \param c
  ///    Consumer that is called in ascending order of the bdd's levels with the (var, value)
  ///    pairs of the assignment.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  void
  bdd_satmin(const bdd& f, const consumer<pair<bdd::label_type, bool>>& c);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically smallest x such that f(x) is true.
  ///
  /// \param f
  ///    BDD of interest.
  ///
  /// \param begin
  ///    Single-pass output iterator for where to place the output.
  ///
  /// \returns The output iterator at its final state.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename OutputIt,
            typename = enable_if<!is_convertible<OutputIt, consumer<pair<bdd::label_type, bool>>>
                                 && !is_convertible<OutputIt, bdd>>>
  OutputIt
  bdd_satmin(const bdd& f, OutputIt iter)
  {
    bdd_satmin(f, make_consumer<pair<bdd::label_type, bool>>(iter));
    return iter;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically largest cube x such that f(x) is true.
  ///
  /// \details Outputs the trace of the high-most path to the true terminal. The resulting
  ///          assignment is lexicographically largest, where every variable is treated as a digit
  ///          and \f$ x_0 > x_1 > \dots \f$.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the maximal assignment.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_satmax(const bdd& f);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically largest x such that f(x) is true within the given domain.
  ///
  /// \param d
  ///    Generator of domain in ascending order.
  ///
  /// \param d_size
  ///    Upper bound on the number of elements generated by `d`.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the maximal assignment.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_satmax(const bdd& f,
             const generator<bdd::label_type>& d,
             const size_t d_size = bdd::max_label + 1);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically largest x such that f(x) is true.
  ///
  /// \param f
  ///    BDD of interest.
  ///
  /// \param begin
  ///    Single-pass forward iterator of *immutable* variables in *ascending* ordering.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the maximal assignment and
  ///          (at least) includes the variables in *[begin, end)*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt, typename = enable_if<is_const<typename ForwardIt::reference>>>
  bdd
  bdd_satmax(const bdd& f, ForwardIt cbegin, ForwardIt cend)
  {
    return bdd_satmax(f, make_generator(cbegin, cend), std::distance(cbegin, cend));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically largest x such that f(x) is true.
  ///
  /// \param f
  ///    BDD of interest.
  ///
  /// \param d
  ///    BDD cube of variables that ought to be included.
  ///
  /// \returns A bdd whos only path to the `true` terminal reflects the maximal assignment.
  ///
  /// \throws domain_error
  ///    If `bdd_iscube(d)` is not satisfied.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bdd
  bdd_satmax(const bdd& f, const bdd& d);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   The lexicographically largest x such that f(x) is true.
  ///
  /// \param c
  ///    Consumer that is called in ascending order of the bdd's levels with the (var, value) pairs
  ///    of the assignment.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  void
  bdd_satmax(const bdd& f, const consumer<pair<bdd::label_type, bool>>& c);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The lexicographically largest x such that f(x) is true.
  ///
  /// \param f
  ///    BDD of interest.
  ///
  /// \param begin
  ///    Single-pass output iterator for where to place the output.
  ///
  /// \returns The output iterator at its final state.
  //////////////////////////////////////////////////////////////////////////////
  template <typename OutputIt,
            typename = enable_if<!is_convertible<OutputIt, consumer<pair<bdd::label_type, bool>>>
                                 && !is_convertible<OutputIt, bdd>>>
  OutputIt
  bdd_satmax(const bdd& f, OutputIt iter)
  {
    bdd_satmax(f, make_consumer<pair<bdd::label_type, bool>>(iter));
    return iter;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the satisfying assignment that is minimal for the given linear cost function
  ///        over the global domain.
  ///
  /// \param f
  ///    The BDD of feasible solutions
  ///
  /// \param c
  ///    A (pure) function that provides the cost function's coefficient.
  ///
  /// \returns A pair of a BDD cube with the best satisfying assignment and its cost. If no solution
  ///          was found, returns the false constant and NaN
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  pair<bdd, double>
  bdd_optmin(const bdd& f, const cost<bdd::label_type>& c);

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief obtain the satisfying assignment that is minimal for the given linear cost function
  ///        over the global domain.
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  pair<bdd, double>
  bdd_optmin(const exec_policy& ep, const bdd& f, const cost<bdd::label_type>& c);

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief    Obtain the satisfying assignment that is minimal for the given linear cost function
  ///           over the global domain.
  ///
  /// \param f
  ///   The BDD of feasible solutions
  ///
  /// \param c
  ///    A (pure) function that provides the cost function's coefficient.
  ///
  /// \param cb
  ///    A callback function that is called in \em descending order.
  ///
  /// \returns The cost of the satisfying solution if any, otherwise NaN.
  ///
  /// \remark If `f` is a terminal, `cb` will never be called
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  double
  bdd_optmin(const bdd& f,
             const cost<bdd::label_type>& c,
             const consumer<pair<bdd::label_type, bool>>& cb);

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the satisfying assignment that is minimal for the given linear cost function
  ///        over the global domain.
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  double
  bdd_optmin(const exec_policy& ep,
             const bdd& f,
             const cost<bdd::label_type>& c,
             const consumer<pair<bdd::label_type, bool>>& cb);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Evaluate a BDD according to an assignment to its variables.
  ///
  /// \details The given assignment function may assume/abuse that it is only called with the
  ///          labels in a strictly increasing order.
  ///
  /// \param f
  ///    The BDD to evaluate
  ///
  /// \param xs
  ///    An assignment function of the type \f$ \texttt{label\_t} \rightarrow \texttt{bool} \f$.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_eval(const bdd& f, const predicate<bdd::label_type>& xs);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Evaluate a BDD according to an assignment to its variables.
  ///
  /// \param f
  ///    The BDD to evaluate.
  ///
  /// \param xs
  ///    Assignments (i,v) to variables in (in ascending order).
  ///
  /// \throws out_of_range
  ///    If traversal of the BDD leads to going beyond the end of the content of `xs`.
  ///
  /// \throws invalid_argument
  ///    If a level in the BDD does not exist in `xs`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  bdd_eval(const bdd& f, const generator<pair<bdd::label_type, bool>>& xs);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Evaluate a BDD according to an assignment to its variables.
  ///
  /// \param f
  ///    The BDD to evaluate.
  ///
  /// \param begin
  ///    Single-pass forward iterator that provides the assignment in \em ascending order.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \throws out_of_range
  ///    If traversal of the BDD leads to going beyond the end of the content of `xs`.
  ///
  /// \throws invalid_argument
  ///    If a level in the BDD does not exist in `xs`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  bool
  bdd_eval(const bdd& f, ForwardIt begin, ForwardIt end)
  {
    return bdd_eval(f, make_generator(begin, end));
  }

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name Conversion from other Decision Diagrams
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given ZDD within the given
  ///        domain.
  ///
  /// \param A
  ///    Family of a set (within the given domain)
  ///
  /// \param dom
  ///    Generator function of domain variables in \em ascending order. These values may not
  ///    exceed `bdd::max_label`.
  ///
  /// \returns BDD that is true for the exact same assignments to variables in the given domain.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_from(const zdd& A, const generator<bdd::label_type>& dom);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given ZDD within the given
  ///        domain.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_from(const exec_policy& ep, const zdd& A, const generator<bdd::label_type>& dom);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given ZDD within the given
  ///        domain.
  ///
  /// \param A
  ///    Family of a set (within the given domain)
  ///
  /// \param begin
  ///    Single-pass forward iterator that provides the domain's variables in \em ascending order.
  ///    These values may not exceed `bdd::max_label`.
  ///
  /// \param end
  ///    Marks the end for `begin`.
  ///
  /// \returns BDD that is true for the exact same assignments to variables in the given domain.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_from(const zdd& A, ForwardIt begin, ForwardIt end)
  {
    return bdd_from(A, make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given ZDD within the given
  ///        domain.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename ForwardIt>
  __bdd
  bdd_from(const exec_policy& ep, const zdd& A, ForwardIt begin, ForwardIt end)
  {
    return bdd_from(ep, A, make_generator(begin, end));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given ZDD within the
  ///        global domain.
  ///
  /// \param A
  ///    Family of a set (within the given global \ref module__domain)
  ///
  /// \pre The global \ref module__domain is set to a set of variables that is equals to or a
  ///      superset of the variables in `A`.
  ///
  /// \returns BDD that is true for the exact same assignments to variables in the global domain.
  ///
  /// \see domain_set domain_isset
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_from(const zdd& A);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtains the BDD that represents the same function/set as the given ZDD within the
  ///        global domain.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_from(const exec_policy& ep, const zdd& A);

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \name DOT Files
  ///
  /// \{

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing of a BDD to the given output stream.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  void
  bdd_printdot(const bdd& f, std::ostream& out = std::cout, bool include_id = false);

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Output a DOT drawing of a BDD to the file with the given name.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  void
  bdd_printdot(const bdd& f, const std::string& file_name, bool include_id = false);

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_BDD_H
