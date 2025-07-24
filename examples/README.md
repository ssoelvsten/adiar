# Examples
![examples](https://github.com/SSoelvsten/adiar/workflows/examples/badge.svg?branch=main)

To provide examples of how to use *Adiar* in your project, we provide a few working examples. These
are also described in the [documentation](https://ssoelvsten.github.io/adiar/). More thorough
examples can be found in the [BDD Benchmarking Suite](https://github.com/SSoelvsten/bdd-benchmark).

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Queens](#queens)

<!-- markdown-toc end -->

## Queens

**Files:** `queens.cpp`

**Target:** `make examples/queens N=<?> M=<?>`

Solve the N-Queens problem for *N = `?`* (default: `8`). This is done by
constructing an BDD row-by-row that represents whether the row is in a legal
state: is at least one queen placed on each row and is it also in no conflicts
with any other? On this BDD we then counts the number of satisfying
assignments [[kunkle10](#references)].

To list the solutions, we take the same BDD, but instead of counting the number
of assignments within, we use it to prune a recursive enumeration of all
possible queen placements. For up to *N = 8* we run both the solution counting
procedure and the enumeration procedure.

You can choose the amount (MiB) of internal memory, *M*, to be used by *Adiar*.
The default is 1024 MiB.

**Notice**: This is a pretty simple example and has all of the normal
shortcomings for BDDs trying to solve the N-Queens problem. At around *N* = 14
the intermediate sizes explodes a lot. One can with about 100 GB of disk space
or memory compute it.

## References

- [[Kunkle10](https://dl.acm.org/doi/abs/10.1145/1837210.1837222)] Daniel
  Kunkle, Vlad Slavici, Gene Cooperman. “*Parallel Disk-Based Computation for
  Large, Monolithic Binary Decision Diagrams*”. In: *PASCO '10: Proceedings of
  the 4th International Workshop on Parallel and Symbolic Computation*. 2010
