# Contribution Guidelines

Thank you for spending your valuable time contributing to the *Adiar* research
project! Below you find information relevant for helping you to get started with
contributing to Adiar.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Getting Started](#getting-started)
    - [Visual Studio Code](#visual-studio-code)
- [Git Practices](#git-practices)
    - [Commits and Messages](#commits-and-messages)
    - [Linear History](#linear-history)
    - [Branching](#branching)
- [Code Formatting](#code-formatting)
- [Generative AI](#generative-ai)
- [Design Principles](#design-principles)
    - [§1 Functional-First](#1-functional-first)
    - [§2 Interoperability with Users' Data Structures](#2-interoperability-with-users-data-structures)
    - [§3 Run-time over Compilation-time](#3-run-time-over-compilation-time)
    - [§4 Naming Scheme](#4-naming-scheme)
    - [§5 Documentation Comments Everywhere](#5-documentation-comments-everywhere)
    - [§6 Test Everything Thoroughly](#6-test-everything-thoroughly)
    - [§7 C++ Style](#7-c++-style)

<!-- markdown-toc end -->


## Getting Started

See the [README](./README.md) for information on all dependencies and how to
build *Adiar* and run its unit tests.

### Visual Studio Code

If you use Visual Studio Code as your editor, please have the following three
*extensions* installed.

| Name                         | ID                        |
|------------------------------|---------------------------|
| **CMake Tools**              | ms-vscode.cmake-tools     |
| **C/C++**                    | ms-vscode.cpptools        |
| **EditorConfig for VS Code** | EditorConfig.EditorConfig |

The following two are also recommended:

| Name                  | ID                           |
|-----------------------|------------------------------|
| **C/C++ Themes**      | ms-vscode.cpptools-themes    |
| **Better C++ Syntax** | jeff-hykin.better-cpp-syntax |

## Git Practices

### Commits and Messages

- Commit Messages are written in imperative, their first word is capitalized,
  and they are written in a way that is pretty in ASCII (not only Markdown),
  e.g. "*Turn 'tuple' into a class*".
- Only **one** change per commit, i.e. a commit message that includes the word
  "and" should be avoided as much as is possible.
- Unit tests for a feature are included in the commit that adds said feature.
  They do not need to be mentioned as part of the commit message.
- Each commit should be possible to build and run *test/* and *src/main.cpp*.
  If you forgot to stage a change or introduced a bug, please *rebase* your
  branch and add the *fixup*.

### Linear History

The *main* branch is kept with a linear history. Hence, all pull requests are
always *rebased* on-top of main.

### Branching

On this repository, we try to follow a simplified version of *git flow* that
aims for shorter lived branches. That is, something between *git flow* and
*trunk based development*. While we try to never break the *main* branch,
refactoring and clean up needed for later features should be merged in early
(and independently).

**Branch Names**

If you are a collaborator on the main repository
(*github.com/SSoelvsten/adiar/*), your branches should try to use the following
prefixes in their name.

- `bdd/<op>/<name>` and `zdd/<op>/<name>`:
  Branches for a specific operation of decision diagram. For example, making
  change *X* to the `bdd_apply` operation would be on branch `bdd/apply/X`. On
  the other hand, adding `bdd_compose` would be on branch `bdd/compose`

- `internal/<subject>/<name>`:
  Branches that primarily change the generalised algorithms and data structures
  in *<adiar/internal/...>*. For example, a clean-up of the *prod2* algorithm
  would be `internal/prod2/clean-up` while adding `has_top()` to the levelized
  priority queue would be `internal/levelized_priority_queue/has_top`.

- `docs/<name>`:
  Changes, fixes and additions to the documentation, i.e. Markdown and
  Doxygen files and documentation comments in the *src/* folder.

There are multiple reasons, we do not use the standard `feature/...` and
`bug/...` branches. First of all, a considerable number of branches are not a
*user feature* but an *optimisation* or a *refactoring*. Both of these should
(if possible) be merged into *main* before development of a feature begins;
doing so decreases the number of conflicts with other branches. Furthermore,
regular git flow branch names would include the above anyway. So, this scheme
keeps the branch names shorter and hence slightly more legible.

The following branch prefixes are reserved.

- `project/<year>/<name>`:
  These branches contain local copies of previous student projects.

- `experiment/<version>/<name>`:
  These are stale branches that contain algorithmic changes needed for
  experimental evaluation but should never be merged into *main*.

- `gh-pages`:
  This is the output branch for the latest Doxygen build of `main`.

## Code Formatting

Most of Adiar has been developed with the
[Spacemacs](https://www.spacemacs.org/) extension of the Emacs editor. Hence,
code in Adiar is indented as dictated by Emacs (version 28 or later). We have
set up *Clang Format* to follow the same style (together with other formatting
requirements).

Before committing anything, please ensure it is properly formatted.

## Generative AI

**Contributions should be without assistance of Generative AI, such as Copilot.**

There are numerous reasons - legally and especially ethically - to be skeptical
towards and critical of the use of AI. For example, using it is at the detriment
to your ability to acquire the skills and knowledge needed to understand the
language, the codebase, the data structures, and the algorithms. Hence, using it
will inevitably introduce bugs due to a lack of understanding or negligence.
Furthermore, it is merely plagiarism with a few extra steps.

Any situation in which an AI assistant would be of benefit to you is a sign of a
missing abstraction and/or documentation. Hence, the underlying issue should
be resolved instead. Please do so in an Issue and/or a Pull Request.

## Design Principles

### §1 Functional-First

The algorithms of Adiar lend themselves very well to a functional style: if the
priority queues are abstracted to ordered sets then the algorithm can in fact be
rephrased as very simple tail-recursive operations on lists. For this C++
implementation, this has the vital benefit of allowing us to easily unit test
every class and function independently and in general decrease the code
complexity. In turn, this decreases the number of errors.

Hence, all code should be written in a functional style insofar it does not
negatively impact performance. We trust the compiler to find the last few
optimisations.

#### §1.1 Public API is Functional

- Member access (and its derived state) are available through functions.
- Manipulation of decision diagrams are handled through functions that have no
  (observable) side-effects on the input.

#### §1.2 Data Types are Immutable

The internal data types, e.g. *ptr_uint64* and *node*, are all treated as
immutable values.

- Member access (and its derived state) are available through member functions.
- Manipulation of data types are handled through functions that have no
  side-effects and instead return a new separate instance of the changed object.

#### §1.3 Imperative Exceptions

For the sake of performance, the only exception to this principle is the
interaction with (1) files, (2) data structures, e.g. the priority queues and the
input streams, and (3) the call stack.

### §2 Interoperability with Users' Data Structures

As a library, Adiar should not enforce the type of any data structure provided
by the user. For example, the `bdd_replace` function of BuDDy breaks this
goal by requiring the user to use a (C-style) vector of `bddPair`s.
```cpp
bdd
bdd_replace(const bdd&, bddPair*);
```

Instead, all information should be passed through a general interface by use of
functions (e.g. `adiar::predicate` and `adiar::generator`.), streams
(e.g. `std::ostream`), and templated iterators. For example, Adiar's version of
the same function should be
```cpp
bdd
bdd_replace(const bdd&, const function<bdd::label_type(bdd::label_type)>&);

bdd
bdd_replace(const bdd&, const generator<pair<bdd::label_type, bdd::label_type>>&);

template <typename ForwardIt>
bdd
bdd_replace(const bdd&, ForwardIt begin, ForwardIt end);
```

### §3 Run-time over Compilation-time

A primary goal of Adiar is high-performance computing. Hence, we want to
sacrifice the compilation time in favour of improving the running time, e.g.
polymorphism is achieved with templates rather than virtual functions.

Yet, the code should still be kept maintainable, i.e. with separated
responsibilities and without any code duplication. Hence, the code base ought to
be modularised with a [policy-based
design](https://en.wikipedia.org/wiki/Modern_C%2B%2B_Design#Policy-based_design).

#### §3.1 Do Not Expose Templates

The above leads to the creation of templates only intended for use within Adiar.
None of these should (if possible) be exposed in a header file to the end user,
but instead be compiled into a non-templated function within a relevant CPP
source file.

### §4 Naming Scheme

The codebase uses [snake case](https://en.wikipedia.org/wiki/Snake_case) as much
as possible. This nicely aligns Adiar's codebase with the *std* (and the *tpie*)
namespaces that are interspersed with Adiar's own code.

> **NOTE:** The entire public API, i.e. everything in the *adiar* namespace,
> should already at this point adhere to the naming scheme below. Yet, the
> internal logic in *adiar::internal* may still not be fully up to date. Please
> fix these when you see them. But, please do those changes in a commit
> independent of the one that includes other changes.

**Casings**

In general, we try to follow the following casing rules.

- *Namespaces*, *classes*, *variables*, and *functions* use `snake_case`.
- Preprocessing variables (and only these) use `SNAKE_CASE`.
- Template parameters use `CamelCase`.
- Enum values cannot use *snake_case* or *SNAKE_CASE* since that may clash with
  keywords or preprocessing variables. Hence, we have settled on `Snake_Case`.

Yet, we run into problems with Adiar's public API. What we really want is to
have the `bdd` and `zdd` "class" prefix all functions as a namespace, e.g. we
would write `adiar::bdd h = adiar::bdd::and(f,g)`. Yet, we cannot do so since
`and`, `or`, and `xor` are keywords in C++. So we need some alternative. Yet,
each is a compromise in some way. Currently, we have chosen to stick with a
naming scheme that aligns with the C API of *BuDDy* and *Sylvan* and does not
clash too much with the use of *snake_case*:

- All functions of the public API are of the form `{prefix}_{functionname}`,
  e.g. `bdd_and(f,g)` by prepending `bdd_` onto the `and(f,g)` function name.
  Functions names that consist of multiple words, e.g. *is true*, is written in
  *nocase*, e.g. `bdd_istrue(f)`.

**Prefixes and Suffixes**

- Type variables exposed from a class have their names suffixed with `_type`.
  Template type parameters are written in `CamelCase` whereas global types, e.g.
  classes, are written in `snake_case` with *no* suffix (except if there is a
  good reason to do it).

  This aligns with *std* and prevents shadowing between the three "kinds" (types
  of types).
- Private class member variables are prefixed with a single `_`; non-static
  public ones may also be prefixed as such.

### §5 Documentation Comments Everywhere

As Adiar grows larger and more complex, one submodule is the foundation on which
others are built. Hence, **all** functions, data structures, classes, and
variables - even the ones in *adiar::internal* that are not part of the public
API - should be well documented for both the *end users* and especially also the
*developers*.

### §6 Test Everything Thoroughly

Adiar's Decision Diagrams are to be used in the context of verification of
critical software. At the same time, Adiar's algorithms are much more complex
than other BDD implementations and have multiple layers from which an error
could originate. Hence, it is vital we can ensure correctness of Adiar by having
as thorough unit testing as possible.

This also applies to everything within *adiar::internal* that is not part of the
public API. Similar to *§6*, this also helps us pin down the origin of a bug
since each level of the code-hierarchy is covered.

### §7 C++ Style

#### §7.A No Almost Always Auto!

Some C++ developers prefer to use the `auto` keyword as much as possible to let
the compiler derive the type. Yet, we want to **not** do so for multiple
reasons:

1. This way, we can be sure the derived type is as intended
2. Since we only use C++17 and do not have *concepts* available, making the type
   explicit allows IDE tools such as Visual Studio Code to provide hints during
   development.
3. This also conveys the programmer's intent to the reader and the compiler. If
   there is a type error, the problem may not be in the type but probably in the
   way it is derived.
4. We provide the information necessary to debug the code while reading it.

#### §7.B (Almost) Always Explicit Constructors!

Every constructor (except *default*, *copy*, and *move*) should be marked as `explicit` to mitigate
unintended implicit conversions. The only exception to this rule are if implicit conversions are
intended, e.g. `bdd` and `__bdd`.
