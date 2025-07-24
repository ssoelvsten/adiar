#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/change.cpp", [&]() {
    shared_levelized_file<zdd::node_type> zdd_F;
    shared_levelized_file<zdd::node_type> zdd_T;

    { // Garbage collect writers to free write-lock
      node_ofstream nw_F(zdd_F);
      nw_F << node(false);

      node_ofstream nw_T(zdd_T);
      nw_T << node(true);
    }

    const ptr_uint64 terminal_F = ptr_uint64(false);
    const ptr_uint64 terminal_T = ptr_uint64(true);

    shared_levelized_file<zdd::node_type> zdd_x0;
    /*
    //         1              ---- x0
    //        / \
    //        F T
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw_0(zdd_x0);
      nw_0 << node(0, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<zdd::node_type> zdd_x1;
    /*
    //         1              ---- x1
    //        / \
    //        F T
    */

    { // Garbage collect writers to free write-lock
      node_ofstream nw_1(zdd_x1);
      nw_1 << node(1, node::max_id, terminal_F, terminal_T);
    }

    /*
    //            1      ---- x0
    //           / \
    //           2  \    ---- x1
    //          / \ /
    //          3  4     ---- x2
    //         / \/ \
    //         5 T  T    ---- x3
    //        / \
    //        F T
    */
    shared_levelized_file<zdd::node_type> zdd_1;

    const node n1_5 = node(3, node::max_id, terminal_F, terminal_T);
    const node n1_4 = node(2, node::max_id, terminal_T, terminal_T);
    const node n1_3 = node(2, node::max_id - 1, n1_5.uid(), terminal_T);
    const node n1_2 = node(1, node::max_id, n1_3.uid(), n1_4.uid());
    const node n1_1 = node(0, node::max_id, n1_2.uid(), n1_4.uid());

    { // Garbage collect writers to free write-lock
      node_ofstream nw(zdd_1);
      nw << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    describe("zdd_change(A, vars)", [&]() {
      // TODO
    });

    describe("zdd_change(A, begin, end)", [&]() {
      it("returns same file for Ø on empty labels", [&]() {
        const std::vector<int> vars = {};

        __zdd out = zdd_change(zdd_F, vars.begin(), vars.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_F));
      });

      it("returns same file for { Ø } on empty labels", [&]() {
        const std::vector<int> vars = {};

        __zdd out = zdd_change(zdd_T, vars.begin(), vars.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_T));
      });

      it("returns same file for { {1} } on empty labels", [&]() {
        const std::vector<int> vars = {};

        __zdd out = zdd_change(zdd_x1, vars.begin(), vars.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x1));
      });

      it("returns same file for Ø on (1,2)", [&]() {
        const std::vector<int> vars = { 1, 2 };

        __zdd out = zdd_change(zdd_F, vars.begin(), vars.end());
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_F));
      });

      it("returns { {1,2} } for { Ø } on (1,2)", [&]() {
        const std::vector<int> vars = { 1, 2 };

        __zdd out = zdd_change(zdd_T, vars.begin(), vars.end());

        node_test_ifstream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, node::max_id, terminal_F, ptr_uint64(2, ptr_uint64::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
                   Is().EqualTo(1u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_False],
          Is().EqualTo(2u));
        AssertThat(
          out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal_True],
          Is().EqualTo(1u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::All],
                   Is().EqualTo(3u));

        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("adds new root for { {1} } on (0)", [&]() {
        const std::vector<int> vars = { 0 };

        __zdd out = zdd_change(zdd_x1, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("adds new root chain for { {2} } on (0,1)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(2, node::max_id, terminal_T, terminal_T);
        }

        const std::vector<int> vars = { 0, 1 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("adds new nodes after root for { {1} } on (2,3)", [&]() {
        const std::vector<int> vars = { 2, 3 };

        __zdd out = zdd_change(zdd_x1, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("adds a new node before and after root for { {1} } on (0,2)", [&]() {
        const std::vector<int> vars = { 0, 2 };

        __zdd out = zdd_change(zdd_x1, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("adds new nodes before and after root for { {1} } on (0,2,4)", [&]() {
        const std::vector<int> vars = { 0, 2, 4 };

        __zdd out = zdd_change(zdd_x1, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(4, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(4, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(4, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(4u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("adds node between root and child for { {1}, {3} } on (2)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(3, node::max_id, terminal_F, terminal_T)
            << node(1, node::max_id, ptr_uint64(3, ptr_uint64::max_id), terminal_T);
        }

        const std::vector<int> vars = { 2 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("returns { Ø } for { {0} } on (0)", [&]() {
        const std::vector<int> vars = { 0 };

        __zdd out = zdd_change(zdd_x0, vars.begin(), vars.end());

        node_test_ifstream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().False());

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

      it("returns { Ø } for { {1} } on (1)", [&]() {
        const std::vector<int> vars = { 1 };

        __zdd out = zdd_change(zdd_x1, vars.begin(), vars.end());

        node_test_ifstream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().False());

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

      it("shortcuts root for { {0,1} } on (0)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(1, node::max_id, terminal_F, terminal_T)
            << node(0, node::max_id, terminal_F, ptr_uint64(1, ptr_uint64::max_id));
        }

        const std::vector<int> vars = { 0 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("shortcuts root for { {0,2} } on (0,1)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(2, node::max_id, terminal_F, terminal_T)
            << node(0, node::max_id, terminal_F, ptr_uint64(2, ptr_uint64::max_id));
        }

        const std::vector<int> vars = { 0, 1 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("shortcuts root and its child for { {0,2}, {0,2,3} } on (0,2)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(3, node::max_id, terminal_T, terminal_T)
            << node(2, node::max_id, terminal_F, ptr_uint64(3, ptr_uint64::max_id))
            << node(0, node::max_id, terminal_F, ptr_uint64(2, ptr_uint64::max_id));
        }

        std::vector<int> vars = { 0, 2 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("flips root for { Ø, {0} } on (0)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(0, node::max_id, terminal_T, terminal_T);
        }

        const std::vector<int> vars = { 0 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("flips root for { {0}, {1} } on (0)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(1, node::max_id, terminal_F, terminal_T)
            << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);
        }

        const std::vector<int> vars = { 0 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("flips root for [1] on (0)", [&]() {
        const std::vector<int> vars = { 0 };

        __zdd out = zdd_change(zdd_1, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(2, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

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

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(4u));
      });

      it("flips node in the middle for { Ø, {0}, {1,2} } on (1)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(2, node::max_id, terminal_F, terminal_T)
            << node(1, node::max_id, terminal_T, ptr_uint64(2, ptr_uint64::max_id))
            << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);
        }

        const std::vector<int> vars = { 1 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("flips and adds a node for [1] on (1)", [&]() {
        const std::vector<int> vars = { 1 };

        __zdd out = zdd_change(zdd_1, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 1), true, ptr_uint64(2, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(4u));
      });

      it("collapses to a terminal for { {0,1} } on (0,1)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(1, node::max_id, terminal_F, terminal_T)
            << node(0, node::max_id, terminal_F, ptr_uint64(1, ptr_uint64::max_id));
        }

        const std::vector<int> vars = { 0, 1 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        node_test_ifstream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().False());

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

      it("bridges over a deleted node for { {0,1,2} } on (1)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(2, node::max_id, terminal_F, terminal_T)
            << node(1, node::max_id, terminal_F, ptr_uint64(2, ptr_uint64::max_id))
            << node(0, node::max_id, terminal_F, ptr_uint64(1, ptr_uint64::max_id));
        }

        const std::vector<int> vars = { 1 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("bridges over a deleted node for { {1,2}, {0,1,2} } on (1)", [&]() {
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(2, node::max_id, terminal_F, terminal_T)
            << node(1, node::max_id, terminal_F, ptr_uint64(2, ptr_uint64::max_id))
            << node(0,
                    node::max_id,
                    ptr_uint64(1, ptr_uint64::max_id),
                    ptr_uint64(1, ptr_uint64::max_id));
        }

        const std::vector<int> vars = { 1 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it(
        "only adds a one node when cutting arcs to the same node for { {2}, {0,2} } on (1)", [&]() {
          shared_levelized_file<zdd::node_type> in;

          { // Garbage collect writer to free write-lock
            node_ofstream w(in);
            w << node(2, node::max_id, terminal_F, terminal_T)
              << node(0,
                      node::max_id,
                      ptr_uint64(2, ptr_uint64::max_id),
                      ptr_uint64(2, ptr_uint64::max_id));
          }

          const std::vector<int> vars = { 1 };

          __zdd out = zdd_change(in, vars.begin(), vars.end());

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

      it("bridges node before the last label and a terminal for { {0}, {1} } on (0,1)", [&]() {
        /*
        //        1     ---- x0
        //       / \
        //       2 T    ---- x1
        //      / \
        //      F T
        //
        //   When resoving for (2) the label is at x1, but as it is pruned then the
        //   source is still at x0 from (1). Hence, if one looks at source.label()
        //   rather than the current level then this edge is by mistake placed into
        //   the "cut edge with a new node" queue.
        */
        shared_levelized_file<zdd::node_type> in;

        { // Garbage collect writer to free write-lock
          node_ofstream w(in);
          w << node(1, node::max_id, terminal_F, terminal_T)
            << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);
        }

        const std::vector<int> vars = { 0, 1 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("flips, adds and bridges nodes for [1] on (2,3)", [&]() {
        /*
        //         1      ---- x0
        //        / \
        //        2  \    ---- x1
        //       / \ /
        //       3  4     ---- x2
        //      / \ ||
        //      | F ||            (The F is the shortcutting on 5)
        //      \_ _//
        //        *       ---- x3 (From the T terminal)
        //       / \
        //       F T
        */

        const std::vector<int> vars = { 2, 3 };

        __zdd out = zdd_change(zdd_1, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(2, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 1), false, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 1), true, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

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

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(3u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("correctly connects pre-root chain with skipped root for { { 1 }, { 1,2 } } on (0,1)",
         [&]() {
           shared_levelized_file<zdd::node_type> in;
           /*
           //          1     ---- x1
           //         / \
           //         F 2    ---- x2
           //          / \
           //          T T
           */

           const node n2 = node(2, node::max_id, terminal_T, terminal_T);
           const node n1 = node(1, node::max_id, terminal_F, n2.uid());

           {
             node_ofstream nw(in);
             nw << n2 << n1;
           }

           const std::vector<int> vars = { 0, 1 };

           __zdd out = zdd_change(in, vars.begin(), vars.end());

           arc_test_ifstream arcs(out);

           AssertThat(arcs.can_pull_internal(), Is().True());
           AssertThat(arcs.pull_internal(),
                      Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(2, 0) }));

           AssertThat(arcs.can_pull_internal(), Is().False());

           AssertThat(arcs.can_pull_terminal(), Is().True());
           AssertThat(arcs.pull_terminal(),
                      Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

           AssertThat(arcs.can_pull_terminal(), Is().True());
           AssertThat(arcs.pull_terminal(),
                      Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

           AssertThat(arcs.can_pull_terminal(), Is().True());
           AssertThat(arcs.pull_terminal(),
                      Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

           AssertThat(arcs.can_pull_terminal(), Is().False());

           level_info_test_ifstream levels(out);

           AssertThat(levels.can_pull(), Is().True());
           AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

           AssertThat(levels.can_pull(), Is().True());
           AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

           AssertThat(levels.can_pull(), Is().False());

           AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

           AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                      Is().GreaterThanOrEqualTo(1u));

           AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                      Is().EqualTo(1u));
           AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                      Is().EqualTo(2u));
         });

      it("cuts collapse of root to a terminal for { { 0 } } on (0,1)", [&]() {
        shared_levelized_file<zdd::node_type> in;
        /*
        //          1     ---- x0
        //         / \
        //         F T
        */
        { // Garbage collect writer to free write-lock
          node_ofstream nw(in);
          nw << node(0, node::max_id, terminal_F, terminal_T);
        }

        const std::vector<int> vars = { 0, 1 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("cuts collapse to a terminal for { { 0,1 } } on (0,1,2)", [&]() {
        shared_levelized_file<zdd::node_type> in;
        /*
        //          1     ---- x0
        //         / \
        //         F 2    ---- x1
        //          / \
        //          F T
        */
        { // Garbage collect writer to free write-lock
          node_ofstream nw(in);
          nw << node(1, node::max_id, terminal_F, terminal_T)
             << node(0, node::max_id, terminal_F, ptr_uint64(1, ptr_uint64::max_id));
        }

        const std::vector<int> vars = { 0, 1, 2 };

        __zdd out = zdd_change(in, vars.begin(), vars.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(0u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("keeps pre-root chain despite collapse to a terminal of the root for { { 1 } } on (0,1)",
         [&]() {
           shared_levelized_file<zdd::node_type> in;
           /*
           //          1     ---- x1
           //         / \
           //         F T
           */
           { // Garbage collect writer to free write-lock
             node_ofstream nw(in);
             nw << node(1, node::max_id, terminal_F, terminal_T);
           }

           const std::vector<int> vars = { 0, 1 };

           __zdd out = zdd_change(in, vars.begin(), vars.end());

           arc_test_ifstream arcs(out);

           AssertThat(arcs.can_pull_internal(), Is().False());

           AssertThat(arcs.can_pull_terminal(), Is().True());
           AssertThat(arcs.pull_terminal(),
                      Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

           AssertThat(arcs.can_pull_terminal(), Is().True());
           AssertThat(arcs.pull_terminal(),
                      Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

           AssertThat(arcs.can_pull_terminal(), Is().False());

           level_info_test_ifstream levels(out);

           AssertThat(levels.can_pull(), Is().True());
           AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

           AssertThat(levels.can_pull(), Is().False());

           AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

           AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                      Is().GreaterThanOrEqualTo(0u));

           AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                      Is().EqualTo(1u));
           AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                      Is().EqualTo(1u));
         });
    });
  });
});
