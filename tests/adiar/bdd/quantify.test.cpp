#include "../../test.h"
#include <algorithm>
#include <vector>

go_bandit([]() {
  describe("adiar/bdd/quantify.cpp", []() {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Sink only BDDs
    shared_levelized_file<bdd::node_type> terminal_F;

    { // Garbage collect writer to free write-lock}
      node_ofstream nw_F(terminal_F);
      nw_F << node(false);
    }

    shared_levelized_file<bdd::node_type> terminal_T;

    { // Garbage collect writer to free write-lock
      node_ofstream nw_T(terminal_T);
      nw_T << node(true);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // x2 variable BDD
    shared_levelized_file<bdd::node_type> bdd_x2;

    { // Garbage collect writer to free write-lock
      node_ofstream nw_x2(bdd_x2);
      nw_x2 << node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 1
    /*
    //   1     ---- x0
    //  / \
    //  T 2    ---- x1
    //   / \
    //   F T
    */
    shared_levelized_file<bdd::node_type> bdd_1;

    const node n1_2 = node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n1_1 = node(0, bdd::max_id, bdd::pointer_type(true), n1_2.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw_1(bdd_1);
      nw_1 << n1_2 << n1_1;
    }

    // BDD 1 above, but it is unreduced, i.e. there are duplicate and redundant nodes. The redundant
    // nodes also adds an additional level, with which we then can see it quantifies the unreduced
    // input (or not).
    /*
    //     1     ---- x0
    //    / \
    //    T 2    ---- x1
    //     / \
    //     3 T   ---- x2
    //     ||
    //     F
    */
    shared_levelized_file<arc> bdd_1__unreduced;
    {
      arc_ofstream aw_1(bdd_1__unreduced);

      const bdd::uid_type n1_3(2, 0);
      const bdd::uid_type n1_2(1, 0);
      const bdd::uid_type n1_1(0, 0);

      aw_1.push_internal({ n1_1, true, n1_2 });
      aw_1.push_internal({ n1_2, false, n1_3 });

      aw_1.push_terminal({ n1_1, false, bdd::pointer_type(true) });
      aw_1.push_terminal({ n1_2, true, bdd::pointer_type(true) });
      aw_1.push_terminal({ n1_3, false, bdd::pointer_type(false) });
      aw_1.push_terminal({ n1_3, true, bdd::pointer_type(false) });

      aw_1.push(level_info(0, 1u));
      aw_1.push(level_info(1, 1u));
      aw_1.push(level_info(2, 1u));

      bdd_1__unreduced->max_1level_cut = 1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 2
    /*
    //       1       ---- x0
    //      / \
    //     2   3     ---- x1
    //    / \ / \
    //   4   5  F    ---- x2
    //  / \ / \
    //  T F F T
    */
    shared_levelized_file<bdd::node_type> bdd_2;

    const node n2_5 = node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n2_4 = node(2, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));
    const node n2_3 = node(1, bdd::max_id, n2_5.uid(), bdd::pointer_type(false));
    const node n2_2 = node(1, bdd::max_id - 1, n2_4.uid(), n2_5.uid());
    const node n2_1 = node(0, bdd::max_id, n2_2.uid(), n2_3.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw_2(bdd_2);
      nw_2 << n2_5 << n2_4 << n2_3 << n2_2 << n2_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 3
    /*
    //       1       ---- x0
    //      / \
    //      |  2     ---- x1
    //      \ / \
    //       3   4   ---- x2
    //      / \ / \
    //      T F F T
    */
    shared_levelized_file<bdd::node_type> bdd_3;

    const node n3_4 = node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n3_3 = node(2, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));
    const node n3_2 = node(1, bdd::max_id, n3_3.uid(), n3_4.uid());
    const node n3_1 = node(0, bdd::max_id, n3_3.uid(), n3_2.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw_3(bdd_3);
      nw_3 << n3_4 << n3_3 << n3_2 << n3_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 4
    /*
    //       1       ---- x0
    //      / \
    //      |  2     ---- x1
    //      \ / \
    //       3   4   ---- x2
    //      / \ / \
    //      F  5  T  ---- x3
    //        / \
    //        F T
    */
    shared_levelized_file<bdd::node_type> bdd_4;

    const node n4_5 = node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n4_4 = node(2, bdd::max_id, n4_5.uid(), bdd::pointer_type(true));
    const node n4_3 = node(2, bdd::max_id - 1, bdd::pointer_type(false), n4_5.uid());
    const node n4_2 = node(1, bdd::max_id, n4_3.uid(), n4_4.uid());
    const node n4_1 = node(0, bdd::max_id, n4_3.uid(), n4_2.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw_4(bdd_4);
      nw_4 << n4_5 << n4_4 << n4_3 << n4_2 << n4_1;
    }

    // BDD 4 above, but it is unreduced, i.e. there are duplicate and redundant nodes. The redundant
    // nodes also add an additional level, with which we then can see it quantifies the unreduced
    // input (or not).
    /*
    //       _1_       ---- x0
    //      /   \
    //      |  _2_     ---- x1
    //      \ /   \
    //       3     4   ---- x2
    //      / \   / \
    //      F 5   6 T  ---- x3
    //       / \ / \
    //       F T F 7   ---- x4
    //             ||
    //             T
    */
    shared_levelized_file<arc> bdd_4__unreduced;
    {
      arc_ofstream aw_4(bdd_4__unreduced);

      const bdd::uid_type n4_7(4, 0);
      const bdd::uid_type n4_6(3, 1);
      const bdd::uid_type n4_5(3, 0);
      const bdd::uid_type n4_4(2, 1);
      const bdd::uid_type n4_3(2, 0);
      const bdd::uid_type n4_2(1, 0);
      const bdd::uid_type n4_1(0, 0);

      aw_4.push_internal({ n4_1, true, n4_2 });
      aw_4.push_internal({ n4_1, false, n4_3 });
      aw_4.push_internal({ n4_2, false, n4_3 });
      aw_4.push_internal({ n4_2, true, n4_4 });
      aw_4.push_internal({ n4_3, true, n4_5 });
      aw_4.push_internal({ n4_4, false, n4_6 });
      aw_4.push_internal({ n4_6, true, n4_7 });

      aw_4.push_terminal({ n4_3, false, bdd::pointer_type(false) });
      aw_4.push_terminal({ n4_4, true, bdd::pointer_type(true) });
      aw_4.push_terminal({ n4_5, false, bdd::pointer_type(false) });
      aw_4.push_terminal({ n4_5, true, bdd::pointer_type(true) });
      aw_4.push_terminal({ n4_6, false, bdd::pointer_type(false) });
      aw_4.push_terminal({ n4_7, false, bdd::pointer_type(true) });
      aw_4.push_terminal({ n4_7, true, bdd::pointer_type(true) });

      aw_4.push(level_info(0, 1u));
      aw_4.push(level_info(1, 1u));
      aw_4.push(level_info(2, 2u));
      aw_4.push(level_info(3, 2u));
      aw_4.push(level_info(4, 1u));

      bdd_4__unreduced->max_1level_cut = 3;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 5
    /*
    //    1      ---- x0
    //   / \
    //   ? 2     ---- x1
    //    / \
    //   3   4   ---- x2
    //  / \ / \
    //  T F F T
    */
    shared_levelized_file<bdd::node_type> bdd_5F;

    const node n5_4(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n5_3(2, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));
    const node n5_2(1, bdd::max_id, n5_3.uid(), n5_4.uid());

    { // Garbage collect writer to free write-lock
      const node n5_1(0, bdd::max_id, bdd::pointer_type(false), n5_2.uid());

      node_ofstream nw_5(bdd_5F);
      nw_5 << n5_4 << n5_3 << n5_2 << n5_1;
    }

    shared_levelized_file<bdd::node_type> bdd_5T;

    { // Garbage collect writer to free write-lock
      const node n5_1(0, bdd::max_id, bdd::pointer_type(true), n5_2.uid());

      node_ofstream nw_5(bdd_5T);
      nw_5 << n5_4 << n5_3 << n5_2 << n5_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 6
    /*
    //         ___1__        ---- x0
    //        /      \
    //      _3__   __2__     ---- x1
    //     /    \ /     \
    //     4     6      5    ---- x2
    //    / \   / \    / \
    //    F  \ /  T   /  F
    //        8       7      ---- x3
    //       / \     / \
    //       F T     T F
    */
    shared_levelized_file<bdd::node_type> bdd_6;

    { // Garbage collect writer to free write-lock
      const node n6_8(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
      const node n6_7(3, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));
      const node n6_6(2, bdd::max_id, n6_8, bdd::pointer_type(true));
      const node n6_5(2, bdd::max_id - 1, n6_7, bdd::pointer_type(false));
      const node n6_4(2, bdd::max_id - 2, bdd::pointer_type(false), n6_8);
      const node n6_3(1, bdd::max_id, n6_4, n6_6);
      const node n6_2(1, bdd::max_id - 1, n6_6, n6_5);
      const node n6_1(0, bdd::max_id, n6_3, n6_2);

      node_ofstream nw_6(bdd_6);
      nw_6 << n6_8 << n6_7 << n6_6 << n6_5 << n6_4 << n6_3 << n6_2 << n6_1;
    }

    // BDD 6 with an 'x4' instead of T that can collapse to T during the
    // transposition sweep
    shared_levelized_file<bdd::node_type> bdd_6_x4T;

    { // Garbage collect writer to free write-lock
      const node n6_9(4, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
      const node n6_8(3, bdd::max_id, bdd::pointer_type(false), n6_9);
      const node n6_7(3, bdd::max_id - 1, n6_9, bdd::pointer_type(false));
      const node n6_6(2, bdd::max_id, n6_8, n6_9);
      const node n6_5(2, bdd::max_id - 1, n6_7, bdd::pointer_type(false));
      const node n6_4(2, bdd::max_id - 2, bdd::pointer_type(false), n6_8);
      const node n6_3(1, bdd::max_id, n6_4, n6_6);
      const node n6_2(1, bdd::max_id - 1, n6_6, n6_5);
      const node n6_1(0, bdd::max_id, n6_3, n6_2);

      node_ofstream nw_6(bdd_6_x4T);
      nw_6 << n6_9 << n6_8 << n6_7 << n6_6 << n6_5 << n6_4 << n6_3 << n6_2 << n6_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 7
    /*
    //           1         ---- x0
    //           X
    //          / \
    //         2   3       ---- x1
    //        / \ / \
    //        \_ | __\
    //           |    \
    //           4    5    ---- x2
    //          / \  / \
    //          T F  F T
    */
    shared_levelized_file<bdd::node_type> bdd_7;

    const node n7_5(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n7_4(2, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));
    const node n7_3(1, bdd::max_id, n7_5, n7_4);
    const node n7_2(1, bdd::max_id - 1, n7_4, n7_5);
    const node n7_1(0, bdd::max_id, n7_3, n7_2);

    { // Garbage collect writer to free write-lock
      node_ofstream nw_7(bdd_7);
      nw_7 << n7_5 << n7_4 << n7_3 << n7_2 << n7_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 8 (b is mirrored on the nodes for x2)
    /*
    //           __1__         ---- x0
    //          /     \
    //         _2_     \       ---- x1
    //        /   \     \
    //        3    4    |      ---- x2
    //       / \  / \   |
    //       F  \ T  \  |
    //           \____\ |
    //                 \|
    //                  5      ---- x3
    //                 / \
    //                 F T
    */
    shared_levelized_file<bdd::node_type> bdd_8a, bdd_8b;

    { // Garbage collect writer to free write-lock
      node_ofstream nw_8a(bdd_8a);
      nw_8a << node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))                      // 5
            << node(2, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(3, bdd::max_id))      // 4
            << node(2, bdd::max_id - 1, bdd::pointer_type(false), bdd::pointer_type(3, bdd::max_id)) // 3
            << node(1,
                    bdd::max_id,
                    bdd::pointer_type(2, bdd::max_id - 1),
                    bdd::pointer_type(2, bdd::max_id)) // 2
            << node(0,
                    bdd::max_id,
                    bdd::pointer_type(1, bdd::max_id),
                    bdd::pointer_type(3, bdd::max_id)) // 1
        ;

      node_ofstream nw_8b(bdd_8b);
      nw_8b << node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))                      // 5
            << node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))      // 4
            << node(2, bdd::max_id - 1, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(false)) // 3
            << node(1,
                    bdd::max_id,
                    bdd::pointer_type(2, bdd::max_id - 1),
                    bdd::pointer_type(2, bdd::max_id)) // 2
            << node(0,
                    bdd::max_id,
                    bdd::pointer_type(1, bdd::max_id),
                    bdd::pointer_type(3, bdd::max_id)) // 1
        ;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 9 (T terminal)
    /*
    //           _1_       ---- x1
    //          /   \
    //          2   3      ---- x2 (inner sweep)
    //         / \ / \
    //         4  5  |     ---- x3
    //        / \/ \ |
    //        6 F  T |     ---- x4 (skippable sweep)
    //       / \__ __/
    //       |    7        ---- x5
    //       |   / \
    //       8   F T       ---- x6 (transposition sweep)
    //      / \
    //      F T
    */
    const node n9T_8 = node(6, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n9T_7 = node(5, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n9T_6 = node(4, bdd::max_id, n9T_8.uid(), n9T_7.uid());
    const node n9T_5 = node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n9T_4 = node(3, bdd::max_id - 1, n9T_6.uid(), bdd::pointer_type(false));
    const node n9T_3 = node(2, bdd::max_id, n9T_5.uid(), n9T_7.uid());
    const node n9T_2 = node(2, bdd::max_id - 1, n9T_4.uid(), n9T_5.uid());
    const node n9T_1 = node(1, bdd::max_id, n9T_2.uid(), n9T_3.uid());

    shared_levelized_file<bdd::node_type> bdd_9T;
    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_9T);
      nw << n9T_8 << n9T_7 << n9T_6 << n9T_5 << n9T_4 << n9T_3 << n9T_2 << n9T_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 9 (F terminal)
    /* input
    //           _1_       ---- x1
    //          /   \
    //          2   3      ---- x2 (final sweep)
    //         / \ / \
    //         4  5  |     ---- x3
    //        / \/ \ |
    //        6 F  T |     ---- x4 (skippable sweep)
    //       / \__ __/
    //       F    7        ---- x5
    //           / \
    //           F 8       ---- x6 (transposition sweep)
    //            / \
    //            F T
    */
    const node n9F_8 = node(6, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n9F_7 = node(5, bdd::max_id, bdd::pointer_type(false), n9F_8.uid());
    const node n9F_6 = node(4, bdd::max_id, bdd::pointer_type(false), n9F_7.uid());
    const node n9F_5 = node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n9F_4 = node(3, bdd::max_id - 1, n9F_6.uid(), bdd::pointer_type(false));
    const node n9F_3 = node(2, bdd::max_id, n9F_5.uid(), n9F_7.uid());
    const node n9F_2 = node(2, bdd::max_id - 1, n9F_4.uid(), n9F_5.uid());
    const node n9F_1 = node(1, bdd::max_id, n9F_2.uid(), n9F_3.uid());

    shared_levelized_file<bdd::node_type> bdd_9F;

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_9F);
      nw << n9F_8 << n9F_7 << n9F_6 << n9F_5 << n9F_4 << n9F_3 << n9F_2 << n9F_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 10 : node (8) dies when quantifying x2 and x3
    /*
    //          _1_           ---- x0
    //         /   \
    //         F   _2_        ---- x1
    //            /   \
    //            3   4       ---- x2
    //           / \ / \
    //           5  6  7      ---- x3
    //          / \| |/ \
    //          T   8   T     ---- x4
    //             / \
    //             F T
    */
    const node n10_8 = node(4, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n10_7 = node(3, bdd::max_id, n10_8.uid(), bdd::pointer_type(true));
    const node n10_6 = node(3, bdd::max_id - 1, n10_8.uid(), n10_8.uid());
    const node n10_5 = node(3, bdd::max_id - 2, bdd::pointer_type(true), n10_8.uid());
    const node n10_4 = node(2, bdd::max_id, n10_6.uid(), n10_7.uid());
    const node n10_3 = node(2, bdd::max_id - 1, n10_5.uid(), n10_6.uid());
    const node n10_2 = node(1, bdd::max_id, n10_3.uid(), n10_4.uid());
    const node n10_1 = node(0, bdd::max_id, bdd::pointer_type(false), n10_2.uid());

    shared_levelized_file<bdd::node_type> bdd_10;

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_10);
      nw << n10_8 << n10_7 << n10_6 << n10_5 << n10_4 << n10_3 << n10_2 << n10_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 11 : Same as BDD 3, but with gaps in-between the levelsn
    /*
    //       1       ---- x2
    //      / \
    //      |  2     ---- x4
    //      \ / \
    //       3   4   ---- x6
    //      / \ / \
    //      T F F T
    */
    shared_levelized_file<bdd::node_type> bdd_11;

    node n11_4 = node(6, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    node n11_3 = node(6, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));
    node n11_2 = node(4, bdd::max_id, n11_3.uid(), n11_4.uid());
    node n11_1 = node(2, bdd::max_id, n11_3.uid(), n11_2.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw_11(bdd_11);
      nw_11 << n11_4 << n11_3 << n11_2 << n11_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 15 : The two subtrees are designed such that their products is more than twice their
    //          original size. If each subtree itself also is duplicated, then it gets close(ish) to
    //          200% the input size.
    //
    //          The variables x0 and x1 are designed such that repeated transposition has to leave
    //          another node for later at x1.
    /*
    //                        __r1__                    ---- x0
    //                       /      \
    //                       r2     r3                  ---- x1
    //                      /  \   /  \
    //                      1  a   5  d
    //
    //                                   a              ---- x2
    //                                  / \
    //                   _1_            | |             ---- x3
    //                  /   \           | |
    //                  2   3           | |             ---- x4
    //                 / \ / \          | |
    //                 F _4_ F        __b c__           ---- x5
    //                  /   \        /  \ /  \
    //                  5   6        F   d   F          ---- x6
    //                 / \ / \          / \
    //                 |  7  |          | |             ---- x7
    //                 \ / \ /          | |
    //                  8   9           f g             ---- x8
    //                  |\ _X          /| X
    //                  | X  \         |\ |\
    //                  |/ \ /         \ X /
    //                  10 11           h i             ---- x9
    //                  |\ _X          /| X
    //                  | X  \         |\ |\
    //                  |/ \ /         \ X /
    //                  12 13           j k             ---- x10
    //                  |\ _X          /| X
    //                  | X  \         |\ |\
    //                  |/ \ /         \ X /
    //                  14 15           l m             ---- x11
    //                  |\ _X          /| X
    //                  | X  \         |\ |\
    //                  |/ \ /         | X |
    //                  16 17          |/ \|            ---- x12
    //                 / | | \         \\ //
    //                 T F F T          n o             ---- x13
    //                                 /| |\
    //                                T F F T
    //
    // Number of nodes:
    //   - roots: 3
    //   - left : 7 + 2i
    //   - right: 5 + 2i
    //
    // Total: 15 + 4i
    */
    //
    /*
    //                            _______(1,a)_______
    //                           /                   \
    //                ________(1,b)________        (1,c)   <-- (1,c) is symmetric to (1,b) (these
    //               /                     \         .          merge with (4,b) and (4,c))
    //              /                       \        .          Size: 3 additional nodes
    //             /                         \       .                (incl. (1,c))
    //           (2,b)                     (3,c)
    //           /   \                      |  \
    //          (b) (4,b)                (4,c) (c)
    //              /   \                /   \
    //             (5) (6,d)           (5,d) (6)            <-- Here, both inputs are copied
    //                 /   \           /   \                    Left subtree:  #nodes - 4
    //              (7,f) (9,g)    (8,d) (7,g)                  Right subtree: #nodes - 1
    //              /   \                /   \
    //          (8,f)   (9,f)          (8,g) (9,g)          <-- (8,g), (9,g) produce same pairs
    //         /   |     |   \          / /   / /
    //        /    |     |    \        . .   . .
    //       /     |     |     \      . .   . .
    //   (10,h) (11,i) (11,h) (10,i)
    //    /  \   /  \   /  \   /  \
    //    .  .   .  .   .  .   .  .
    //   (12,j) (13,k) (12,j) (12,k)
    //    /  \   /  \   /  \   /  \
    //    .  .   .  .   .  .   .  .
    //   (14,l) (14,m) (15,l) (15,m)
    //    /  \   /  \   /  \   /  \
    //    .  .   .  .   .  .   .  .
    //   (16,n) (16,o) (17,n) (17,o)
    //    /  \   /  \   /  \   /  \
    //    T (n)  T (o)  T (n)  F (o)
    //
    // Number of nodes (after one partial quantification):
    //   - roots   : 1
    //   - left    : (7-4) + 2i
    //   - right   : (5-1) + 2i
    //   - product : 13 + 4i + 2 = 15 + 4i
    //   - (5,d)   : 1
    //
    // Total: 22 + 4i + 4(i-1)
    //
    // For i = 5, this should construct roughly 58 unreduced nodes in the first
    // sweep, which is 1.65 of the original 35 nodes.
    */
    // NOTE: This is not going to be canonical (i.e. ordered) because that would
    //       become too messy.

    const node n15_o = node(13, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n15_n = node(13, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));

    const node n15_17 = node(12, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    const node n15_16 = node(12, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));

    const node n15_m  = node(11, bdd::max_id, n15_o.uid(), n15_n.uid());
    const node n15_l  = node(11, bdd::max_id - 1, n15_n.uid(), n15_o.uid());
    const node n15_15 = node(11, bdd::max_id - 2, n15_17.uid(), n15_16.uid());
    const node n15_14 = node(11, bdd::max_id - 3, n15_16.uid(), n15_17.uid());

    const node n15_k  = node(10, bdd::max_id, n15_m.uid(), n15_l.uid());
    const node n15_j  = node(10, bdd::max_id - 1, n15_l.uid(), n15_m.uid());
    const node n15_13 = node(10, bdd::max_id - 2, n15_15.uid(), n15_14.uid());
    const node n15_12 = node(10, bdd::max_id - 3, n15_14.uid(), n15_15.uid());

    const node n15_i  = node(9, bdd::max_id, n15_k.uid(), n15_j.uid());
    const node n15_h  = node(9, bdd::max_id - 1, n15_j.uid(), n15_k.uid());
    const node n15_11 = node(9, bdd::max_id - 2, n15_13.uid(), n15_12.uid());
    const node n15_10 = node(9, bdd::max_id - 3, n15_12.uid(), n15_13.uid());

    const node n15_g = node(8, bdd::max_id, n15_i.uid(), n15_h.uid());
    const node n15_f = node(8, bdd::max_id - 1, n15_h.uid(), n15_i.uid());
    const node n15_9 = node(8, bdd::max_id - 2, n15_11.uid(), n15_10.uid());
    const node n15_8 = node(8, bdd::max_id - 3, n15_10.uid(), n15_11.uid());

    const node n15_7 = node(7, bdd::max_id, n15_8.uid(), n15_9.uid());

    const node n15_d = node(6, bdd::max_id, n15_f.uid(), n15_g.uid());
    const node n15_6 = node(6, bdd::max_id - 1, n15_7.uid(), n15_9.uid());
    const node n15_5 = node(6, bdd::max_id - 2, n15_8.uid(), n15_7.uid());

    const node n15_c = node(5, bdd::max_id, n15_d.uid(), bdd::pointer_type(false));
    const node n15_b = node(5, bdd::max_id - 1, bdd::pointer_type(false), n15_d.uid());
    const node n15_4 = node(5, bdd::max_id - 2, n15_5.uid(), n15_6.uid());

    const node n15_3 = node(4, bdd::max_id, n15_4.uid(), bdd::pointer_type(false));
    const node n15_2 = node(4, bdd::max_id - 1, bdd::pointer_type(false), n15_4.uid());

    const node n15_1 = node(3, bdd::max_id, n15_2.uid(), n15_3.uid());

    const node n15_a = node(2, bdd::max_id, n15_b.uid(), n15_c.uid());

    const node n15_r3 = node(1, bdd::max_id, n15_5.uid(), n15_d.uid());
    const node n15_r2 = node(1, bdd::max_id - 1, n15_1.uid(), n15_a.uid());

    const node n15_r1 = node(0, bdd::max_id, n15_r2.uid(), n15_r3.uid());

    shared_levelized_file<bdd::node_type> bdd_15;

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_15);
      nw << n15_o << n15_n << n15_17 << n15_16 << n15_m << n15_l << n15_15 << n15_14 << n15_k
         << n15_j << n15_13 << n15_12 << n15_i << n15_h << n15_11 << n15_10 << n15_g << n15_f
         << n15_9 << n15_8 << n15_7 << n15_d << n15_6 << n15_5 << n15_c << n15_b << n15_4 << n15_3
         << n15_2 << n15_1 << n15_a << n15_r3 << n15_r2 << n15_r1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 16
    //
    // Designed to make outer root arcs dominate the 1-level cut within Nested Sweeping.
    //
    // NOTE: Designing this counter-example such that it is fully reduced is not easy to do (nor
    //       easy to understand). So, we "cheat" by making x4 a level full of duplicates.
    /*
    //               _1_              ---- x0
    //              /   \
    //            _2_    \            ---- x1
    //           /   \    \
    //          3     4    \          ---- x2
    //         / \   / \    \
    //         5 6   7 8     \        ---- x3
    //        /| |\ /| |\     \
    //        ---- 9 ----     |       ---- x4   <--- nodes for x6 having 1-level cut = 1
    //        \.|./  \.|./    |
    //          a      b      |       ---- x5   <--- forwarding of all in-going arcs of (a) to (b/c)
    //         / \    / \     |
    //         T F    F T     c       ---- x6   <--- transposition quantification
    //                       / \
    //                       F T
    */
    shared_levelized_file<bdd::node_type> bdd_16;

    {
      const node nc = node(6, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
      const node nb = node(5, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
      const node na = node(5, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));

      const node n9_3 = node(4, bdd::max_id, na.uid(), nb.uid());
      const node n9_2 = node(4, bdd::max_id - 1, na.uid(), nb.uid());
      const node n9_1 = node(4, bdd::max_id - 2, na.uid(), nb.uid());

      const node n8 = node(3, bdd::max_id, n9_2.uid(), n9_3.uid());
      const node n7 = node(3, bdd::max_id - 1, n9_1.uid(), n9_3.uid());
      const node n6 = node(3, bdd::max_id - 2, n9_2.uid(), n9_1.uid());
      const node n5 = node(3, bdd::max_id - 3, n9_1.uid(), n9_2.uid());
      const node n4 = node(2, bdd::max_id, n7.uid(), n8.uid());
      const node n3 = node(2, bdd::max_id - 1, n5.uid(), n6.uid());
      const node n2 = node(1, bdd::max_id, n3.uid(), n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), nc.uid());

      node_ofstream nw(bdd_16);
      nw << nc << nb << na << n9_3 << n9_2 << n9_1 << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 17
    //
    // Designed to expose a different processing order between using a secondary priority queue and
    // random access.
    /*
    //               _1_          ---- x0
    //              /   \
    //              2   3         ---- x1
    //             _X  / \
    //            /  \ |  \
    //            4   5   6       ---- x2
    //           / \ / \ / \
    //           7 F T F F T      ---- x3
    //          / \
    //          F T
    */
    // Now, when existentially quantifying x0, one should obtain the unreduced diagram shown below.
    /*
    //               (2,3)         ---- x1
    //               /   \
    //           (5,_)   (4,6)     ---- x2
    //           /   \   /   \
    //           T   F (7,_) T     ---- x3
    //                 /   \
    //                 F   T
    */
    // Where (5,_) and (4,6) are resolved in two different orders depending on the variant of the
    // algorithm. If a secondary priority queue is used the result should be as shown above. If
    // random access is used the two nodes at x2 should be swapped.
    shared_levelized_file<bdd::node_type> bdd_17a;

    {
      const node n7 = node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
      const node n6 = node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
      const node n5 = node(2, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));
      const node n4 = node(2, bdd::max_id - 2, n7.uid(), bdd::pointer_type(false));
      const node n3 = node(1, bdd::max_id, n5.uid(), n6.uid());
      const node n2 = node(1, bdd::max_id - 1, n5.uid(), n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_17a);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    // Same as BDD 17a, but with the terminals flipped. When universally quantifying x0, one should
    // obtain the unreduced diagram shown below.
    /*
    //               (2,3)         ---- x1
    //               /   \
    //           (5,_)   (4,6)     ---- x2
    //           /   \   /   \
    //           F   T (7,_) F     ---- x3
    //                 /   \
    //                 T   F
    */
    // Again, the order of (5,_) and (4,6) depends on the access mode.
    shared_levelized_file<bdd::node_type> bdd_17b;

    {
      const node n7 = node(3, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false));
      const node n6 = node(2, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false));
      const node n5 = node(2, bdd::max_id - 1, bdd::pointer_type(false), bdd::pointer_type(true));
      const node n4 = node(2, bdd::max_id - 2, n7.uid(), bdd::pointer_type(true));
      const node n3 = node(1, bdd::max_id, n5.uid(), n6.uid());
      const node n2 = node(1, bdd::max_id - 1, n5.uid(), n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_17b);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 18T
    /*
    //         _1_        ---- x0
    //        /   \
    //       /    2       ---- x1
    //      /    / \
    //      3    T |      ---- x2
    //     / \     |
    //     F  4    |      ---- x3
    //       / \   |
    //       F T   5      ---- x4
    //            / \
    //            T F
    */
    shared_levelized_file<bdd::node_type> bdd_18T;

    const node n18_5(4, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false));
    const node n18_4(3, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false));
    const node n18_3(2, bdd::max_id, bdd::pointer_type(false), n18_4.uid());

    {
      const node n18_2(1, bdd::max_id, bdd::pointer_type(true), n18_5.uid());
      const node n18_1(0, bdd::max_id, n18_3.uid(), n18_2.uid());

      node_ofstream nw(bdd_18T);
      nw << n18_5 << n18_4 << n18_3 << n18_2 << n18_1;
    }

    // BDD 18F
    /*
    //         _1_        ---- x0
    //        /   \
    //       /    2       ---- x1
    //      /    / \
    //      3    F |      ---- x2
    //     / \     |
    //     F  4    |      ---- x3
    //       / \   |
    //       F T   5      ---- x4
    //            / \
    //            T F
    */
    shared_levelized_file<bdd::node_type> bdd_18F;

    {
      const node n18_2(1, bdd::max_id, bdd::pointer_type(false), n18_5.uid());
      const node n18_1(0, bdd::max_id, n18_3.uid(), n18_2.uid());

      node_ofstream nw(bdd_18F);
      nw << n18_5 << n18_4 << n18_3 << n18_2 << n18_1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 19
    //
    // When quantifying x4 and x6, Nested Sweeping's outer Reduce sweep runs into a 2-level cut as
    // it combines the requests for node (9).
    //
    // This counter-example only works for 'simple transposition', 'pruning transposition', and
    // 'deepest quantification' but NOT for 'partial quantification'. Level x5 is here to force the
    // terminal arcs from (14) and (15) into the priority queue.
    /*
    //         _1_              ---- x0 (to not fully collapse to the true terminal)
    //        /   \
    //        F  _2_            ---- x1
    //          /   \
    //          3   4           ---- x2
    //         / \ / \
    //        _5 | | 6          ---- x3
    //       /  \\ // \                  <-- (max) 1-level cut: 6
    //       7_   9   _8        ---- x4  <--  max  2-level cut: 8
    //      /  \ / \ /  \                <-- (max) 1-level cut: 6
    //     12   10 13   11      ---- x5
    //    / |  / | | \  | \
    //    | T  T | | T  T /
    //    \____  / \  ___/
    //         14   15          ---- x6
    //        /  \ /  \
    //        T  F F  T
    */
    // In this case, it is expected to collapse to 'x0'
    /*
    //           1              ---- x0
    //          / \
    //          F T
    */
    shared_levelized_file<bdd::node_type> bdd_19;

    {
      const node n15(6, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
      const node n14(6, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false));
      const node n13(5, bdd::max_id, n15.uid(), bdd::pointer_type(true));
      const node n12(5, bdd::max_id - 1, n14.uid(), bdd::pointer_type(true));
      const node n11(5, bdd::max_id - 2, bdd::pointer_type(true), n15.uid());
      const node n10(5, bdd::max_id - 3, bdd::pointer_type(true), n14.uid());
      const node n9(4, bdd::max_id, n10.uid(), n13.uid());
      const node n8(4, bdd::max_id - 1, n13.uid(), n11.uid());
      const node n7(4, bdd::max_id - 2, n12.uid(), n10.uid());
      const node n6(3, bdd::max_id, n9.uid(), n8.uid());
      const node n5(3, bdd::max_id - 1, n7.uid(), n9.uid());
      const node n4(2, bdd::max_id, n9.uid(), n6.uid());
      const node n3(2, bdd::max_id - 1, n5.uid(), n9.uid());
      const node n2(1, bdd::max_id, n3.uid(), n4.uid());
      const node n1(0, bdd::max_id, bdd::pointer_type(false), n2.uid());

      node_ofstream nw(bdd_19);
      nw << n15 << n14 << n13 << n12 << n11 << n10 << n9 << n8 << n7 << n6 << n5 << n4 << n3 << n2
         << n1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // BDD 20
    //
    // Similar to BDD 19, this induces a 2-level cut during the outer Reduce. Yet, as it is already
    // semi-transposed we skip the transposition algorithms and so can design have a much smaller
    // and simpler counter-example.
    //
    /* input
    //       1_       ---- x1
    //      /  \
    //      2  3      ---- x2
    //     / \/ \              <-- (max) 1-level cut: 4
    //     4 5  6     ---- x3  <--  max  2-level cut: 5
    //     |X| / \             <-- (max) 1-level cut: 4
    //     7 8 F T    ---- x4
    //    /| |\
    //    TF FT
    */

    shared_levelized_file<arc> bdd_20__unreduced;
    {
      arc_ofstream aw(bdd_20__unreduced);

      const bdd::uid_type n8(4, 1);
      const bdd::uid_type n7(4, 0);
      const bdd::uid_type n6(3, 2);
      const bdd::uid_type n5(3, 1);
      const bdd::uid_type n4(3, 0);
      const bdd::uid_type n3(2, 1);
      const bdd::uid_type n2(2, 0);
      const bdd::uid_type n1(1, 0);

      aw.push_internal({ n1, false, n2 });
      aw.push_internal({ n1, true, n3 });
      aw.push_internal({ n2, false, n4 });
      aw.push_internal({ n2, true, n5 });
      aw.push_internal({ n3, false, n5 });
      aw.push_internal({ n3, true, n6 });
      aw.push_internal({ n4, false, n7 });
      aw.push_internal({ n5, false, n7 });
      aw.push_internal({ n4, true, n8 });
      aw.push_internal({ n5, true, n8 });

      aw.push_terminal({ n6, false, bdd::pointer_type(false) });
      aw.push_terminal({ n6, true, bdd::pointer_type(true) });
      aw.push_terminal({ n7, false, bdd::pointer_type(false) });
      aw.push_terminal({ n7, true, bdd::pointer_type(true) });
      aw.push_terminal({ n8, false, bdd::pointer_type(true) });
      aw.push_terminal({ n8, true, bdd::pointer_type(false) });

      aw.push(level_info(1, 1u));
      aw.push(level_info(2, 2u));
      aw.push(level_info(3, 3u));
      aw.push(level_info(4, 2u));

      bdd_20__unreduced->max_1level_cut = 4;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    describe("bdd_exists(const bdd&, bdd::label_type)", [&]() {
      it("quantifies T terminal-only BDD as itself [const &]", [&]() {
        const bdd in = terminal_T;
        __bdd out    = bdd_exists(in, 42);

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(terminal_T));
        AssertThat(out._negate, Is().False());
      });

      it("quantifies F terminal-only BDD as itself [&&]", [&]() {
        __bdd out = bdd_exists(bdd(terminal_F), 21);

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(terminal_F));
        AssertThat(out._negate, Is().False());
      });

      describe("access mode: random access", [&]() {
        const exec_policy ep = exec_policy::access::Random_Access;

        it("shortcuts quantification on non-existent label in input [const &]", [&]() {
          const bdd in = bdd_1;
          __bdd out    = bdd_exists(ep, in, 42);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().False());
        });

        it("shortcuts quantification of root into T terminal [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_1), 0);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->width, Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("shortcuts quantification of root into T terminal [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_x2), 2);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->width, Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("quantifies root without terminal arcs [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_2), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // (4,5)
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // (5,_)
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("quantifies root with F terminal [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_5F), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.low()
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.high()
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("quantifies root with F terminal in a shifted diagram [&&]", [&]() {
          // If the shifted level is not quantified, then the result changes.
          __bdd out = bdd_exists(ep, bdd(bdd_5F, false, +1), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.low()
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.high()
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("quantifies bottom-most nodes [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_1), 1);

          arc_test_ifstream arcs(out);
          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("quantifies nodes with terminal or nodes as children [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_2), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // (4,5)
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // (5,_)
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("outputs terminal arcs in order, despite the order of resolvement [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_2), 2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 5.high()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 5.high()
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to its own leaf
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("outputs terminal arcs in order, despite the order of resolvement [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_3), 2);

          arc_test_ifstream arcs(out);

          // NOTE: node (2,0) := (3,nil) while n4 < nil since we process this request without
          //       forwarding n3 through the secondary priority queue
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("keeps nodes as is when skipping quantified level [&&]", [&]() {
          // NOTE: The order of resolvement is different than with a secondary priority queue!
          //
          // NOTE: Node (2,1) := (3,nil)

          __bdd out = bdd_exists(ep, bdd(bdd_3), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());

          AssertThat(arcs.pull_terminal(), // n3
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // n3
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("resolves terminal-terminal requests in [&&]", [&]() {
          // NOTE: (2,0) := (3,nil)

          __bdd out = bdd_exists(ep, bdd(bdd_5F), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 3.high()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("collapses tuple requests of the same node back into request on a single node [&&]",
           [&]() {
             __bdd out = bdd_exists(ep, bdd(bdd_8a), 1);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(3, 0) }));
             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_terminal(),                   // true due to 3.low()
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                        Is().GreaterThanOrEqualTo(2u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(1u));
             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(2u));
           });

        it("collapses tuple requests of the same node back into request on a single node [BDD 8b]",
           [&]() {
             __bdd out = bdd_exists(ep, bdd_8b, 1);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(3, 0) }));
             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_terminal(),                   // true due to 3.low()
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                        Is().GreaterThanOrEqualTo(2u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(1u));
             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(2u));
           });

        it("resolves nodes in a different order than with a secondary priority queue [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_17a), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(true) }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));
          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });
      });

      describe("access mode: priority queue", [&]() {
        const exec_policy ep = exec_policy::access::Priority_Queue;

        it("shortcuts quantification on non-existent label in input [const &]", [&]() {
          const bdd in = bdd_1;
          __bdd out    = bdd_exists(ep, in, 42);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().False());
        });

        it("shortcuts quantification of root into T terminal [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_1), 0);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("shortcuts quantification of root into T terminal [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_x2), 2);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->width, Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("quantifies root without terminal arcs [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_2), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // (4,5)
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // (5,_)
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("quantifies root with F terminal [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_5F), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("quantifies root with F terminal in a shifted diagram [&&]", [&]() {
          // If the shifted level is not quantified, then the result changes.
          __bdd out = bdd_exists(ep, bdd(bdd_5F, false, +1), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.low()
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.high()
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("quantifies bottom-most nodes [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_1), 1);

          arc_test_ifstream arcs(out);
          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("quantifies nodes with terminal or nodes as children [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_2), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // (4,5)
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // (5,_)
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("outputs terminal arcs in order, despite the order of resolvement [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_2), 2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 5.high()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 5.high()
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to its own leaf
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("outputs terminal arcs in order, despite the order of resolvement [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_3), 2);

          arc_test_ifstream arcs(out);

          // NOTE: (2,0) := (3,nil) while n4 < nil since we process this request without forwarding
          //       n3 through the secondary priority queue
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("keeps nodes as is when skipping quantified level [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_3), 1);

          arc_test_ifstream arcs(out);

          // NOTE: (2,0) := reflects (3,nil) since while n4 < nil we process this request without
          //       forwarding n3 through the secondary priority queue
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // n3
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // n3
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("resolves terminal-terminal requests in [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_5F), 1);

          arc_test_ifstream arcs(out);

          // NOTE: (2,0) := (3,nil) while n4 < nil since we process this request without forwarding
          //       n3 through the secondary priority queue
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 3.high()
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("can shortcut/prune irrelevant subtrees [const &]", [&]() {
          shared_levelized_file<bdd::node_type> bdd_chain;

          node n4 = node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
          node n3 = node(2, bdd::max_id, n4.uid(), bdd::pointer_type(true));
          node n2 = node(1, bdd::max_id, n3.uid(), bdd::pointer_type(true));
          node n1 = node(0, bdd::max_id, n2.uid(), bdd::pointer_type(true));

          { // Garbage collect writer to free write-lock
            node_ofstream bdd_chain_w(bdd_chain);
            bdd_chain_w << n4 << n3 << n2 << n1;
          }

          const bdd in = bdd_chain;

          __bdd out = bdd_exists(ep, in, 2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to quantification of x2
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("can forward information across a level [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_6), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,8)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (8,F)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());
          AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,8)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(true) }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (8,F)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, bdd::pointer_type(false) }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(5u));
        });

        it("can forward multiple arcs to the same node across a level [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_7), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,5)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());
          AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,5)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));
          AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,5)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("collapses tuple requests of the same node back into request on a single node [&&]",
           [&]() {
             __bdd out = bdd_exists(ep, bdd(bdd_8a), 1);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(3, 0) }));
             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_terminal(),                   // true due to 3.low()
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                        Is().GreaterThanOrEqualTo(2u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(1u));
             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(2u));
           });

        it("collapses tuple requests of the same node back into request on a single node [&&]",
           [&]() {
             __bdd out = bdd_exists(ep, bdd(bdd_8b), 1);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(3, 0) }));
             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_terminal(),                   // true due to 3.low()
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                        Is().GreaterThanOrEqualTo(2u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(1u));
             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(2u));
           });

        it("resolves nodes in a different order than with random access [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_17a), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));
          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });
      });
    });

    describe("bdd_exists(__bdd&&, bdd::label_type)", [&]() {
      // TODO
    });

    describe("bdd_exists(const bdd&, const predicate<bdd::label_type>&)", [&]() {
      it("returns original file on always-false predicate BDD 1 [const &]", [&]() {
        bdd in    = bdd_1;
        __bdd out = bdd_exists(in, [](const bdd::label_type) -> bool { return false; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      it("returns original file on always-false predicate BDD 1 [&&]", [&]() {
        __bdd out = bdd_exists(bdd(bdd_1), [](const bdd::label_type) -> bool { return false; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies odd variables in BDD 4 [&&]", [&]() {
          bdd out =
            bdd_exists(ep, bdd(bdd_4), [](const bdd::label_type x) -> bool { return x % 2; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies 1, 2 in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(
            ep, bdd(bdd_4), [](const bdd::label_type x) -> bool { return x == 1 || x == 2; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies even variables in BDD 4 [const &]", [&]() {
          const bdd in = bdd_4;
          const bdd out =
            bdd_exists(ep, in, [](const bdd::label_type x) -> bool { return !(x % 2); });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in BDD 1 [&&]", [&]() {
          bdd out =
            bdd_exists(ep, bdd(bdd_1), [](const bdd::label_type x) -> bool { return x % 2; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("terminates early when quantifying to a terminal in BDD 1 [&&]", [&]() {
          // TODO: top-down dependant?
          int calls = 0;

          const bdd out = bdd_exists(ep, bdd(bdd_1), [&calls](const bdd::label_type) -> bool {
            calls++;
            return true;
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // One call per level for the profile
          AssertThat(calls, Is().EqualTo(2));
        });

        it("quantifies with always-true predicate in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(ep, bdd(bdd_4), [](const bdd::label_type) -> bool { return true; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies single variable in a shifted diagram [&&]", [&]() {
          // Same as 'quantifies root with F terminal' shifted by one.
          __bdd out =
            bdd_exists(ep, bdd(bdd_5F, false, +1), [](const bdd::label_type x) { return x == 1; });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(
            nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(),
                     Is().EqualTo(node(
                       3, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(),
                     Is().EqualTo(node(2,
                                       bdd::max_id,
                                       bdd::pointer_type(3, bdd::max_id - 1),
                                       bdd::pointer_type(3, bdd::max_id))));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        describe("access mode: random access", [&]() {
          it("quantifies odd variables in BDD 1 [&&]", [&]() {
            bdd out = bdd_exists(ep & exec_policy::access::Random_Access,
                                 bdd(bdd_1),
                                 [](const bdd::label_type x) -> bool { return x % 2; });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().False());

            // TODO: meta variables...
          });

          it("leaves unprunable to-be quantified node behind as-is [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Random_Access,
                                 bdd(bdd_4),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // (3)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (1)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

            AssertThat(out_meta.can_pull(), Is().False());

            // TODO: meta variables...

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(11u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));
            AssertThat(call_history.at(3), Is().EqualTo(3u));

            // - Pruning sweep
            AssertThat(call_history.at(4), Is().EqualTo(0u));
            AssertThat(call_history.at(5), Is().EqualTo(1u));
            AssertThat(call_history.at(6), Is().EqualTo(2u));
            AssertThat(call_history.at(7), Is().EqualTo(3u));

            // - Nested sweep looking for the 'next_inner' bottom-up
            AssertThat(call_history.at(8), Is().EqualTo(2u));
            AssertThat(call_history.at(9), Is().EqualTo(1u));
            AssertThat(call_history.at(10), Is().EqualTo(0u));
          });

          it("prunes to-be quantified nodes with T terminals [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Random_Access,
                                 bdd(bdd_18T),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2 == 1;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // (2)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (1)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().False());

            // TODO: meta variables...

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(11u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));
            AssertThat(call_history.at(3), Is().EqualTo(3u));
            AssertThat(call_history.at(4), Is().EqualTo(4u));

            // - Pruning sweep
            AssertThat(call_history.at(5), Is().EqualTo(0u));
            AssertThat(call_history.at(6), Is().EqualTo(1u));
            AssertThat(call_history.at(7), Is().EqualTo(2u));
            AssertThat(call_history.at(8), Is().EqualTo(3u));

            // - Nested sweep (nothing to be done)
            AssertThat(call_history.at(9), Is().EqualTo(2u));
            AssertThat(call_history.at(10), Is().EqualTo(0u));
          });

          it("collapses root with T terminal during pruning transposition [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Random_Access,
                                 bdd(bdd_5T),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2 == 0;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(4u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));

            // - Pruning sweep
            AssertThat(call_history.at(3), Is().EqualTo(0u));
          });

          it("skips to-be quantified nodes with false terminals [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Random_Access,
                                 bdd(bdd_18F),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2 == 1;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // (5)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (2)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (1)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(0,
                                         bdd::max_id,
                                         bdd::pointer_type(2, bdd::max_id),
                                         bdd::pointer_type(4, bdd::max_id))));

            AssertThat(out_nodes.can_pull(), Is().False());

            // TODO: meta variables...

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(13u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));
            AssertThat(call_history.at(3), Is().EqualTo(3u));
            AssertThat(call_history.at(4), Is().EqualTo(4u));

            // - Pruning sweep
            AssertThat(call_history.at(5), Is().EqualTo(0u));
            AssertThat(call_history.at(6), Is().EqualTo(1u));
            AssertThat(call_history.at(7), Is().EqualTo(2u));
            AssertThat(call_history.at(8), Is().EqualTo(3u));
            AssertThat(call_history.at(9), Is().EqualTo(4u));

            // - Nested sweep (nothing to be done)
            AssertThat(call_history.at(10), Is().EqualTo(4u));
            AssertThat(call_history.at(11), Is().EqualTo(2u));
            AssertThat(call_history.at(12), Is().EqualTo(0u));
          });

          it("collapses to terminal during transposition as root with false is pruned [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Random_Access,
                                 bdd(bdd_5F),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2 == 0;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(7u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));

            // - Pruning sweep
            AssertThat(call_history.at(3), Is().EqualTo(0u));
            AssertThat(call_history.at(4), Is().EqualTo(1u));
            AssertThat(call_history.at(5), Is().EqualTo(2u));
          });
        });

        describe("access mode: priority queue", [&]() {
          it("quantifies odd variables in BDD 1 [&&]", [&]() {
            bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                                 bdd(bdd_1),
                                 [](const bdd::label_type x) -> bool { return x % 2; });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().False());

            // TODO: meta variables...
          });

          it("leaves unprunable to-be quantified node behind as-is [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                                 bdd(bdd_4),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // (3)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (1)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

            AssertThat(out_meta.can_pull(), Is().False());

            // TODO: meta variables...

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(11u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));
            AssertThat(call_history.at(3), Is().EqualTo(3u));

            // - Pruning sweep
            AssertThat(call_history.at(4), Is().EqualTo(0u));
            AssertThat(call_history.at(5), Is().EqualTo(1u));
            AssertThat(call_history.at(6), Is().EqualTo(2u));
            AssertThat(call_history.at(7), Is().EqualTo(3u));

            // - Nested sweep looking for the 'next_inner' bottom-up
            AssertThat(call_history.at(8), Is().EqualTo(2u));
            AssertThat(call_history.at(9), Is().EqualTo(1u));
            AssertThat(call_history.at(10), Is().EqualTo(0u));
          });

          it("prunes to-be quantified nodes with T terminals [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                                 bdd(bdd_18T),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2 == 1;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // (2)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (1)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().False());

            // TODO: meta variables...

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(11u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));
            AssertThat(call_history.at(3), Is().EqualTo(3u));
            AssertThat(call_history.at(4), Is().EqualTo(4u));

            // - Pruning sweep
            AssertThat(call_history.at(5), Is().EqualTo(0u));
            AssertThat(call_history.at(6), Is().EqualTo(1u));
            AssertThat(call_history.at(7), Is().EqualTo(2u));
            AssertThat(call_history.at(8), Is().EqualTo(3u));

            // - Nested sweep (nothing to be done)
            AssertThat(call_history.at(9), Is().EqualTo(2u));
            AssertThat(call_history.at(10), Is().EqualTo(0u));
          });

          it("collapses root with T terminal during pruning transposition [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                                 bdd(bdd_5T),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2 == 0;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(4u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));

            // - Pruning sweep
            AssertThat(call_history.at(3), Is().EqualTo(0u));
          });

          it("skips to-be quantified nodes with false terminals [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                                 bdd(bdd_18F),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2 == 1;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // (5)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (2)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (1)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(0,
                                         bdd::max_id,
                                         bdd::pointer_type(2, bdd::max_id),
                                         bdd::pointer_type(4, bdd::max_id))));

            AssertThat(out_nodes.can_pull(), Is().False());

            // TODO: meta variables...

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(13u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));
            AssertThat(call_history.at(3), Is().EqualTo(3u));
            AssertThat(call_history.at(4), Is().EqualTo(4u));

            // - Pruning sweep
            AssertThat(call_history.at(5), Is().EqualTo(0u));
            AssertThat(call_history.at(6), Is().EqualTo(1u));
            AssertThat(call_history.at(7), Is().EqualTo(2u));
            AssertThat(call_history.at(8), Is().EqualTo(3u));
            AssertThat(call_history.at(9), Is().EqualTo(4u));

            // - Nested sweep (nothing to be done)
            AssertThat(call_history.at(10), Is().EqualTo(4u));
            AssertThat(call_history.at(11), Is().EqualTo(2u));
            AssertThat(call_history.at(12), Is().EqualTo(0u));
          });

          it("collapses to terminal during transposition as root with false is pruned [&&]", [&]() {
            std::vector<bdd::label_type> call_history;

            bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                                 bdd(bdd_5F),
                                 [&call_history](const bdd::label_type x) -> bool {
                                   call_history.push_back(x);
                                   return x % 2 == 0;
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            // Check call history
            //
            // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
            //       verify that this change makes sense and is as intended.
            AssertThat(call_history.size(), Is().EqualTo(7u));

            // - Generate predicate profile
            AssertThat(call_history.at(0), Is().EqualTo(0u));
            AssertThat(call_history.at(1), Is().EqualTo(1u));
            AssertThat(call_history.at(2), Is().EqualTo(2u));

            // - Pruning sweep
            AssertThat(call_history.at(3), Is().EqualTo(0u));
            AssertThat(call_history.at(4), Is().EqualTo(1u));
            AssertThat(call_history.at(5), Is().EqualTo(2u));
          });
        });

        it("quantifies with always-true predicate in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(ep, bdd(bdd_4), [](const bdd::label_type) -> bool { return true; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("bails out on a level that only shortcuts [&&]", [&]() {
          bdd out =
            bdd_exists(ep, bdd(bdd_9T), [](const bdd::label_type x) -> bool { return !(x % 2); });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant [&&]", [&]() {
          bdd out =
            bdd_exists(ep, bdd(bdd_9F), [](const bdd::label_type x) -> bool { return !(x % 2); });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that both shortcuts and is irrelevant [&&]", [&]() {
          bdd out = bdd_exists(ep, bdd(bdd_6_x4T), [](const bdd::label_type x) -> bool {
            return x == 4 || x == 2 || x == 1;
          });

          // TODO predict output!
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solution [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          bdd out = bdd_exists(ep, bdd(bdd_10), [&call_history](const bdd::label_type x) -> bool {
            call_history.push_back(x);
            return x == 3 || x == 2;
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(15u));

          // - Generate predicate profile
          AssertThat(call_history.at(0), Is().EqualTo(0u));
          AssertThat(call_history.at(1), Is().EqualTo(1u));
          AssertThat(call_history.at(2), Is().EqualTo(2u));
          AssertThat(call_history.at(3), Is().EqualTo(3u));
          AssertThat(call_history.at(4), Is().EqualTo(4u));

          // - Pruning sweep
          AssertThat(call_history.at(5), Is().EqualTo(0u));
          AssertThat(call_history.at(6), Is().EqualTo(1u));
          AssertThat(call_history.at(7), Is().EqualTo(2u));
          AssertThat(call_history.at(8), Is().EqualTo(3u));
          AssertThat(call_history.at(9), Is().EqualTo(4u));

          // - Nested sweep looking for the 'next_inner' bottom-up
          AssertThat(call_history.at(10), Is().EqualTo(4u));
          AssertThat(call_history.at(11), Is().EqualTo(3u));
          AssertThat(call_history.at(12), Is().EqualTo(2u));
          AssertThat(call_history.at(13), Is().EqualTo(1u));
          AssertThat(call_history.at(14), Is().EqualTo(0u));
        });

        it("kills intermediate dead partial solutions multiple times [&&]", [&]() {
          /* expected
          //
          //         _1_
          //        /   \
          //        *   *
          //         \ /
          //          |
          //          /
          //         /
          //        *
          //        |
          //        T
          */
          bdd out =
            bdd_exists(ep, bdd(bdd_6), [](const bdd::label_type x) -> bool { return (x % 2); });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("quantifies x0 and x1 in exploding BDD 15 [&&]", [&]() {
          bdd out =
            bdd_exists(ep, bdd(bdd_15), [](const bdd::label_type x) -> bool { return x < 2; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (o)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       13, bdd::max_id, node::pointer_type(false), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       13, bdd::max_id - 1, node::pointer_type(true), node::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17,o)
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(
              12, bdd::max_id, node::pointer_type(13, bdd::max_id), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (17,n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(12,
                                       bdd::max_id - 1,
                                       node::pointer_type(13, bdd::max_id - 1),
                                       node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,o)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(12,
                                       bdd::max_id - 2,
                                       node::pointer_type(true),
                                       node::pointer_type(13, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (16,n)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(12,
                                       bdd::max_id - 3,
                                       node::pointer_type(true),
                                       node::pointer_type(13, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,l)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       bdd::max_id,
                                       node::pointer_type(12, bdd::max_id - 3),
                                       node::pointer_type(12, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (14,m)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       bdd::max_id - 1,
                                       node::pointer_type(12, bdd::max_id - 2),
                                       node::pointer_type(12, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,l)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       bdd::max_id - 2,
                                       node::pointer_type(12, bdd::max_id - 1),
                                       node::pointer_type(12, bdd::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (15,m)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(11,
                                       bdd::max_id - 3,
                                       node::pointer_type(12, bdd::max_id),
                                       node::pointer_type(12, bdd::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,k)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       bdd::max_id,
                                       node::pointer_type(11, bdd::max_id - 3),
                                       node::pointer_type(11, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (13,j)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       bdd::max_id - 1,
                                       node::pointer_type(11, bdd::max_id - 2),
                                       node::pointer_type(11, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,k)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       bdd::max_id - 2,
                                       node::pointer_type(11, bdd::max_id - 1),
                                       node::pointer_type(11, bdd::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (12,j)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(10,
                                       bdd::max_id - 3,
                                       node::pointer_type(11, bdd::max_id),
                                       node::pointer_type(11, bdd::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,h)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       bdd::max_id,
                                       node::pointer_type(10, bdd::max_id - 3),
                                       node::pointer_type(10, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (10,i)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       bdd::max_id - 1,
                                       node::pointer_type(10, bdd::max_id - 2),
                                       node::pointer_type(10, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,h)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       bdd::max_id - 2,
                                       node::pointer_type(10, bdd::max_id - 1),
                                       node::pointer_type(10, bdd::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (11,i)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9,
                                       bdd::max_id - 3,
                                       node::pointer_type(10, bdd::max_id),
                                       node::pointer_type(10, bdd::max_id - 3))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (9,g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       bdd::max_id,
                                       node::pointer_type(9, bdd::max_id - 3),
                                       node::pointer_type(9, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       bdd::max_id - 1,
                                       node::pointer_type(9, bdd::max_id - 1),
                                       node::pointer_type(9, bdd::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (8,f)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(8,
                                       bdd::max_id - 2,
                                       node::pointer_type(9, bdd::max_id),
                                       node::pointer_type(9, bdd::max_id - 3))));

          // NOTE: (9,f) because the pair (7,f) is is merged with (8) which
          //       prunes that entire subtree away.

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,8,f)
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(
              7, bdd::max_id, node::pointer_type(8, bdd::max_id - 2), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g,9)
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(
              7, bdd::max_id - 1, node::pointer_type(true), node::pointer_type(8, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (7,g)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(7,
                                       bdd::max_id - 2,
                                       node::pointer_type(8, bdd::max_id - 1),
                                       node::pointer_type(8, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (6,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       bdd::max_id,
                                       node::pointer_type(7, bdd::max_id),
                                       node::pointer_type(7, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       bdd::max_id - 1,
                                       node::pointer_type(8, bdd::max_id - 2),
                                       node::pointer_type(7, bdd::max_id - 2))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (4,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5,
                                       bdd::max_id,
                                       node::pointer_type(6, bdd::max_id - 1),
                                       node::pointer_type(6, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (3,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4,
                                       bdd::max_id,
                                       node::pointer_type(5, bdd::max_id),
                                       node::pointer_type(6, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4,
                                       bdd::max_id - 1,
                                       node::pointer_type(6, bdd::max_id - 1),
                                       node::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1,5,d)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3,
                                       bdd::max_id,
                                       node::pointer_type(4, bdd::max_id - 1),
                                       node::pointer_type(4, bdd::max_id))));

          // NOTE: The root (1,a,5,d) has x2 suppressed as the choice at (a)
          //       only is relevant for (b) and (c), not for (d).

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          // TODO

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(13u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u, 4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(11u, 4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(10u, 4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9u, 4u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8u, 3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7u, 3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4u, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("accounts for number of root arcs from Outer Sweep [&&]", [&]() {
          /* expected
          //
          //        T
          */
          bdd out = bdd_exists(ep, bdd(bdd_16), [](int x) -> bool { return x != 5; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          __bdd out =
            bdd_exists(ep, bdd(bdd_19), [](const bdd::label_type x) { return x == 4 || x == 6; });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(
            nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("prunes root during transposition of a shifted diagram [&&]", [&]() {
          // Same as 'quantifies root with F terminal' shifted by one.
          __bdd out =
            bdd_exists(ep, bdd(bdd_5F, false, +1), [](const bdd::label_type x) { return x == 1; });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(
            nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(),
                     Is().EqualTo(node(
                       3, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(),
                     Is().EqualTo(node(2,
                                       bdd::max_id,
                                       bdd::pointer_type(3, bdd::max_id - 1),
                                       bdd::pointer_type(3, bdd::max_id))));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("quantifies even variables in shifted BDD 4(+1) [&&]", [&]() {
          // Similar to 'unprunable to-be quantified node behind as-is'
          std::vector<bdd::label_type> call_history;

          bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                               bdd(bdd_4, false, +1),
                               [&call_history](const bdd::label_type x) -> bool {
                                 call_history.push_back(x);
                                 return (x + 1) % 2;
                               });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please
          //       verify that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(11u));

          // - Generate predicate profile
          AssertThat(call_history.at(0), Is().EqualTo(1u));
          AssertThat(call_history.at(1), Is().EqualTo(2u));
          AssertThat(call_history.at(2), Is().EqualTo(3u));
          AssertThat(call_history.at(3), Is().EqualTo(4u));

          // - Pruning sweep
          AssertThat(call_history.at(4), Is().EqualTo(1u));
          AssertThat(call_history.at(5), Is().EqualTo(2u));
          AssertThat(call_history.at(6), Is().EqualTo(3u));
          AssertThat(call_history.at(7), Is().EqualTo(4u));

          // - Nested sweep looking for the 'next_inner' bottom-up
          AssertThat(call_history.at(8), Is().EqualTo(3u));
          AssertThat(call_history.at(9), Is().EqualTo(2u));
          AssertThat(call_history.at(10), Is().EqualTo(1u));
        });
      });
    });

    describe("bdd_exists(__bdd&&, const predicate<bdd::label_type>&)", [&]() {
      it("returns original file on reduced BDD 1 and always-false predicate [&&]", [&]() {
        __bdd out = bdd_exists(__bdd(bdd_1), [](const bdd::label_type) -> bool { return false; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      it("returns original file on reduced BDD 2 and always-false predicate [&&]", [&]() {
        __bdd out = bdd_exists(__bdd(bdd_2), [](const bdd::label_type) -> bool { return false; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_2));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies even variables in unreduced BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;

          const bdd out = bdd_exists(
            ep, __bdd(bdd_4__unreduced, ep), [&call_history](const bdd::label_type x) -> bool {
              call_history.push_back(x);
              return !(x % 2);
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(9u));

          // - Generate predicate profile / First quantification
          AssertThat(call_history.at(0), Is().EqualTo(0u));
          AssertThat(call_history.at(1), Is().EqualTo(1u));
          AssertThat(call_history.at(2), Is().EqualTo(2u));
          AssertThat(call_history.at(3), Is().EqualTo(3u));

          // - Second quantification
          AssertThat(call_history.at(4), Is().EqualTo(3u));
          AssertThat(call_history.at(5), Is().EqualTo(1u));
          AssertThat(call_history.at(6), Is().EqualTo(0u));

          // - Third quantification
          AssertThat(call_history.at(7), Is().EqualTo(3u));
          AssertThat(call_history.at(8), Is().EqualTo(1u));
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies x0 in unreduced BDD 1 into terminal [&&]", [&]() {
          std::vector<bdd::label_type> call_history;

          const bdd out = bdd_exists(
            ep, __bdd(bdd_1__unreduced, ep), [&call_history](const bdd::label_type x) -> bool {
              call_history.push_back(x);
              return x == 0;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(3u));

          // - Nested Sweep
          AssertThat(call_history.at(0), Is().EqualTo(2u));
          AssertThat(call_history.at(1), Is().EqualTo(1u));
          AssertThat(call_history.at(2), Is().EqualTo(0u)); // <-- bail out into terminal!
        });

        it("quantifies even variables in reduced BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;

          const bdd out =
            bdd_exists(ep, __bdd(bdd_4), [&call_history](const bdd::label_type x) -> bool {
              call_history.push_back(x);
              return !(x % 2);
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(11u));

          // - Generate predicate profile
          AssertThat(call_history.at(0), Is().EqualTo(0u));
          AssertThat(call_history.at(1), Is().EqualTo(1u));
          AssertThat(call_history.at(2), Is().EqualTo(2u));
          AssertThat(call_history.at(3), Is().EqualTo(3u));

          // - Pruning sweep
          AssertThat(call_history.at(4), Is().EqualTo(0u));
          AssertThat(call_history.at(5), Is().EqualTo(1u));
          AssertThat(call_history.at(6), Is().EqualTo(2u)); // <-- pruned!
          AssertThat(call_history.at(7), Is().EqualTo(3u));

          // - Nested sweep looking for the 'next_inner' bottom-up
          AssertThat(call_history.at(8), Is().EqualTo(3u));
          AssertThat(call_history.at(9), Is().EqualTo(1u));
          AssertThat(call_history.at(10), Is().EqualTo(0u));
        });

        it("quantifies even variables in unreduced BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;

          const bdd out = bdd_exists(
            ep, __bdd(bdd_4__unreduced, ep), [&call_history](const bdd::label_type x) -> bool {
              call_history.push_back(x);
              return !(x % 2);
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(5u));

          // - Nested Sweep
          AssertThat(call_history.at(0), Is().EqualTo(4u)); // <-- bail out!
          AssertThat(call_history.at(1), Is().EqualTo(3u));
          AssertThat(call_history.at(2), Is().EqualTo(2u)); // <-- bail out!
          AssertThat(call_history.at(3), Is().EqualTo(1u));
          AssertThat(call_history.at(4), Is().EqualTo(0u)); // <-- quantification!
        });

        it("quantifies odd variables in reduced BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;

          const bdd out =
            bdd_exists(ep, __bdd(bdd_4), [&call_history](const bdd::label_type x) -> bool {
              call_history.push_back(x);
              return x % 2;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(11u));

          // - Generate predicate profile
          AssertThat(call_history.at(0), Is().EqualTo(0u));
          AssertThat(call_history.at(1), Is().EqualTo(1u));
          AssertThat(call_history.at(2), Is().EqualTo(2u));
          AssertThat(call_history.at(3), Is().EqualTo(3u));

          // - Pruning sweep
          AssertThat(call_history.at(4), Is().EqualTo(0u));
          AssertThat(call_history.at(5), Is().EqualTo(1u));
          AssertThat(call_history.at(6), Is().EqualTo(2u));
          AssertThat(call_history.at(7), Is().EqualTo(3u)); // <-- pruned!

          // - Nested sweep looking for the 'next_inner' bottom-up
          AssertThat(call_history.at(8), Is().EqualTo(2u));
          AssertThat(call_history.at(9), Is().EqualTo(1u)); // <-- bail out!
          AssertThat(call_history.at(10), Is().EqualTo(0u));
        });

        it("quantifies odd variables in unreduced BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;

          const bdd out = bdd_exists(
            ep, __bdd(bdd_4__unreduced, ep), [&call_history](const bdd::label_type x) -> bool {
              call_history.push_back(x);
              return x % 2;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(5u));

          // - Nested Sweep
          AssertThat(call_history.at(0), Is().EqualTo(4u));
          AssertThat(call_history.at(1), Is().EqualTo(3u)); // <-- bail out!
          AssertThat(call_history.at(2), Is().EqualTo(2u));
          AssertThat(call_history.at(3), Is().EqualTo(1u)); // <-- bail out!
          AssertThat(call_history.at(4), Is().EqualTo(0u));
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          __bdd out = bdd_exists(
            ep, __bdd(bdd_20__unreduced, ep), [](const bdd::label_type x) { return x == 3; });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });

      // TODO: More tests on arc-based input to independently recreate the more complex tests of the
      //       nested sweeping framework above.
    });

    describe("bdd_exists(const bdd&, const generator<bdd::label_type>&)", [&]() {
      it("returns original file on 'optional::none' generator [&&]", [&]() {
        __bdd out = bdd_exists(bdd(bdd_1), []() -> optional<bdd::label_type> { return {}; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies 3, 1 in BDD 4 [&&]", [&]() {
          bdd out = bdd_exists(ep, bdd(bdd_4), [var = 3]() mutable -> optional<bdd::label_type> {
            if (var == 42) { return {}; }

            const bdd::label_type ret = var;
            var                       = ret == 1 ? 42 : var - 2;
            return { ret };
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 2, 0 in BDD 4 [const &]", [&]() {
          const bdd in = bdd_4;

          bdd out = bdd_exists(ep, in, [var = 2]() mutable -> optional<bdd::label_type> {
            if (var == 42) { return {}; }

            const bdd::label_type res = var;
            var                       = res == 0 ? 42 : var - 2;
            return { res };
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1 in BDD 1 [&&]", [&]() {
          bdd out = bdd_exists(ep, bdd(bdd_1), [var = 1]() mutable -> optional<bdd::label_type> {
            if (var == 0) { return {}; }
            return { var-- };
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("terminates early when quantifying to a terminal in BDD 3 [&&]", [&]() {
          int calls = 0;

          const bdd out =
            bdd_exists(ep, bdd(bdd_3), [&calls]() -> bdd::label_type { return 2 - 2 * (calls++); });

          // What could be expected is 3 calls: 2, 0 . But, here it terminates early.
          AssertThat(calls, Is().EqualTo(2));

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        describe("access mode: random access", [&]() {
          it("quantifies 3, 1 in BDD 4 [&&]", [&]() {
            bdd out = bdd_exists(ep & exec_policy::access::Random_Access,
                                 bdd(bdd_4),
                                 [var = 3]() mutable -> optional<bdd::label_type> {
                                   if (var == 42) { return {}; }

                                   const bdd::label_type ret = var;
                                   var                       = ret == 1 ? 42 : var - 2;
                                   return { ret };
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // (3)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (1)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

            AssertThat(out_meta.can_pull(), Is().False());
          });

          it("quantifies 1 in BDD 1 [&&]", [&]() {
            bdd out = bdd_exists(ep & exec_policy::access::Random_Access,
                                 bdd(bdd_1),
                                 [var = 1]() mutable -> optional<bdd::label_type> {
                                   if (var == 0) { return {}; }
                                   return { var-- };
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().False());
          });
        });

        describe("access mode: priority queue", [&]() {
          it("quantifies 3, 1 in BDD 4 [&&]", [&]() {
            bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                                 bdd(bdd_4),
                                 [var = 3]() mutable -> optional<bdd::label_type> {
                                   if (var == 42) { return {}; }

                                   const bdd::label_type ret = var;
                                   var                       = ret == 1 ? 42 : var - 2;
                                   return { ret };
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // (3)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().True()); // (1)
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

            AssertThat(out_meta.can_pull(), Is().False());
          });

          it("quantifies 1 in BDD 1 [&&]", [&]() {
            bdd out = bdd_exists(ep & exec_policy::access::Priority_Queue,
                                 bdd(bdd_1),
                                 [var = 1]() mutable -> optional<bdd::label_type> {
                                   if (var == 0) { return {}; }
                                   return { var-- };
                                 });

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().False());
          });
        });

        it("bails out on a level that only shortcuts [&&]", [&]() {
          bdd out = bdd_exists(ep, bdd(bdd_9T), [var = 6]() mutable -> optional<bdd::label_type> {
            if (var == 42) { return {}; }

            const bdd::label_type res = var;
            var                       = res == 0 ? 42 : var - 2;
            return { res };
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant", [&]() {
          bdd out = bdd_exists(ep, bdd(bdd_9F), [var = 6]() mutable -> optional<bdd::label_type> {
            if (var == 42) { return make_optional<bdd::label_type>(); }

            const bdd::label_type res = var;
            var                       = res == 0 ? 42 : var - 2;
            return res;
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that both shortcuts and is irrelevant [&&]", [&]() {
          bdd out =
            bdd_exists(ep, bdd(bdd_6_x4T), [var = 4]() mutable -> optional<bdd::label_type> {
              if (var == 42) { return {}; }
              const bdd::label_type res = var;
              switch (res) {
              case 4: {
                var = 2;
                break;
              } // <-- 4: transposing
              case 2: {
                var = 1;
                break;
              } // <-- 2: shortuctting / irrelevant
              default: {
                var = 42;
                break;
              } // <-- 1: final sweep
              }
              return { res };
            });

          // TODO predict output!
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solution [&&]", [&]() {
          bdd out =
            bdd_exists(ep, bdd(bdd_10), [var = make_optional<bdd::label_type>(3)]() mutable {
              if (!var) { return var; }

              const optional<bdd::label_type> res = var;
              if (2 < var.value()) {
                var = var.value() - 1;
              } else {
                var = {};
              }

              return res;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("kills intermediate dead partial solutions multiple times [&&]", [&]() {
          /* expected
          //
          //         _1_
          //        /   \
          //        *   *
          //         \ /
          //          |
          //          /
          //         /
          //        *
          //        |
          //        T
          */
          bdd out = bdd_exists(ep, bdd(bdd_6), [var = 7]() mutable -> optional<bdd::label_type> {
            if (var == 42) { return make_optional<bdd::label_type>(); }

            const bdd::label_type ret = var;
            var                       = ret == 1 ? 42 : var - 2;
            return ret;
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("accounts for number of root arcs from Outer Sweep [&&]", [&]() {
          /* expected
          //
          //        T
          */
          bdd out = bdd_exists(ep, bdd(bdd_16), [var = 6]() mutable -> optional<bdd::label_type> {
            var -= 1;

            if (var < 0) { return {}; }
            if (var == 5) { var -= 1; }
            return { var };
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          __bdd out = bdd_exists(ep, bdd(bdd_19), [var = 6]() mutable -> optional<bdd::label_type> {
            var -= 2;
            if (var < 4) { return {}; }
            return { var };
          });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(
            nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });
    });

    describe("bdd_exists(__bdd&&, const generator<bdd::label_type>&)", [&]() {
      it("returns original file on reduced BDD 1 and 'optional::none' generator [&&]", [&]() {
        __bdd out = bdd_exists(__bdd(bdd_1), []() -> optional<int> { return {}; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      it("returns original file on reduced BDD 2 and 'optional::none' generator [&&]", [&]() {
        __bdd out = bdd_exists(__bdd(bdd_2), []() -> optional<int> { return {}; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_2));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies even variables in unreduced BDD 4 [&&]", [&]() {
          const bdd out = bdd_exists(
            ep, __bdd(bdd_4__unreduced, ep), [var = 4]() mutable -> optional<bdd::label_type> {
              if (var < 0) { return {}; }
              return var -= 2;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies x0 in unreduced BDD 1 into terminal [&&]", [&]() {
          const bdd out = bdd_exists(
            ep, __bdd(bdd_1__unreduced, ep), [called = false]() mutable -> optional<int> {
              if (called) { return {}; }
              called = true;
              return { 0 };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies even variables in reduced BDD 4 [&&]", [&]() {
          const bdd out =
            bdd_exists(ep, __bdd(bdd_4), [var = 4]() mutable -> optional<bdd::label_type> {
              if (var < 0) { return {}; }
              return var -= 2;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies even variables in unreduced BDD 4 [&&]", [&]() {
          const bdd out = bdd_exists(
            ep, __bdd(bdd_4__unreduced, ep), [var = 4]() mutable -> optional<bdd::label_type> {
              if (var < 0) { return {}; }
              return var -= 2;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in reduced BDD 4 [&&]", [&]() {
          const bdd out =
            bdd_exists(ep, __bdd(bdd_4), [var = 5]() mutable -> optional<bdd::label_type> {
              if (var < 0) { return {}; }
              return var -= 2;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in unreduced BDD 4 [&&]", [&]() {
          const bdd out = bdd_exists(
            ep, __bdd(bdd_4__unreduced, ep), [var = 5]() mutable -> optional<bdd::label_type> {
              if (var < 0) { return {}; }
              return var -= 2;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          __bdd out = bdd_exists(ep,
                                 __bdd(bdd_20__unreduced, ep),
                                 [called = false]() mutable -> optional<bdd::label_type> {
                                   if (called) { return {}; }
                                   called = true;
                                   return { 3 };
                                 });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });

      // TODO: More tests on arc-based input to independently recreate the more complex tests of the
      //       nested sweeping framework above.
    });

    describe("bdd_exists(const bdd&, ForwardIt, ForwardIt)", [&]() {
      it("returns original file for [].begin() in BDD 1 [const &]", [&]() {
        const bdd in = bdd_1;

        const std::vector<bdd::label_type> vars = {};
        const bdd out                           = bdd_exists(in, vars.begin(), vars.end());

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_1));
      });

      it("returns original file for [3, 5].rbegin() in BDD 11 [const &]", [&]() {
        const bdd in = bdd_11;

        const std::vector<bdd::label_type> vars = { 3, 5 };
        const bdd out                           = bdd_exists(in, vars.rbegin(), vars.rend());

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_11));
      });

      it("returns original file for [0, 3].rbegin() in BDD 11 [&&]", [&]() {
        const std::vector<bdd::label_type> vars = { 0, 3 };
        const bdd out = bdd_exists(bdd(bdd_11), vars.rbegin(), vars.rend());

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_11));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies [1, 3].rbegin() in BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> vars = { 1, 3 };

          bdd out = bdd_exists(ep, bdd(bdd_4), vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies [2, 0].begin() in BDD 4 [const &]", [&]() {
          const bdd in                            = bdd_4;
          const std::vector<bdd::label_type> vars = { 2, 0 };

          bdd out = bdd_exists(ep, in, vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies [4, 2, 0].begin() in BDD 4 [const &]", [&]() {
          const bdd in                            = bdd_4;
          const std::vector<bdd::label_type> vars = { 4, 2, 0 };

          bdd out = bdd_exists(ep, in, vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies [1, 3].rbegin() in BDD 4 [&&]", [&]() {
          std::vector<bdd::label_type> vars = { 1, 3 };

          bdd out = bdd_exists(ep, bdd(bdd_4), vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies [2, 0].begin() in BDD 4 [const &]", [&]() {
          const bdd in                            = bdd_4;
          const std::vector<bdd::label_type> vars = { 2, 0 };

          bdd out = bdd_exists(ep, in, vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies [4, 2, 0].begin() in BDD 4 [const &]", [&]() {
          const bdd in                            = bdd_4;
          const std::vector<bdd::label_type> vars = { 4, 2, 0 };

          bdd out = bdd_exists(ep, in, vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          const std::vector<bdd::label_type> vars = { 6, 4 };
          __bdd out = bdd_exists(ep, bdd(bdd_19), vars.begin(), vars.end());

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(
            nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });
    });

    describe("bdd_exists(__bdd&&, ForwardIt, ForwardIt)", [&]() {
      it("returns original file on reduced BDD 1 and [].begin [&&]", [&]() {
        const std::vector<int> vars = {};
        __bdd out                   = bdd_exists(__bdd(bdd_1), vars.begin(), vars.end());
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      it("returns original file on reduced BDD 2 and [].begin [&&]", [&]() {
        const std::vector<int> vars = {};
        __bdd out                   = bdd_exists(__bdd(bdd_2), vars.begin(), vars.end());
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_2));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies 'x0' in unreduced BDD 1 into terminal [&&]", [&]() {
          const std::vector<int> vars = { 0 };

          const bdd out = bdd_exists(ep, __bdd(bdd_1__unreduced, ep), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies even variables in reduced BDD 4 [&&]", [&]() {
          const std::vector<int> vars = { 0, 2, 4 };
          const bdd out               = bdd_exists(ep, __bdd(bdd_4), vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies even variables in unreduced BDD 4 [&&]", [&]() {
          const std::vector<int> vars = { 4, 2, 0 };
          const bdd out = bdd_exists(ep, __bdd(bdd_4__unreduced, ep), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in unreduced BDD 4 [&&]", [&]() {
          const std::vector<int> vars = { 3, 1 };
          const bdd out = bdd_exists(ep, __bdd(bdd_4__unreduced, ep), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies [0].begin() in unreduced BDD 1 into terminal [&&]", [&]() {
          const std::vector<int> vars = { 0 };

          const bdd out = bdd_exists(ep, __bdd(bdd_1__unreduced, ep), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies [0, 2, 4].rbegin() in reduced BDD 4 [&&]", [&]() {
          const std::vector<int> vars = { 0, 2, 4 };
          const bdd out               = bdd_exists(ep, __bdd(bdd_4), vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies even variables in unreduced BDD 4 [&&]", [&]() {
          const std::vector<int> vars = { 0, 2, 4 };
          const bdd out = bdd_exists(ep, __bdd(bdd_4__unreduced, ep), vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (5)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (2')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in reduced BDD 4 [&&]", [&]() {
          const std::vector<int> vars = { 3, 1 };
          const bdd out               = bdd_exists(ep, __bdd(bdd_4), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in unreduced BDD 4 [&&]", [&]() {
          const std::vector<int> vars = { 3, 1 };
          const bdd out = bdd_exists(ep, __bdd(bdd_4__unreduced, ep), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (3)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          std::vector<int> vars = { 3 };
          __bdd out = bdd_exists(ep, __bdd(bdd_20__unreduced, ep), vars.begin(), vars.end());

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });

      // TODO: More tests on arc-based input to independently recreate the more complex tests of the
      //       nested sweeping framework above.
    });

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // We will not test the Forall operator as much, since it is the same
    // underlying algorithm, but just with the AND operator.
    describe("bdd_forall(const bdd&, bdd::label_type)", [&]() {
      it("quantifies T terminal-only BDD as itself [const &]", [&]() {
        const bdd in = terminal_T;
        __bdd out    = bdd_forall(in, 42);

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(terminal_T));
        AssertThat(out._negate, Is().False());
      });

      it("quantifies F terminal-only BDD as itself [&&]", [&]() {
        __bdd out = bdd_forall(bdd(terminal_F), 21);

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(terminal_F));
        AssertThat(out._negate, Is().False());
      });

      describe("access mode: random access", [&]() {
        const exec_policy ep = exec_policy::access::Random_Access;

        it("quantifies root with non-shortcutting terminal [const &]", [&]() {
          const bdd in = bdd_1;
          __bdd out    = bdd_forall(ep, in, 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("quantifies root (in a different order than with a secondary priority queue) [&&]",
           [&]() {
             // With random access, the order between (3,_) and (3,4) changes; remember that _ is
             // represented by 'nil'.
             __bdd out = bdd_forall(ep, bdd(bdd_3), 0);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(false) }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(false) }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(false) }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                        Is().GreaterThanOrEqualTo(2u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(3u));
             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(1u));
           });

        it("prunes shortcuttable requests [&&]", [&]() {
          __bdd out = bdd_forall(ep, bdd(bdd_4), 2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.low()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 5.high() and 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("resolves nodes in a different order than with a secondary priority queue [&&]", [&]() {
          __bdd out = bdd_forall(ep, bdd(bdd_17b), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(false) }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(true) }));
          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("quantifies a shifted diagram [&&]", [&]() {
          __bdd out = bdd_forall(ep, bdd(bdd_5T, false, +1), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 4.low()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.low()
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.high()
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });
      });

      describe("access mode: priority queue", [&]() {
        const exec_policy ep = exec_policy::access::Priority_Queue;

        it("quantifies root with non-shortcutting terminal [const &]", [&]() {
          const bdd in = bdd_1;
          __bdd out    = bdd_forall(ep, in, 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("quantifies root (in a different order than with random access) [&&]", [&]() {
          __bdd out = bdd_forall(ep, bdd(bdd_3), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("prunes shortcuttable requests [&&]", [&]() {
          __bdd out = bdd_forall(ep, bdd(bdd_4), 2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 3.low()
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // false due to 5.low()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), // true due to 5.high() and 4.high()
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("can forward information across a level [&&]", [&]() {
          __bdd out = bdd_forall(ep, bdd(bdd_6), 1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,8)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());
          AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,8)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (8,T)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, bdd::pointer_type(false) }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(5u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("collapses tuple requests of the same node back into request on a single node [&&]",
           [&]() {
             __bdd out = bdd_forall(ep, bdd(bdd_8a), 1);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(3, 0) }));
             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (3,4)
             AssertThat(arcs.pull_terminal(),                   // false due to 4.low()
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(false) }));

             AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(false) }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(true) }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                        Is().GreaterThanOrEqualTo(2u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(2u));
             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(1u));
           });

        it("resolves nodes in a different order than with random access [&&]", [&]() {
          __bdd out = bdd_forall(ep, bdd(bdd_17b), 0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,_)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (5,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(false) }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(true) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (4,6)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::pointer_type(true) }));
          AssertThat(arcs.can_pull_terminal(), Is().True()); // (7,_)
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(false) }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });
      });
    });

    describe("bdd_forall(__bdd&&, bdd::label_type)", [&]() {
      // TODO
    });

    describe("bdd_forall(const bdd&, const predicate<bdd::label_type>&)", [&]() {
      it("returns original file on always-false predicate BDD 1 [const &]", [&]() {
        bdd in    = bdd_1;
        __bdd out = bdd_forall(in, [](const bdd::label_type) -> bool { return false; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      it("returns original file on always-false predicate BDD 1 [&&]", [&]() {
        __bdd out = bdd_forall(bdd(bdd_1), [](const bdd::label_type) -> bool { return false; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies even variables in BDD 1 [const &]", [&]() {
          const bdd in = bdd_1;
          const bdd out =
            bdd_forall(ep, in, [](const bdd::label_type x) -> bool { return !(x % 2); });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies odd variables in BDD 1 [&&]", [&]() {
          const bdd out =
            bdd_forall(ep, bdd(bdd_1), [](const bdd::label_type x) -> bool { return x % 2; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies <= 2 variables in BDD 4 [&&]", [&]() {
          const bdd out =
            bdd_forall(ep, bdd(bdd_4), [](const bdd::label_type x) -> bool { return x <= 2; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("terminates early when quantifying to a terminal in BDD 5 [&&]", [&]() {
          // TODO: top-down dependant?
          int calls = 0;

          const bdd out = bdd_forall(ep, bdd(bdd_5F), [&calls](const bdd::label_type) -> bool {
            calls++;
            return true;
          });

          AssertThat(calls, Is().EqualTo(3));

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies single variable in a shifted diagram [&&]", [&]() {
          __bdd out =
            bdd_forall(ep, bdd(bdd_5T, false, +1), [](const bdd::label_type x) { return x == 1; });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(
            nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(),
                     Is().EqualTo(node(
                       3, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(),
                     Is().EqualTo(node(2,
                                       bdd::max_id,
                                       bdd::pointer_type(3, bdd::max_id - 1),
                                       bdd::pointer_type(3, bdd::max_id))));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies even variables in BDD 1 [&&]", [&]() {
          const bdd out =
            bdd_forall(ep, bdd(bdd_1), [](const bdd::label_type x) -> bool { return !(x % 2); });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies odd variables in BDD 1 [&&]", [&]() {
          const bdd out =
            bdd_forall(ep, bdd(bdd_1), [](const bdd::label_type x) -> bool { return x % 2; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("bails out on a level that only shortcuts [&&]", [&]() {
          bdd out = bdd_forall(
            ep, bdd_not(bdd_9T), [](const bdd::label_type x) -> bool { return !(x % 2); });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant [&&]", [&]() {
          bdd out = bdd_forall(
            ep, bdd_not(bdd_9F), [](const bdd::label_type x) -> bool { return !(x % 2); });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("accounts for number of root arcs from Outer Sweep [&&]", [&]() {
          /* expected
          //
          //        F
          */
          bdd out = bdd_forall(ep, bdd(bdd_16), [](int x) -> bool { return x != 5; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          __bdd out = bdd_forall(
            ep, bdd(bdd_19, true), [](const bdd::label_type x) { return x == 4 || x == 6; });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(
            nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("prunes root during transposition of a shifted diagram [&&]", [&]() {
          __bdd out =
            bdd_forall(ep, bdd(bdd_5T, false, +1), [](const bdd::label_type x) { return x == 1; });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(
            nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(),
                     Is().EqualTo(node(
                       3, bdd::max_id - 1, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(),
                     Is().EqualTo(node(2,
                                       bdd::max_id,
                                       bdd::pointer_type(3, bdd::max_id - 1),
                                       bdd::pointer_type(3, bdd::max_id))));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3u, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2u, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });
    });

    describe("bdd_forall(__bdd&&, const predicate<bdd::label_type>&)", [&]() {
      it("returns original file on reduced BDD 1 and always-false predicate [&&]", [&]() {
        __bdd out = bdd_forall(__bdd(bdd_1), [](const bdd::label_type) -> bool { return false; });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies x0 in unreduced BDD 1 [&&]", [&]() {
          const bdd out = bdd_forall(
            ep, __bdd(bdd_1__unreduced, ep), [](bdd::label_type x) -> bool { return x == 0u; });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies x0 in unreduced BDD 1 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          const bdd out =
            bdd_forall(ep, __bdd(bdd_1__unreduced, ep), [&call_history](bdd::label_type x) -> bool {
              call_history.push_back(x);
              return x == 0u;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(3u));

          // - Nested Sweep
          AssertThat(call_history.at(0), Is().EqualTo(2u));
          AssertThat(call_history.at(1), Is().EqualTo(1u));
          AssertThat(call_history.at(2), Is().EqualTo(0u)); // <-- bail out into terminal!
        });

        it("quantifies x1 in unreduced BDD 1 [&&]", [&]() {
          std::vector<bdd::label_type> call_history;
          const bdd out =
            bdd_forall(ep, __bdd(bdd_1__unreduced, ep), [&call_history](bdd::label_type x) -> bool {
              call_history.push_back(x);
              return x == 1u;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...

          // Check call history
          //
          // NOTE: Test failure does NOT indicate a bug, but only indicates a change. Please verify
          //       that this change makes sense and is as intended.
          AssertThat(call_history.size(), Is().EqualTo(3u));

          // - Nested Sweep
          AssertThat(call_history.at(0), Is().EqualTo(2u));
          AssertThat(call_history.at(1), Is().EqualTo(1u)); // <-- bail out!
          AssertThat(call_history.at(2), Is().EqualTo(0u));
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          __bdd out = bdd_forall(
            ep, __bdd(bdd_20__unreduced, ep), [](const bdd::label_type x) { return x == 3; });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });
    });

    describe("bdd_forall(const bdd&, const generator<bdd::label_type>&)", [&]() {
      it("returns original file on 'optional::none' generator in BDD 1 [&&]", [&]() {
        __bdd out = bdd_forall(bdd(bdd_1), []() { return make_optional<bdd::label_type>(); });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies 0 in BDD 1 [const &]", [&]() {
          const bdd in  = bdd_1;
          const bdd out = bdd_forall(ep, in, [var = 0]() mutable -> optional<bdd::label_type> {
            if (var > 0) { return make_optional<bdd::label_type>(); }
            return var++;
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1 in BDD 1 [&&]", [&]() {
          const bdd out =
            bdd_forall(ep, bdd(bdd_1), [var = 1]() mutable -> optional<bdd::label_type> {
              if (var == 0) { return {}; }
              return { var-- };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("terminates early when quantifying 2, 0 to a terminal in BDD 3 [&&]", [&]() {
          int calls = 0;

          const bdd out = bdd_forall(ep, bdd(bdd_3), [&calls]() -> optional<bdd::label_type> {
            return { 2 - 2 * (calls++) };
          });

          // What could be expected is 3 calls: 2, 0, none . But, here it terminates early.
          AssertThat(calls, Is().EqualTo(2));

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies 0 in BDD 1 [&&]", [&]() {
          const bdd out =
            bdd_forall(ep, bdd(bdd_1), [var = 0]() mutable -> optional<bdd::label_type> {
              if (var == 1) { return {}; }
              return { var++ };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 1 in BDD 1 [&&]", [&]() {
          const bdd out =
            bdd_forall(ep, bdd(bdd_1), [var = 1]() mutable -> optional<bdd::label_type> {
              if (var == 0) { return make_optional<bdd::label_type>(); }

              const bdd::label_type ret = var;
              var--;
              return ret;
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("bails out on a level that only shortcuts [&&]", [&]() {
          bdd out =
            bdd_forall(ep, bdd_not(bdd_9T), [var = 6]() mutable -> optional<bdd::label_type> {
              if (var == 42) { return {}; }

              const bdd::label_type res = var;
              var                       = res == 0 ? 42 : var - 2;
              return { res };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("bails out on a level that only is irrelevant [&&]", [&]() {
          bdd out =
            bdd_forall(ep, bdd_not(bdd_9F), [var = 6]() mutable -> optional<bdd::label_type> {
              if (var == 42) { return {}; }

              const bdd::label_type res = var;
              var                       = res == 0 ? 42 : var - 2;
              return { res };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (7')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().True()); // (5')
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5u, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("accounts for number of root arcs from Outer Sweep [&&]", [&]() {
          /* expected
          //
          //        F
          */
          bdd out = bdd_forall(ep, bdd(bdd_16), [var = 6]() mutable -> optional<bdd::label_type> {
            var -= 1;
            if (var < 0) { return {}; }
            if (var == 5) { var -= 1; }
            return { var };
          });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          __bdd out = bdd_forall(ep,
                                 __bdd(bdd_20__unreduced, ep),
                                 [called = false]() mutable -> optional<bdd::label_type> {
                                   if (called) { return {}; }
                                   called = true;
                                   return { 3 };
                                 });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });
    });

    describe("bdd_forall(__bdd&&, const generator<bdd::label_type>&)", [&]() {
      it("returns original file on reduced BDD 1 and 'optional::none' generator [&&]", [&]() {
        __bdd out = bdd_forall(__bdd(bdd_1), []() { return make_optional<bdd::label_type>(); });
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies x0 in unreduced BDD 1 [&&]", [&]() {
          const bdd out = bdd_forall(
            ep, __bdd(bdd_1__unreduced, ep), [called = false]() mutable -> optional<int> {
              if (called) { return {}; }
              called = true;
              return { 0 };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies x1 in unreduced BDD 1 [&&]", [&]() {
          const bdd out = bdd_forall(
            ep, __bdd(bdd_1__unreduced, ep), [called = false]() mutable -> optional<int> {
              if (called) { return {}; }
              called = true;
              return { 1 };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies x0 in unreduced BDD 1 [&&]", [&]() {
          const bdd out = bdd_forall(
            ep, __bdd(bdd_1__unreduced, ep), [called = false]() mutable -> optional<int> {
              if (called) { return {}; }
              called = true;
              return { 0 };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies x1 in unreduced BDD 1 [&&]", [&]() {
          const bdd out = bdd_forall(
            ep, __bdd(bdd_1__unreduced, ep), [called = false]() mutable -> optional<int> {
              if (called) { return {}; }
              called = true;
              return { 1 };
            });

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          __bdd out = bdd_forall(ep,
                                 __bdd(bdd_20__unreduced, ep),
                                 [called = false]() mutable -> optional<bdd::label_type> {
                                   if (called) { return {}; }
                                   called = true;
                                   return { 3 };
                                 });

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });
    });

    describe("bdd_forall(const bdd&, ForwardIt, ForwardIt)", [&]() {
      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies 'x0' in BDD 1 [const &]", [&]() {
          const bdd in                            = bdd_1;
          const std::vector<bdd::label_type> vars = { 0 };

          const bdd out = bdd_forall(in, vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 'x1' in BDD 1 [&&]", [&]() {
          const std::vector<bdd::label_type> vars = { 1 };
          const bdd out = bdd_forall(ep, bdd(bdd_1), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies 'x0' in BDD 1 [const &]", [&]() {
          const bdd in                            = bdd_1;
          const std::vector<bdd::label_type> vars = { 0 };

          const bdd out = bdd_forall(in, vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("quantifies 'x1' in BDD 1 [&&]", [&]() {
          const std::vector<bdd::label_type> vars = { 1 };
          const bdd out = bdd_forall(ep, bdd(bdd_1), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (1')
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });
      });
    });

    describe("bdd_forall(__bdd&&, ForwardIt, ForwardIt)", [&]() {
      it("returns original file on reduced BDD 1 and [].begin() [&&]", [&]() {
        const std::vector<int> vars = {};
        __bdd out                   = bdd_forall(__bdd(bdd_1), vars.begin(), vars.end());
        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
      });

      describe("algorithm: Singleton", [&]() {
        const exec_policy ep = exec_policy::quantify::Singleton;

        it("quantifies 'x0' in unreduced BDD 1 [&&]", [&]() {
          const std::vector<int> vars = { 0 };
          const bdd out = bdd_forall(ep, __bdd(bdd_1__unreduced, ep), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies 'x1' in unreduced BDD 1 [&&]", [&]() {
          const std::vector<int> vars = { 1 };
          const bdd out = bdd_forall(ep, __bdd(bdd_1__unreduced, ep), vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });
      });

      describe("algorithm: Nested", [&]() {
        const exec_policy ep = exec_policy::quantify::Nested;

        it("quantifies 'x0' in unreduced BDD 1 [&&]", [&]() {
          const std::vector<int> vars = { 0 };
          const bdd out = bdd_forall(ep, __bdd(bdd_1__unreduced, ep), vars.begin(), vars.end());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("quantifies 'x1' in unreduced BDD 1 [&&]", [&]() {
          const std::vector<int> vars = { 1 };
          const bdd out = bdd_forall(ep, __bdd(bdd_1__unreduced, ep), vars.rbegin(), vars.rend());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // (2)
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(true), bdd::pointer_type(false))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // TODO: meta variables...
        });

        it("handles 2-level cut in Outer Sweep [&&]", [&]() {
          std::vector<int> vars = { 3 };
          __bdd out = bdd_forall(ep, __bdd(bdd_20__unreduced, ep), vars.rbegin(), vars.rend());

          node_test_ifstream nodes(out);

          AssertThat(nodes.can_pull(), Is().True());
          AssertThat(nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(nodes.can_pull(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().False());

          // TODO: meta variables
        });
      });
    });
  });
});
