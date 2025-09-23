#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/relprod.cpp", []() {
    const bdd::pointer_type terminal_T = bdd::pointer_type(true);
    const bdd::pointer_type terminal_F = bdd::pointer_type(false);

    // ------------------------------------------------------------------------------------------ //

    // The preliminaries of Kalin's & Dahlsen-Jensen's BSc Thesis, "Symbolic Algorithms for
    // Computing Strongly Connected Components", includes a presentation of BDDs on a simple graph
    // of four nodes.
    /*
    // Figure 6:
    //                  _________
    //                 /         v
    //    {00} ---> {01}        {10} ---> {11}
    //                ^_________/
    */

    // In the thesis, they use a separated 'pre' and 'post' variable ordering:
    const auto kalin_relnext_pred = [](int x) -> bool { return x < 2; };

    const auto kalin_relnext_map = [&kalin_relnext_pred](int x) -> optional<int> {
      return kalin_relnext_pred(x) ? make_optional<int>() : make_optional<int>(x - 2);
    };

    const auto kalin_relprev_pred = [](int x) -> bool { return 1 < x; };

    const auto kalin_relprev_map = [&kalin_relprev_pred](int x) -> optional<int> {
      return kalin_relprev_pred(x) ? make_optional<int>() : make_optional<int>(x + 2);
    };

    shared_levelized_file<bdd::node_type> kalin_fig7_a;
    /*
    // Figure 7 (left) of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
    // Strongly Connected Components"
    //
    // Relation {(00,01)} in a separated 'pre' and 'post' variable ordering
    //
    //             _1_       ---- x0 (0)
    //            /   \
    //           _2_  F      ---- x1 (1)
    //          /   \
    //         _3_  F        ---- x2 (0')
    //        /   \
    //        4   F          ---- x3 (1')
    //       / \
    //       F T
    */
    { // Garbage collect early and free write-lock
      const node n4 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n3 = node(2, bdd::max_id, n4.uid(), terminal_F);
      const node n2 = node(1, bdd::max_id, n3.uid(), terminal_F);
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(kalin_fig7_a);
      nw << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> kalin_fig7_b;
    /*
    // Figure 7 (right) of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
    // Strongly Connected Components"
    //
    // Relation {(01,10)} in a separated 'pre' and 'post' variable ordering.
    //
    //            _1_       ---- x0 (0)
    //           /   \
    //          _2_  F      ---- x1 (1)
    //         /   \
    //         F  _3_       ---- x2 (0')
    //           /   \
    //           F   4      ---- x3 (1')
    //              / \
    //              T F
    */
    { // Garbage collect early and free write-lock
      const node n4 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n3 = node(2, bdd::max_id, terminal_F, n4.uid());
      const node n2 = node(1, bdd::max_id, terminal_F, n3.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(kalin_fig7_b);
      nw << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> kalin_fig8;
    /*
    // Figure 8 of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
    // Strongly Connected Components"
    //
    // Relation {(00,01), (01,10)} in a separated 'pre' and 'post' variable ordering.
    //
    //                _1_       ---- x0 (0)
    //               /   \
    //               2   F      ---- x1 (1)
    //             __X__
    //            /     \
    //            3     4       ---- x2 (0')
    //           / \   / \
    //           F 5   6 F      ---- x3 (1')
    //            / \ / \
    //            T F F T
    */
    { // Garbage collect early and free write-lock
      const node n6 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n5 = node(3, bdd::max_id - 1, terminal_T, terminal_F);
      const node n4 = node(2, bdd::max_id, n6.uid(), terminal_F);
      const node n3 = node(2, bdd::max_id - 1, terminal_F, n5.uid());
      const node n2 = node(1, bdd::max_id, n4.uid(), n3.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(kalin_fig8);
      nw << n6 << n5 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> kalin_fig9;
    /*
    // Figure 9 of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
    // Strongly Connected Components"
    //
    // Relation {(00,01), (01,10), (10,01), (10,11)} in a separated 'pre' and 'post' variable
    // ordering.
    //
    //               __1__         ---- x0 (0)
    //              /     \
    //             _2_    3        ---- x1 (1)
    //            /   \  / \
    //            4   5  | F       ---- x2 (0')
    //           / \ / \ |
    //           | F F | |
    //           \__ __/ |
    //             _X_   /
    //            /   \ /
    //            6    7           ---- x3 (1')
    //           / \  / \
    //           T F  F T
    */
    { // Garbage collect early and free write-lock
      const node n7 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n6 = node(3, bdd::max_id - 1, terminal_T, terminal_F);
      const node n5 = node(2, bdd::max_id, terminal_F, n6.uid());
      const node n4 = node(2, bdd::max_id - 1, n7.uid(), terminal_F);
      const node n3 = node(1, bdd::max_id, n7.uid(), terminal_F);
      const node n2 = node(1, bdd::max_id - 1, n4.uid(), n5.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(kalin_fig9);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> kalin_00;
    /*
    //                  1       ---- x0 (0)
    //                 / \
    //                 2 F      ---- x1 (1)
    //                / \
    //                F T
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(1, bdd::max_id, terminal_T, terminal_F);
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(kalin_00);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> kalin_01;
    /*
    //                  1       ---- x0 (0)
    //                 / \
    //                 2 F      ---- x1 (1)
    //                / \
    //                F T
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(1, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(kalin_01);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> kalin_10;
    /*
    // Figure 10 of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
    // Strongly Connected Components"
    //
    //                  1       ---- x0 (0)
    //                 / \
    //                 F 2      ---- x1 (1)
    //                  / \
    //                  T F
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(1, bdd::max_id, terminal_T, terminal_F);
      const node n1 = node(0, bdd::max_id, terminal_F, n2.uid());

      node_ofstream nw(kalin_10);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> kalin_11;
    /*
    //                  1       ---- x0 (0)
    //                 / \
    //                 F 2      ---- x1 (1)
    //                  / \
    //                  F T
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(1, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, terminal_F, n2.uid());

      node_ofstream nw(kalin_11);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> kalin_fig11;
    /*
    // Figure 11 of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
    // Strongly Connected Components"
    //
    //                   1      ---- x1 (1)
    //                  / \
    //                  F T
    */
    { // Garbage collect early and free write-lock
      node_ofstream nw(kalin_fig11);
      nw << node(1, bdd::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> kalin_00_01;
    /*
    //                   1      ---- x0 (0)
    //                  / \
    //                  F T
    */
    { // Garbage collect early and free write-lock
      node_ofstream nw(kalin_00_01);
      nw << node(0, bdd::max_id, terminal_T, terminal_F);
    }

    shared_levelized_file<bdd::node_type> kalin_01_10;
    /*
    //                  _1_     ---- x0 (0)
    //                 /   \
    //                 2   3    ---- x1 (1)
    //                / \ / \
    //                F T T F
    */
    { // Garbage collect early and free write-lock
      const node n3 = node(1, bdd::max_id, terminal_T, terminal_F);
      const node n2 = node(1, bdd::max_id - 1, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(kalin_01_10);
      nw << n3 << n2 << n1;
    }

    // ------------------------------------------------------------------------------------------ //

    // ------------------------------------------------------------------------------------------ //

    // Chapter 6 of Huth and Ryan's "Logic in Computer Science" provides the following 3-state
    // machine as an example for CTL model checking.
    /*
    // Figure 6.24:
    //
    //       {10} ----> {01}
    //         ^          |
    //         |          |
    //         '-- {00} <-'
    //             ^  |
    //             '--'
    */

    // Here, they use a interleave variable ordering 'pre' and 'post'
    const auto huth_relnext_pred = [](int x) -> bool { return x % 2 == 0; };

    const auto huth_relnext_map = [&huth_relnext_pred](int x) -> optional<int> {
      return huth_relnext_pred(x) ? make_optional<int>() : make_optional<int>(x - 1);
    };

    const auto huth_relprev_pred = [](int x) -> bool { return x % 2 == 1; };

    const auto huth_relprev_map = [&huth_relprev_pred](int x) -> optional<int> {
      return huth_relprev_pred(x) ? make_optional<int>() : make_optional<int>(x + 1);
    };

    shared_levelized_file<bdd::node_type> huth_fig28;
    /*
    //               __1__        ---- x0 (0)
    //              /     \
    //              2     3       ---- x1 (0')
    //             / \   / \
    //             | 4   5 F      ---- x2 (1)
    //             |/ \ / \
    //             6  F 7 F       ---- x3 (1')
    //            / \  / \
    //            T F  F T
    //
    // Notice, that the BDD as such also allows for {11} even if it isn't part of the system. This
    // is fair, as the transition relation is false for this state.
    */
    { // Garbage collect early and free write-lock
      const node n7 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n6 = node(3, bdd::max_id - 1, terminal_T, terminal_F);
      const node n5 = node(2, bdd::max_id, n7.uid(), terminal_T);
      const node n4 = node(2, bdd::max_id - 1, n6.uid(), terminal_F);
      const node n3 = node(1, bdd::max_id, n5.uid(), terminal_F);
      const node n2 = node(1, bdd::max_id - 1, n6.uid(), n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(huth_fig28);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> huth_00;
    /*
    //               1           ---- x0 (0)
    //              / \
    //              | F
    //              |
    //              2            ---- x2 (1)
    //             / \
    //             T F
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(huth_00);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> huth_01;
    /*
    //               1           ---- x0 (0)
    //              / \
    //              | F
    //              |
    //              2            ---- x2 (1)
    //             / \
    //             F T
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(huth_01);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> huth_10;
    /*
    //               1           ---- x0 (0)
    //              / \
    //              F |
    //                |
    //                2          ---- x2 (1)
    //               / \
    //               F T
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n1 = node(0, bdd::max_id, terminal_F, n2.uid());

      node_ofstream nw(huth_10);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> huth_fig26_a;
    /*
    // Figure 6.26 (a)
    //
    //               1           ---- x0 (0)
    //              / \
    //              | T
    //              |
    //              2            ---- x2 (1)
    //             / \
    //             F T
    //
    // Notice, that the BDD as such also allows for {11} even if it isn't part of the system. This
    // is fair, as the transition relation is false for this state.
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n2.uid(), terminal_T);

      node_ofstream nw(huth_fig26_a);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> huth_fig26_b;
    /*
    // Figure 6.26 (b)
    //
    //               1           ---- x0 (0)
    //               X
    //              / \
    //             /   \
    //             2   3         ---- x2 (1)
    //            / \ / \
    //            T F F T
    */
    { // Garbage collect early and free write-lock
      const node n3 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n2 = node(2, bdd::max_id - 1, terminal_T, terminal_F);
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(huth_fig26_b);
      nw << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> huth_00_01;
    /*
    //               1           ---- x0 (0)
    //              / \
    //              T F
    */
    { // Garbage collect early and free write-lock
      node_ofstream nw(huth_00_01);
      nw << node(0, bdd::max_id, terminal_T, terminal_F);
    }

    // ------------------------------------------------------------------------------------------ //

    // ------------------------------------------------------------------------------------------ //

    /*
    // Fig. 17 in Randal E. Bryant's "Symbolic Boolean Manipulation with Ordered Binary Decision
    // Diagrams" includes the following non-deterministic finite state machine.
    //
    //
    //             ____{01}____
    //          1 /            \ 0
    //           /   ________   \
    //           v  v   1    \  v
    //         __{00}        {10}__
    //     0,1 \_^  \________^  ^_/ 0
    //                  0
    */

    // Here, an *input* variable, x, and two *current*/*next* state variables, 0,1 / 0', 1', are
    // used.
    const auto bryant_relnext_pred = [](int x) -> bool { return x < 3; };

    const auto bryant_relnext_map = [&bryant_relnext_pred](int x) -> optional<int> {
      return bryant_relnext_pred(x) ? make_optional<int>() : make_optional<int>(x - 2);
    };

    const auto bryant_relprev_pred = [](int x) -> bool { return 2 < x || x == 0; };

    const auto bryant_relprev_map = [&bryant_relprev_pred](int x) -> optional<int> {
      return bryant_relprev_pred(x) ? make_optional<int>() : make_optional<int>(x + 2);
    };

    /*
    // Fig. 18 in Randal E. Bryant's "Symbolic Boolean Manipulation with Ordered Binary Decision
    // Diagrams" shows the following encoding of the above non-deterministic finite state machine
    // into a BDD.
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

    /*
    // Subset of Fig. 18 with 'x := false'
    //
    //                1         ---- x1 (0)
    //               / \
    //               2 |        ---- x2 (1)
    //              / \|
    //              |  3        ---- x3 (0')
    //              \ / \
    //               X  |
    //              / \_|
    //              F   4       ---- x4 (1')
    //                 / \
    //                 T F
    */

    shared_levelized_file<bdd::node_type> bryant_fig18_x0;
    { // Garbage collect early and free write-lock
      const node n4 = node(4, bdd::max_id, terminal_T, terminal_F);
      const node n3 = node(3, bdd::max_id - 1, terminal_F, n4.uid());
      const node n2 = node(2, bdd::max_id, n4.uid(), n3.uid());
      const node n1 = node(1, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bryant_fig18_x0);
      nw << n4 << n3 << n2 << n1;
    }

    /*
    // Subset of Fig. 18 with 'x := true'
    //
    //            1          ---- x3 (0')
    //           / \
    //           2 F         ---- x4 (1')
    //          / \
    //          T F
    */
    shared_levelized_file<bdd::node_type> bryant_fig18_x1;
    { // Garbage collect early and free write-lock
      const node n2 = node(4, bdd::max_id, terminal_T, terminal_F);
      const node n1 = node(3, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(bryant_fig18_x1);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bryant_00;
    /*
    //                  1       ---- x1 (0)
    //                 / \
    //                 2 F      ---- x2 (1)
    //                / \
    //                T F
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n1 = node(1, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(bryant_00);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bryant_01;
    /*
    //                  1       ---- x1 (0)
    //                 / \
    //                 2 F      ---- x2 (1)
    //                / \
    //                F T
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(1, bdd::max_id, n2.uid(), terminal_F);

      node_ofstream nw(bryant_01);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bryant_10;
    /*
    //                  1       ---- x1 (0)
    //                 / \
    //                 F 2      ---- x2 (1)
    //                  / \
    //                  T F
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n1 = node(1, bdd::max_id, terminal_F, n2.uid());

      node_ofstream nw(bryant_10);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bryant_01_10;
    /*
    //                  1       ---- x1 (0)
    //                 / \
    //                 2 T      ---- x2 (1)
    //                / \
    //                F T
    */
    { // Garbage collect early and free write-lock
      const node n2 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(1, bdd::max_id, n2.uid(), terminal_T);

      node_ofstream nw(bryant_01_10);
      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bryant_00_10;
    /*
    //                 1        ---- x2 (1)
    //                / \
    //                T F
    */
    { // Garbage collect early and free write-lock
      node_ofstream nw(bryant_00_10);
      nw << node(2, bdd::max_id, terminal_T, terminal_F);
    }

    // ------------------------------------------------------------------------------------------ //

    describe("bdd_relprod(const bdd&, const bdd&, <exists>)", [&]() {
      it("has single successor [{01} + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprod(kalin_01, kalin_fig7_b, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{01} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(kalin_01, kalin_fig9, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{00} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(kalin_00, kalin_fig9, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{10} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(huth_10, huth_fig28, huth_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{01} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(huth_01, huth_fig28, huth_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{01} + B Fig. 18 (x:=0)]", [&]() {
        const bdd out = bdd_relprod(bryant_01, bryant_fig18_x0, bryant_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{01} + K&D Fig. 7(a)]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x2 (0')
        //                 / \
        //                 2 F      ---- x3 (1')
        //                / \
        //                F T
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_F, terminal_T);
          const node n1 = node(2, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig7_a, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{01}(+2) + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relprod(bdd(kalin_01, false, +2), kalin_fig7_a, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10} + K&D Fig. 7(b)]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x2 (0')
        //                 / \
        //                 F 2      ---- x3 (1')
        //                  / \
        //                  T F
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(2, bdd::max_id, terminal_F, n2.uid());

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig7_b, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10}(+2) + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprod(bdd(kalin_10, false, +2), kalin_fig7_b, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10} + K&D Fig. 9]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x2 (0')
        //                 / \
        //                 F 2      ---- x3 (1')
        //                  / \
        //                  T F
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(2, bdd::max_id, terminal_F, n2.uid());

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10}(+2) + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(bdd(kalin_10, false, +2), kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{01} + H&R Fig. 6.28]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //               1           ---- x1 (0')
        //              / \
        //              | F
        //              |
        //              2            ---- x3 (1')
        //             / \
        //             F T
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_F, terminal_T);
          const node n1 = node(1, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

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

      it("has single predecessor [{01}(+1) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(bdd(huth_01, false, +1), huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

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

      it("has single predecessor [{10} + H&R Fig. 6.28]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //               1           ---- x1 (0')
        //              / \
        //              F |
        //                |
        //                2          ---- x3 (1')
        //               / \
        //               F T
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(1, bdd::max_id, terminal_F, n2.uid());

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10}(+1) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(bdd(huth_10, false, +1), huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{00} + B Fig. 18 (x:=0)]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x3 (0')
        //                 / \
        //                 2 F      ---- x4 (1')
        //                / \
        //                T F
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(4, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(3, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, bryant_fig18_x0, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{00}(+2) + B Fig. 18 (x:=0)]", [&]() {
        const bdd out =
          bdd_relprod(bdd(bryant_00, false, +2), bryant_fig18_x0, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no successors [{11} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(kalin_11, kalin_fig9, kalin_relnext_pred);

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors [{00} + K&D Fig. 9]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x2 (0')
        //                 / \
        //                 2 F      ---- x3 (1')
        //                / \
        //                F T
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(2, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors [{00}(+2) + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(bdd(kalin_00, false, +2), kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors [{01} + B Fig. 18]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x3 (0')
        //                 / \
        //                 2 F      ---- x4 (1')
        //                / \
        //                F T
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(4, bdd::max_id, terminal_F, terminal_T);
          const node n1 = node(3, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, bryant_fig18, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors [{01}(+2) + B Fig. 18]", [&]() {
        const bdd out = bdd_relprod(bdd(bryant_01, false, +2), bryant_fig18, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors of non-current states [{01} + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relprod(kalin_01, kalin_fig7_a, kalin_relnext_pred);

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{10} + K&D Fig. 7(a)]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x2 (0')
        //                 / \
        //                 F 2      ---- x3 (1')
        //                  / \
        //                  T F
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(2, bdd::max_id, terminal_F, n2.uid());

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig7_a, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{10}(+2) + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relprod(bdd(kalin_10, false, +2), kalin_fig7_a, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{01} + K&D Fig. 7(b)]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x2 (0')
        //                 / \
        //                 2 F      ---- x3 (1')
        //                / \
        //                F T
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_F, terminal_T);
          const node n1 = node(2, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig7_b, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{01}(+2) + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprod(bdd(kalin_01, false, +2), kalin_fig7_b, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{10} + B Fig. 18 (x:=1)]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x3 (0)
        //                 / \
        //                 F 2      ---- x4 (1)
        //                  / \
        //                  T F
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(4, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(3, bdd::max_id, terminal_F, n2.uid());

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, bryant_fig18_x1, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{10}(+2) + B Fig. 18 (x:=1)]", [&]() {
        const bdd out =
          bdd_relprod(bdd(bryant_10, false, +2), bryant_fig18_x1, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has multiple successors [{10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(kalin_10, kalin_fig9, kalin_relnext_pred);

        // Check it looks all right
        //
        // NOTE: Fig. 11 in Kalin's and Dahlsen-Jensen's BSc Thesis
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

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

      it("has multiple successors [{00} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(huth_00, huth_fig28, huth_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

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

      it("has multiple successors [{00} + B Fig. 18 (x:=0)]", [&]() {
        const bdd out = bdd_relprod(bryant_00, bryant_fig18_x0, bryant_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

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

      it("has multiple successors [{00} + B Fig. 18]", [&]() {
        const bdd out = bdd_relprod(bryant_00, bryant_fig18, bryant_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

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

      it("has multiple successors [{10} + B Fig. 18]", [&]() {
        const bdd out = bdd_relprod(bryant_10, bryant_fig18, bryant_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

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

      it("has multiple predecessors [{01} + K&D Fig. 9]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x2 (0')
        //                 / \
        //                 2 F      ---- x3 (1')
        //                / \
        //                F T
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_F, terminal_T);
          const node n1 = node(2, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has multiple predecessors [{01}(+2) + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(bdd(kalin_01, false, +2), kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has multiple predecessors [{00} + H&R Fig. 6.28]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //               1           ---- x1 (0')
        //              / \
        //              | F
        //              |
        //              2            ---- x3 (1')
        //             / \
        //             T F
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(1, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has multiple predecessors [{00}(+1) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(bdd(huth_00, false, +1), huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has multiple predecessors [{00} + B Fig. 18]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x3 (0')
        //                 / \
        //                 2 F      ---- x4 (1')
        //                / \
        //                T F
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(4, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(3, bdd::max_id, n2.uid(), terminal_F);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, bryant_fig18, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has multiple predecessors [{00}(+2) + B Fig. 18]", [&]() {
        const bdd out = bdd_relprod(bdd(bryant_00, false, +2), bryant_fig18, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has multiple predecessors [{10} + B Fig. 18]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  1       ---- x3 (0')
        //                 / \
        //                 F 2      ---- x4 (1')
        //                  / \
        //                  T F
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(4, bdd::max_id, terminal_T, terminal_F);
          const node n1 = node(3, bdd::max_id, terminal_F, n2.uid());

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, bryant_fig18, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has multiple predecessors [{10}(+2) + B Fig. 18]", [&]() {
        const bdd out = bdd_relprod(bdd(bryant_10, false, +2), bryant_fig18, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has successors of all current states [K&D Fig. 11 + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprod(kalin_fig11, kalin_fig7_b, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has successors of all current states [K&D Fig. 11 + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(kalin_fig11, kalin_fig9, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has successors of all current states [{01,10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(kalin_01_10, kalin_fig9, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has successors of all current states [H&R Fig. 6.26(a) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(huth_fig26_a, huth_fig28, huth_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has successors of all current states [H&R Fig. 6.26(b) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(huth_fig26_b, huth_fig28, huth_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has successors of all current states [{01,10} + B Fig. 18 (x:=1)]", [&]() {
        const bdd out = bdd_relprod(bryant_01_10, bryant_fig18_x1, bryant_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has predecessors of all current states [{00,10} + K&D Fig. 9]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                   1      ---- x2 (0')
        //                  / \
        //                  F T
        */
        { // Garbage collect early and free write-lock
          node_ofstream nw(shifted_states);
          nw << node(2, bdd::max_id, terminal_T, terminal_F);
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has predecessors of all current states [{01,10} + K&D Fig. 9]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                  _1_     ---- x2 (0')
        //                 /   \
        //                 2   3    ---- x3 (1')
        //                / \ / \
        //                F T T F
        */
        { // Garbage collect early and free write-lock
          const node n3 = node(3, bdd::max_id, terminal_T, terminal_F);
          const node n2 = node(3, bdd::max_id - 1, terminal_F, terminal_T);
          const node n1 = node(2, bdd::max_id, n2.uid(), n3.uid());

          node_ofstream nw(shifted_states);
          nw << n3 << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [K&D Fig. 11 + K&D Fig. 9]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        // Figure 11 of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
        // Strongly Connected Components"
        //
        //                   1      ---- x3 (1')
        //                  / \
        //                  F T
        */
        { // Garbage collect early and free write-lock
          node_ofstream nw(shifted_states);
          nw << node(3, bdd::max_id, terminal_F, terminal_T);
        }

        const bdd out = bdd_relprod(shifted_states, kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has predecessors of all current states [H&R Fig.6.26(a) + H&R Fig. 6.28]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        // Figure 6.26 (a)
        //
        //               1           ---- x1 (0')
        //              / \
        //              | T
        //              |
        //              2            ---- x3 (1')
        //             / \
        //             F T
        */
        { // Garbage collect early and free write-lock
          const node n2 = node(3, bdd::max_id, terminal_F, terminal_T);
          const node n1 = node(1, bdd::max_id, n2.uid(), terminal_T);

          node_ofstream nw(shifted_states);
          nw << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [H&R Fig.6.26(b) + H&R Fig. 6.28]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        // Figure 6.26 (b)
        //
        //               1           ---- x1 (0')
        //               X
        //              / \
        //             /   \
        //             2   3         ---- x3 (1')
        //            / \ / \
        //            T F F T
        */
        { // Garbage collect early and free write-lock
          const node n3 = node(3, bdd::max_id, terminal_F, terminal_T);
          const node n2 = node(3, bdd::max_id - 1, terminal_T, terminal_F);
          const node n1 = node(1, bdd::max_id, n3.uid(), n2.uid());

          node_ofstream nw(shifted_states);
          nw << n3 << n2 << n1;
        }

        const bdd out = bdd_relprod(shifted_states, huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [{00,01} + H&R Fig. 6.28]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //               1           ---- x1 (0')
        //              / \
        //              T F
        */
        { // Garbage collect early and free write-lock
          node_ofstream nw(shifted_states);
          nw << node(1, bdd::max_id, terminal_T, terminal_F);
        }

        const bdd out = bdd_relprod(shifted_states, huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has predecessors of all current states [{00,10} + B Fig. 18 (x:=0)]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        //                 1        ---- x4 (1')
        //                / \
        //                T F
        */
        { // Garbage collect early and free write-lock
          node_ofstream nw(shifted_states);
          nw << node(4, bdd::max_id, terminal_T, terminal_F);
        }

        const bdd out = bdd_relprod(shifted_states, bryant_fig18, bryant_relprev_pred);
        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("excludes source states [{__} + B Fig. 18]", [&]() {
        const bdd out = bdd_relprod(true, bryant_fig18, bryant_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

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

      it("excludes deadlocks [{__} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(true, kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has no successors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relprod(false, true, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [<true>] [~]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relprod(bdd_not(T), T, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(false, kalin_fig9, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(false, huth_fig28, huth_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [B Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(false, bryant_fig18, bryant_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relprod(false, true, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [<true>] [~]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relprod(bdd_not(T), T, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprod(false, kalin_fig9, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(false, huth_fig28, huth_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [B Fig. 6.28]", [&]() {
        const bdd out = bdd_relprod(false, bryant_fig18, bryant_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
        const bdd out = bdd_relprod(kalin_fig11, false, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprod(bdd_not(F), F, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprod(F, F, kalin_relnext_pred);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has no predecessors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        // Figure 11 of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
        // Strongly Connected Components"
        //
        //                   1      ---- x3 (1')
        //                  / \
        //                  F T
        */
        { // Garbage collect early and free write-lock
          node_ofstream nw(shifted_states);
          nw << node(3, bdd::max_id, terminal_F, terminal_T);
        }

        const bdd out = bdd_relprod(shifted_states, false, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprod(bdd_not(F), F, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprod(F, F, kalin_relprev_pred);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all successors for total graph [K&D Fig. 11 + <true>]", [&]() {
        const bdd out = bdd_relprod(kalin_fig11, true, kalin_relnext_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all successors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relprod(T, T, kalin_relnext_pred);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all predecessors for total graph [K&D Fig. 11 + <true>]", [&]() {
        shared_levelized_file<bdd::node_type> shifted_states;
        /*
        // Figure 11 of Kalin's & Dahlsen-Jensen's BSc Thesis "Symbolic Algorithms for Computing
        // Strongly Connected Components"
        //
        //                   1      ---- x3 (1')
        //                  / \
        //                  F T
        */
        { // Garbage collect early and free write-lock
          node_ofstream nw(shifted_states);
          nw << node(3, bdd::max_id, terminal_F, terminal_T);
        }

        const bdd out = bdd_relprod(shifted_states, true, kalin_relprev_pred);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all predecessors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relprod(T, T, kalin_relprev_pred);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });
    });

    describe("bdd_relnext(const bdd&, const bdd&, <exists + replace>)", [&]() {
      it("has single successor [{01} + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relnext(kalin_01, kalin_fig7_b, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{01} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_01, kalin_fig9, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{00} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_00, kalin_fig9, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{10} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_10, huth_fig28, huth_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{01} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_01, huth_fig28, huth_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{01} + B Fig. 18 (x:=0)]", [&]() {
        const bdd out = bdd_relnext(bryant_01, bryant_fig18_x0, bryant_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no successors [{11} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_11, kalin_fig9, kalin_relnext_map);

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors of non-current states [{01} + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relnext(kalin_01, kalin_fig7_a, kalin_relnext_map);

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has multiple successors [{10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_10, kalin_fig9, kalin_relnext_map);

        // Check it looks all right
        //
        // NOTE: Fig. 11 in Kalin's and Dahlsen-Jensen's BSc Thesis
        node_test_ifstream out_nodes(out);

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

      it("has multiple successors [{00} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_00, huth_fig28, huth_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

      it("has multiple successors [{00} + B Fig. 18 (x:=0)]", [&]() {
        const bdd out = bdd_relnext(bryant_00, bryant_fig18_x0, bryant_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

      it("has multiple successors [{00} + B Fig. 18]", [&]() {
        const bdd out = bdd_relnext(bryant_00, bryant_fig18, bryant_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

      it("has multiple successors [{10} + B Fig. 18]", [&]() {
        const bdd out = bdd_relnext(bryant_10, bryant_fig18, bryant_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

      it("has successors of all current states [K&D Fig. 11 + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relnext(kalin_fig11, kalin_fig7_b, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has successors of all current states [K&D Fig. 11 + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_fig11, kalin_fig9, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has successors of all current states [{01,10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_01_10, kalin_fig9, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has successors of all current states [H&R Fig. 6.26(a) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_fig26_a, huth_fig28, huth_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_T, terminal_F)));

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

      it("has successors of all current states [H&R Fig. 6.26(b) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_fig26_b, huth_fig28, huth_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_T, terminal_F)));

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

      it("has successors of all current states [{01,10} + B Fig. 18 (x:=1)]", [&]() {
        const bdd out = bdd_relnext(bryant_01_10, bryant_fig18_x1, bryant_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("excludes source states [{__} + B Fig. 18]", [&]() {
        const bdd out = bdd_relnext(true, bryant_fig18, bryant_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

      it("has no successors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relnext(false, true, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [<true>] [~]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relnext(bdd_not(T), T, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(false, kalin_fig9, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(false, huth_fig28, huth_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [B Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(false, bryant_fig18, bryant_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
        const bdd out = bdd_relnext(kalin_fig11, false, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relnext(bdd_not(F), F, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relnext(F, F, kalin_relnext_map);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all successors for total graph [K&D Fig. 11 + <true>]", [&]() {
        const bdd out = bdd_relnext(kalin_fig11, true, kalin_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all successors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relnext(T, T, kalin_relnext_map);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("throws exception if mapping is not monotone [{10} + K&D Fig. 9]", [&]() {
        const auto kalin_relnext_map__flipped = [&kalin_relnext_pred](int x) -> optional<int> {
          return kalin_relnext_pred(x) ? make_optional<int>() : make_optional<int>(!(x - 2));
        };
        AssertThrows(invalid_argument,
                     bdd_relnext(kalin_10, kalin_fig9, kalin_relnext_map__flipped));
      });
    });

    describe(
      "bdd_relnext(const bdd&, const bdd&, <exists + replace>, replace_type::Non_Monotone)", [&]() {
        it("throws exception if mapping is not monotone [{10} + K&D Fig. 9]", [&]() {
          // NOTE: We actually provide a Monotone map, but do not claim to do so!
          AssertThrows(
            invalid_argument,
            bdd_relnext(kalin_10, kalin_fig9, kalin_relnext_map, replace_type::Non_Monotone));
        });
      });

    describe(
      "bdd_relnext(const bdd&, const bdd&, <exists + replace>, replace_type::Monotone)", [&]() {
        it("has single successor [{00} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relnext(kalin_00, kalin_fig9, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has single successor [{01} + H&R Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relnext(huth_01, huth_fig28, huth_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has no successors [{11} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relnext(kalin_11, kalin_fig9, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());
          AssertThat(bdd_iscanonical(out), Is().True());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors of non-current states [{01} + K&D Fig. 7(a)]", [&]() {
          const bdd out =
            bdd_relnext(kalin_01, kalin_fig7_a, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());
          AssertThat(bdd_iscanonical(out), Is().True());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has multiple successors [{10} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relnext(kalin_10, kalin_fig9, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          //
          // NOTE: Fig. 11 in Kalin's and Dahlsen-Jensen's BSc Thesis
          node_test_ifstream out_nodes(out);

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

        it("has multiple successors [{00} + H&R Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relnext(huth_00, huth_fig28, huth_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

        it("has multiple successors [{00} + B Fig. 18]", [&]() {
          const bdd out =
            bdd_relnext(bryant_00, bryant_fig18, bryant_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

        it("has successors of all current states [K&D Fig. 11 + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relnext(kalin_fig11, kalin_fig9, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has successors of all current states [{01,10} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relnext(kalin_01_10, kalin_fig9, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("has successors of all current states [H&R Fig. 6.26(b) + H&R Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relnext(huth_fig26_b, huth_fig28, huth_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_T, terminal_F)));

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

        it("has successors of all current states [{01,10} + B Fig. 18 (x:=1)]", [&]() {
          const bdd out =
            bdd_relnext(bryant_01_10, bryant_fig18_x1, bryant_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("excludes source states [{__} + B Fig. 18]", [&]() {
          const bdd out =
            bdd_relnext(true, bryant_fig18, bryant_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

        it("has no successors for empty set of states [<true>]", [&]() {
          const bdd out = bdd_relnext(false, true, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of states [<true>] [~]", [&]() {
          const bdd T   = true;
          const bdd out = bdd_relnext(bdd_not(T), T, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of states [K&D Fig. 9]", [&]() {
          const bdd out = bdd_relnext(false, kalin_fig9, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of states [H&R Fig. 6.28]", [&]() {
          const bdd out = bdd_relnext(false, huth_fig28, huth_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of states [B Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relnext(false, bryant_fig18, bryant_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
          const bdd out =
            bdd_relnext(kalin_fig11, false, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of edges [~<false> + <false>]", [&]() {
          const bdd F   = false;
          const bdd out = bdd_relnext(bdd_not(F), F, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of nodes and edges [<false>]", [&]() {
          const bdd F   = false;
          const bdd out = bdd_relnext(F, F, kalin_relnext_map, replace_type::Monotone);

          AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
          AssertThat(out.is_negated(), Is().False());
        });

        it("has all successors for total graph [K&D Fig. 11 + <true>]", [&]() {
          const bdd out = bdd_relnext(kalin_fig11, true, kalin_relnext_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has all successors for all nodes and total graph [<true>]", [&]() {
          const bdd T   = true;
          const bdd out = bdd_relnext(T, T, kalin_relnext_map, replace_type::Monotone);

          AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
          AssertThat(out.is_negated(), Is().False());
        });
      });

    describe(
      "bdd_relnext(const bdd&, const bdd&, <exists + replace>, replace_type::Monotone)", [&]() {
        it("has single successor [{00} + K&D Fig. 9]", [&]() {
          const bdd out = bdd_relnext(kalin_00, kalin_fig9, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has single successor [{01} + H&R Fig. 6.28]", [&]() {
          const bdd out = bdd_relnext(huth_01, huth_fig28, huth_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has no successors [{11} + K&D Fig. 9]", [&]() {
          const bdd out = bdd_relnext(kalin_11, kalin_fig9, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());
          AssertThat(bdd_iscanonical(out), Is().True());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors of non-current states [{01} + K&D Fig. 7(a)]", [&]() {
          const bdd out =
            bdd_relnext(kalin_01, kalin_fig7_a, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());
          AssertThat(bdd_iscanonical(out), Is().True());

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);
          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has multiple successors [{10} + K&D Fig. 9]", [&]() {
          const bdd out = bdd_relnext(kalin_10, kalin_fig9, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          //
          // NOTE: Fig. 11 in Kalin's and Dahlsen-Jensen's BSc Thesis
          node_test_ifstream out_nodes(out);

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

        it("has multiple successors [{00} + H&R Fig. 6.28]", [&]() {
          const bdd out = bdd_relnext(huth_00, huth_fig28, huth_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

        it("has multiple successors [{00} + B Fig. 18]", [&]() {
          const bdd out =
            bdd_relnext(bryant_00, bryant_fig18, bryant_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

        it("has successors of all current states [K&D Fig. 11 + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relnext(kalin_fig11, kalin_fig9, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has successors of all current states [{01,10} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relnext(kalin_01_10, kalin_fig9, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("has successors of all current states [H&R Fig. 6.26(b) + H&R Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relnext(huth_fig26_b, huth_fig28, huth_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_T, terminal_F)));

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

        it("has successors of all current states [{01,10} + B Fig. 18 (x:=1)]", [&]() {
          const bdd out =
            bdd_relnext(bryant_01_10, bryant_fig18_x1, bryant_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("excludes source states [{__} + B Fig. 18]", [&]() {
          const bdd out = bdd_relnext(true, bryant_fig18, bryant_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

        it("has no successors for empty set of states [<true>]", [&]() {
          const bdd out = bdd_relnext(false, true, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of states [<true>] [~]", [&]() {
          const bdd T   = true;
          const bdd out = bdd_relnext(bdd_not(T), T, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of states [K&D Fig. 9]", [&]() {
          const bdd out = bdd_relnext(false, kalin_fig9, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of states [H&R Fig. 6.28]", [&]() {
          const bdd out = bdd_relnext(false, huth_fig28, huth_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of states [B Fig. 6.28]", [&]() {
          const bdd out = bdd_relnext(false, bryant_fig18, bryant_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
          const bdd out = bdd_relnext(kalin_fig11, false, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of edges [~<false> + <false>]", [&]() {
          const bdd F   = false;
          const bdd out = bdd_relnext(bdd_not(F), F, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no successors for empty set of nodes and edges [<false>]", [&]() {
          const bdd F   = false;
          const bdd out = bdd_relnext(F, F, kalin_relnext_map, replace_type::Shift);

          AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
          AssertThat(out.is_negated(), Is().False());
        });

        it("has all successors for total graph [K&D Fig. 11 + <true>]", [&]() {
          const bdd out = bdd_relnext(kalin_fig11, true, kalin_relnext_map, replace_type::Shift);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has all successors for all nodes and total graph [<true>]", [&]() {
          const bdd T   = true;
          const bdd out = bdd_relnext(T, T, kalin_relnext_map, replace_type::Shift);

          AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
          AssertThat(out.is_negated(), Is().False());
        });
      });

    describe("bdd_relnext(const bdd&, const bdd&, const bdd::label_type)", [&]() {
      it("has single successor [{01} + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relnext(kalin_01, kalin_fig7_b, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{01} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_01, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{00} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_00, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no successors [{11} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_11, kalin_fig9, 2);

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors of non-current states [{01} + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relnext(kalin_01, kalin_fig7_a, 2);

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has multiple successors [{10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_10, kalin_fig9, 2);

        // Check it looks all right
        //
        // NOTE: Fig. 11 in Kalin's and Dahlsen-Jensen's BSc Thesis
        node_test_ifstream out_nodes(out);

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

      it("has successors of all current states [K&D Fig. 11 + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relnext(kalin_fig11, kalin_fig7_b, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has successors of all current states [K&D Fig. 11 + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_fig11, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has successors of all current states [{01,10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(kalin_01_10, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has no successors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relnext(false, true, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [<true>] [~]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relnext(bdd_not(T), T, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [K&D Fig. 9]", [&]() {
        const bdd out = bdd_relnext(false, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
        const bdd out = bdd_relnext(kalin_fig11, false, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relnext(bdd_not(F), F, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relnext(F, F, 2);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all successors for total graph [K&D Fig. 11 + <true>]", [&]() {
        const bdd out = bdd_relnext(kalin_fig11, true, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all successors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relnext(T, T, 2);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });
    });

    describe("bdd_relnext(const bdd&, const bdd&)", [&]() {
      it("has single successor [{10} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_10, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single successor [{01} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_01, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has multiple successors [{00} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_00, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

      it("has successors of all current states [H&R Fig. 6.26(a) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(huth_fig26_a, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_T, terminal_F)));

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

      it("has no successors for empty set of states [H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relnext(false, huth_fig28, huth_relnext_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relnext(false, true);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [<true>] [~]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relnext(bdd_not(T), T);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of states [H&R Fig. 28]", [&]() {
        const bdd out = bdd_relnext(false, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of edges [H&R Fig. 26(b) + <false>]", [&]() {
        const bdd out = bdd_relnext(huth_fig26_b, false);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relnext(bdd_not(F), F);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no successors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relnext(F, F);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all successors for total graph [H&R Fig. 26(b) + <true>]", [&]() {
        const bdd out = bdd_relnext(huth_fig26_b, true);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all successors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relnext(T, T);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });
    });

    describe("bdd_relprev(const bdd&, const bdd&, <exists + replace>)", [&]() {
      it("has single predecessor [{01} + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relprev(kalin_01, kalin_fig7_a, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10} + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprev(kalin_10, kalin_fig7_b, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_10, kalin_fig9, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{01} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_01, huth_fig28, huth_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

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

      it("has single predecessor [{10} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_10, huth_fig28, huth_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{00} + B Fig. 18 (x:=0)]", [&]() {
        const bdd out = bdd_relprev(bryant_00, bryant_fig18_x0, bryant_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no predecessors [{00} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_00, kalin_fig9, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors [{01} + B Fig. 18]", [&]() {
        const bdd out = bdd_relprev(bryant_01, bryant_fig18, bryant_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{10} + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relprev(kalin_10, kalin_fig7_a, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{01} + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprev(kalin_01, kalin_fig7_b, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{10} + B Fig. 18 (x:=1)]", [&]() {
        const bdd out = bdd_relprev(bryant_10, bryant_fig18_x1, bryant_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has multiple predecessors [{01} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_01, kalin_fig9, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has multiple predecessors [{00} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_00, huth_fig28, huth_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has multiple predecessors [{00} + B Fig. 18]", [&]() {
        const bdd out = bdd_relprev(bryant_00, bryant_fig18, bryant_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has multiple predecessors [{10} + B Fig. 18]", [&]() {
        const bdd out = bdd_relprev(bryant_10, bryant_fig18, bryant_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has predecessors of all current states [{00,10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_00_01, kalin_fig9, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has predecessors of all current states [{01,10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_01_10, kalin_fig9, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [K&D Fig. 11 + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_fig11, kalin_fig9, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has predecessors of all current states [H&R Fig.6.26(a) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_fig26_a, huth_fig28, huth_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [H&R Fig.6.26(b) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_fig26_b, huth_fig28, huth_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [{00,01} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_00_01, huth_fig28, huth_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has predecessors of all current states [{00,10} + B Fig. 18 (x:=0)]", [&]() {
        const bdd out = bdd_relprev(bryant_00_10, bryant_fig18, bryant_relprev_map);
        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("excludes deadlocks [{__} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(true, kalin_fig9, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has no deadlocks [{__} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(true, huth_fig28, huth_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no deadlocks [{__} + B Fig. 18]", [&]() {
        const bdd out = bdd_relprev(true, bryant_fig18, bryant_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no predecessors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relprev(false, true, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(false, kalin_fig9, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(false, huth_fig28, huth_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [B Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(false, bryant_fig18, bryant_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
        const bdd out = bdd_relprev(kalin_fig11, false, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprev(bdd_not(F), F, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprev(F, F, kalin_relprev_map);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all predecessors for total graph [K&D Fig. 11 + <true>]", [&]() {
        const bdd out = bdd_relprev(kalin_fig11, true, kalin_relprev_map);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all predecessors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relprev(T, T, kalin_relprev_map);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("throws exception if mapping is not monotone [{01} + K&D Fig. 9]", [&]() {
        const auto kalin_relprev_map__flipped = [&kalin_relprev_pred](int x) -> optional<int> {
          return kalin_relprev_pred(x) ? make_optional<int>() : make_optional<int>(2 + !x);
        };
        AssertThrows(invalid_argument,
                     bdd_relprev(kalin_01, kalin_fig9, kalin_relprev_map__flipped));
      });
    });

    describe(
      "bdd_relprev(const bdd&, const bdd&, <exists + replace>, replace_type::Non_Monotone)", [&]() {
        it("throws exception if mapping is not monotone [{01} + K&D Fig. 9]", [&]() {
          // NOTE: We actually provide a Monotone map, but do not claim to do so!
          AssertThrows(
            invalid_argument,
            bdd_relprev(kalin_01, kalin_fig9, kalin_relprev_map, replace_type::Non_Monotone));
        });
      });

    describe(
      "bdd_relprev(const bdd&, const bdd&, <exists + replace>, replace_type::Monotone)", [&]() {
        it("has single predecessor [{10} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relprev(kalin_10, kalin_fig9, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has single predecessor [{01} + H&R Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relprev(huth_01, huth_fig28, huth_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

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

        it("has single predecessor [{00} + B Fig. 18 (x:=0)]", [&]() {
          const bdd out =
            bdd_relprev(bryant_00, bryant_fig18_x0, bryant_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has no predecessors [{00} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relprev(kalin_00, kalin_fig9, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors [{01} + B Fig. 18]", [&]() {
          const bdd out =
            bdd_relprev(bryant_01, bryant_fig18, bryant_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors of non-current states [{01} + K&D Fig. 7(b)]", [&]() {
          const bdd out =
            bdd_relprev(kalin_01, kalin_fig7_b, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors of non-current states [{10} + B Fig. 18 (x:=1)]", [&]() {
          const bdd out =
            bdd_relprev(bryant_10, bryant_fig18_x1, bryant_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has multiple predecessors [{01} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relprev(kalin_01, kalin_fig9, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

        it("has multiple predecessors [{00} + H&R Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relprev(huth_00, huth_fig28, huth_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("has multiple predecessors [{00} + B Fig. 18]", [&]() {
          const bdd out =
            bdd_relprev(bryant_00, bryant_fig18, bryant_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has predecessors of all current states [{01,10} + K&D Fig. 9]", [&]() {
          const bdd out =
            bdd_relprev(kalin_01_10, kalin_fig9, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("has predecessors of all current states [H&R Fig.6.26(a) + H&R Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relprev(huth_fig26_a, huth_fig28, huth_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("has predecessors of all current states [{00,10} + B Fig. 18 (x:=0)]", [&]() {
          const bdd out =
            bdd_relprev(bryant_00_10, bryant_fig18, bryant_relprev_map, replace_type::Monotone);
          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("excludes deadlocks [{__} + K&D Fig. 9]", [&]() {
          const bdd out = bdd_relprev(true, kalin_fig9, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("has no deadlocks [{__} + H&R Fig. 6.28]", [&]() {
          const bdd out = bdd_relprev(true, huth_fig28, huth_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has no deadlocks [{__} + B Fig. 18]", [&]() {
          const bdd out =
            bdd_relprev(true, bryant_fig18, bryant_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has no predecessors for empty set of states [<true>]", [&]() {
          const bdd out = bdd_relprev(false, true, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors for empty set of states [K&D Fig. 9]", [&]() {
          const bdd out = bdd_relprev(false, kalin_fig9, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors for empty set of states [H&R Fig. 6.28]", [&]() {
          const bdd out = bdd_relprev(false, huth_fig28, huth_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors for empty set of states [B Fig. 6.28]", [&]() {
          const bdd out =
            bdd_relprev(false, bryant_fig18, bryant_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
          const bdd out =
            bdd_relprev(kalin_fig11, false, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors for empty set of edges [~<false> + <false>]", [&]() {
          const bdd F   = false;
          const bdd out = bdd_relprev(bdd_not(F), F, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
        });

        it("has no predecessors for empty set of nodes and edges [<false>]", [&]() {
          const bdd F   = false;
          const bdd out = bdd_relprev(F, F, kalin_relprev_map, replace_type::Monotone);

          AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
          AssertThat(out.is_negated(), Is().False());
        });

        it("has all predecessors for total graph [K&D Fig. 11 + <true>]", [&]() {
          const bdd out = bdd_relprev(kalin_fig11, true, kalin_relprev_map, replace_type::Monotone);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("has all predecessors for all nodes and total graph [<true>]", [&]() {
          const bdd T   = true;
          const bdd out = bdd_relprev(T, T, kalin_relprev_map, replace_type::Monotone);

          AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
          AssertThat(out.is_negated(), Is().False());
        });
      });

    describe("bdd_relprev(const bdd&, const bdd&, <exists + replace>, replace_type::Shift)", [&]() {
      it("has single predecessor [{10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_10, kalin_fig9, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{01} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_01, huth_fig28, huth_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

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

      it("has single predecessor [{00} + B Fig. 18 (x:=0)]", [&]() {
        const bdd out =
          bdd_relprev(bryant_00, bryant_fig18_x0, bryant_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no predecessors [{00} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_00, kalin_fig9, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors [{01} + B Fig. 18]", [&]() {
        const bdd out =
          bdd_relprev(bryant_01, bryant_fig18, bryant_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{01} + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprev(kalin_01, kalin_fig7_b, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{10} + B Fig. 18 (x:=1)]", [&]() {
        const bdd out =
          bdd_relprev(bryant_10, bryant_fig18_x1, bryant_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has multiple predecessors [{01} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_01, kalin_fig9, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has multiple predecessors [{00} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_00, huth_fig28, huth_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has multiple predecessors [{00} + B Fig. 18]", [&]() {
        const bdd out =
          bdd_relprev(bryant_00, bryant_fig18, bryant_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has predecessors of all current states [{01,10} + K&D Fig. 9]", [&]() {
        const bdd out =
          bdd_relprev(kalin_01_10, kalin_fig9, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [H&R Fig.6.26(a) + H&R Fig. 6.28]", [&]() {
        const bdd out =
          bdd_relprev(huth_fig26_a, huth_fig28, huth_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [{00,10} + B Fig. 18 (x:=0)]", [&]() {
        const bdd out =
          bdd_relprev(bryant_00_10, bryant_fig18, bryant_relprev_map, replace_type::Shift);
        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("excludes deadlocks [{__} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(true, kalin_fig9, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has no deadlocks [{__} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(true, huth_fig28, huth_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no deadlocks [{__} + B Fig. 18]", [&]() {
        const bdd out = bdd_relprev(true, bryant_fig18, bryant_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no predecessors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relprev(false, true, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(false, kalin_fig9, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(false, huth_fig28, huth_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [B Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(false, bryant_fig18, bryant_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
        const bdd out = bdd_relprev(kalin_fig11, false, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprev(bdd_not(F), F, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprev(F, F, kalin_relprev_map, replace_type::Shift);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all predecessors for total graph [K&D Fig. 11 + <true>]", [&]() {
        const bdd out = bdd_relprev(kalin_fig11, true, kalin_relprev_map, replace_type::Shift);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all predecessors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relprev(T, T, kalin_relprev_map, replace_type::Shift);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });
    });

    describe("bdd_relprev(const bdd&, const bdd&, const bdd::label_type)", [&]() {
      it("has single predecessor [{01} + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relprev(kalin_01, kalin_fig7_a, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10} + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprev(kalin_10, kalin_fig7_b, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has single predecessor [{10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_10, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no predecessors [{00} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_00, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{10} + K&D Fig. 7(a)]", [&]() {
        const bdd out = bdd_relprev(kalin_10, kalin_fig7_a, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors of non-current states [{01} + K&D Fig. 7(b)]", [&]() {
        const bdd out = bdd_relprev(kalin_01, kalin_fig7_b, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has multiple predecessors [{01} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_01, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has predecessors of all current states [{00,10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_00_01, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("has predecessors of all current states [{01,10} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_01_10, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [K&D Fig. 11 + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(kalin_fig11, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

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

      it("excludes deadlocks [{__} + K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(true, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has no predecessors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relprev(false, true, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [K&D Fig. 9]", [&]() {
        const bdd out = bdd_relprev(false, kalin_fig9, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [K&D Fig. 11 + <false>]", [&]() {
        const bdd out = bdd_relprev(kalin_fig11, false, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprev(bdd_not(F), F, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprev(F, F, 2);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all predecessors for total graph [K&D Fig. 11 + <true>]", [&]() {
        const bdd out = bdd_relprev(kalin_fig11, true, 2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all predecessors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relprev(T, T, 2);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });
    });

    describe("bdd_relprev(const bdd&, const bdd&)", [&]() {
      it("has single predecessor [{01} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_01, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

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

      it("has single predecessor [{10} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_10, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has multiple predecessors [{00} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_00, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_T, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [H&R Fig.6.26(a) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_fig26_a, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [H&R Fig.6.26(b) + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_fig26_b, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("has predecessors of all current states [{00,01} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(huth_00_01, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no deadlocks [{__} + H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(true, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has no predecessors for empty set of states [<true>]", [&]() {
        const bdd out = bdd_relprev(false, true);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of states [H&R Fig. 6.28]", [&]() {
        const bdd out = bdd_relprev(false, huth_fig28);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [H&R Fig. 26(b) + <false>]", [&]() {
        const bdd out = bdd_relprev(huth_fig26_b, false);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of edges [~<false> + <false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprev(bdd_not(F), F);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("has no predecessors for empty set of nodes and edges [<false>]", [&]() {
        const bdd F   = false;
        const bdd out = bdd_relprev(F, F);

        AssertThat(out.file_ptr(), Is().EqualTo(F.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });

      it("has all predecessors for total graph [H&R Fig. 26 (b) + <true>]", [&]() {
        const bdd out = bdd_relprev(huth_fig26_b, true);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("has all predecessors for all nodes and total graph [<true>]", [&]() {
        const bdd T   = true;
        const bdd out = bdd_relprev(T, T);

        AssertThat(out.file_ptr(), Is().EqualTo(T.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
      });
    });
  });
});
