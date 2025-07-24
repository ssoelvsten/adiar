\page page__basic Basic Usage

[TOC]

Creating BDDs
===============================

Constants
-------------------------------

The Boolean constants *true* and *false* can be obtained directly with the
`adiar::bdd_true` and `adiar::bdd_false` functions.

\snippet basic.cpp constants.top

Alternatively, boolean constants can also be created directly from a value of
type `bool`.

\snippet basic.cpp constants.bot

Variables
-------------------------------

For a single variable, e.g. x<sub>i</sub>, parse the label *i* to
`adiar::bdd_ithvar`.

\snippet basic.cpp ithvar.0
\snippet basic.cpp ithvar.1
\snippet basic.cpp ithvar.2

For the negation of a single variable, use `adiar::bdd_nithvar`

Combining BDDs
===============================

Negation
-------------------------------

Use `adiar::bdd_not` or the `~` operator. For example, `res` below is equivalent
to `adiar::bdd_nithvar` of *0*.

\snippet basic.cpp not.1
\snippet basic.cpp not.2

Binary Operators
-------------------------------

Use `adiar::bdd_apply` with an `adiar::bool_op`.

\snippet basic.cpp apply.1

Alternatively, there is a specializations of `adiar::bdd_apply` for each
`adiar::bool_op` and for *and*, *or*, and *xor* one can also use the `&`, `|`,
and `^` operators.

\snippet basic.cpp apply.2
\snippet basic.cpp apply.3
\snippet basic.cpp apply.4

If-Then-Else Operator
-------------------------------

Use `adiar::bdd_ite`.

\snippet basic.cpp ite

In other BDD packages, this function is good for manually constructing a BDD
bottom-up. But, here you should use `adiar::bdd_builder` instead (see \ref
page__builder).

Restricting Variables
-------------------------------

Use `adiar::bdd_restrict`. For example, `res` below is equivalent to
`~x1`.

\snippet basic.cpp restrict

Specifically to restrict the *top* variable, you can use `adiar::bdd_low` and
`adiar::bdd_high`.

See also the \ref page__functional tutorial for better ways to use this
operation.

Quantification
-------------------------------

Use `adiar::bdd_exists` and `adiar::bdd_forall`. For example, `res` below is
equivalent to `adiar::bdd_true()`.

\snippet basic.cpp exists

See also the \ref page__functional tutorial for better ways to use these
operations.

Satisfying Assignments
===============================

To get the number of satisfying assignments, use `adiar::bdd_satcount`. Its
second (optional) argument is the total number of variables (including the
possibly suppressed ones in the BDD).

\snippet basic.cpp satcount

To get a cube of the *lexicographical minimal* or *maximal* assignment, use
`adiar::bdd_satmin` and `adiar::bdd_satmax` respectively.

\snippet basic.cpp satmin
\snippet basic.cpp satmax

Predicates
===============================

Boolean Constants
-------------------------------

Use `adiar::bdd_isconst` to check whether a BDD is a constant Boolean value.

\snippet basic.cpp isconst.top
\snippet basic.cpp isconst.bot
\snippet basic.cpp isconst.x0

Use `adiar::bdd_istrue` and `adiar::bdd_isfalse` to check for it being
specifically \f$\top\f$ or \f$\bot\f$, respectively.

Single Variables
-------------------------------

Use `adiar::bdd_isvar` check whether a BDD represents a formula of exactly one
variable, i.e. a *literal*.

\snippet basic.cpp isvar.top
\snippet basic.cpp isvar.x0
\snippet basic.cpp isvar.~x0

Use `adiar::bdd_isithvar`, and `adiar::bdd_isnithvar` to check whether a BDD
represents a formula of a positive or negative variable, respectively.

Cubes
-------------------------------

To check whether a BDD represents a cube, i.e. where all the variables in its
*support* have a fixed value, use `adiar::bdd_iscube`.

\snippet basic.cpp iscube.f
\snippet basic.cpp iscube.f_min

Equality
-------------------------------

Use `adiar::bdd_equal` and `adiar::bdd_unequal` or the `==` and `!=` operators.

\snippet basic.cpp equal.1
\snippet basic.cpp equal.2

BDD Information
===============================

Counting Operations
-------------------------------

Use `adiar::bdd_nodecount` to get the number of BDD nodes.

\snippet basic.cpp nodecount

Use `adiar::bdd_varcount` to get the number of variables present in the BDD.

\snippet basic.cpp varcount

Use `adiar::bdd_pathcount` to get the number of paths to *true*.

\snippet basic.cpp pathcount

Support
-------------------------------

The *top* (*smallest*) variable present in the BDD can be obtained with
`adiar::bdd_topvar` (`adiar::bdd_minvar`).

\snippet basic.cpp topvar

On the other hand, the last variable present in the BDD can be obtained with
`adiar::bdd_maxvar`.

\snippet basic.cpp maxvar

Graphical Output
-------------------------------

The BDD can be exported to the *DOT* format with `adiar::bdd_printdot`. The
second argument can either be an output stream, e.g. `std::cout`, or a filename.

\snippet basic.cpp dot
