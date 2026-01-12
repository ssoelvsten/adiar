\page page__cite Citing this Project

Please consider to cite one or more of the following *relevant* papers, if you
use Adiar in some of your academic work. By relevant, we ask you to cite the
paper(s) that "generated" the cited knowledge.

[TOC]

Lars Arge
========================

Adiar is based on the theoretical work of Lars Arge back in the 90s.

- Lars Arge.
  “[*The I/O-complexity of Ordered Binary-Decision Diagram Manipulation*](https://link.springer.com/chapter/10.1007/BFb0015411)”.
  In: *International Symposium on Algorithms and Computation* (ISAAC). (1995)
  ```bibtex
  @InProceedings{arge1995:ISAAC,
    title     = {The {I/O}-complexity of Ordered Binary-Decision Diagram manipulation},
    author    = {Arge, Lars},
    editor    = {Staples, John
             and Eades, Peter
             and Katoh, Naoki
             and Moffat, Alistair},
    booktitle = {Sixth International Symposium on Algorithms and Computation},
    year      = {1995},
    publisher = {Springer Berlin Heidelberg},
    address   = {Berlin, Heidelberg},
    series    = {Lecture Notes in Computer Science},
    volume    = {1004},
    pages     = {82--91},
    isbn      = {978-3-540-47766-2},
    doi       = {10.1007/BFb0015411},
  }
  ```

- Lars Arge.
  “[*The I/O-complexity of Ordered Binary-Decision Diagram Manipulation*](https://tidsskrift.dk/brics/issue/view/2576)”.
  In: *BRICS RS Preprint*. (1996)
  ```bibtex
  @InProceedings{arge1996:BRICS,
    author    = {Arge, Lars},
    title     = {The {I/O}-Complexity of Ordered Binary-Decision Diagram manipulation},
    booktitle = {BRICS RS preprint series},
    volume    = {29},
    year      = {1996},
    publisher = {Department of Computer Science, University of Aarhus},
    doi       = {10.7146/brics.v3i29.20010},
  }
  ```


Adiar
========================

We have, for your convenience, grouped all papers based on the semantic version
of Adiar. Depending on what feature of Adiar that you are referring to, then
pick the most relevant paper(s) to cite.

v1.0
------------------------

With [v1.0.0](https://github.com/SSoelvsten/adiar/releases/tag/v1.0.0) (and its
patch [v1.0.1](https://github.com/SSoelvsten/adiar/releases/tag/v1.0.1)) we
provide an implementation of Arge's algorithms and also add non-trivial
optimisations, extensions, and new theoretical contributions to make it usable
in practice.

- Steffan Christ Sølvsten, Jaco van de Pol, Anna Blume Jakobsen, Mathias Weller Berg Thomasen.
  “[*Adiar: Binary Decision Diagrams in External Memory*](https://link.springer.com/chapter/10.1007/978-3-030-99527-0_16)”.
  In: *Tools and Algorithms for the Construction and Analysis of Systems* (TACAS). (2022)
  ```bibtex
  @InProceedings{soelvsten2022:TACAS,
    title         = {{Adiar}: Binary Decision Diagrams in External Memory},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco
                 and Jakobsen, Anna Blume
                 and Thomasen, Mathias Weller Berg},
    year          = {2022},
    booktitle     = {Tools and Algorithms for the Construction and Analysis of Systems},
    editor        = {Fisman, Dana
                 and Rosu, Grigore},
    pages         = {295--313},
    numPages      = {19},
    publisher     = {Springer},
    series        = {Lecture Notes in Computer Science},
    volume        = {13244},
    isbn          = {978-3-030-99527-0},
    doi           = {10.1007/978-3-030-99527-0\_16},
  }
  ```

- Steffan Christ Sølvsten, Jaco van de Pol, Anna Blume Jakobsen, Mathias Weller Berg Thomasen.
  “[*Efficient Binary Decision Diagram Manipulation in External Memory*](https://arxiv.org/abs/2104.12101)”.
  In: *arXiv preprint*. (2021)
  ```bibtex
  @Misc{soelvsten2021:arXiv,
    title         = {Efficient Binary Decision Diagram Manipulation in External Memory},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco
                 and Jakobsen, Anna Blume
                 and Thomasen, Mathias Weller Berg},
    year          = {2021},
    archivePrefix = {arXiv},
    eprint        = {2104.12101},
    primaryClass  = {cs.DS},
    numPages      = {38},
    howPublished  = {arXiv},
    url           = {https://arxiv.org/abs/2104.12101},
  }
  ```

v1.1
------------------------

With [v1.1.0](https://github.com/SSoelvsten/adiar/releases/tag/v1.1.0) we add
support for *Zero-suppressed Decision Diagrams* to *Adiar*.

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Adiar 1:1: Zero-suppressed Decision Diagrams in External Memory*](https://link.springer.com/chapter/10.1007/978-3-031-33170-1_28)”.
  In: *NASA Formal Methods* (NFM). (2023)
  ```bibtex
  @InProceedings{soelvsten2023:NFM,
    title         = {{A}diar 1.1: {Z}ero-suppressed {D}ecision {D}iagrams in {E}xternal {M}emory},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco},
    year          = {2023},
    booktitle     = {NASA Formal Methods},
    editor        = {Rozier, Kristin Yvonne
                 and Chaudhuri, Swarat},
    pages         = {464--471},
    numPages      = {8},
    publisher     = {Springer},
    series        = {Lecture Notes in Computer Science},
    volume        = {13903},
    doi           = {10.1007/978-3-031-33170-1\_28},
  }
  ```

v1.2
------------------------

With [v1.2.0](https://github.com/SSoelvsten/adiar/releases/tag/v1.2.0) (and its
patches [v1.2.1](https://github.com/SSoelvsten/adiar/releases/tag/v1.2.1),
[v1.2.2](https://github.com/SSoelvsten/adiar/releases/tag/v1.2.2)) we introduce
the notion of an *i-level cut* of a graph and use these to derive sound upper
bounds on the size of the data structures. This drastically decreases the
threshold as to when *Adiar*'s running time is only a small constant factor
slower than other implementations.

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Predicting Memory Demands of BDD Operations Using Maximum Graph Cuts*](https://link.springer.com/chapter/10.1007/978-3-031-45332-8_4)”.
  In: *Automated Technology for Verification and Analysis* (ATVA). (2023)
  ```bibtex
  @InProceedings{soelvsten2023:ATVA,
    title     = {Predicting Memory Demands of {BDD} Operations Using Maximum Graph Cuts},
    author    = {S{\o}lvsten, Steffan Christ
             and van de Pol, Jaco},
    booktitle = {Automated Technology for Verification and Analysis},
    year      = {2023},
    editor    = {Andr{\'e}, {\'E}tienne
             and Sun, Jun},
    pages     = {72--92},
    numPages  = {21},
    publisher = {Springer},
    series    = {Lecture Notes in Computer Science},
    volume    = {14216},
    isbn      = {978-3-031-45332-8},
    doi       = {10.1007/978-3-031-45332-8\_4}
  }
  ```

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Predicting Memory Demands of BDD Operations Using Maximum Graph Cuts (Extended Paper)*](https://arxiv.org/abs/2307.04488)”.
  In: *arXiv preprint*. (2023)
  ```bibtex
  @Misc{soelvsten2023:arXiv,
    title         = {Predicting Memory Demands of {BDD} Operations using Maximum Graph Cuts (Extended Paper)},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco},
    year          = {2023},
    archivePrefix = {arXiv},
    eprint        = {2307.04488},
    primaryClass  = {cs.DS},
    numPages      = {25},
    howPublished  = {arXiv},
    url           = {https://arxiv.org/abs/2307.04488},
  }
  ```


v2.0
------------------------
With [v2.0](https://github.com/SSoelvsten/adiar/releases/tag/v2.0.0), we
introduce the *Nested Sweeping* framework as a way to more closely recreate the
more complex BDD operations within the *time-forward processing* algorithmic
paradigm in Adiar. This allows us, among other things, to implement
*multi-variable quantification* more akin to the one in other BDD packages.

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Multi-variable Quantification of BDDs in External Memory using Nested Sweeping*](https://link.springer.com/chapter/10.1007/978-3-032-15700-3_17)”.
  In: *Verification, Model Checking, and Abstract Interpretation* (VMCAI). (2026)
  ```bibtex
  @Misc{soelvsten2024:arXiv,
    title         = {Multi-variable Quantification of {BDD}s in External Memory using Nested Sweeping},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco},
    year          = {2026},
    editor        = {Chen, Yu-Fang
                and Jensen, Thomas
                and Leng{\'a}l, Ond{\v{r}}ej},
    booktitle     = {Verification, Model Checking, and Abstract Interpretation},
    publisher     = {Springer},
    pages         = {359--382},
    series        = {Lecture Notes in Computer Science},
    volume        = {16417},
    isbn          = {978-3-032-15700-3},
    doi           = {10.1007/978-3-032-15700-3\_17},
  }
  ```

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Multi-variable Quantification of BDDs in External Memory using Nested Sweeping (Extended Paper)*](https://arxiv.org/abs/2408.14216)”.
  In: *arXiv preprint*. (2024)
  ```bibtex
  @Misc{soelvsten2024:arXiv,
    title         = {Multi-variable Quantification of {BDD}s in External Memory using Nested Sweeping (Extended Paper)},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco},
    year          = {2024},
    archivePrefix = {arXiv},
    eprint        = {2408.14216},
    primaryClass  = {cs.DS},
    numPages      = {26},
    howPublished  = {arXiv},
    url           = {https://arxiv.org/abs/2408.14216},
  }
  ```

Furthermore, this version also introduces *levelized random access* to BDDs to
further improve performance of the product construction algorithms.

- Steffan Christ Sølvsten, Casper Moldup Rysgaard, Jaco van de Pol.
  “[*Random Access on Narrow Decision Diagrams in External Memory*](https://link.springer.com/chapter/10.1007/978-3-031-66149-5_7)”.
  In: *Model Checking Software* (SPIN). (2024)
  ```bibtex
  @InProceedings{soelvsten2024:SPIN,
    title     = {Random Access on Narrow Decision Diagrams in External Memory},
    author    = {S{\o}lvsten, Steffan Christ
             and Rysgaard, Casper Moldrup
             and van de Pol, Jaco},
    booktitle = {Model Checking Software},
    year      = {2024},
    editor    = {Neele, Thomas
             and Wijs, Anton},
    pages     = {137--145},
    numPages  = {9},
    publisher = {Springer},
    series    = {Lecture Notes in Computer Science},
    volume    = {14624},
    isbn      = {978-3-031-66148-8},
    doi       = {10.1007/978-3-031-66149-5\_7}
  }
  ```


v2.1
------------------------
With [v2.1](https://github.com/SSoelvsten/adiar/releases/tag/v2.1.0), we add
the *Relational Product* such that Adiar can be used in the context of model
checking.

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Symbolic Model Checking in External Memory*](https://arxiv.org/abs/2505.11229)”.
  In: *arXiv preprint*. (2025)
  ```bibtex
  @Misc{soelvsten2025:arXiv,
    title         = {Symbolic Model Checking in External Memory},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco},
    year          = {2025},
    archivePrefix = {arXiv},
    eprint        = {2505.11229},
    primaryClass  = {cs.DS},
    numPages      = {22},
    howPublished  = {arXiv},
    url           = {https://arxiv.org/abs/2505.11229},
  }
  ```

Future Work
------------------------
Next to the papers above, the Sølvsten's PhD thesis also explores (1) how to design *Variable
Reordering* algorithms for large instances and (2) how to integrate Adiar's I/O algorithms with
conventional recursive algorithms via a *Unique Node Table*.

- Steffan Christ Sølvsten.
  “[*I/O-efficient Symbolic Model Checking*](https://soeg.kb.dk/permalink/45KBDK_KGL/1pioq0f/alma99126389524805763)”.
  In: *Royal Library, Denmark*. (2025)
  ```bibtex
  @Thesis{soelvsten2025:thesis,
    title         = {{I/O}-efficient Symbolic Model Checking},
    author        = {S{\o}lvsten, Steffan Christ},
    year          = {2025},
    month         = {05},
    school        = {Aarhus University},
    type          = {PhD thesis},
    url           = {https://soeg.kb.dk/permalink/45KBDK_KGL/1pioq0f/alma99126389524805763}
  }
  ```

Student Projects
------------------------
Clausen and Nielsen investigated whether it is possible to translate the algorithm by [Savický &
Wegener](https://doi.org/10.1007/s002360050083) to Adiar. Despite of the good work they did, their
results were negative. Yet, this spawned the ideas in Sølvsten's thesis.

- Anders Benjamin Clausen and Kent Nielsen.
  “*I/O-efficient Static Variable Reordering for Binary Decision Diagrams*”.
  (2023)
  ```bibtex
  @Thesis{Clausen2022,
    title   = {{I/O}-efficient Static Variable Reordering for Binary Decision Diagrams},
    author  = {Clausen, Anders Benjamin
           and Nielsen, Kent},
    year    = {2023},
    school  = {Department of Computer Science, University of Aarhus},
    type    = {Bachelor's Thesis}
  }
  ```

Carstensen investigated the possibility to implement the `adiar::bdd_optmin` operation (part of
[v2.0](https://github.com/SSoelvsten/adiar/releases/tag/v2.0.0)) which can be used to obtain the
smallest satisfying assignment according to some linear cost function.

- Erik Funder Carstensen.
  “*Solving optimization problems I/O efficiently: An extension to Adiar*”.
  (2024)
  ```bibtex
  @Misc{Carstensen2024,
    title   = {Solving optimization problems {I/O} efficiently: An extension to {Adiar}},
    author  = {Carstensen, Erik Funder},
    year    = {2024},
    school  = {Department of Computer Science, University of Aarhus},
    type    = {Master's Course Report}
  }
  ```
