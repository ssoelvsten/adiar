# Examples
![examples](https://github.com/SSoelvsten/adiar/workflows/examples/badge.svg?branch=main)

To benchmark the implementation and also to provide examples of how to use
*Adiar* in your project, we provide a few examples. Even more examples can be
found at the [BDD Benchmarking repository](https://github.com/SSoelvsten/bdd-benchmark).

All examples have an *M* variable to be set for the MiB of memory that is to be
available to *Adiar*. The default is 1024 MiB.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Examples](#examples)
    - [N-Queens](#n-queens)
    - [References](#references)

<!-- markdown-toc end -->

## N-Queens

**Files:** `queens.cpp`

**Target:** `make example/queens N=<?>`

Solve the N-Queens problem for *N = `?`* (default: `8`). This is done by
constructing an BDD row-by-row that represents whether the row is in a legal
state: is at least one queen placed on each row and is it also in no conflicts
with any other? On this BDD we then counts the number of satisfying
assignments [[kunkle10](#references)].

To list the solutions, we take the same BDD, but instead of counting the number
of assignments within, we use it to prune a recursive enumeration of all
possible queen placements. For up to *N = 8* we run both the solution counting
procedure and the enumeration procedure.

**Notice**: This is a pretty simple example and has all of the normal
shortcomings for BDDs trying to solve the N-Queens problem. At around *N* = 14
the intermediate sizes explodes a lot. One can with about 100 GB of disk space
or memory compute it.

## Knight's Tours

**Files:** `knights_tour.cpp`

**Target:** `make example/knights_tour/<?> N=<?>`

Counts the number of Knight's Tours on an *N = `?`* sized square board. Either
*all* or only the *closed* tours are counted. This is done by combining ZDDs
that describe the possible transitions at every time step together with ZDDs
that make every square visited only once. If only the *closed* tours are
supposed to be used, then the first time-step is fixed to the top-left corner
and the next and last time-step fixed to the two possible moves by a Knight.

## References

- [[Kunkle10](https://dl.acm.org/doi/abs/10.1145/1837210.1837222)] Daniel
  Kunkle, Vlad Slavici, Gene Cooperman. “*Parallel Disk-Based Computation for
  Large, Monolithic Binary Decision Diagrams*”. In: *PASCO '10: Proceedings of
  the 4th International Workshop on Parallel and Symbolic Computation*. 2010
