#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/subset.cpp", [&]() {
    shared_levelized_file<zdd::node_type> zdd_F;
    shared_levelized_file<zdd::node_type> zdd_T;

    { // Garbage collect writers to free write-lock
      node_ofstream nw_F(zdd_F);
      nw_F << node(false);

      node_ofstream nw_T(zdd_T);
      nw_T << node(true);
    }

    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    // { {3}, {0,3}, {3,4}, {0,3,4}, {1,2,4}, {1,2,3}, {1,3,4}, {0,1,2,4}, {0,1,2,3}, {0,1,3,4} }
    /*
    //            1      ---- x0
    //            ||
    //            2      ---- x1
    //           / \
    //           | 3     ---- x2
    //           |/ \
    //           4  5    ---- x3
    //          / \/ \
    //          F 6  T   ---- x4
    //           / \
    //           T T
    */
    shared_levelized_file<zdd::node_type> zdd_1;

    node n1_6 = node(4, node::max_id, terminal_T, terminal_T);
    node n1_5 = node(3, node::max_id, n1_6.uid(), terminal_T);
    node n1_4 = node(3, node::max_id - 1, terminal_F, n1_6.uid());
    node n1_3 = node(2, node::max_id, n1_4.uid(), n1_5.uid());
    node n1_2 = node(1, node::max_id, n1_4.uid(), n1_3.uid());
    node n1_1 = node(0, node::max_id, n1_2.uid(), n1_2.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream w(zdd_1);
      w << n1_6 << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    // { Ø, {0}, {0,2}, {1,2}, {0,2,3}, {1,2,3} }
    /*
    //     1       ---- x0
    //    / \
    //    2 |      ---- x1
    //   / \|
    //   T  3      ---- x2
    //     / \
    //     T 4     ---- x3
    //      / \
    //      F T
    */
    shared_levelized_file<zdd::node_type> zdd_2;

    const node n2_4 = node(3, node::max_id, terminal_F, terminal_T);
    const node n2_3 = node(2, node::max_id, terminal_T, n2_4.uid());
    const node n2_2 = node(1, node::max_id, terminal_T, n2_3.uid());
    const node n2_1 = node(0, node::max_id, n2_2.uid(), n2_3.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream w(zdd_2);
      w << n2_4 << n2_3 << n2_2 << n2_1;
    }

    // { {3}, {4}, {1,2} }
    /*
    //      1        ---- x0
    //     / \
    //     | 2       ---- x1
    //     |/ \
    //     3  T      ---- x2
    //    / \
    //    4 T        ---- x3
    //   / \
    //   F T
    */
    shared_levelized_file<zdd::node_type> zdd_2_mirror;

    const node n2m_4 = node(3, node::max_id, terminal_F, terminal_T);
    const node n2m_3 = node(2, node::max_id, n2m_4.uid(), terminal_T);
    const node n2m_2 = node(1, node::max_id, n2m_3.uid(), terminal_T);
    const node n2m_1 = node(0, node::max_id, n2m_3.uid(), n2m_2.uid());

    { // Garbage collect writer to free write-lock
      node_ofstream w(zdd_2_mirror);
      w << n2m_4 << n2m_3 << n2m_2 << n2m_1;
    }

    // { Ø, {1}, {2,3} }
    /*
    //         1       ---- x1
    //        / \
    //        2 T      ---- x2
    //       / \
    //       T 3       ---- x3
    //        / \
    //        F T
    */
    shared_levelized_file<zdd::node_type> zdd_3;

    const node n3_3(3, node::max_id, terminal_F, terminal_T);
    const node n3_2(2, node::max_id, terminal_T, n3_3.uid());
    const node n3_1(1, node::max_id, n3_2.uid(), terminal_T);

    {
      node_ofstream w(zdd_3);
      w << n3_3 << n3_2 << n3_1;
    }

    // { Ø, {1,2} }
    /*
    //           1       ---- x1
    //          / \
    //          T 2      ---- x2
    //           / \
    //           F T
    */
    shared_levelized_file<zdd::node_type> zdd_4;

    const node n4_2(2, node::max_id, terminal_F, terminal_T);
    const node n4_1(1, node::max_id, terminal_T, n4_2.uid());

    {
      node_ofstream nw(zdd_4);
      nw << n4_2 << n4_1;
    }

    // { Ø, {1}, {2}, {1,2} }
    /*
    //        1         ---- x1
    //        ||
    //        2         ---- x2
    //       / \
    //       T T
    */
    shared_levelized_file<zdd::node_type> zdd_5;

    const node n5_2(2, node::max_id, terminal_T, terminal_T);
    const node n5_1(1, node::max_id, n5_2.uid(), n5_2.uid());
    ;

    {
      node_ofstream nw(zdd_5);
      nw << n5_2 << n5_1;
    }

    // { Ø, {1}, {1,2} }
    /*
    //         1       ---- x0
    //        / \
    //        T 2      ---- x2
    //         / \
    //         T T
    */
    shared_levelized_file<zdd::node_type> zdd_6;

    const node n6_2(2, node::max_id, terminal_T, terminal_T);
    const node n6_1(0, node::max_id, terminal_T, n6_2.uid());

    {
      node_ofstream w(zdd_6);
      w << n6_2 << n6_1;
    }

    // { Ø, {0} }
    /*
    //         1       ---- x0
    //        / \
    //        T T
    */
    shared_levelized_file<zdd::node_type> zdd_7;

    const node n7_1(0, node::max_id, terminal_T, terminal_T);

    {
      node_ofstream w(zdd_7);
      w << n7_1;
    }

    describe("zdd_offset(const zdd&, const generator<...>&)", [&]() {
      // TODO
    });

    describe("zdd_offset(const zdd&, IT, IT)", [&]() {
      // TODO: unit tests for behaviour only tested with 'zdd_offset(A, var)'

      it("returns input unchanged when given Ø", [&]() {
        std::vector<int> labels = { 21, 42 };

        __zdd out = zdd_offset(zdd_F, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_F));
      });

      it("returns input unchanged when given { Ø }", [&]() {
        std::vector<int> labels = { 7, 42 };

        __zdd out = zdd_offset(zdd_T, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_T));
      });

      it("returns input unchanged when given empty set of labels", [&]() {
        std::vector<int> labels = {};

        __zdd out = zdd_offset(zdd_1, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_1));
      });

      it("returns input unchanged for [4] without (0,3,4,5,6)", [&]() {
        std::vector<int> labels = { 0, 3, 4, 5, 6 };

        __zdd out = zdd_offset(zdd_4, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_4));
      });

      it("restricts to a terminal in [1] without (0,1,3)", [&]() {
        std::vector<int> labels = { 0, 1, 3 };

        __zdd out = zdd_offset(zdd_1, labels.begin(), labels.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("restricts to a terminal in [3] without (1,2)", [&]() {
        std::vector<int> labels = { 1, 2 };

        __zdd out = zdd_offset(zdd_3, labels.begin(), labels.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("restricts to a (reduced) terminal in [6] without (1,2)", [&]() {
        std::vector<int> labels = { 0, 2 };

        __zdd out = zdd_offset(zdd_6, labels.begin(), labels.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });
    });

    describe("zdd_offset(const zdd&, zdd::label_type)", [&]() {
      it("returns input unchanged when given Ø", [&]() {
        __zdd out = zdd_offset(zdd_F, 42);
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_F));
      });

      it("returns input unchanged when given { Ø }", [&]() {
        __zdd out = zdd_offset(zdd_T, 42);
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_T));
      });

      it("returns input unchanged when given [4] without 0", [&]() {
        __zdd out = zdd_offset(zdd_4, 0);
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_4));
      });

      it("returns { Ø } for [4] without 2", [&]() {
        __zdd out = zdd_offset(zdd_4, 2);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4_1.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4_1.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("returns { Ø, { 1 } } for [5] without 2", [&]() {
        __zdd out = zdd_offset(zdd_5, 2);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_1.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_1.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("returns { Ø, { 2 } } for [5] without 1", [&]() {
        __zdd out = zdd_offset(zdd_5, 1);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_2.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_2.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("returns { Ø, { 2 } } for [5] without 1", [&]() {
        __zdd out = zdd_offset(zdd_5, 1);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_2.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5_2.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("skips 'dead' nodes in [1] without 1", [&]() {
        __zdd out = zdd_offset(zdd_1, 1);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), true, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_4.uid(), true, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("restricts to a (reduced) terminal in [6] without 1", [&]() {
        __zdd out = zdd_offset(zdd_6, 0);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("bridges levels in [2] without 3", [&]() {
        __zdd out = zdd_offset(zdd_2, 3);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_1.uid(), false, n2_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_1.uid(), true, n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_2.uid(), true, n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_2.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_3.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_3.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });
    });

    describe("zdd_offset(const zdd&)", [&]() {
      it("skips root of [1]", [&]() {
        __zdd out = zdd_offset(zdd_1);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), true, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_3.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_3.uid(), true, n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_4.uid(), true, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_5.uid(), false, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("skips 'dead' nodes of [2 mirrored]", [&]() {
        __zdd out = zdd_offset(zdd_2_mirror);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2m_3.uid(), false, n2m_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2m_3.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2m_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2m_4.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("throws an exception when given the F terminal",
         [&]() { AssertThrows(invalid_argument, zdd_offset(zdd_F)); });

      it("throws an throw exception when given the T terminal",
         [&]() { AssertThrows(invalid_argument, zdd_offset(zdd_T)); });
    });

    describe("zdd_onset(const zdd&, const generator<...>&)", [&]() {
      // TODO
    });

    describe("zdd_onset(const zdd&, IT, IT)", [&]() {
      // TODO: unit tests for behaviour only tested with 'zdd_onset(A, var)'

      it("returns input unchanged when given Ø", [&]() {
        std::vector<int> labels = { 21, 42 };

        __zdd out = zdd_onset(zdd_F, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_F));
      });

      it("returns input unchanged when given { Ø } for ()", [&]() {
        std::vector<int> labels = {};

        __zdd out = zdd_onset(zdd_T, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_T));
      });

      it("returns input unchanged when given [1] for ()", [&]() {
        std::vector<int> labels = {};

        __zdd out = zdd_onset(zdd_1, labels.begin(), labels.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_1));
      });

      it("returns Ø when given { Ø } for (21,42)", [&]() {
        std::vector<int> labels = { 21, 42 };

        __zdd out = zdd_onset(zdd_T, labels.begin(), labels.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("returns Ø when given disjoint labels", [&]() {
        std::vector<int> labels = { 5, 6 };

        __zdd out = zdd_onset(zdd_1, labels.begin(), labels.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("returns Ø when given { Ø, { 0 } } for (0,1)", [&]() {
        std::vector<int> labels = { 0, 1 };

        __zdd out = zdd_onset(zdd_7, labels.begin(), labels.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("returns { Ø } in [2] for (0,2,3)", [&]() {
        // One would normally expect this to return { Ø }, but for zdd_onset
        // this is not the case!
        std::vector<int> labels = { 0, 2, 3 };

        __zdd out = zdd_onset(zdd_2, labels.begin(), labels.end());

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_1.uid(), true, n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_3.uid(), true, n2_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_4.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("returns Ø in [2] for (0,2,3,4)", [&]() {
        // Similar to above, while it inevitably will be Ø, the top-down nature
        // of zdd_onset cannot know this before having output nodes. The later
        // Reduce will put it to this.
        //
        // Yet, it can still skip the bottom-most node (4) as it has not yet
        // output in-going arcs.

        std::vector<int> labels = { 0, 2, 3, 4 };

        __zdd out = zdd_onset(zdd_2, labels.begin(), labels.end());

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_1.uid(), true, n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_3.uid(), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("skips 'dead' nodes of [1] for (1,2)", [&]() {
        std::vector<int> labels = { 1, 2 };

        __zdd out = zdd_onset(zdd_1, labels.begin(), labels.end());

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), false, n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), true, n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), true, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_3.uid(), true, n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_5.uid(), false, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("skips root in [2] due to cut on high edge for (1,3)", [&]() {
        std::vector<int> labels = { 1 };

        __zdd out = zdd_onset(zdd_2, labels.begin(), labels.end());

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_2.uid(), true, n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_3.uid(), true, n2_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_3.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_4.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("cuts high edge on restricted node, if it goes past the next label", [&]() {
        /*
                   _1_       ---- x0
                  /   \
                  2   3      ---- x1
                 / \ / \
                 T  |  4     ---- x2
                    \ / \
                     5  6    ---- x3
                    / \/ \
                    F T  T
        */
        shared_levelized_file<zdd::node_type> in;

        node n6 = node(3, node::max_id, terminal_T, terminal_T);
        node n5 = node(3, node::max_id - 1, terminal_F, terminal_T);
        node n4 = node(2, node::max_id, n5.uid(), n6.uid());
        node n3 = node(1, node::max_id, n5.uid(), n4.uid());
        node n2 = node(1, node::max_id - 1, terminal_T, n5.uid());
        node n1 = node(0, node::max_id, n2.uid(), n3.uid());

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << n6 << n5 << n4 << n3 << n2 << n1;
        }

        std::vector<int> labels = { 1, 2 };

        __zdd out = zdd_onset(in, labels.begin(), labels.end());

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), true, n3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n3.uid(), true, n4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n4.uid(), true, n6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n6.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });
    });

    describe("zdd_onset(const zdd&, zdd::label_type)", [&]() {
      it("returns input unchanged when given Ø", [&]() {
        __zdd out = zdd_onset(zdd_F, 0);
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_F));
      });

      it("returns Ø when given { Ø } for 0", [&]() {
        __zdd out = zdd_onset(zdd_T, 0);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("returns Ø when given non-existent label (after)", [&]() {
        __zdd out = zdd_onset(zdd_1, 5);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("returns Ø when given non-existent label (in-between)", [&]() {
        __zdd out = zdd_onset(zdd_6, 1);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("returns { { 0 } } when given [7] with 0", [&]() {
        __zdd out = zdd_onset(zdd_7, 0);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n7_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n7_1.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("returns Ø in { Ø, {0}, {0,2} } with 1", [&]() {
        /*
        //     1       ---- x0
        //    / \
        //    T |      ---- x1
        //      |
        //      2      ---- x2
        //     / \
        //     T T
        */
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(2, node::max_id, terminal_T, terminal_T)
            << node(0, node::max_id, terminal_T, ptr_uint64(2, ptr_uint64::max_id));
        }

        __zdd out = zdd_onset(in, 1);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream meta_arcs(out);
        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(0u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(0u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(1u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(0u));
      });

      it("cuts edge going across onset label in { {2}, {0,1}, {0,2}, {0,1,2} } with 1", [&]() {
        /*
        //     1       ---- x0
        //    / \
        //    |  2     ---- x1
        //    \ //
        //     3       ---- x2
        //    / \
        //    F T
        */
        shared_levelized_file<zdd::node_type> in;

        const node n3 = node(2, node::max_id, terminal_F, terminal_T);
        const node n2 = node(1, node::max_id, n3.uid(), n3.uid());
        const node n1 = node(0, node::max_id, n3.uid(), n2.uid());

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << n3 << n2 << n1;
        }

        __zdd out = zdd_onset(in, 1);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), true, n2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2.uid(), true, n3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("cuts edge and ignore 'dead' node in { {2}, {0,1}, {0,2} } with 1", [&]() {
        /*
        //       1       ---- x0
        //      / \
        //      |  2     ---- x1
        //      \ / \
        //       3  T    ---- x2
        //      / \
        //      F T
        */
        shared_levelized_file<zdd::node_type> in;

        const node n3 = node(2, node::max_id, terminal_F, terminal_T);
        const node n2 = node(1, node::max_id, n3.uid(), terminal_T);
        const node n1 = node(0, node::max_id, n3.uid(), n2.uid());

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << n3 << n2 << n1;
        }

        __zdd out = zdd_onset(in, 1);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), true, n2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("falsifies early terminals in [2] for (3)", [&]() {
        std::vector<int> labels = { 3 };

        __zdd out = zdd_onset(zdd_2, labels.begin(), labels.end());

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_1.uid(), false, n2_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_1.uid(), true, n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_2.uid(), true, n2_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_3.uid(), true, n2_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_2.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_3.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_4.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("falsifies early terminal and bridge over removed node in [1] with 4", [&]() {
        __zdd out = zdd_onset(zdd_1, 4);

        arc_test_ifstream arcs(out);
        AssertThat(arcs.can_pull_internal(), Is().True());

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), false, n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), true, n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), true, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_3.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_3.uid(), true, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_4.uid(), true, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });
    });

    describe("zdd_onset(const zdd&)", [&]() {
      it("keeps root but shortcut its low for [1]", [&]() {
        __zdd out = zdd_onset(zdd_1);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_1.uid(), true, n1_2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), true, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_2.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_3.uid(), false, n1_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_3.uid(), true, n1_5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_4.uid(), true, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1_5.uid(), false, n1_6.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_5.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n1_6.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 2u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("skips 'dead' nodes for [2]", [&]() {
        __zdd out = zdd_onset(zdd_2);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_1.uid(), true, n1_3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2_3.uid(), true, n2_4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_1.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_3.uid(), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_4.uid(), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2_4.uid(), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream meta_arcs(out);

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().True());
        AssertThat(meta_arcs.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(meta_arcs.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("throws an exception when given the F terminal",
         [&]() { AssertThrows(invalid_argument, zdd_onset(zdd_F)); });

      it("throws an throw exception when given the T terminal",
         [&]() { AssertThrows(invalid_argument, zdd_onset(zdd_T)); });
    });
  });
});
