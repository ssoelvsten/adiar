#include "../../test.h"

// TODO: Quite a few of the tests are not counting how many times the mapping function is used. This
//       should be fixed, especially to be able to "prove" the type is derived by an expected number
//       of calls.

go_bandit([]() {
  describe("adiar/bdd/replace.cpp", []() {
    using mapping_type = function<bdd::label_type(bdd::label_type)>;

    shared_levelized_file<bdd::node_type> bdd_F_nf;
    /*
    //        F
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(bdd_F_nf);
      nw << node(false);
    }
    const bdd bdd_F(bdd_F_nf);

    shared_levelized_file<bdd::node_type> bdd_T_nf;
    /*
    //        T
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(bdd_T_nf);
      nw << node(true);
    }
    const bdd bdd_T(bdd_T_nf);

    const bdd::pointer_type terminal_T = bdd::pointer_type(true);
    const bdd::pointer_type terminal_F = bdd::pointer_type(false);

    shared_levelized_file<bdd::node_type> bdd_x0_nf;
    /*
    //          1        ---- x0
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x0_nf);
      nw << node(0, bdd::max_id, terminal_F, terminal_T);
    }
    const bdd bdd_x0(bdd_x0_nf);

    shared_levelized_file<bdd::node_type> bdd_x1_nf;
    /*
    //          1        ---- x1
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x1_nf);
      nw << node(1, bdd::max_id, terminal_F, terminal_T);
    }
    const bdd bdd_x1(bdd_x1_nf);

    shared_levelized_file<bdd::node_type> bdd_x2_nf;
    /*
    //          1        ---- x2
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x2_nf);
      nw << node(2, bdd::max_id, terminal_F, terminal_T);
    }
    const bdd bdd_x2(bdd_x2_nf);

    shared_levelized_file<bdd::node_type> bdd_1_nf;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x2
    //       |/ \
    //       3  T      ---- x4
    //      / \
    //      F T
    */
    { // Garbage collect early and free write-lock
      const node n3 = node(4, bdd::max_id, terminal_F, terminal_T);
      const node n2 = node(2, bdd::max_id, n3.uid(), terminal_T);
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_1_nf);
      nw << n3 << n2 << n1;
    }
    const bdd bdd_1(bdd_1_nf);

    shared_levelized_file<bdd::node_type> bdd_1_ext_nf;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x2
    //       |/ \
    //       3  |      ---- x4
    //      / \ |
    //     4   5       ---- x5
    //    / \ / \
    //   F   T   F
    */
    { // Garbage collect early and free write-lock
      const node n5 = node(5, bdd::max_id, terminal_T, terminal_F);
      const node n4 = node(5, bdd::max_id - 1, terminal_F, terminal_T);
      const node n3 = node(4, bdd::max_id, n4, n5);
      const node n2 = node(2, bdd::max_id, n3.uid(), n5);
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_1_ext_nf);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_1_ext(bdd_1_ext_nf);

    shared_levelized_file<bdd::node_type> bdd_2_nf;
    /*
    // NOTE: This BDD is on-purpose not canonical (to check whether it has been run through the
    //       Reduce algorithm or not).
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     F T T F
    */

    { // Garbage collect early and free write-lock
      const node n3 = node(1, bdd::max_id, terminal_T, terminal_F);
      const node n2 = node(1, bdd::max_id - 2, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_2_nf);
      nw << n3 << n2 << n1;
    }
    const bdd bdd_2(bdd_2_nf);

    shared_levelized_file<bdd::node_type> bdd_3_nf;
    /*
    // NOTE: This BDD is on-purpose not canonical (to check whether it has been run through the
    //       Reduce algorithm or not)
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     | F F |
    //      \   /
    //       \ /
    //        4         ---- x2
    //       / \
    //       T F
    */

    { // Garbage collect early and free write-lock
      const node n4 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n3 = node(1, bdd::max_id, terminal_F, n4.uid());
      const node n2 = node(1, bdd::max_id - 1, n4.uid(), terminal_F);
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_3_nf);
      nw << n4 << n3 << n2 << n1;
    }
    const bdd bdd_3(bdd_3_nf);

    shared_levelized_file<bdd::node_type> bdd_4_nf;
    /*
    //
    //        1_       ---- x0
    //        | \
    //        2 |      ---- x1
    //       / \|
    //      F   T
    */

    { // Garbage collect early and free write-lock
      const node n2 = node(1, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_T);

      node_ofstream nw(bdd_4_nf);
      nw << n2 << n1;
    }
    const bdd bdd_4(bdd_4_nf);

    shared_levelized_file<bdd::node_type> bdd_5_nf;
    /*
    //
    //        __1__         ---- x0
    //       /     \
    //      2      3        ---- x1
    //     / \    / \
    //    F   4  F   5      ---- x3
    //       / \    / \
    //      F   T  T   F
    //
    */

    { // Garbage collect early and free write-lock
      const node n5 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n4 = node(3, bdd::max_id - 2, terminal_F, terminal_T);
      const node n3 = node(1, bdd::max_id, terminal_F, n5.uid());
      const node n2 = node(1, bdd::max_id - 1, terminal_F, n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_5_nf);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_5(bdd_5_nf);

    // Big tree from the apply tests
    shared_levelized_file<bdd::node_type> bdd_6_nf;
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

      node_ofstream nw(bdd_6_nf);
      nw << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_6(bdd_6_nf);

    shared_levelized_file<bdd::node_type> bdd_7_nf;
    /*
    //          1         ---- x0
    //        _/ \
    //       2    |       ---- x1
    //      / \   |
    //     F  |   3       ---- x2
    //         \ / \_
    //          4    5    ---- x3
    //         / \  / \
    //        F  T T   F
    */

    { // Garbage collect early and free write-lock
      const node n5 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n4 = node(3, bdd::max_id - 1, terminal_F, terminal_T);
      const node n3 = node(2, bdd::max_id, n4.uid(), n5.uid());
      const node n2 = node(1, bdd::max_id, terminal_F, n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_7_nf);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_7(bdd_7_nf);

    shared_levelized_file<bdd::node_type> bdd_8_nf;
    // purpose - minimal(ish) example highligting need to change nested sweeping terminal case
    //  sweeping on lvl 1 here -> request with 2 leaf children, should not be surpressed
    /*
    //          1         ---- x0
    //         / \
    //        /   2       ---- x1
    //       /    |\
    //      3     | \     ---- x2
    //     / \    |  \
    //    F   T   F   T
    */

    { // Garbage collect early and free write-lock
      const node n3 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n2 = node(1, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_8_nf);
      nw << n3 << n2 << n1;
    }
    const bdd bdd_8(bdd_8_nf);

    shared_levelized_file<bdd::node_type> bdd_9_nf;
    // This provides many levels to facilitate swaps that are not side by side.
    //
    // This also exposes the worst case if a bad order is used
    /*
    //  (x0 /\ x1) \/ (x2 /\ x3) \/ (x4 /\ x5) \/ (x6 /\ x7)
    //
    //             0
    //            / \
    //            1  \
    //           / \ |
    //           | T |
    //           \_ _/
    //             2
    //            / \
    //            3  \
    //           / \ |
    //           . . .
    //           . . .
    //           . . .
    */

    { // Garbage collect early and free write-lock
      const node n7 = node(7, bdd::max_id, terminal_F, terminal_T);
      const node n6 = node(6, bdd::max_id, terminal_F, n7.uid());
      const node n5 = node(5, bdd::max_id, n6.uid(), terminal_T);
      const node n4 = node(4, bdd::max_id, n6.uid(), n5.uid());
      const node n3 = node(3, bdd::max_id, n4.uid(), terminal_T);
      const node n2 = node(2, bdd::max_id, n4.uid(), n3.uid());
      const node n1 = node(1, bdd::max_id, n2.uid(), terminal_T);
      const node n0 = node(0, bdd::max_id, n2.uid(), n1.uid());

      node_ofstream nw(bdd_9_nf);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1 << n0;
    }
    const bdd bdd_9(bdd_9_nf);

    shared_levelized_file<bdd::node_type> bdd_10_nf;
    // purpose - has edge crossing 2 layers
    /*
    //       _1      ---- x0
    //      / |
    //     /  2      ---- x1
    //    /  / \
    //   /  3   4    ---- x2
    //   | / \ / \
    //   | F | F T
    //   \___|
    //       5       ---- x3
    //      / \
    //      F T
    */

    { // Garbage collect early and free write-lock
      const node n5 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n4 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n3 = node(2, bdd::max_id - 1, terminal_F, n5.uid());
      const node n2 = node(1, bdd::max_id, n3.uid(), n4.uid());
      const node n1 = node(0, bdd::max_id, n5.uid(), n2.uid());

      node_ofstream nw(bdd_10_nf);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_10(bdd_10_nf);

    /* example stolen from rel-prod - has many levels
    //
    //                 1           ---- x0 (x)
    //                / \
    //                2  \         ---- x1 (0)
    //               / \  \
    //               3 |   \       ---- x2 (1)
    //              / \|   |
    //              |  4   5       ---- x3 (0')
    //              \ / \ / \
    //               X  | | F
    //              / \_|_/
    //              F   6          ---- x4 (1')
    //                 / \
    //                 T F
    */
    shared_levelized_file<bdd::node_type> bryant_fig18;
    { // Garbage collect early and free write-lock
      const node n6 = node(4, bdd::max_id, terminal_T, terminal_F);
      const node n5 = node(3, bdd::max_id, n6.uid(), terminal_F);
      const node n4 = node(3, bdd::max_id - 1, terminal_F, n6.uid());
      const node n3 = node(2, bdd::max_id, n6.uid(), n4.uid());
      const node n2 = node(1, bdd::max_id, n3.uid(), n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n5.uid());

      node_ofstream nw(bryant_fig18);
      nw << n6 << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_b18(bryant_fig18);

    describe("bdd_replace(const bdd&, <...>, replace_type)", [&]() {
      it("returns the original file for 'F' [x -> 4-x]", [&]() {
        const mapping_type m = [](const int x) { return 4 - x; };
        const bdd out        = bdd_replace(exec_policy(), bdd_F, m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'F' [x -> 2x+1]", [&]() {
        const mapping_type m = [](const int x) { return 2 * x + 1; };
        const bdd out        = bdd_replace(bdd_F, m, replace_type::Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'F' [x -> x+1]", [&]() {
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out        = bdd_replace(bdd_F, m, replace_type::Shift);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("preserves negation flag when returning original file for 'F'", [&]() {
        const mapping_type m = [](const int x) { return x * x; };
        const bdd out        = bdd_replace(bdd_not(bdd_F), m, replace_type::Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().True());
      });

      it("returns the original file for 'T' [x -> 4-x]", [&]() {
        const mapping_type m = [](const int x) { return 4 - x; };
        const bdd out        = bdd_replace(bdd_T, m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'T' [x -> x+42]", [&]() {
        const mapping_type m = [](const int x) { return x + 42; };
        const bdd out        = bdd_replace(bdd_T, m, replace_type::Shift);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'T' [x -> x]", [&]() {
        const mapping_type m = [](const int x) { return x; };
        const bdd out        = bdd_replace(bdd_T, m, replace_type::Identity);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("preserves negation flag when returning original file for 'T'", [&]() {
        const mapping_type m = [](const int x) { return x * x; };
        const bdd out        = bdd_replace(bdd_not(bdd_T), m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().True());
      });

      describe("<jump-down>", [&]() {
        it("jumps down with node and leaf children", [&]() {
          /*
          //        1        ---- x2
          //       / \
          //       | 2       ---- x0 !
          //       |/ \
          //       3  T      ---- x4
          //      / \
          //      F T
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 3;
            return x;
          };
          __bdd res = bdd_replace(bdd_1, m, replace_type::Jump_Down);

          arc_test_ifstream out_arcs(res);

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::uid_type(3, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::uid_type(4, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::uid_type(4, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().False());

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(4, 0), false, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(4, 0), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().False());
        });

        it("jumps down with subtree children", [&]() {
          /*
          //        1        ---- x2
          //       / \
          //       | 2       ---- x0...3?
          //       |/ \
          //       3  |      ---- x4
          //      / \ /
          //     4   5       ---- x5
          //    / \ / \
          //    F T T F
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 3;
            return x;
          };

          __bdd res = bdd_replace(bdd_1_ext, m, replace_type::Jump_Down);

          arc_test_ifstream out_arcs(res);

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::uid_type(3, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::uid_type(4, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::uid_type(4, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(4, 0), false, bdd::pointer_type(5, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::pointer_type(5, 1) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(4, 0), true, bdd::pointer_type(5, 1) }));

          AssertThat(out_arcs.can_pull_internal(), Is().False());

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 0), false, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 0), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 1), false, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 1), true, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().False());
        });

        it("jumps down for 2 nodes in same mapping", [&]() {
          /*
          //        1        ---- x2
          //       / \
          //      /  2       ---- x0 !
          //      |_/ \_
          //      3     4    ---- x5
          //     / \   / \
          //    5   6  T F   ---- x4 !
          //   / \ / \
          //   F T T F
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 3;
            if (x == 4) return 6;
            return x;
          };

          __bdd res = bdd_replace(bdd_1_ext, m, replace_type::Jump_Down);

          arc_test_ifstream out_arcs(res);

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::uid_type(3, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::uid_type(5, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, bdd::uid_type(5, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, bdd::uid_type(5, 1) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 0), false, bdd::pointer_type(6, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 0), true, bdd::pointer_type(6, 1) }));

          AssertThat(out_arcs.can_pull_internal(), Is().False());

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 1), false, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 1), true, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(6, 0), false, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(6, 0), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(6, 1), false, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(6, 1), true, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().False());
        });

        it("jumps the root down to the bottom layer", [&]() {
          /*
          //        _1_         ---- x2
          //       /   \
          //      2     3       ---- x4
          //     / \   / \
          //    /  T  4  T      ---- x0 !
          //    |    / \
          //    F   F   T
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 5;
            else return x;
          };

          __bdd res = bdd_replace(bdd_1, m, replace_type::Jump_Down);

          arc_test_ifstream out_arcs(res);

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(4, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(4, 1) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(4, 1), false, bdd::pointer_type(5, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().False());

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(4, 0), false, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(4, 0), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(4, 1), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 0), false, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(5, 0), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().False());
        });

        it("jumps that move through a double layer", [&]() {
          /*
          //
          //          __1__        ---- x1
          //         /     \
          //        2      3       ---- x2
          //       / \    / \
          //      /  F   /  F
          //      4      5         ---- x0 !
          //     / \    / \
          //    T  F   F   T
          //
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 3;
            else return x;
          };

          __bdd res = bdd_replace(bdd_3, m, replace_type::Jump_Down);

          arc_test_ifstream out_arcs(res);

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(3, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 1) }));

          AssertThat(out_arcs.can_pull_internal(), Is().False());

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().False());
        });

        it("jumps down and has two children", [&]() {
          /*
          //
          //        __1__            ---- x1
          //       /     \
          //      F    __3__         ---- x0 !
          //          /     \
          //         4       5       ---- x3
          //        / \     / \
          //       F   T   T   F
          //
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 2;
            else return x;
          };

          __bdd res = bdd_replace(bdd_5, m, replace_type::Jump_Down);

          arc_test_ifstream out_arcs(res);

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(out_arcs.can_pull_internal(), Is().True());
          AssertThat(out_arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, bdd::pointer_type(3, 1) }));

          AssertThat(out_arcs.can_pull_internal(), Is().False());

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_T }));

          AssertThat(out_arcs.can_pull_terminal(), Is().True());
          AssertThat(out_arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_F }));

          AssertThat(out_arcs.can_pull_terminal(), Is().False());
        });

        // TODO: More tests, in particular ones where the target level is not empty? Or, is that
        //       covered with the <adjacent swap> below?
      });

      describe("<adjacent swap>", [&]() {
        // TODO: Turn these into arc-based tests

        it("swaps top in 'BDD 4'", [&]() {
          /*
          //
          //         1       ---- x1 !
          //        / \
          //        2 |      ---- x0 !
          //       / \|
          //       F  T
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 1;
            if (x == 1) return 0;
            return x;
          };

          __bdd res = bdd_replace(bdd_4, m, replace_type::Adjacent_Swap);

          arc_test_ifstream arcs(res);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(res);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(res.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("swaps top in 'BDD 6'", [&]() {
          /*
          //            1           ---- x1 !
          //           / \
          //           2 3          ---- x0 !
          //         _/ X \_
          //        | _/ \_ |
          //        _X     X_
          //       /  \   /  \
          //      4   5  6    7     ---- x2
          //     / \ / \/ \  / \
          //     T F T 8  T  F T    ---- x3
          //          / \
          //          F T
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 1;
            if (x == 1) return 0;
            else return x;
          };

          __bdd res = bdd_replace(bdd_6, m, replace_type::Adjacent_Swap);

          arc_test_ifstream arcs(res);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 3), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(res);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 4u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(res.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(4u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(5u));
        });

        it("swaps bottom levels in 'BDD 3'", [&]() {
          // map swapping levels 1 and 2
          /*
          //       _1_       ---- x0
          //      /   \
          //      2   3      ---- x2 !
          //     / \ / \
          //    4  F 5 F     ---- x1 !
          //   / \  / \
          //   T F  F T
          */
          const mapping_type m = [](const int x) {
            if (x == 1) return 2;
            if (x == 2) return 1;
            else return x;
          };

          __bdd res = bdd_replace(bdd_3, m, replace_type::Adjacent_Swap);

          arc_test_ifstream arcs(res);

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

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(res);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(res.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(4u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("handles arcs that span across the swap in 'BDD 10'", [&]() {
          /*
          //            1           ---- x0
          //           / \
          //          /   2         ---- x2 !
          //         /   / \
          //        /   3   4       ---- x1 !
          //       /   / \ / \
          //       |   F T | F
          //       5_______/        ---- x3
          //      / \
          //      F T
          */

          const mapping_type m = [](const int x) {
            if (x == 1) return 2;
            if (x == 2) return 1;
            return x;
          };

          __bdd res = bdd_replace(bdd_10, m, replace_type::Adjacent_Swap);

          arc_test_ifstream arcs(res);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(res);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(res.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(3u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("handles two swaps with no in-between levels in 'BDD 6'", [&]() {
          /*
          //           _1_        ---- x1 !
          //          /   \
          //         _2   3_      ---- x0 !
          //        /  \ /  \
          //        4  | |  5     ---- x3 !
          //       / \ | | / \
          //       | T | | | T
          //       \_ _/ \ /
          //         6    7       ---- x2 !
          //        / \  / \
          //        T F  F T
          */
          const mapping_type m = [](const int x) {
            if (x == 0) return 1;
            if (x == 1) return 0;
            if (x == 2) return 3;
            if (x == 3) return 2;
            else return x;
          };

          __bdd res = bdd_replace(bdd_6, m, replace_type::Adjacent_Swap);

          arc_test_ifstream arcs(res);

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
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), true, bdd::pointer_type(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), false, bdd::pointer_type(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(1, 1), false, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), false, bdd::pointer_type(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ bdd::uid_type(3, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(res);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(res.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(res.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });

        /*
        it("handles two swaps with in-between levels in 'BDD 9'", [&]() {
          const mapping_type m = [](const int x) {
            if (x == 1) return 2;
            if (x == 2) return 1;
            if (x == 5) return 6;
            if (x == 6) return 5;
            return x;
          };

          __bdd out = bdd_replace(bdd_9, m, replace_type::Adjacent_Swap);

          // TODO
        });
        */
      });

      describe("<jump-up>", [&]() {
        // TODO
      });

      describe("<non-monotonic> [nested sweeping only]", [&]() {
        // TODO
      });

      describe("<non-monotonic> [with pre-processing]", [&]() {
        // TODO
      });

      describe("<monotonic>", [&]() {
        // NOTE: To future-proof these tests against the introduction of constant time 'Affine' or
        //       'Shift' variable replacement, we test with quadratic variable replacement.
        it("squares all variables in 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_1, m, replace_type::Monotone);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_1->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_1->indexable));

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(16, bdd::max_id, terminal_F, terminal_T)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(16, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(16, bdd::max_id),
                                       bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(16, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_1->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_1->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_1->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_1->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_1->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_1->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_1->number_of_terminals[true]));
        });

        it("bakes negation into output when squaring variables in 'BDD 3'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_not(bdd_3), m, replace_type::Monotone);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_3->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_3->indexable));

          node_test_ifstream out_nodes(out);

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_T, bdd::pointer_type(4, bdd::max_id))));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id - 1, bdd::pointer_type(4, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(1, bdd::max_id - 1),
                                       bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_3->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_3->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_3->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_3->number_of_terminals[true]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_3->number_of_terminals[false]));
        });
      });

      describe("<affine>", [&]() {
        it("doubles variables in 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return 2 * x; };
          const bdd out        = bdd_replace(bdd_2, m, replace_type::Monotone);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id - 2, terminal_F, terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(2, bdd::max_id - 2),
                                       bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_2->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_2->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_2->number_of_terminals[true]));
        });
      });

      describe("<shift>", [&]() {
        it("shifts variables in 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x + 1; };
          const bdd out        = bdd_replace(bdd_1, m, replace_type::Shift);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_1_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(1));
        });

        it("shifts variables in 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return x + 4; };
          const bdd out        = bdd_replace(bdd_2, m, replace_type::Shift);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_2_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(4));
        });

        it("shifts variables in 'BDD 3' multiple times [+3, +3]", [&]() {
          const mapping_type m = [](const int x) { return x + 3; };
          const bdd out        = bdd_replace(bdd_replace(bdd_3, m), m, replace_type::Shift);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_3_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(6));
        });

        it("shifts variables in 'BDD 1' multiple times [+2, -1]", [&]() {
          const bdd out = bdd_replace(
            bdd_replace(
              bdd_1, [](const int x) { return x + 2; }, replace_type::Shift),
            [](const int x) { return x - 1; },
            replace_type::Shift);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_1_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(1));
        });
      });

      describe("<identity>", [&]() {
        it("returns the original file for 'x0'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(bdd_x0, m, replace_type::Identity);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'x1'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(bdd_x1, m, replace_type::Identity);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x1_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(bdd_1, m, replace_type::Identity);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_1_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(bdd_2, m, replace_type::Identity);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_2_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("preserves negation flag when returning the original file for 'BDD 3'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(bdd_not(bdd_3), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_3_nf));
          AssertThat(out.is_negated(), Is().True());
        });
      });
    });

    describe("bdd_replace(const bdd&, <...>)", [&]() {
      // TODO: test inference of 'replace_type'

      it("returns the original file for 'F' [x -> x*x]", [&]() {
        const mapping_type m = [](const int x) { return x * x; };
        const bdd out        = bdd_replace(bdd_F, m);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("preserves negation flag when returning original file for 'F'", [&]() {
        const mapping_type m = [](const int x) { return x + 42; };
        const bdd out        = bdd_replace(bdd_not(bdd_F), m);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().True());
      });

      it("returns the original file for 'T' [x -> x*x]", [&]() {
        const mapping_type m = [](const int x) { return x; };
        const bdd out        = bdd_replace(bdd_T, m);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("preserves negation flag when returning original file for 'T'", [&]() {
        const mapping_type m = [](const int x) { return 4 - x; };
        const bdd out        = bdd_replace(bdd_not(bdd_T), m);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().True());
      });

      describe("<non-monotone>", [&]() {
        // TODO
      });

      describe("<jump-up>", [&]() {
        // TODO
      });

      describe("<jump-down> / <adjacent swap>", [&]() {
        // TODO
      });

      describe("<monotone>", [&]() {
        // TODO
      });

      describe("<affine>", [&]() {
        it("2x+1 variables in 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return 2 * x + 1; };
          const bdd out        = bdd_replace(bdd_2, m);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id - 2, terminal_F, terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       bdd::max_id,
                                       bdd::pointer_type(3, bdd::max_id - 2),
                                       bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_2->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_2->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_2->number_of_terminals[true]));
        });
      });

      describe("<shift>", [&]() {
        it("shifts variables in 'BDD 2'", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&](const int x) {
            m_calls++;
            return x + 2;
          };
          const bdd out = bdd_replace(bdd_2, m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_2_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(2));

          // Check if the function is only called for type inference
          //
          // 1. For inferring the type
          // 2. To infer the shift
          AssertThat(m_calls, Is().EqualTo(3));
        });

        it("shifts variables in 'BDD 2' multiple times [+2, +2]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&](const int x) {
            m_calls++;
            return x + 2;
          };
          const bdd out = bdd_replace(bdd_replace(bdd_2, m), m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_2_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(4));

          // Check if the function is only called for type inference
          //
          // 1. For inferring the type
          // 2. To infer the shift
          //
          // Twice!
          AssertThat(m_calls, Is().EqualTo(6));
        });

        it("identifies 2x+1 as a mere shift for 'x0'", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&](const int x) {
            m_calls++;
            return 2 * x + 1;
          };
          const bdd out = bdd_replace(bdd_x0, m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(1));

          // Check if the function is only called for type inference
          //
          // 1. For inferring the type
          // 2. To infer the shift
          AssertThat(m_calls, Is().EqualTo(2));
        });

        it("identifies 4-x as a mere shift on 'x0'", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return 4 - x;
          };
          const bdd out = bdd_replace(bdd_x0, m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(4));

          // Check if the function is only called for type inference
          //
          // 1. For inferring the type
          // 2. To infer the shift
          AssertThat(m_calls, Is().EqualTo(2));
        });
      });

      describe("<identity>", [&]() {
        it("returns the original file for 'BDD 4'", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x;
          };
          const bdd out = bdd_replace(bdd_4, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_4_nf));
          AssertThat(out.is_negated(), Is().False());

          // Check if the function is only called for type inference
        });

        it("preserves negation flag when returning the original file for 'BDD 5'", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x;
          };
          const bdd out = bdd_replace(bdd_not(bdd_5), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_5_nf));
          AssertThat(out.is_negated(), Is().True());

          // Check if the function is only called for type inference
        });
      });
    });

    shared_levelized_file<arc> __bdd_x0;
    /*
    //          1      ---- x0
    //         / \
    //         F T
    */
    {
      const bdd::uid_type n1(0, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_x0);

      aw.push_terminal({ n1, false, terminal_F });
      aw.push_terminal({ n1, true, terminal_T });

      aw.push(level_info(0, 1u));

      __bdd_x0->max_1level_cut = 0;
    }

    shared_levelized_file<arc> __bdd_x0_unreduced;
    /*
    // NOTE: Due to the reduction rules, this does not need Nested Sweeping to swap its levels.
    //       Yet, one cannot know this before having done the computation.
    //
    //            1      ---- x0
    //           / \
    //           F 2     ---- x2
    //            / \
    //            T T
    */
    {
      const bdd::uid_type n1(0, 0);
      const bdd::uid_type n2(2, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_x0_unreduced);

      aw.push_internal({ n1, true, n2 });

      aw.push_terminal({ n1, false, terminal_F });
      aw.push_terminal({ n2, false, terminal_T });
      aw.push_terminal({ n2, true, terminal_T });

      aw.push(level_info(0, 1u));
      aw.push(level_info(2, 1u));

      __bdd_x0_unreduced->max_1level_cut = 1;
    }

    shared_levelized_file<arc> __bdd_1;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x2
    //       |/ \
    //       3  T      ---- x4
    //      / \
    //      F T
    */
    {
      const bdd::uid_type n1(0, 0);
      const bdd::uid_type n2(2, 0);
      const bdd::uid_type n3(4, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_1);

      aw.push_internal({ n1, true, n2 });
      aw.push_internal({ n1, false, n3 });
      aw.push_internal({ n2, false, n3 });

      aw.push_terminal({ n2, true, terminal_T });
      aw.push_terminal({ n3, false, terminal_F });
      aw.push_terminal({ n3, true, terminal_T });

      aw.push(level_info(0, 1u));
      aw.push(level_info(2, 1u));
      aw.push(level_info(4, 1u));

      __bdd_1->max_1level_cut = 2;
    }

    shared_levelized_file<arc> __bdd_2;
    /*
    // NOTE: When reduced, the nodes (2) and (3) are swapped to make it canonical.
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     F T T F
    */
    {
      const bdd::uid_type n1(0, 0);
      const bdd::uid_type n2(1, 0);
      const bdd::uid_type n3(1, 1);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_2);

      aw.push_internal({ n1, false, n2 });
      aw.push_internal({ n1, true, n3 });

      aw.push_terminal({ n2, false, terminal_F });
      aw.push_terminal({ n2, true, terminal_T });
      aw.push_terminal({ n3, false, terminal_T });
      aw.push_terminal({ n3, true, terminal_F });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 2u));

      __bdd_2->max_1level_cut = 2;
    }

    shared_levelized_file<arc> __bdd_3;
    /*
    // NOTE: This BDD is on-purpose not canonical (to check whether it has been run through the
    //       Reduce algorithm or not)
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     | F F |
    //      \   /
    //       \ /
    //        4         ---- x2
    //       / \
    //       T F
    */
    {
      const bdd::uid_type n1(0, 0);
      const bdd::uid_type n2(1, 0);
      const bdd::uid_type n3(1, 1);
      const bdd::uid_type n4(2, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_3);

      aw.push_internal({ n1, false, n2 });
      aw.push_internal({ n1, true, n3 });
      aw.push_internal({ n2, false, n4 });
      aw.push_internal({ n3, true, n4 });

      aw.push_terminal({ n2, true, terminal_F });
      aw.push_terminal({ n3, false, terminal_F });
      aw.push_terminal({ n4, false, terminal_T });
      aw.push_terminal({ n4, true, terminal_F });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 2u));
      aw.push(level_info(2, 1u));

      __bdd_3->max_1level_cut = 2;
    }

    shared_levelized_file<arc> __bdd_3_unreduced;
    /*
    // NOTE: A more extreme version of '__bdd_3' with duplicate and redundant nodes.
    //
    //       __1__        ---- x0
    //      /     \
    //     _2_   _3_      ---- x1
    //    /   \ /   \
    //    4    5    6     ---- x2
    //   / \   ||  / \
    //   T F   F   T F
    */
    {
      const bdd::uid_type n6(2, 2);
      const bdd::uid_type n5(2, 1);
      const bdd::uid_type n4(2, 0);
      const bdd::uid_type n3(1, 1);
      const bdd::uid_type n2(1, 0);
      const bdd::uid_type n1(0, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_3_unreduced);

      aw.push_internal({ n1, false, n2 });
      aw.push_internal({ n1, true, n3 });
      aw.push_internal({ n2, false, n4 });
      aw.push_internal({ n2, true, n5 });
      aw.push_internal({ n3, false, n5 });
      aw.push_internal({ n3, true, n6 });

      aw.push_terminal({ n4, false, terminal_T });
      aw.push_terminal({ n4, true, terminal_F });
      aw.push_terminal({ n5, false, terminal_F });
      aw.push_terminal({ n5, true, terminal_F });
      aw.push_terminal({ n6, false, terminal_T });
      aw.push_terminal({ n6, true, terminal_F });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 2u));
      aw.push(level_info(2, 3u));

      __bdd_3_unreduced->max_1level_cut = 4;
    }

    describe("bdd_replace(__bdd&&, <...>, replace_type)", [&]() {
      it("returns the original file for 'F' [x -> 4-x]", [&]() {
        const mapping_type m = [](const int x) { return 4 - x; };
        const bdd out = bdd_replace(exec_policy(), __bdd(bdd_F), m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'F' [x -> 2x+1]", [&]() {
        const mapping_type m = [](const int x) { return 2 * x + 1; };
        const bdd out        = bdd_replace(__bdd(bdd_F), m, replace_type::Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("preserves negation flag when returning original file for 'F'", [&]() {
        const mapping_type m = [](const int x) { return 42 - x; };
        const bdd out        = bdd_replace(__bdd(bdd_not(bdd_F)), m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().True());
      });

      it("returns the original file for 'T' [x -> x+42]", [&]() {
        const mapping_type m = [](const int x) { return x + 42; };
        const bdd out        = bdd_replace(__bdd(bdd_T), m, replace_type::Shift);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'T' [x -> x]", [&]() {
        const mapping_type m = [](const int x) { return x; };
        const bdd out        = bdd_replace(__bdd(bdd_T), m, replace_type::Identity);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("preserves negation flag when returning original file for 'T'", [&]() {
        const mapping_type m = [](const int x) { return x * x; };
        const bdd out        = bdd_replace(__bdd(bdd_not(bdd_T)), m, replace_type::Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().True());
      });

      describe("<jump-down>", [&]() {
        // TODO: Test this is done on the reduced input
      });

      describe("<adjacent-swap>", [&]() {
        // TODO: Test this is done on the reduced input
      });

      describe("<jump-up>", [&]() {
        // TODO
      });

      describe("<non-monotone> [nested sweeping only]", [&]() {
        // TODO
      });

      describe("<non-monotone> [with pre-processing]", [&]() {
        // TODO
      });

      describe("<monotone>", [&]() {
        it("squares variables in ~'BDD 3'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_not(bdd_3), m);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_3->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_3->indexable));

          node_test_ifstream out_nodes(out);

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_T, bdd::pointer_type(4, bdd::max_id))));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id - 1, bdd::pointer_type(4, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(1, bdd::max_id - 1),
                                       bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_3->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_3->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_3->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_3->number_of_terminals[true]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_3->number_of_terminals[false]));
        });

        it("reduces and squares variables [__bdd_3_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x * x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m);

          // Check first called to identify type and then called once per level
          AssertThat(m_calls, Is().EqualTo(3 + 3));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n4 / n6
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id - 1, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(1, bdd::max_id),
                                       bdd::pointer_type(1, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          // Over-approximation, since (5) is removed?
          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(4u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("<affine>", [&]() {
        it("doubles variables in 'BDD 2' [bdd_2]", [&]() {
          const mapping_type m = [](const int x) { return 2 * x; };
          const bdd out        = bdd_replace(__bdd(bdd_2_nf), m, replace_type::Monotone);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id - 2, terminal_F, terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(2, bdd::max_id - 2),
                                       bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_2->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_2->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_2->number_of_terminals[true]));
        });

        it("reduces and doubles variables in 'BDD 2' [__bdd_2]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return 2 * x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_2, exec_policy()), m, replace_type::Monotone);

          // Check only called once per level
          AssertThat(m_calls, Is().EqualTo(2));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id - 1, terminal_T, terminal_F)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(2, bdd::max_id),
                                       bdd::pointer_type(2, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("reduces and offsets a single level [__bdd_2]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return 2 * x + 1;
          };
          const bdd out = bdd_replace(__bdd(__bdd_2, exec_policy()), m, replace_type::Monotone);

          // Check only called once per level
          AssertThat(m_calls, Is().EqualTo(2));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id - 1, terminal_T, terminal_F)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       bdd::max_id,
                                       bdd::pointer_type(3, bdd::max_id),
                                       bdd::pointer_type(3, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });
      });

      describe("<shift>", [&]() {
        it("returns shifted original file [bdd_x0]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x + 1;
          };
          const bdd out = bdd_replace(__bdd(bdd_x0), m, replace_type::Shift);

          // Check only called once to obtain the amount to shift
          AssertThat(m_calls, Is().EqualTo(1));

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(1));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("reduces and shifts variables [__bdd_x0]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x + 1;
          };
          const bdd out = bdd_replace(__bdd(__bdd_x0, exec_policy()), m, replace_type::Shift);

          // Check only called once to obtain the amount to shift
          AssertThat(m_calls, Is().EqualTo(1));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("reduces and shifts variables [__bdd_x0_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x + 1;
          };
          const bdd out =
            bdd_replace(__bdd(__bdd_x0_unreduced, exec_policy()), m, replace_type::Shift);

          // Check only called once per level
          AssertThat(m_calls, Is().EqualTo(2));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("<identity>", [&]() {
        it("reduces 'x0' [__bdd_x0]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_x0, exec_policy()), m, replace_type::Identity);

          // Check it is never called
          AssertThat(m_calls, Is().EqualTo(0));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("reduces 'x0' [__bdd_x0_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x;
          };
          const bdd out =
            bdd_replace(__bdd(__bdd_x0_unreduced, exec_policy()), m, replace_type::Identity);

          // Check it is never called
          AssertThat(m_calls, Is().EqualTo(0));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });
    });

    describe("bdd_replace(__bdd&&, <...>)", [&]() {
      // TODO: test inference of 'replace_type'

      it("returns the original file for 'F' [x -> x+1]", [&]() {
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out        = bdd_replace(__bdd(bdd_F), m);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("preserves negation flag when returning original file for 'F'", [&]() {
        const mapping_type m = [](const int x) { return 42 - x; };
        const bdd out        = bdd_replace(__bdd(bdd_not(bdd_F)), m);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().True());
      });

      it("returns the original file for 'T' [x -> 2x+2]", [&]() {
        const mapping_type m = [](const int x) { return 2 * x + 42; };
        const bdd out        = bdd_replace(__bdd(bdd_T), m);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("preserves negation flag when returning original file for 'T'", [&]() {
        const mapping_type m = [](const int x) { return x * x; };
        const bdd out        = bdd_replace(__bdd(bdd_not(bdd_T)), m);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().True());
      });

      describe("<non-monotone>", [&]() {
        // TODO
      });

      describe("<jump-up>", [&]() {
        // TODO
      });

      describe("<jump-down> / <adjacent swap>", [&]() {
        // TODO
      });

      describe("<monotone>", [&]() {
        it("squares variables in ~'BDD 5'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(__bdd(bdd_not(bdd_5)), m);

          // Check it looks all right
          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().EqualTo(bdd_5->indexable));

          node_test_ifstream out_nodes(out);

          // n5
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(9, bdd::max_id, terminal_F, terminal_T)));

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(9, bdd::max_id - 2, terminal_T, terminal_F)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_T, bdd::pointer_type(9, bdd::max_id))));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       1, bdd::max_id - 1, terminal_T, bdd::pointer_type(9, bdd::max_id - 2))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(1, bdd::max_id - 1),
                                       bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(9, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_5->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_5->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_5->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_5->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_5->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_5->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_5->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_5->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_5->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_5->number_of_terminals[true]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_5->number_of_terminals[false]));
        });

        it("reduces and squares variable [__bdd_1]", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(__bdd(__bdd_1, exec_policy()), m);

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(16, bdd::max_id, terminal_F, terminal_T)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(16, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(16, bdd::max_id),
                                       bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(16, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_1->width));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });
      });

      describe("<affine>", [&]() {
        it("2x+1 variables in 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return 2 * x + 1; };
          const bdd out        = bdd_replace(__bdd(bdd_2), m);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, bdd::max_id - 2, terminal_F, terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       bdd::max_id,
                                       bdd::pointer_type(3, bdd::max_id - 2),
                                       bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_2->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_2->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_2->number_of_terminals[true]));
        });

        it("reduces and 2x+1 variables [__bdd_3_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return 2 * x + 1;
          };
          const bdd out = bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m);

          // Check only called once to determine type of mapping and then for each level
          AssertThat(m_calls, Is().EqualTo(3 + 3));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n4 / n6
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id - 1, terminal_F, bdd::pointer_type(5, bdd::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       bdd::max_id,
                                       bdd::pointer_type(3, bdd::max_id),
                                       bdd::pointer_type(3, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          // Over-approximation, since (5) is removed?
          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(4u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("<shift>", [&]() {
        it("identifies 4-x as a mere shift on reduced 'x0'", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(exec_policy(), __bdd(bdd_x0_nf), m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(4));
        });

        it("preserves negation flag shifting for 'x0'", [&]() {
          const mapping_type m = [](const int x) { return x + 8; };
          const bdd out        = bdd_replace(__bdd(bdd_not(bdd_x0)), m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().True());
          AssertThat(out.shift(), Is().EqualTo(8));
        });

        it("reduces and shifts +42 variables [__bdd_3_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x + 42;
          };
          const bdd out = bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m);

          // Check only called once to determine type of mapping and then for each level
          AssertThat(m_calls, Is().EqualTo(3 + 3));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n4 / n6
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(44, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(43, bdd::max_id, bdd::pointer_type(44, bdd::max_id), terminal_F)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(43, bdd::max_id - 1, terminal_F, bdd::pointer_type(44, bdd::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(42,
                                       bdd::max_id,
                                       bdd::pointer_type(43, bdd::max_id),
                                       bdd::pointer_type(43, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(44, 1u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(43, 2u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(42, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          // Over-approximation, since (5) is removed?
          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(4u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("<identity>", [&]() {
        it("returns the original file on reduced 'x0'", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&](const int x) {
            m_calls++;
            return x;
          };
          const bdd out = bdd_replace(exec_policy(), __bdd(bdd_x0_nf), m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(0));
        });

        it("reduces 'BDD 3'", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m);

          // Check only called once (to determine type of mapping)
          AssertThat(m_calls, Is().EqualTo(3));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n4 / n6
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id - 1, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(1, bdd::max_id),
                                       bdd::pointer_type(1, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          // Over-approximation, since (5) is removed?
          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(4u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });
    });
  });
});
