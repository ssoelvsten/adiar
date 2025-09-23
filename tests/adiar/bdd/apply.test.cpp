#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/apply.cpp", []() {
    // Setup shared bdd's
    shared_levelized_file<bdd::node_type> bdd_F;
    /*
    //        F
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(bdd_F);
      nw << node(false);
    }

    shared_levelized_file<bdd::node_type> bdd_T;
    /*
    //        T
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(bdd_T);
      nw << node(true);
    }

    // TODO: all of these may not be shared
    const bdd::pointer_type terminal_T(true);
    const bdd::pointer_type terminal_F(false);

    shared_levelized_file<bdd::node_type> bdd_x0;
    /*
    //          1        ---- x0
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x0);
      nw << node(0, bdd::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_not_x0;
    /*
    //          1        ---- x0
    //         / \
    //         T F
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_not_x0);
      nw << node(0, bdd::max_id, terminal_T, terminal_F);
    }

    shared_levelized_file<bdd::node_type> bdd_x1;
    /*
    //          1        ---- x1
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x1);
      nw << node(1, bdd::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_x2;
    /*
    //          1        ---- x2
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x2);
      nw << node(2, bdd::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_1;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x1
    //       |/ \
    //       3   4     ---- x2
    //      / \ / \
    //      F T T 5    ---- x3
    //           / \
    //           F T
    */

    { // Garbage collect early and free write-lock
      const node n5 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n4 = node(2, bdd::max_id, terminal_T, n5.uid());
      const node n3 = node(2, bdd::max_id - 1, terminal_F, terminal_T);
      const node n2 = node(1, bdd::max_id, n3.uid(), n4.uid());
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_1);
      nw << n5 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_2;
    /*
    //               ---- x0
    //
    //         1     ---- x1
    //        / \
    //        | T    ---- x2
    //        |
    //        2      ---- x3
    //       / \
    //       T F
    */

    { // Garbage collect early and free write-lock
      const node n2 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n1 = node(1, bdd::max_id, n2.uid(), terminal_T);

      node_ofstream nw(bdd_2);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_3;
    /*
    //            1         ---- x0
    //           / \
    //           2 3        ---- x1
    //         _/ X \_
    //        | _/ \_ |
    //         X     X
    //        / \   / \
    //       4  5  6  7     ---- x2
    //      / \/ \/ \/ \
    //      F T  8  T  F    ---- x3
    //          / \
    //          F T
    */

    { // Garbage collect early and free write-lock
      const node n8 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n7 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n6 = node(2, bdd::max_id - 1, n8.uid(), terminal_T);
      const node n5 = node(2, bdd::max_id - 2, terminal_T, n8.uid());
      const node n4 = node(2, bdd::max_id - 3, terminal_F, terminal_T);
      const node n3 = node(1, bdd::max_id, n4.uid(), n6.uid());
      const node n2 = node(1, bdd::max_id - 1, n5.uid(), n7.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_3);
      nw << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    // TODO: should these be defined here?
    shared_levelized_file<bdd::node_type> bdd_0xnor2;
    /*
    //           1     ---- x0
    //          / \
    //         /   \   ---- x1
    //         |   |
    //         2   3   ---- x2
    //        / \ / \
    //        T F F T
    */

    { // Garbage collect early and free write-lock
      const node n3 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n2 = node(2, bdd::max_id - 1, terminal_T, terminal_F);
      const node n1 = node(0, bdd::max_id, n2, n3);

      node_ofstream nw(bdd_0xnor2);
      nw << n3 << n2 << n1;
    }

    // Some are shared, and some are not - easier to find with all in one location - cluttering
    // maybe though?
    shared_levelized_file<bdd::node_type> bdd_thin;
    /*
    //                  1          ---- x0
    //                 / \
    //                2   3        ---- x1
    //                X_ / \
    //               /  X   \
    //               |_/ \__|
    //               4      5      ---- x2
    //              / \    / \
    //              T F    F T
    */

    { // Garbage collect early and free write-lock
      const node n5 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n4 = node(2, bdd::max_id - 1, terminal_T, terminal_F);
      const node n3 = node(1, bdd::max_id, n4, n5);
      const node n2 = node(1, bdd::max_id - 1, n5, n4);
      const node n1 = node(0, bdd::max_id, n2, n3);

      node_ofstream nw(bdd_thin);
      nw << n5 << n4 << n3 << n2 << n1;
    }

    // bdd_thin->indexable == true
    // bdd_thin->width == 2u

    shared_levelized_file<bdd::node_type> bdd_wide;
    /*
    //                1         ---- x0
    //               / \
    //              2   3       ---- x1
    //              X   X
    //             / \ / \
    //            4   5   6     ---- x2
    //           / \ / \ / \
    //           F 7 T F F T    ---- x3
    //            / \
    //            F T
    */

    { // Garbage collect early and free write-lock
      const node n7 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n6 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n5 = node(2, bdd::max_id - 1, terminal_T, terminal_F);
      const node n4 = node(2, bdd::max_id - 2, terminal_F, n7);
      const node n3 = node(1, bdd::max_id, n6, n5);
      const node n2 = node(1, bdd::max_id - 1, n5, n4);
      const node n1 = node(0, bdd::max_id, n2, n3);

      node_ofstream nw(bdd_wide);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    // bdd_wide->indexable == true
    // bdd_wide->width == 3u

    /* Result of [thin] ^ [wide]
    //
    //                      (1,1)                   ---- x0
    //                      /   \
    //                  (2,2)   (3,3)               ---- x1
    //                    X       X
    //              _____/ \     / \_____
    //          (4,4)       (5,5)       (4,6)       ---- x2
    //          /   \       /   \       /   \
    //       (T,F) (F,7) (F,T) (T,F) (T,F) (F,T)    ---- x3
    //             /   \
    //          (F,F) (F,T)
    */

    shared_levelized_file<bdd::node_type> bdd_canon;
    /*
    //                 1              ---- x0
    //                / \
    //               2   3            ---- x1
    //           ___/ \ / \___
    //          /    __X__    \
    //         4    5     6    7      ---- x2
    //        / \  / \   / \  / \
    //        F 8  F 9   T F  F T     ---- x3
    //         / \  / \
    //         T F  F T
    */

    { // Garbage collect early and free write-lock
      const node n9 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n8 = node(3, bdd::max_id - 1, terminal_T, terminal_F);
      const node n7 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n6 = node(2, bdd::max_id - 1, terminal_T, terminal_F);
      const node n5 = node(2, bdd::max_id - 2, terminal_F, n9);
      const node n4 = node(2, bdd::max_id - 3, terminal_F, n8);
      const node n3 = node(1, bdd::max_id, n5, n7);
      const node n2 = node(1, bdd::max_id - 1, n4, n6);
      const node n1 = node(0, bdd::max_id, n2, n3);

      node_ofstream nw(bdd_canon);
      nw << n9 << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    // bdd_canon->indexable == true
    // bdd_canon->width == 4u

    shared_levelized_file<bdd::node_type> bdd_indexable;
    /*
    //                1            ---- x0
    //               / \
    //              2   3          ---- x1
    //             / \ _X_
    //             |__X   \
    //             4   5   6       ---- x2
    //            / \ / \ / \
    //            T F 7 F F T      ---- x3
    //               / \
    //               T F
    */

    { // Garbage collect early and free write-lock
      const node n7 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n6 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n5 = node(2, bdd::max_id - 1, n7, terminal_F);
      const node n4 = node(2, bdd::max_id - 2, terminal_T, terminal_F);
      const node n3 = node(1, bdd::max_id, n6, n4);
      const node n2 = node(1, bdd::max_id - 1, n4, n5);
      const node n1 = node(0, bdd::max_id, n2, n3);

      node_ofstream nw(bdd_indexable);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    // bdd_indexable->indexable == true
    // bdd_indexable->width == 3u

    /* Result of [canonical] ^ [indexable]
    //
    //                         (1,1)                              ---- x0
    //                   ______/   \______
    //                  /                 \
    //              (2,2)                 (3,3)                   ---- x1
    //              /   \________ __________X
    //            _/          ___X___        \___
    //           /           /       \           \
    //       (4,4)       (7,4)       (6,5)       (5,6)            ---- x2
    //       /   \       /   \       /   \       /   \
    //    (F,T) (8,F) (F,T) (T,F) (T,7) (F,F) (F,F) (9,T)         ---- x3
    //          /   \             /   \             /   \
    //      (T,F)   (F,F)     (T,T)   (T,F)     (F,T)   (T,T)
    */

    shared_levelized_file<bdd::node_type> bdd_unindexable;
    /*
    // Same as `bdd_indexable` but nodes are not consecutive.
    */

    { // Garbage collect early and free write-lock
      const node n7 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n6 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n5 = node(2, bdd::max_id - 1, n7, terminal_F);
      const node n4 = node(2, bdd::max_id - 2, terminal_T, terminal_F);
      const node n3 = node(1, bdd::max_id, n6, n4);
      const node n2 = node(1, bdd::max_id - 2, n4, n5); // <-- bad index
      const node n1 = node(0, bdd::max_id - 1, n2, n3); // <-- bad index

      node_ofstream nw(bdd_unindexable);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    // bdd_indexable->indexable == false
    // bdd_indexable->width == 3u

    describe("simple cases without access mode requirements", [&]() {
      // Cases with the same file (same DAG) or at least one terminal does not need to run
      // complicated algorithm

      describe("bdd_and(f,g)", [&]() {
        it("resolves F /\\ T terminal-only BDDs", [&]() {
          __bdd out = bdd_and(bdd_F, bdd_T);
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("resolves T /\\ T terminal-only BDDs", [&]() {
          shared_levelized_file<bdd::node_type> bdd_T2;
          {
            node_ofstream w(bdd_T2);
            w << node(true);
          }

          __bdd out = bdd_and(bdd_T, bdd_T2);
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

        it("shortcuts on irrelevance on x0 /\\ T", [&]() {
          __bdd out = bdd_and(bdd_x0, bdd_T);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_x0));
          AssertThat(out._negate, Is().False());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("shortcuts F /\\ x0", [&]() {
          __bdd out = bdd_and(bdd_F, bdd_x0);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("shortcuts F /\\ [2]", [&]() {
          __bdd out = bdd_and(bdd_F, bdd_2);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("returns input on being given the same BDD twice", [&]() {
          __bdd out = bdd_and(bdd_1, bdd_1);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().False());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("collapses on the same BDD twice, where one is negated [left]", [&]() {
          __bdd out = bdd_and(bdd(bdd_1, true), bdd(bdd_1, false));

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("collapses on the same BDD twice, where one is negated [right]", [&]() {
          __bdd out = bdd_and(bdd(bdd_1, false), bdd(bdd_1, true));

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("returns shifted input on being given the same shifted BDD twice", [&]() {
          __bdd out = bdd_and(bdd(bdd_1, false, +1), bdd(bdd_1, false, +1));

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().False());
          AssertThat(out._shift, Is().EqualTo(1));
        });
      });

      describe("bdd_nand(f,g)", [&]() {
        it("shortcuts on negating on T and x0", [&]() {
          __bdd out = bdd_nand(bdd_x0, bdd_T);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_x0));
          AssertThat(out._negate, Is().True());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("shortcuts on negating on x0 and T", [&]() {
          __bdd out = bdd_nand(bdd_T, bdd_x0);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_x0));
          AssertThat(out._negate, Is().True());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("returns input on being given the same BDD twice", [&]() {
          __bdd out = bdd_nand(bdd_1, bdd_1);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().True());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("collapses on the same BDD twice, where one is negated [left]", [&]() {
          __bdd out = bdd_nand(bdd(bdd_2, true), bdd(bdd_2, false));

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

        it("collapses on the same BDD twice, where one is negated [right]", [&]() {
          __bdd out = bdd_nand(bdd(bdd_2, false), bdd(bdd_2, true));

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

        it("returns shifted input on being given the same shifted BDD twice", [&]() {
          __bdd out = bdd_nand(bdd(bdd_1, false, +1), bdd(bdd_1, false, +1));

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().True());
          AssertThat(out._shift, Is().EqualTo(1));
        });
      });

      describe("bdd_or(f,g)", [&]() {
        it("resolves T \\/ F terminal-only BDDs", [&]() {
          __bdd out = bdd_or(bdd_T, bdd_F);
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

        it("resolves F \\/ F terminal-only BDDs", [&]() {
          shared_levelized_file<bdd::node_type> bdd_F2;

          {
            node_ofstream w(bdd_F2);
            w << node(false);
          }

          __bdd out = bdd_or(bdd_F, bdd_F2);
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("shortcuts on irrelevance on x0 \\/ F", [&]() {
          __bdd out = bdd_or(bdd_x0, bdd_F);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_x0));
          AssertThat(out._negate, Is().False());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("shortcuts on irrelevance F \\/ x0", [&]() {
          __bdd out = bdd_or(bdd_F, bdd_x0);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_x0));
          AssertThat(out._negate, Is().False());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("shortcuts [1] \\/ T", [&]() {
          __bdd out = bdd_or(bdd_1, bdd_T);

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

        it("shortcuts [2] \\/ T", [&]() {
          __bdd out = bdd_or(bdd_2, bdd_T);

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

        it("returns input on being given the same BDD twice", [&]() {
          __bdd out = bdd_or(bdd_1, bdd_1);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().False());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("collapses on the same BDD twice, where one is negated [left]", [&]() {
          __bdd out = bdd_or(bdd(bdd_1, true), bdd(bdd_1, false));

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

        it("collapses on the same BDD twice, where one is negated [right]", [&]() {
          __bdd out = bdd_or(bdd(bdd_1, false), bdd(bdd_1, true));

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

        it("returns input on being given the same BDD twice, when both are negated", [&]() {
          __bdd out = bdd_or(bdd(bdd_1, true), bdd(bdd_1, true));

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().True());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("returns shifted input on being given the same shifted BDD twice", [&]() {
          __bdd out = bdd_or(bdd(bdd_1, false, +1), bdd(bdd_1, false, +1));

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().False());
          AssertThat(out._shift, Is().EqualTo(1));
        });
      });

      describe("bdd_nor(f,g)", [&]() {
        it("returns input on being given the same BDD twice", [&]() {
          __bdd out = bdd_nor(bdd_1, bdd_1);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().True());
        });

        it("collapses on the same BDD twice, where one is negated [left]", [&]() {
          __bdd out = bdd_nor(bdd(bdd_3, true), bdd(bdd_3, false));

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("collapses on the same BDD twice, where one is negated [right]", [&]() {
          __bdd out = bdd_nor(bdd(bdd_3, false), bdd(bdd_3, true));

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("returns shifted input on being given the same shifted BDD twice", [&]() {
          __bdd out = bdd_nor(bdd(bdd_1, false, +1), bdd(bdd_1, false, +1));

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
          AssertThat(out._negate, Is().True());
          AssertThat(out._shift, Is().EqualTo(1));
        });
      });

      describe("bdd_xor(f,g)", [&]() {
        it("resolves F ^ T terminal-only BDDs", [&]() {
          __bdd out = bdd_xor(bdd_F, bdd_T);
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

        it("resolves T ^ T terminal-only BDDs", [&]() {
          __bdd out = bdd_xor(bdd_T, bdd_T);
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("shortcuts on negating on x0 ^ T", [&]() {
          __bdd out = bdd_xor(bdd_x0, bdd_T);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_x0));
          AssertThat(out._negate, Is().True());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("shortcuts on negating on T ^ x0", [&]() {
          __bdd out = bdd_xor(bdd_x0, bdd_T);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_x0));
          AssertThat(out._negate, Is().True());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("collapses on the same BDD twice", [&]() {
          __bdd out = bdd_xor(bdd_1, bdd_1);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("collapses on the same BDD twice, where one is negated [left]", [&]() {
          __bdd out = bdd_xor(bdd(bdd_1, true), bdd(bdd_1, false));

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

        it("collapses on the same BDD twice, where one is negated [right]", [&]() {
          __bdd out = bdd_xor(bdd(bdd_2, false), bdd(bdd_2, true));

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

        it("collapses on the same BDD twice, when both are negated", [&]() {
          __bdd out = bdd_xor(bdd(bdd_1, true), bdd(bdd_1, true));

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });
      });

      describe("bdd_imp(f,g)", [&]() {
        it("resolves F -> T terminal-only BDDs", [&]() {
          __bdd out = bdd_imp(bdd_F, bdd_T);
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

        it("resolves T -> F terminal-only BDDs", [&]() {
          __bdd out = bdd_imp(bdd_T, bdd_F);
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_node_file_type>()->levels(), Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->max_1level_cut[cut::All],
                     Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_node_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("resolves T -> T terminal-only BDDs", [&]() {
          __bdd out = bdd_imp(bdd_T, bdd_T);
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

        it("shortcuts on irrelevance on T -> x0", [&]() {
          __bdd out = bdd_imp(bdd_T, bdd_x0);

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_x0));
          AssertThat(out._negate, Is().False());
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("shortcuts F -> [1]", [&]() {
          __bdd out = bdd_imp(bdd_F, bdd_1);

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

        it("collapses when given the same BDD twice", [&]() {
          __bdd out = bdd_imp(bdd_1, bdd_1);

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

        it("returns input when given the same BDD twice, where one is negated [left]", [&]() {
          __bdd out = bdd_imp(bdd(bdd_2, true), bdd(bdd_2, false));

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_2));
          AssertThat(out._negate, Is().False()); // negated the already negated input
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("returns input when given the same BDD twice, where one is negated [right]", [&]() {
          __bdd out = bdd_imp(bdd(bdd_2, false), bdd(bdd_2, true));

          AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_2));
          AssertThat(out._negate, Is().True()); // negated the first of the two
          AssertThat(out._shift, Is().EqualTo(0));
        });

        it("returns shifted input when given the same shifted BDD twice, where one is negated "
           "[left]",
           [&]() {
             __bdd out = bdd_imp(bdd(bdd_2, true, +3), bdd(bdd_2, false, +3));

             AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_2));
             AssertThat(out._negate, Is().False()); // negated the already negated input
             AssertThat(out._shift, Is().EqualTo(3));
           });

        it("returns shifted input when given the same shifted BDD twice, where one is negated "
           "[right]",
           [&]() {
             __bdd out = bdd_imp(bdd(bdd_2, false, +2), bdd(bdd_2, true, +2));

             AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_2));
             AssertThat(out._negate, Is().True()); // negated the first of the two
             AssertThat(out._shift, Is().EqualTo(2));
           });

        it("collapses when given the same BDD twice, when both are negated", [&]() {
          __bdd out = bdd_imp(bdd(bdd_1, true), bdd(bdd_1, true));

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
      });
    });

    describe("access mode: priority queues", [&]() {
      // Set access mode to priority queues for this batch of tests
      const exec_policy ep = exec_policy::access::Priority_Queue;

      /* The product construction of bbd_1 and bdd_2 above is as follows in sorted order.
      //
      //                                        (1,1)                       ---- x0
      //                                        \_ _/
      //                                         _X_                        // Match in first, but
      not coordinatewise
      //                                        /   \
      //                                    (3,1)   (2,1)                   ---- x1
      //                                   /    \_ _/    \
      //                                  /       X       \
      //                                 /_______/ \       \
      //                                 |          \       \
      //                             (3,2)          (3,T)   (4,T)           ---- x2
      //                              \ \           /   \   /   \
      //                               \ \      (F,T)   (T,T)   /
      //                                \ \________ ___________/
      //                                 \________ X________
      //                                          X_________\ _______
      //                                         /           \       \
      //                                        /             \       \
      //                                     (5,T)         (F,2)     (T,2)   ---- x3
      //                                     /   \         /   \     /   \
      //                                  (F,T) (T,T)   (F,T)(F,F)  (T,T)(T,F)
      */

      describe("bdd_and(f,g)", [&]() {
        it("computes x0 and !x0", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F F
          */

          __bdd out = bdd_and(ep, bdd_x0, bdd_not_x0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(0, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("computes x0 /\\ x2", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F 2    ---- x2
          //           / \
          //           F T
          */

          __bdd out = bdd_and(ep, bdd_x0, bdd_x2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes x0 /\\ x0(+1)", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F 2    ---- x1
          //           / \
          //           F T
          */

          __bdd out = bdd_and(ep, bdd(bdd_x0, false, +0), bdd(bdd_x0, false, +1));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes (and shortcut) [1] /\\ [2]", [&]() {
          /*
          //                    1                        ---- x0
          //                    X
          //                   / \
          //                  2   3                      ---- x1
          //                 / \ / \
          //                /   X   \
          //               /___/ \   \
          //              /      |    \
          //             4       5     6                 ---- x2
          //            / \     / \_ _/ \
          //            F 7     F   T   8                ---- x3
          //             / \           / \
          //             T F           F T
          */

          __bdd out = bdd_and(ep, bdd_1, bdd_2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(4u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });

        it("computes [1] /\\ [2](-1)", [&]() {
          /*
          //        _(1,1)_         ---- x0
          //       /       \
          //       |     (2,T)      ---- x1
          //       |     /   \
          //     (3,2)   3   4      ---- x2
          //      / \   / \ / \
          //      F F   F T T 5     ---- x3
          //                 / \
          //                 F T
          */

          __bdd out = bdd_and(ep, bdd(bdd_1, false, +0), bdd(bdd_2, false, -1));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(4u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("groups all recursion requests together", [&]() {
          // This is a counter-example to the prior "break ties on first() with
          // second()" approach. Here we will have three requests to the level of
          // x2, but in the following order:
          //
          // [((2,0),(2,1)), ((2,1),(2,0)), ((2,0),(2,1))]
          //
          // which all are tied, and hence the prior version would create
          // three nodes on this level rather than just two.

          // TODO: This drawing seems to not fit - fix it?

          /*
          //         1    ---- x0
          //        / \
          //        2 |   ---- x1
          //       / \|
          //       3  4   ---- x2
          //      / \/ \
          //      T  F T
          */

          // The second version is the same but has the nodes 3 and 4 mirrored
          // and the T terminals are replaced with an arc to a node for x3.

          shared_levelized_file<bdd::node_type> bdd_group_1, bdd_group_2;
          { // Garbage collect writers to free write-lock
            node_ofstream w1(bdd_group_1);
            w1 << node(2, 1, bdd::pointer_type(false), bdd::pointer_type(true))
               << node(2, 0, bdd::pointer_type(true), bdd::pointer_type(false))
               << node(1, 0, bdd::pointer_type(2, 0), bdd::pointer_type(2, 1))
               << node(0, 1, bdd::pointer_type(1, 0), bdd::pointer_type(2, 1));

            node_ofstream w2(bdd_group_2);
            w2 << node(3, 0, bdd::pointer_type(false), bdd::pointer_type(true))
               << node(2, 1, bdd::pointer_type(3, 0), bdd::pointer_type(false))
               << node(2, 0, bdd::pointer_type(false), bdd::pointer_type(3, 0))
               << node(1, 0, bdd::pointer_type(2, 1), bdd::pointer_type(2, 0))
               << node(0, 1, bdd::pointer_type(1, 0), bdd::pointer_type(2, 0));
          }

          __bdd out = bdd_and(ep, bdd_group_1, bdd_group_2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 1) }));
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (T,5) i.e. the added node
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));
          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });
      });

      describe("bdd_nand(f,g)", [&]() {
        // TODO
      });

      describe("bdd_or(f,g)", [&]() {
        // TODO: this one too?
        it("computes x0(+2) \\/ x2", [&]() {
          /*
          //           1     ---- x2
          //          / \
          //          F T
          */

          __bdd out = bdd_or(ep, bdd(bdd_x0, false, +2), bdd_x2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("shortcuts on x0 \\/ x2", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          | T
          //          |
          //          2      ---- x2
          //         / \
          //         F T
          */

          __bdd out = bdd_or(ep, bdd_x0, bdd_x2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("shortcuts on x2 \\/ x1", [&]() {
          /*
          //           1     ---- x1
          //          / \
          //          2 T    ---- x2
          //         / \
          //         F T
          */

          __bdd out = bdd_or(ep, bdd_x2, bdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("shortcuts on x0(+2) \\/ x1", [&]() {
          /*
          //           1     ---- x1
          //          / \
          //          2 T    ---- x2
          //         / \
          //         F T
          */

          __bdd out = bdd_or(ep, bdd(bdd_x0, false, +2), bdd(bdd_x1, false, +0));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes (and shortcut) [1] \\/ [2]", [&]() {
          /*
          //                   1       ---- x0
          //                  / \
          //                 2   3     ---- x1
          //                / \ / \
          //                | T | T
          //                \_ _/
          //                  4        ---- x2
          //                 / \
          //                 5  T      ---- x3
          //                / \
          //                T F
          */

          __bdd out = bdd_or(ep, bdd_1, bdd_2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });

        it("computes (and shortcuts) [1] \\/ [2](-1)", [&]() {
          /*
          //        _(1,1)_       ---- x0
          //       /       \
          //       |       T      ---- x1
          //       |
          //     (3,2)            ---- x2
          //      / \
          //      T T
          */
          __bdd out = bdd_or(ep, bdd(bdd_1, false, +0), bdd(bdd_2, false, -1));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });
      });

      describe("bdd_nor(f,g)", [&]() {
        // TODO
      });

      describe("bdd_xor(f,g)", [&]() {
        it("computes x0(+1) ^ x1", [&]() {
          /*
          //           1     ---- x1
          //          / \
          //          F F
          */

          __bdd out = bdd_xor(ep, bdd(bdd_x0, false, +1), bdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("computes x0 ^ x1", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests
          // after evaluating x0.
          //
          //           1     ---- x0
          //          / \
          //         2   3   ---- x1
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd_x0, bdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes x0(+2) ^ x1", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests
          //   after evaluating x0
          //
          //           1     ---- x1
          //          / \
          //         2   3   ---- x2
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd(bdd_x0, false, +2), bdd_x1);

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
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes x0 ^ x0(+2)", [&]() {
          /*
          //          _1_    ---- x0
          //         /   \
          //         2   3   ---- x2
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd_x0, bdd(bdd_x0, false, +2));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes [2] ^ x2", [&]() {
          /*
          //                                 ---- x0
          //
          //                 (1,1)           ---- x1
          //                 /   \
          //             (2,1)   (T,1)       ---- x2
          //             /   \   /   \
          //            /     \  T   F
          //            |     |
          //        (2,F)     (2,T)          ---- x3
          //        /   \     /   \
          //        T   F     F   T
          */
          __bdd out = bdd_xor(ep, bdd_2, bdd_x2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("computes [1] ^ [2]", [&]() {
          /* There is no shortcutting possible on an XOR, so see the product
             construction above. */

          __bdd out = bdd_xor(ep, bdd_1, bdd_2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(5u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });

        it("computes [3] ^ [1]", [&]() {
          /* The queue pq_2 is used to forward data across the level. When
          // [1] and 3 are combined, this is needed
          //
          //          The product between the [3] and [1] then is
          //
          //                           (1,1)                      ---- x0
          //                   ________/   \_______
          //                  /                    \
          //                (2,3)                  (3,2)          ---- x1
          //                /   \_________ ________/   \
          //                |             X            |          //      (5,3) (7,3) (4,3) (6,4)
          //                \__ _________/ \__________ /          // min:   0     0     0     1
          //                ___X___                   X           // max:   1     3     0     2
          //               /       \            _____/ \          // coord: 2     3     1     4
          //              /         \          /        \
          //           (4,3)       (5,3)    (6,4)     (7,3)       ---- x2
          //           /   \       /   \    /   \     /   \
          //        (F,F) (T,T) (T,F)  |   /     \  (T,F) (F,T)
          //                           |  /       \
          //                           | /        |
          //                           |/         |
          //                         (8,T)      (T,5)             ---- x3
          //                         /   \      /   \
          //                      (F,T) (T,T) (T,F) (T,T)
          */

          __bdd out = bdd_xor(ep, bdd_3, bdd_1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,2)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (6,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (T,5)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 3), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 4u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(4u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(5u));
        });

        it("computes in different order than random access", [&]() {
          /*
          // Result of [canon] ^ [indexable]
          //
          //                         (1,1)                         ---- x0
          //                   ______/   \______
          //                  /                 \
          //              (2,2)                 (3,3)              ---- x1
          //             _/   \_               _/   \_
          //            /       \             /       \
          //        (4,4)       (6,5)     (5,6)       (7,4)        ---- x2
          //        /   \       /   \     /   \       /   \
          //    (F,T)    |     |     (F,F)     |  (F,T)   (T,F)
          //             /      \______ ______/
          //            /              X
          //           /              / \________
          //        (8,F)         (9,T)         (T,7)              ---- x3
          //        /   \         /   \         /   \
          //     (T,F) (F,F)   (F,T) (T,T)   (T,T) (T,F)
          */

          __bdd out = bdd_xor(ep, bdd_canon, bdd_indexable);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 3), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 4u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(5u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });
      });

      describe("bdd_xnor(f,g)", [&]() {
        // TODO
      });

      describe("bdd_imp(f,g)", [&]() {
        it("shortcuts on x0 -> x1", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests
          // after evaluating x0
          //
          //           1     ---- x0
          //          / \
          //          T 2    ---- x1
          //           / \
          //           F T
          */

          __bdd out = bdd_imp(ep, bdd_x0, bdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });
      });

      describe("bdd_invimp(f,g)", [&]() {
        // TODO
      });

      describe("bdd_equiv(f,g)", [&]() {
        // TODO
      });

      describe("bdd_diff(f,g)", [&]() {
        // TODO
      });

      describe("bdd_less(f,g)", [&]() {
        // TODO
      });
    });

    describe("access mode: random access", [&]() {
      // Set access mode to random access for this batch of tests
      const exec_policy ep = exec_policy::access::Random_Access;

      // Trivial canonical: bdd_F, bdd_T, bdd_x0, bdd_not_x0, bdd_x1, bdd_x2
      // Canonical:     bdd_2
      // Non-Canonical: bdd_1, bdd_3

      describe("bdd_and(f,g)", [&]() {
        it("computes x0 and !x0", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F F
          */

          __bdd out = bdd_and(ep, bdd_x0, bdd_not_x0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(0, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("computes x0 /\\ x2", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F 2    ---- x2
          //           / \
          //           F T
          */

          __bdd out = bdd_and(ep, bdd_x0, bdd_x2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes x0 /\\ x0(+1)", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F 2    ---- x1
          //           / \
          //           F T
          */

          __bdd out = bdd_and(ep, bdd(bdd_x0, false, +0), bdd(bdd_x0, false, +1));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes (and shortcut) [1] /\\ [2]", [&]() {
          /*
          //                    1                        ---- x0
          //                    X
          //                   / \
          //                  2   3                      ---- x1
          //                 / \ / \
          //                /   X   \
          //               /___/ \   \
          //              /      |    \
          //             4       5     6                 ---- x2
          //            / \     / \_ _/ \
          //            F 7     F   T   8                ---- x3
          //             / \           / \
          //             T F           F T
          */

          __bdd out = bdd_and(ep, bdd_1, bdd_2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(4u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });
      });

      describe("bdd_or(f,g)", [&]() {
        it("computes x0(+2) \\/ x2", [&]() {
          /*
          //           1     ---- x2
          //          / \
          //          F T
          */

          __bdd out = bdd_or(ep, bdd(bdd_x0, false, +2), bdd_x2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("shortcuts on x0 \\/ x2", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          | T
          //          |
          //          2      ---- x2
          //         / \
          //         F T
          */

          __bdd out = bdd_or(ep, bdd_x0, bdd_x2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("shortcuts on x2 \\/ x1", [&]() {
          /*
          //           1     ---- x1
          //          / \
          //          2 T    ---- x2
          //         / \
          //         F T
          */

          __bdd out = bdd_or(ep, bdd_x2, bdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("shortcuts on x0(+2) \\/ x1", [&]() {
          /*
          //           1     ---- x1
          //          / \
          //          2 T    ---- x2
          //         / \
          //         F T
          */

          __bdd out = bdd_or(ep, bdd(bdd_x0, false, +2), bdd(bdd_x1, false, +0));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes (and shortcuts) [1] \\/ [2](-1)", [&]() {
          /*
          //        _(1,1)_       ---- x0
          //       /       \
          //       |       T      ---- x1
          //       |
          //     (3,2)            ---- x2
          //      / \
          //      T T
          */
          __bdd out = bdd_or(ep, bdd(bdd_1, false, +0), bdd(bdd_2, false, -1));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());

          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });
      });

      describe("bdd_xor(f,g)", [&]() {
        it("computes x0 ^ !x0 (same level)", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          T T
          */

          __bdd out = bdd_xor(ep, bdd_x0, bdd_not_x0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes x0(+1) ^ x1 (same level)", [&]() {
          /*
          //           1     ---- x1
          //          / \
          //          F F
          */

          __bdd out = bdd_xor(ep, bdd(bdd_x0, false, +1), bdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("computes x0 ^ x1 (different levels, random access for first level)", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests after evaluating x0
          //
          //           1     ---- x0
          //          / \
          //         2   3   ---- x1
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd_x1, bdd_x0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes x0(+2) ^ x1 (different levels, random access for first level)", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests after evaluating x1
          //
          //           1     ---- x1
          //          / \
          //         2   3   ---- x2
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd_x1, bdd(bdd_x0, false, +2));

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
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes x1 ^ x0 (different levels, random access for second level)", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests
          //   after evaluating x0
          //
          //           1     ---- x0
          //          / \
          //         2   3   ---- x1
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd_x0, bdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(1, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes x2 ^ x1 (different levels, random access for second level)", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests after evaluating x1
          //
          //           1     ---- x1
          //          / \
          //         2   3   ---- x2
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd_x2, bdd_x1);

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
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes x0(+2) ^ x1 (different levels, random access for second level)", [&]() {
          /* The order on the leaves are due to the sorting of terminal requests after evaluating x1
          //
          //           1     ---- x1
          //          / \
          //         2   3   ---- x2
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd(bdd_x0, false, +2), bdd_x1);

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
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes x0 ^ x0(+2)", [&]() {
          /*
          //          _1_    ---- x0
          //         /   \
          //         2   3   ---- x2
          //        / \ / \
          //        F T T F
          */

          __bdd out = bdd_xor(ep, bdd_x0, bdd(bdd_x0, false, +2));

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("handles requests where scanning index is on a later level", [&]() {
          shared_levelized_file<bdd::node_type> bdd_ra;
          /*
          //         1       ---- x0
          //        / \
          //        F 2      ---- x1
          //         / \
          //         F 3     ---- x2
          //          / \
          //          F T
          */

          node nra_3 = node(2, bdd::max_id, terminal_F, terminal_T);
          node nra_2 = node(1, bdd::max_id, terminal_F, nra_3);
          node nra_1 = node(0, bdd::max_id, terminal_F, nra_2);

          { // Garbage collect early and free write-lock
            node_ofstream nw_ra(bdd_ra);
            nw_ra << nra_3 << nra_2 << nra_1;
          }

          // Since 'bdd_ra' is the thinnest, the arguments will be flipped to
          // have random access run on this one.
          //
          // If one always scans ahead to the requests scanning index (which is
          // reasonable to do due to the comparator), then one gets a bug in the
          // following edge case: a request (scan, ra) has only 'ra' be on the
          // current level. That is, if going to 'scan' in the input then you
          // will unintentionally jump over some nodes.
          /*
          //          (1,1)           ---- x0
          //          /   \
          //         /   (3,2)        ---- x1
          //        /     __X__
          //       /     /     \
          //     (2,F) (3,2)  (3,F)   ---- x2
          //      / \   / \    / \
          //      T F   F F    F T
          */

          __bdd out = bdd_xor(bdd_ra, bdd_0xnor2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(4u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("sorts requests first by scanning index then by random acces index", [&]() {
          shared_levelized_file<bdd::node_type> bdd_wide2;
          /*
          //           1          ---- x0
          //          / \
          //         2   3        ---- x1
          //        / \ / \
          //       /   X   \__
          //       |__/ \     \
          //       4     5     6  ---- x2
          //      / \   / \   / \
          //      F  7  T F   F T ---- x3
          //        / \
          //        F T
          */

          node nw_7 = node(3, bdd::max_id, terminal_F, terminal_T);
          node nw_6 = node(2, bdd::max_id, terminal_F, terminal_T);
          node nw_5 = node(2, bdd::max_id - 1, terminal_T, terminal_F);
          node nw_4 = node(2, bdd::max_id - 2, terminal_F, nw_7);
          node nw_3 = node(1, bdd::max_id, nw_4, nw_6);
          node nw_2 = node(1, bdd::max_id - 1, nw_4, nw_5);
          node nw_1 = node(0, bdd::max_id, nw_2, nw_3);

          { // Garbage collect early and free write-lock
            node_ofstream nw_w(bdd_wide2);
            nw_w << nw_7 << nw_6 << nw_5 << nw_4 << nw_3 << nw_2 << nw_1;
          }

          // bdd_wide->is_canonical() == true
          // bdd_wide->width == 3

          /*
          // Result of [0xnor2] ^ [wide2]
          //
          // Level x2 is not in the same order as pq, as (3,4) and (2,5) are in different order
          //
          //                       (1,1)               ---- x0
          //                      /     \
          //                    (2,2) (3,3)            ---- x1
          //                 ___/   \ /   \___
          //                /        X___     \
          //               /        /    \     \
          //           (2,4)     (3,4)   (2,5) (3,6)   ---- x2
          //           /   |     /   |    | |   | |
          //          T  (F,7)  F  (T,7)  F F   F F    ---- x3
          //              / \       / \
          //             F   T     T   F
          */

          __bdd out = bdd_xor(ep, bdd_0xnor2, bdd_wide2);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 3), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 4u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(7u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("uses random access on the thinnest ([thin] ^ [wide])", [&]() {
          /* Result of [thin] ^ [wide]
          //
          //                      (1,1)                   ---- x0
          //                      /   \
          //                  (2,2)   (3,3)               ---- x1
          //                    X       X
          //              _____/ \     / \_____
          //          (4,4)       (5,5)       (4,6)       ---- x2
          //          /   \       /   \       /   \
          //         T   (F,7)    T   T       T   T    ---- x3
          //             /   \
          //             F   T
          */

          __bdd out = bdd_xor(ep, bdd_thin, bdd_wide);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });

        it("uses random access on the thinnest ([wide] ^ [thin])", [&]() {
          /* Result of [wide] ^ [thin]
          //
          //                      (1,1)                   ---- x0
          //                      /   \
          //                  (2,2)   (3,3)               ---- x1
          //                    X       X
          //              _____/ \     / \_____
          //          (4,4)       (5,5)       (6,4)       ---- x2
          //          /   \       /   \       /   \
          //         T   (7,F)    T   T       T   T    ---- x3
          //             /   \
          //             F   T
          */

          __bdd out = bdd_xor(ep, bdd_wide, bdd_thin);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });

        it("uses random access on non-canonical but indexable ([canonical] ^ [indexable])", [&]() {
          __bdd out = bdd_xor(ep, bdd_canon, bdd_indexable);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (6,5)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (8,F)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (9,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (T,7)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, bdd::pointer_type(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 3), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 4u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(5u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });

        it("uses random access non-canonical but indexable ([indexable] ^ [canonical])", [&]() {
          __bdd out = bdd_xor(ep, bdd_indexable, bdd_canon);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (6,5)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,7)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (F,8)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (T,9)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, bdd::pointer_type(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 3), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 2), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 4u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(5u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });

        it("uses random access on indexable ([canonical] ^ [non-indexable])", [&]() {
          __bdd out = bdd_xor(ep, bdd_canon, bdd_unindexable);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (6,5)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (T,7)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (8,F)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (9,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), true, bdd::pointer_type(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 2), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 4u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(5u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });

        it("uses random access on indexable ([non-indexable] ^ [canonical])", [&]() {
          __bdd out = bdd_xor(ep, bdd_unindexable, bdd_canon);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,7)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (6,5)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (F,8)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (T,9)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), true, bdd::pointer_type(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 2), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 4u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(5u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });
      });

      describe("bdd_imp(f,g)", [&]() {
        it("uses random access on thinnest ([wide] -> [thin])", [&]() {
          /* Result of [wide] ^ [thin]
          //
          //                      (1,1)                   ---- x0
          //                      /   \
          //                  (2,2)   (3,3)               ---- x1
          //                    X       X
          //              _____/ \     / \_____
          //          (4,4)       (5,5)       (6,4)       ---- x2
          //          /   \       /   \       /   \
          //          T  (7,F)    F   T       T   F       ---- x3
          //             /   \
          //             T   F
          */
          __bdd out = bdd_imp(ep, bdd_wide, bdd_thin);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });

        it("uses random access on thinnest ([thin] -> [wide] : flips non-commutative operator)",
           [&]() {
             /* Result of [thin] -> [wide]
             //
             //                      (1,1)                   ---- x0
             //                      /   \
             //                  (2,2)   (3,3)               ---- x1
             //                    X       X
             //              _____/ \     / \_____
             //          (4,4)       (5,5)       (4,6)       ---- x2
             //          /   \       /   \       /   \
             //          F   T*      T   F       F   T
             //
             // * Shortcut due to '->' operator
             */
             __bdd out = bdd_imp(ep, bdd_thin, bdd_wide);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(2, 2) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 2), false, terminal_F }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2, 3u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(3u));
             AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(3u));
           });
      });
    });
  });
});
