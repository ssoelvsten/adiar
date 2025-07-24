\page page__queens Queens

[TOC]

Queens
===============================

The queens problem is a classic problem that is easy to state and understand but
fascinatingly hard to solve efficiently.

> Given \f$N\f$, in how many ways can \f$N\f$ queens be placed on an \f$N \times
> N\f$ chess board without threatening each other?

All code shown here can be found in `examples/queens.cpp`. You can also compile
and run this file to play around with it yourself.

Variable Ordering
-------------------------------

Well stay with the simple row-by-row ordering of variables for now. That is, we
represent whether a queen is placed at position \f$(i,j)\f$ on an \f$N \times
N\f$ chess board as the truth value of the BDD variable \f$x_{i,j} :=
x_{N\cdot i+j}\f$.

In C++, we can derive the label of each chess board position as follows.

\snippet queens.cpp label


Constructing the Board
-------------------------------

We construct the set of solutions for the Queens problem as described by Kunkle,
Slavici, and Cooperman.

- [[Kunkle10](https://dl.acm.org/doi/abs/10.1145/1837210.1837222)] Daniel
  Kunkle, Vlad Slavici, Gene Cooperman. “*Parallel Disk-Based Computation for
  Large, Monolithic Binary Decision Diagrams*”. In: *PASCO '10: Proceedings of
  the 4th International Workshop on Parallel and Symbolic Computation*. 2010

### S(i,j) : Single Position

Let us first restrict our attention to the base case of expressing the state of
a single cell \f$(i,j)\f$. We need to express that a single queen is placed here
and that it is in no conflict with any other queens on the board, i.e. there
cannot be any queen on the same row, column or diagonal.

```
  ┌─────────────────┐
8 │╲    │     ╱     │
7 │  ╲  │   ╱       │
6 │    ╲│ ╱         │
5 │─────👑──────────│
4 │    ╱│  ╲        │
3 │  ╱  │    ╲      │
2 │╱    │      ╲    │
1 │     │        ╲  │
  └─────────────────┘
    a b c d e f g h
```

This essentially is the formula \f$fx_{i,j} \wedge \neg
\mathit{is\_threatened}(i,j)\f$ where \f$\mathit{is\_threatened}(i,j)\f$ is
\f$\top\f$ if one or more queens are placed on conflicting positions. We could
construct this BDD using `adiar::bdd_ithvar`, `adiar::bdd_not`, and
`adiar::bdd_and`. Yet, the resulting (reduced) BDD is very well structured. So,
we can construct it explicitly! All BDDs are stored on disk bottom-up, so we'll
have to start at the bottom-right corner and work ourselves back up to the
top-left corner. We can skip all labels for cells that don't lie on the same
row, column or the two diagonals. For the remaining cells, we have to check that
all but the one for cell \f$(i,j)\f$ they are \f$\bot\f$ while \f$(i,j)\f$ is
\f$\top\f$.

For example, for \f$N = 3\f$ and \f$(i,j) = (1,0)\f$, i.e. for a queen placed at
position 2A on a \f$3\times 3\f$ chess board, the variable is \f$x_3\f$ and no
queen is allowed to be placed on the same row, \f$x_4\f$ and \f$x_5\f$, nor its
column, \f$x_0\f$ and \f$6\f$, nor either column, \f$x_1\f$ and \f$x_7\f$. The
resulting BDD looks like this.

\dot
digraph queens_S_example {
  n00 [label=<x<sub>0</sub>>, shape=circle];
  n01 [label=<x<sub>1</sub>>, shape=circle];
  n10 [label=<x<sub>3</sub>>, shape=circle];
  n11 [label=<x<sub>4</sub>>, shape=circle];
  n12 [label=<x<sub>5</sub>>, shape=circle];
  n20 [label=<x<sub>6</sub>>, shape=circle];
  n21 [label=<x<sub>7</sub>>, shape=circle];

  t0 [label=0, shape=box];
  t1 [label=1, shape=box];

  n00 -> n01 [style=dashed]
  n00 -> t0  [style=solid]

  n01 -> n10 [style=dashed]
  n01 -> t0  [style=solid]

  n10 -> t0  [style=dashed]
  n10 -> n11 [style=solid]

  n11 -> n12 [style=dashed]
  n11 -> t0  [style=solid]

  n12 -> n20 [style=dashed]
  n12 -> t0  [style=solid]

  n20 -> n21 [style=dashed]
  n20 -> t0  [style=solid]

  n21 -> t1  [style=dashed]
  n21 -> t0  [style=solid]

  { rank=same; n00 }
  { rank=same; n01 }
  { rank=same; n10 }
  { rank=same; n11 }
  { rank=same; n12 }
  { rank=same; n20 }
  { rank=same; n21 }
  { rank=same; t0 t1 }
}
\enddot

Since we construct it explicitly using the `adiar::bdd_builder`, then the work
we do in the base case goes down to only \f$O(\text{scan}(N))\f$ time I/Os
rather than \f$O(\text{sort}(Nˆ2))\f$. One pretty much cannot do this base
case faster.

\snippet queens.cpp queens_S


### R(i) : Row

Now that we have a formula each cell, we can accumulate them with an \f$\vee\f$.
This ensures that there is at least one queen on each row since each
`n_queens_S` is only \f$\top\f$ if the queen is placed. Furthermore, the placed
queen(s) are not in conflicts. Implicitly, this also provides an at-most-one
constraints.

We could do so in two ways:

- Recursively split the row in half until we reach the base case of
  `n_queens_S`. This will minimise the number of Apply's that we will make.

- Similar to a `list.fold` in functional programming languages, we iteratively
  combine them. This will minimise the number of BDDs that are "active" at any
  given time, since we only need to persist the input and output of each
  iteration.

For Adiar to be able to achieve optimality on disk, it sacrifices the
possibility of a hash-table to instantiate the entire forest of all currently
active BDDs. In other words, each BDD is completely separate and no memory is
saved if there is a major overlap. So, we will choose to do it iteratively.

\snippet queens.cpp queens_R


### B() : Board

Now that we have each entire row done, we only need to combine them. Here we
again iterate over all rows to combine them one-by-one. One can probably remove
the code duplication that we now introduce.

\snippet queens.cpp queens_B


Counting Solutions
-------------------------------

Above, we constructed a single BDD that os \f$\top\f$ if and only if we have
placed \f$N\f$ queen on each row and without putting them in conflict with each
other. That is, we have constructed the set of all valid solutions to the Queens
problem. So, now we can merely count the number of satisfying assignments.

\snippet queens.cpp queens_count


Enumerating all Solutions
-------------------------------

Based on this [project
report](github.com/MartinFaartoft/n-queens-bdd/blob/master/report.tex) by
Thorbjørn Nielsen and Martin Faartoft, we can use the BDD constructed above to
guide a recursive procedure to enumerate all satisfying solutions. The BDD
prunes the search tree when recursing.

Starting from the BDD created with `queens_B` above, we use
`adiar::bdd_restrict` to place queens one row at a time: starting from the
left-most column in a row, we place the queen and then recurse. Recursion can
stop early in two cases:

- If the given BDD already is trivially false. We have placed a queen, such that
  it conflicts with another.

- If the number of unique paths in the restricted BDD is exactly one. Then we
  are forced to place the remaining queens.

Since we want to back track in our choices, we may keep BDDs for each column.
This is easily achieved by writing it as a recursive procedure. One should
notice though, that this will result in multiple BDDs concurrently being
instantiated in memory or on disk at the same time.

\snippet queens.cpp queens_list_rec

The recursion can then be started with an empty assignment.

\snippet queens.cpp queens_list

This uses a helper function, `print_assignment` which converts the variable
labels \f$0, 1, \dots, N^2-1\f$ back to their coordinate and prints them in
chess notation.

\snippet queens.cpp print_assignment
