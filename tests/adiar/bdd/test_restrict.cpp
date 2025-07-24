#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/restrict.cpp", []() {
    const ptr_uint64 terminal_T(true);
    const ptr_uint64 terminal_F(false);

    shared_levelized_file<bdd::node_type> bdd_F;
    /*
    //          F
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_F);
      nw << node(false);
    }

    shared_levelized_file<bdd::node_type> bdd_T;
    /*
    //          T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_T);
      nw << node(true);
    }

    shared_levelized_file<bdd::node_type> bdd_1;
    /*
    //             1               ---- x0
    //            / \
    //            | 2              ---- x1
    //            |/ \
    //            3   4            ---- x2
    //           / \ / \
    //           F T T 5           ---- x3
    //                / \
    //                F T
    */

    const node n1_5(3, 0, terminal_F, terminal_T);
    const node n1_4(2, 1, terminal_T, n1_5.uid());
    const node n1_3(2, 0, terminal_F, terminal_T);
    const node n1_2(1, 0, n1_3.uid(), n1_4.uid());
    const node n1_1(0, 0, n1_3.uid(), n1_2.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_1);
      nw << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    shared_levelized_file<bdd::node_type> bdd_2_high_F;
    /*
    //                 1             ---- x1
    //                / \
    //                2 F            ---- x2
    //               / \
    //               F T
    */

    { // Garbage collect writer to free write-lock
      const node n2_2(2, node::max_id, terminal_F, terminal_T);
      const node n2_1(1, node::max_id, n2_2.uid(), terminal_F);

      node_ofstream nw(bdd_2_high_F);
      nw << n2_2 << n2_1;
    }

    shared_levelized_file<bdd::node_type> bdd_2_low_T;
    /*
    //                 1              ---- x0
    //                / \
    //                T |             ---- x1
    //                  |
    //                  2             ---- x2
    //                 / \
    //                 F T
    */

    { // Garbage collect writer to free write-lock
      const node n2_2(2, node::max_id, terminal_F, terminal_T);
      const node n2_1(0, node::max_id, terminal_T, n2_2.uid());

      node_ofstream nw(bdd_2_low_T);
      nw << n2_2 << n2_1;
    }

    shared_levelized_file<bdd::node_type> bdd_3;
    /*
    //                    1            ---- x0
    //                   / \
    //                  2   3          ---- x1
    //                 / \ / \
    //                 4 F 5 F         ---- x2
    //                / \ / \
    //                T F F T
    */
    const node n3_5 = node(2, 1, terminal_F, terminal_T);
    const node n3_4 = node(2, 0, terminal_T, terminal_F);
    const node n3_3 = node(1, 1, n3_5.uid(), terminal_F);
    const node n3_2 = node(1, 0, n3_4.uid(), terminal_F);
    const node n3_1 = node(0, 0, n3_2.uid(), n3_3.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_3);
      nw << n3_5 << n3_4 << n3_3 << n3_2 << n3_1;
    }

    shared_levelized_file<bdd::node_type> bdd_4;
    /*
    //                    1             ---- x0
    //                   / \
    //                  2   3           ---- x1
    //                 / \ / \
    //                 4 F T F          ---- x2
    //                / \
    //                T F
    */
    const node n4_4(2, 0, terminal_T, terminal_F);
    const node n4_3(1, 1, terminal_T, terminal_F);
    const node n4_2(1, 0, n4_4.uid(), terminal_F);
    const node n4_1(0, 0, n4_2.uid(), n4_3.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_4);
      nw << n4_4 << n4_3 << n4_2 << n4_1;
    }

    /*
    //                   _1_              ---- x0
    //                  /   \
    //                 2     3            ---- x1
    //                / \   / \
    //               4  5   6  7          ---- x2
    //              / \/ \ / \/ \
    //              T F  8 F  9 T         ---- x3
    //                  / \  / \
    //                  F T  T F
    //
    //                 Here, node 4 and 6 are going to be dead, when x1 -> T.
    */

    shared_levelized_file<bdd::node_type> bdd_5;

    const node n5_9(3, 1, terminal_T, terminal_F);
    const node n5_8(3, 0, terminal_F, terminal_T);
    const node n5_7(2, 3, n5_9.uid(), terminal_T);
    const node n5_6(2, 2, terminal_T, n5_9.uid());
    const node n5_5(2, 1, terminal_F, n5_8.uid());
    const node n5_4(2, 0, terminal_T, terminal_F);
    const node n5_3(1, 1, n5_6.uid(), n5_7.uid());
    const node n5_2(1, 0, n5_4.uid(), n5_5.uid());
    const node n5_1(0, 0, n5_2.uid(), n5_3.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_5);
      nw << n5_9 << n5_8 << n5_7 << n5_6 << n5_5 << n5_4 << n5_3 << n5_2 << n5_1;
    }

    describe("bdd_restrict(const bdd&, const generator<...>&)", [&]() {
      // TODO
    });

    describe("bdd_restrict(const bdd&, IT, IT)", [&]() {
      it("bridges levels in BDD 1 for (_,T,T,_)", [&]() {
        /*
        //                 1      ---- x0
        //                / \
        //                T  \     ---- x1
        //                   |
        //                   |
        //                   |
        //                   5    ---- x3
        //                  / \
        //                  F T
        */

        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 1, true }, { 2, true } };

        __bdd out = bdd_restrict(exec_policy(), bdd_1, ass.begin(), ass.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), true, n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_1.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), true, terminal_T }));

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("removes root of BDD 1 for (T,_,_,F)", [&]() {
        /*
        //                  2     ---- x1
        //                 / \
        //                /   \
        //                3   4   ---- x2
        //               / \ / \
        //               F T T F
        */

        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 0, true }, { 3, false } };

        __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), false, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), true, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), true, terminal_F }));

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("removes root of shifted BDD 1(+1) for (T,_,_,F)", [&]() {
        /*
        //                  2     ---- x1
        //                 / \
        //                /   \
        //                3   4   ---- x2
        //               / \ / \
        //               F T T F
        */

        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 1, true }, { 4, false } };

        __bdd out = bdd_restrict(bdd(bdd_1, false, +1), ass.begin(), ass.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(2, 0), false, bdd::pointer_type(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(2, 0), true, bdd::pointer_type(3, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 1), true, terminal_F }));

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("ignores skipped variables in BDD 1 for (F,T,_,F)", [&]() {
        /*
        //                 3      ---- x2
        //                / \
        //                F T
        */

        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 0, false },
                                                                { 1, true },
                                                                { 3, false } };

        __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), true, terminal_T }));

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("returns F terminal in BDD 1 for (F,_,F,_)", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 0, false }, { 2, false } };

        __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("returns T terminal in BDD 1 for (T,T,F,_)", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 0, true },
                                                                { 1, true },
                                                                { 2, false } };

        __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("returns terminal for restricted root in BDD 2 [F]", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 1, true }, { 2, true } };

        __bdd out = bdd_restrict(bdd_2_high_F, ass.begin(), ass.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("returns terminal for restricted root in BDD 2 [T]", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 0, false }, { 2, false } };

        __bdd out = bdd_restrict(bdd_2_low_T, ass.begin(), ass.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("returns input unchanged when given a T terminal", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 0, true },
                                                                { 2, true },
                                                                { 42, false } };

        __bdd out = bdd_restrict(bdd_T, ass.begin(), ass.end());

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_T));
        AssertThat(out._negate, Is().False());
      });

      it("returns input unchanged when given a F terminal", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass{ { 2, true },
                                                             { 21, true },
                                                             { 28, false } };

        __bdd out = bdd_restrict(bdd_F, ass.begin(), ass.end());

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_F));
        AssertThat(out._negate, Is().False());
      });

      it("returns input unchanged when given an empty assignment", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass;

        __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
        AssertThat(out._negate, Is().False());
      });

      it("returns input unchanged if assignment is disjoint of its variables", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 5, false },
                                                                { 6, true },
                                                                { 7, true } };

        __bdd out = bdd_restrict(bdd_1, ass.begin(), ass.end());

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
        AssertThat(out._negate, Is().False());
      });

      it("sorts restricted terminal arc in BDD 4 for (_,F,T,_)", [&]() {
        /*
        //                    _1_       ---- x0
        //                   /   \
        //                   F*  T      ---- x1
        //                                        * This arc will be resolved as the last one
        */

        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 1, false }, { 2, true } };

        __bdd out = bdd_restrict(bdd_4, ass.begin(), ass.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4_1.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("skips 'dead' nodes in BDD 5 for (F,T,_,_)", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 0, false }, { 1, true } };

        __bdd out = bdd_restrict(bdd_5, ass.begin(), ass.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n5_5.uid(), true, n5_8.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_5.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_8.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_8.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("skips 'dead' nodes in BDD 5 for (T,T,_,_)", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 0, true }, { 1, true } };

        __bdd out = bdd_restrict(bdd_5, ass.begin(), ass.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n5_7.uid(), false, n5_9.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_7.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_9.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_9.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });
    });

    describe("bdd_restrict(const bdd&, bdd::label_type, bool)", [&]() {
      it("bridges over a level in BDD 1 for x2 = T", [&]() {
        /*
        //                 1      ---- x0
        //                / \
        //                T 2     ---- x1
        //                 / \
        //                 T |
        //                   |
        //                   5    ---- x3
        //                  / \
        //                  F T
        */

        __bdd out = bdd_restrict(bdd_1, 2, true);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), true, n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), true, n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_1.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_2.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), true, terminal_T }));

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("bridges over levels in BDD 1 for x1 = F", [&]() {
        /*
        //                 1      ---- x0
        //                / \
        //                | |
        //                \ /
        //                 3      ---- x2
        //                / \
        //                F T
        */

        __bdd out = bdd_restrict(bdd_1, 1, false);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), false, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), true, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), true, terminal_T }));

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("bridges over levels in shifted BDD 1(+1) for x2 = F", [&]() {
        /*
        //                 1      ---- x1
        //                / \
        //                | |
        //                \ /
        //                 3      ---- x3
        //                / \
        //                F T
        */

        __bdd out = bdd_restrict(bdd(bdd_1, false, +1), 2, false);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(1, 0), false, bdd::pointer_type(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(1, 0), true, bdd::pointer_type(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 0), true, terminal_T }));

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("bridges over levels in BDD 1 for x1 = T", [&]() {
        /*
        //                  1         ---- x0
        //                 / \
        //                /   \
        //                |   |
        //                3   4       ---- x2
        //               / \ / \
        //               F T T 5      ---- x3
        //                    / \
        //                    F T
        */

        __bdd out = bdd_restrict(bdd_1, 1, true);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), false, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), true, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_4.uid(), true, n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), true, terminal_T }));

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("removes root of BDD 1 for x0 = T", [&]() {
        /*
        //              2        ---- x1
        //             / \
        //            3   4      ---- x2
        //           / \ / \
        //           F T T 5     ---- x3
        //                / \
        //                F T
        */

        __bdd out = bdd_restrict(bdd_1, 0, true);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), false, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), true, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_4.uid(), true, n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("removes root of shifted BDD 1(+1) for x1 = T", [&]() {
        /*
        //              2        ---- x2
        //             / \
        //            3   4      ---- x3
        //           / \ / \
        //           F T T 5     ---- x4
        //                / \
        //                F T
        */

        __bdd out = bdd_restrict(bdd(bdd_1, false, +1), 1, true);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(2, 0), false, bdd::pointer_type(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(2, 0), true, bdd::pointer_type(3, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 1), true, bdd::pointer_type(4, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(4, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(4, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("returns terminal of restricted root [F]", [&]() {
        __bdd out = bdd_restrict(bdd_2_high_F, 1, true);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("returns terminal of restricted root [T]", [&]() {
        __bdd out = bdd_restrict(bdd_2_low_T, 0, false);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("returns input unchanged when given a T terminal", [&]() {
        __bdd out = bdd_restrict(bdd_T, 42, true);

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_T));
        AssertThat(out._negate, Is().False());
      });

      it("returns input unchanged when given a F terminal", [&]() {
        __bdd out = bdd_restrict(bdd_F, 8, false);

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_F));
        AssertThat(out._negate, Is().False());
      });

      it("returns input unchanged if variable does not exist", [&]() {
        std::vector<adiar::pair<bdd::label_type, bool>> ass = { { 5, false },
                                                                { 6, true },
                                                                { 7, true } };

        __bdd out = bdd_restrict(bdd_1, 4, true);

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(bdd_1));
        AssertThat(out._negate, Is().False());
      });

      it("sorts restricted terminal arcs in BDD 3", [&]() {
        /*
        //                     1         ---- x0
        //                   /   \
        //                 _2_    3      ---- x1
        //                /   \  / \
        //                F*  F  T F     ---- x2
        //                                       * This arc will be resolved as the last one
        */

        __bdd out = bdd_restrict(bdd_3, 2, true);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n3_1.uid(), false, n3_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n3_1.uid(), true, n3_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3_2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3_2.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3_3.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3_3.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("should sort restricted terminal arc in BDD 4", [&]() {
        /*
        //                      1         ---- x0
        //                    /   \
        //                  _2_    3      ---- x1
        //                 /   \  / \
        //                 F*  F  T F     ---- x2
        //                                                         * This arc will be resolved as
        the last one
        */

        __bdd out = bdd_restrict(bdd_4, 2, true);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n4_1.uid(), false, n4_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n4_1.uid(), true, n4_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4_2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4_2.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4_3.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4_3.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("skips 'dead' nodes in BDD 5 for x1 = T", [&]() {
        __bdd out = bdd_restrict(bdd_5, 1, true);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n5_1.uid(), false, n5_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n5_1.uid(), true, n5_7.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n5_5.uid(), true, n5_8.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n5_7.uid(), false, n5_9.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_5.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_7.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_8.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_8.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_9.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_9.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });
    });

    describe("bdd_low(const bdd&)", [&]() {
      it("constructs low subtree of BDD 1", [&]() {
        /*
        //            3
        //           / \
        //           F T
        */

        __bdd out = bdd_low(bdd_1);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("constructs low subtree of shifted BDD 1(+2)", [&]() {
        /*
        //            3
        //           / \
        //           F T
        */

        __bdd out = bdd_low(bdd(bdd_1, false, +2));

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(4, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(4, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("returns terminal in BDD 2", [&]() {
        __bdd out = bdd_low(bdd_2_low_T);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("returns terminal in shifted BDD 2(+1)", [&]() {
        __bdd out = bdd_low(bdd(bdd_2_low_T, false, +1));

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("throws an exception when given the F terminal",
         [&]() { AssertThrows(invalid_argument, bdd_low(bdd_F)); });

      it("throws an exception when given the T terminal",
         [&]() { AssertThrows(invalid_argument, bdd_low(bdd_T)); });
    });

    describe("bdd_high(const bdd&)", [&]() {
      it("constructs high subtree of BDD 1", [&]() {
        /*
        //              2        ---- x1
        //             / \
        //            3   4      ---- x2
        //           / \ / \
        //           F T T 5     ---- x3
        //                / \
        //                F T
        */

        __bdd out = bdd_high(bdd_1);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), false, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), true, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_4.uid(), true, n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("constructs high subtree of shifted BDD 1(+2)", [&]() {
        /*
        //              2        ---- x1
        //             / \
        //            3   4      ---- x2
        //           / \ / \
        //           F T T 5     ---- x3
        //                / \
        //                F T
        */

        __bdd out = bdd_high(bdd(bdd_1, false, +2));

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 0), false, bdd::pointer_type(4, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(3, 0), true, bdd::pointer_type(4, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ bdd::pointer_type(4, 1), true, bdd::pointer_type(5, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(4, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(4, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(4, 1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(5, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(),
                   Is().EqualTo(arc{ bdd::pointer_type(5, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("returns terminal in BDD 2", [&]() {
        __bdd out = bdd_high(bdd_2_high_F);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("returns terminal in shifted BDD 2(+1)", [&]() {
        __bdd out = bdd_high(bdd(bdd_2_high_F, false, +1));

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

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

      it("throws an exception when given the F terminal",
         [&]() { AssertThrows(invalid_argument, bdd_high(bdd_F)); });

      it("throws an exception when given the T terminal",
         [&]() { AssertThrows(invalid_argument, bdd_high(bdd_T)); });
    });
  });
});
