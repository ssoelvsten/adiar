\page page__functional Function Objects

\ref page__basic contained examples for how to use the basic version of most BDD
operations. For more advanced usage, you need to use \ref module__functional as
a bridge between your own code and Adiar.

To play around with the examples below, please take a look at
`examples/functional.cpp`.

Predicates
==================================

An `adiar::predicate` is a function that given a value of some type `T` returns
a `bool`. For example, the following *lambda* is a predicate for whether a BDD
variable is odd.

\snippet functional.cpp is_odd

In \ref page__basic, we showed how to use it to quantify a single variable. You
can parse the above predicate to `adiar::bdd_exists` to quantify all odd variables
at once.

\snippet functional.cpp predicate


Iterators
==================================

As part of your program, you most likely store the to-be quantified variables
somewhere. The *idiomatic* way in C++ to interact with data structures is to use
*iterators*. A data structure can be parsed directly to Adiar's operations, if
it can provide its data in the same order as they need to be consumed by the BDD
operation. In the case of `adiar::bdd_exists`, the iterator's data has to be in
*descending* order.

\snippet functional.cpp iterators_1

Reversely, one can also parse information from Adiar's algorithms back into
one's own data structures with an output iterator. For example, the variables
within a BDD can be copied (in *ascending* order) into a `std::vector` with the
`adiar::bdd_support` function as follows.

\snippet functional.cpp iterators_2


Generators and Consumers
==================================

Under the hood, Adiar wraps the iterator-pair into an `adiar::generator` or an
`adiar::consumer`.

Generators
----------------------------------

A generator function is a stateful function object that when called provides an
`adiar::optional` of the next value; in many ways this can be thought of as a
*coroutine*. That is, the above example for quantifying x<sub>5</sub>,
x<sub>3</sub>, and x<sub>1</sub> can also be done with the following *lambda*
function.

\snippet functional.cpp generators

Similarly, one can parse an (*ascending*) iterator or generator
`adiar::bdd_restrict` to restrict multiple variables. Doing so is going to be
significantly faster.

Consumers
----------------------------------

A consumer function essentially is a *callback* that consumes the result. For
example, we can print all of the variables that are present within a BDD to the
console with the following *lambda* function.

\snippet functional.cpp consumers

Similarly, one can obtain the satisfying assignment of `adiar::bdd_satmin` and
`adiar::bdd_satmax` with iterators and/or consumer functions.
