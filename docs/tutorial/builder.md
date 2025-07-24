\page page__builder Manual Construction

In some cases, one may already know the shape of an `adiar::bdd` for a more
complex function. In those cases, it is much cheaper to construct them by hand
than to manipulate logic formulas. The `adiar::bdd_builder` class allows you to
do exactly this by providing you with the “pointers” of prior constructed nodes
such that they can be referenced by new ones.

For example, consider the following BDD consisting of 3 nodes that represents
the formula \f$f(x_0, x_1, x_2) \equiv (x_0 \land x_1) \lor x_2 \f$.

\dot
digraph builder_example {
  n0 [label=<x<sub>0</sub>>, shape=circle];
  n1 [label=<x<sub>1</sub>>, shape=circle];
  n2 [label=<x<sub>2</sub>>, shape=circle];

  t0 [label=0, shape=box];
  t1 [label=1, shape=box];

  n0 -> n2 [style=dashed]
  n0 -> n1 [style=solid]

  n1 -> n2 [style=dashed]
  n1 -> t1 [style=solid]

  n2 -> t0 [style=dashed]
  n2 -> t1 [style=solid]

  { rank=same; n0 }
  { rank=same; n1 }
  { rank=same; n2 }
  { rank=same; t0 t1 }
}
\enddot

This can be created manually using an `adiar::bdd_builder` (see `adiar::builder`
for all its member functions) as follows.

\snippet builder.cpp example

To play around with this example, please take a look at `examples/builder.cpp`.
