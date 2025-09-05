\page page__amend Amendments

Despite the best efforts of us and the reviewers, some of the published papers
include errors. Furthermore, throughout the development of Adiar, we have
revisited, revised, and iterated upon our previous ideas. What follows is a
record of errors and changes we have made.

[TOC]

Error: Reduce PseudoCode
========================

Figure 6 of the [TACAS
22](https://link.springer.com/chapter/10.1007/978-3-030-99527-0_16) paper has an
error on line 3. It should be `Q_red != Ø or F_leaf.has_next()`; this has been
fixed in the [extended arXiv paper](https://arxiv.org/abs/2104.12101).

Change: 64-bit Encoding of Pointers
========================

In the [extended arXiv paper](https://arxiv.org/abs/2104.12101) for [[TACAS
22](https://link.springer.com/chapter/10.1007/978-3-030-99527-0_16)], we show in
Section 3.5 how to encode the *unique identifier* into a single 64-bit integer.
This is (1) to decrease the BDD size and especially (2) to make the pointer
sorting operations as fast as possible.

v1.2
------------------------

It was always the intention that the *flag* bit on children should be used for
the addition of *complement edges* in BDDs. Yet, up to this point it was left
unused. Hence, we reassigned the meaning of this bit to instead propagate an
edge being *tainted* by the decision diagram's reduction rule [[ATVA
23](https://link.springer.com/chapter/10.1007/978-3-031-45332-8_4)].

v2.0
------------------------

For *Nested Sweeping*, we need to *taint* an arc's *source* to be originating
from the *outer sweep*. Yet, the *flag* was up to this point used to store the
*out-index* of the source. Hence, for *internal nodes* we introduce the
out-index (*p*) on the second least-significant bit - essentially adding a
second (possibly non-Boolean) *flag*.

**Figure 10 (b):**

|     |                            |                                         |     |     |
|-----|----------------------------|-----------------------------------------|-----|-----|
| `0` | `. . . . . label. . . . .` | `. . . . . . . identifier. . . . . . .` | `o` | `f` |

At this point, *o* only uses 1 bit, but in the case of *Quantum Multi-valued
Decision Diagrams* this would use 2 bits to handle the 4-ary outdegree.

Yet, the above introduction of the out-index, *o*, removes a bit from the number
of *level identifiers*. This decreases the maximum width without an overflow
downto only 3 TiB. Hence, the maximum size of a BDD without possible overflow is
only 6 TiB.

To get the maximum width safe from overflows back up to 6 TiB, we have removed
the most significant *terminal bit flag*. Instead, now all pointers dedicate the
ℓ most significant bits to its *level*. The largest level is dedicated to *null*
and the second-largest to *terminals*. This decreases the number of BDD labels
by 2 and also restricts *terminal* values to fit into 64-ℓ-1-1 = 62-ℓ bits. Yet,
this is fine for `bool`, `char`, `int`, and `float`.

**Figure 10 (a):**

|                            |                                          |     |
|----------------------------|------------------------------------------|-----|
| `. . . . . level. . . . .` | `. . . . . . . . . v. . . . . . . . . .` | `f` |

**Figure 10 (b):**

|                            |                                          |     |     |
|----------------------------|------------------------------------------|-----|-----|
| `. . . . . level. . . . .` | `. . . . . . . identifier . . . . . . .` | `o` | `f` |
