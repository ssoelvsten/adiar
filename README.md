# Adiar
[![MIT License](https://img.shields.io/github/license/ssoelvsten/adiar)](LICENSE.md)
&nbsp;
[![Zenodo](https://img.shields.io/badge/doi-10.5281%2Fzenodo.4718218-blue)](https://doi.org/10.5281/zenodo.4718218)

[![Documentation](https://img.shields.io/website?down_message=not%20available&label=docs&up_message=available&url=https%3A%2F%2Fssoelvsten.github.io%2Fadiar)](https://ssoelvsten.github.io/adiar)
&nbsp;
[![test](http://img.shields.io/github/actions/workflow/status/ssoelvsten/adiar/linux.yml?branch=main&label=tests%20%28linux%29&logo=github&logoColor=white)](https://github.com/SSoelvsten/adiar/actions/workflows/linux.yml)
&nbsp;
[![test](http://img.shields.io/github/actions/workflow/status/ssoelvsten/adiar/mac.yml?branch=main&label=tests%20%28mac%29&logo=github&logoColor=white)](https://github.com/SSoelvsten/adiar/actions/workflows/mac.yml)
&nbsp;
[![test](http://img.shields.io/github/actions/workflow/status/ssoelvsten/adiar/windows.yml?branch=main&label=tests%20%28windows%29&logo=github&logoColor=white)](https://github.com/SSoelvsten/adiar/actions/workflows/windows.yml)
&nbsp;
[![codecov](https://img.shields.io/codecov/c/github/ssoelvsten/adiar?logo=codecov&logoColor=white&token=106RCIR4DJ)](https://codecov.io/gh/SSoelvsten/adiar)

Based on the work of Lars Arge [[Arge96](#references)],
Adiar[&#185;](#footnotes) is a BDD package [[Bryant86](#references)] that makes
use of time-forward processing to improve the I/O complexity of BDD
manipulation. This makes it able to achieve efficient manipulation of BDDs, even
when they outgrow the memory limit of the given machine.


<div align="center">
  <img src="/docs/tikz/queens.png"
       alt="Running Time of Adiar and other BDD packages solving the N-Queens problem"
       style="max-width:32rem; width:32rem;" />
</div>
<div align="center">
  <p style="margin-bottom:1rem;">Figure: Running time solving N Queens (lower is better).</p>
</div>

This project has been developed at the
[Programming Languages, Logic, and Software
Security](https://cs.au.dk/research/programming-languages-logic-and-software-security)
section at [Aarhus University](https://cs.au.dk).

If you have obtained a copy of Adiar via a fork or downloaded it from Zenodo,
then it might be lacking some features or bug fixes. You can find the most
recent version at the following repository.

<div align="center">
  https://github.com/ssoelvsten/adiar/
</div>

**Table of Contents**

- [Documentation](#documentation)
- [Dependencies](#dependencies)
- [Usage](#usage)
    - [Makefile Targets](#makefile-targets)
    - [Playground](#playground)
    - [Examples](#examples)
- [Contributions](#contributions)
- [License](#license)
- [Citing this project](#citing-this-project)
- [References](#references)


## Documentation
The documentation is available on
[Github Pages](https://ssoelvsten.github.io/adiar/). To compile it locally, you
need *CMake* and *Doxygen* (see [Dependencies](#dependencies) and
[Usage](#usage)).

## Dependencies
The implementation is dependant on the following external libraries

- [**TPIE**](https://github.com/thomasmoelhave/tpie):

  Framework for implementation of I/O efficient algorithms. Among other things,
  it provides an implementation of files, streams, sorting algorithms and
  priotity queues. Both are much faster than the algorithms in the *C++*
  standard library [[Vengroff94](#references), [Mølhave12](#references)].

- [**Bandit**](https://github.com/banditcpp/bandit):

  Unit testing framework that is both easy to read and write.

- [**CNL**](https://github.com/johnmcfarlane/cnl):

  Templated and efficient implementation of fixed-precision numbers.

All of these are included within the repository as submodules. If you have not
cloned the repository recursively, then run the following command

```bash
git submodule update --init --recursive
```

Other dependencies that we cannot provide as a submodule are shown below. The
*ticked* dependencies are mandatory to have installed.

- [x] **CMake** (3.21+) and a **C++ compiler** (C++17)

  Adiar compiles with the *GNU* (10+), *Clang* (12+), and *MSVC* C++ compilers.
  We do not monitor compatibility with other compilers, so we cannot guarantee
  they will work out-of-the-box.

- [x] **Boost** (1.74.0+)

  Furthermore, TPIE has a dependency on the [Boost](https://www.boost.org/)
  Library.

- [ ] **Doxygen** (1.8.0+)

  The documentation is created with the [Doxygen](https://www.doxygen.nl/)
  documentation generator.

- [ ] **DOT**

  As a visual aid, the internal representation of the Decision Diagrams can be
  output as
  [.dot](https://en.wikipedia.org/wiki/DOT_(graph_description_language)) files.
  These can then be turned into a graphical representation by use of a number of
  tools, such as *graphviz* (2.40+).

- [ ] **LCOV**

  To analyse code coverage and to produce the reports, we use the *LTP GCOV extension code coverage
  tool*

To install all of the above, run the respective command below.

| Operating System | Shell command                                                             |
|------------------|---------------------------------------------------------------------------|
| Ubuntu 22+       | `apt install        cmake g++     libboost-all-dev doxygen graphviz lcov` |
| Fedora 36+       | `dnf install        cmake gcc-c++ boost-devel      doxygen graphviz lcov` |
| Arch Linux       | `pacman -S --needed cmake gcc     boost-libs       doxygen graphviz lcov` |

## Usage

For how to use Adiar in your project, see the
[Getting Started](https://ssoelvsten.github.io/adiar/page__getting_started.html)
page in the documentation.

### Makefile Targets

The project is build with *CMake*, though for convenience I have simplified the
*CMake* interactions to a single *Makefile* which works on a local machine.

The *Makefile* provides the following targets

| target         | effect                                                |
|----------------|-------------------------------------------------------|
| `build`        | Build the source files                                |
| `docs`         | Build the documentation files                         |
| `clean`        | Remove all build files                                |
|                |                                                       |
| `test`         | Build and run all unit tests                          |
| `test/...`     | Build and run a subset of the unit tests              |
|                |                                                       |
| `coverage`     | Build and run all unit tests and create *lcov* report |
|                |                                                       |
| `clang/format` | Format all files in *src/* and *test/*                |

### Playground

To quickly get started running small pieces of code, we provide in
*src/playground.cpp* a tiny program where all of the boiler-plate is taken care
of. This can be useful for trying out a feature, creating a minimal example for
a bug report, and to guide development of new features.

To build and run *src/playground.cpp* with `<MiB>` MiB of
memory (default: 1024 MiB), just run the following make command.
```bash
make playground M=<MiB>
```

### Examples

The *example/* folder contains larger examples for how to use Adiar. The
*README.md* file in said folder contains a more in-depth description of each of
the examples. For benchmarking Adiar against other BDD packages, see the
[BDD Benchmarking repository](https://github.com/SSoelvsten/bdd-benchmark).

You can use `make examples/<name>` to compile and run them.

## Contributions

Adiar is not yet feature complete. If you notice anything is missing, please
open an [issue](https://github.com/SSoelvsten/adiar/issues/new).

Your contribution to the project is also very welcome! We list multiple
“[issues](https://github.com/SSoelvsten/adiar/issues)” where you can help -
these tasks range from the smallest of tasks to entire student projects. Please
get familiar with our [Contribution Guidelines](/CONTRIBUTING.md) before you
start to work on something.


## License
The software and documentation files in this repository are provided under the
[MIT License](/LICENSE.md).

Using Adiar will indirectly use [TPIE](https://github.com/thomasmoelhave/tpie)
underneath, which in turn is licensed under the _LGPL v3_ license. Hence, a
binary of yours that is statically linked to Adiar will be affected by that
license. That is, if you share that binary with others, then you will be obliged
to make the source public. This can be resolved by using Adiar as a shared
library or have it use an alternative to TPIE, such as
[STXXL](https://github.com/stxxl/stxxl).


## Citing this project

If you use Adiar in some of your academic work, then please consider to cite one
or more of the papers in
[docs/papers/cite.md](https://ssoelvsten.github.io/adiar/page__cite.html).


## References

- [[Arge96](https://tidsskrift.dk/brics/article/view/20010/17643)]
  Lars Arge. “*The I/O-complexity of Ordered Binary-Decision Diagram
  Manipulation*”. In: *Efficient External-Memory Data Structures and
  Applications*. (1996)

- [[Bryant86](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1676819)]
  Randal E. Bryant. “*Graph-Based Algorithms for Boolean Function Manipulation*”.
  In: *IEEE Transactions on Computers*. (1986)

- [[Mølhave12](https://dl.acm.org/doi/pdf/10.1145/2367574.2367579)]
  Thomas Mølhave. “*Using TPIE for Processing Massive Data Sets in C++*”. (2012)

- [[Vengroff94](https://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.38.3030)]
  D.E. Vengroff. “*A transparent parallel I/O environment*”. In: *In Proc. 1994
  DAGS Symposium on Parallel Computation*. pp. 117–134 (1994)

## Footnotes

1. **adiar** &#10216; portugese &#10217; ( *verb* ) : to defer, to postpone
