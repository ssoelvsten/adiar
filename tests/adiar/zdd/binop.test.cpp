#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/binop.cpp", []() {
    shared_levelized_file<zdd::node_type> zdd_F;
    /*
    //          F
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(zdd_F);
      nw << node(false);
    }

    shared_levelized_file<zdd::node_type> zdd_T;
    /*
    //          T
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(zdd_T);
      nw << node(true);
    }

    const zdd::pointer_type terminal_F(false);
    const zdd::pointer_type terminal_T(true);

    shared_levelized_file<zdd::node_type> zdd_x0;
    /*
    //            1          ---- x0
    //           / \
    //           F T
    */
    { // Garbage collect writers early
      node_ofstream nw(zdd_x0);
      nw << node(0, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<zdd::node_type> zdd_x1;
    /*
    //            1          ---- x1
    //           / \
    //           F T
    */
    { // Garbage collect writers early
      node_ofstream nw(zdd_x1);
      nw << node(1, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<zdd::node_type> zdd_x1_T;
    /*
    //            1          ---- x1
    //           / \
    //           T T
    */
    { // Garbage collect writers early
      node_ofstream nw(zdd_x1_T);
      nw << node(1, node::max_id, terminal_T, terminal_T);
    }

    shared_levelized_file<zdd::node_type> zdd_x0x1_x1;
    /*
    //            1          ---- x0
    //           / \
    //           \ /
    //            2          ---- x1
    //           / \
    //           F T
    */
    { // Garbage collect writers early
      const node n2(1, node::max_id, terminal_F, terminal_T);
      const node n1(0, node::max_id, n2.uid(), n2.uid());

      node_ofstream nw(zdd_x0x1_x1);
      nw << n2 << n1;
    }

    shared_levelized_file<zdd::node_type> zdd_x0_x1;
    /*
    //            1          ---- x0
    //           / \
    //           2 T        ---- x1
    //          / \
    //          F T
    */
    { // Garbage collect writers early
      const node n2(1, node::max_id, terminal_F, terminal_T);
      const node n1(0, node::max_id, n2.uid(), terminal_T);

      node_ofstream nw(zdd_x0_x1);
      nw << n2 << n1;
    }

    describe("simple cases without access mode requirements", [&]() {
      describe("zdd_union", [&]() {
        it("should shortcut Ø on same file", [&]() {
          __zdd out = zdd_union(zdd_F, zdd_F);
          AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_F));
        });

        it("should shortcut { Ø } on same file", [&]() {
          __zdd out = zdd_union(zdd_T, zdd_T);
          AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_T));
        });

        it("should shortcut { {0} } on same file", [&]() {
          __zdd out = zdd_union(zdd_x0, zdd_x0);
          AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x0));
        });

        it("should shortcut { {1} } on same file", [&]() {
          __zdd out = zdd_union(zdd_x1, zdd_x1);
          AssertThat(out.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x1));
        });

        it("computes Ø U { {Ø} }", [&]() {
          __zdd out = zdd_union(zdd_F, zdd_T);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes { Ø } U Ø", [&]() {
          __zdd out = zdd_union(zdd_T, zdd_F);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream ms(out);
          AssertThat(ms.can_pull(), Is().False());

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("should shortcut on irrelevance for { {0} } U Ø", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          F T
          */

          __zdd out_1 = zdd_union(zdd_x0, zdd_F);
          AssertThat(out_1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x0));

          __zdd out_2 = zdd_union(zdd_F, zdd_x0);
          AssertThat(out_2.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x0));
        });
      });

      describe("zdd_intsec", [&]() {
        it("should shortcut on same file", [&]() {
          __zdd out_1 = zdd_intsec(zdd_x0, zdd_x0);
          AssertThat(out_1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x0));

          __zdd out_2 = zdd_intsec(zdd_x1, zdd_x1);
          AssertThat(out_2.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x1));
        });

        it("computes Ø ∩ { {Ø} }", [&]() {
          __zdd out = zdd_intsec(zdd_F, zdd_T);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes { Ø } ∩ Ø", [&]() {
          __zdd out = zdd_intsec(zdd_T, zdd_F);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes (and shortcut) { {0} } ∩ Ø", [&]() {
          /*
          //           1       F              F          ---- x0
          //          / \           ==>
          //          F T
          */

          __zdd out = zdd_intsec(zdd_x0, zdd_F);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes (and shortcut) Ø ∩ { {0} }", [&]() {
          __zdd out = zdd_intsec(zdd_F, zdd_x0);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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
      });

      describe("zdd_diff", [&]() {
        it("should shortcut to Ø on same file for { {0} }", [&]() {
          __zdd out = zdd_diff(zdd_x0, zdd_x0);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("should shortcut to Ø on same file for { {1} }", [&]() {
          __zdd out = zdd_diff(zdd_x1, zdd_x1);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes { Ø } \\ Ø", [&]() {
          __zdd out = zdd_diff(zdd_T, zdd_F);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes Ø \\ { Ø }", [&]() {
          __zdd out = zdd_diff(zdd_F, zdd_T);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("should shortcut on irrelevance on { {0} } \\ Ø", [&]() {
          __zdd out_1 = zdd_diff(zdd_x0, zdd_F);
          AssertThat(out_1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x0));
        });

        it("should shortcut on irrelevance on { {1} } \\ Ø", [&]() {
          __zdd out_2 = zdd_diff(zdd_x1, zdd_F);
          AssertThat(out_2.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(zdd_x1));
        });

        it("computes (and shortcut) Ø  \\ { {0} }", [&]() {
          __zdd out = zdd_intsec(zdd_F, zdd_x0);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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
      });
    });

    describe("access mode: priority queues", [&]() {
      const exec_policy ep = exec_policy::access::Priority_Queue;

      describe("zdd_union", [&]() {
        it("computes { {0} } U { Ø }", [&]() {
          /*
          //           1     ---- x0
          //          / \
          //          T T
          */

          __zdd out = zdd_union(ep, zdd_x0, zdd_T);

          arc_test_ifstream arcs(out);
          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes { {0} } U { {1} }", [&]() {
          /*
          //             1     ---- x0
          //            / \
          //            2 T    ---- x1
          //           / \
          //           F T
          */

          __zdd out = zdd_union(ep, zdd_x0, zdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

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

        it("computes { {0,1}, {0,3} } U { {0,2}, {2} }", [&]() {
          /*
          //            1           1                (1,1)         ---- x0
          //           / \         / \                 X
          //           F 2         | |            (2,2) \          ---- x1
          //            / \        \ /   ==>      /   \  \
          //            | T         2          (3,2)  T (F,2)      ---- x2
          //            |          / \         /   \     / \
          //            3          F T       (3,F) T     F T       ---- x3
          //           / \                    / \
          //           F T                    F T
          */
          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(3, node::max_id, terminal_F, terminal_T)
                 << node(1, node::max_id, ptr_uint64(3, ptr_uint64::max_id), terminal_T)
                 << node(0, node::max_id, terminal_F, ptr_uint64(1, ptr_uint64::max_id));

            node_ofstream nw_b(zdd_b);
            nw_b << node(2, node::max_id, terminal_F, terminal_T)
                 << node(0,
                         node::max_id,
                         ptr_uint64(2, ptr_uint64::max_id),
                         ptr_uint64(2, ptr_uint64::max_id));
          }

          __zdd out = zdd_union(ep, zdd_a, zdd_b);

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
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));

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
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });

        it("computes { {0,1}, {1} } U { {0,2}, {2} }", [&]() {
          /*
          //           1           1             (1,1)
          //           ||         / \             | |
          //           2          | |            (2,2)
          //          / \         \ /   ==>       / \
          //          F T          2          (F,2) (T,F) <-- since 2nd (2) skipped level
          //                      / \          / \
          //                      F T          F T
          */
          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(1, node::max_id, terminal_F, terminal_T)
                 << node(0,
                         node::max_id,
                         ptr_uint64(1, ptr_uint64::max_id),
                         ptr_uint64(1, ptr_uint64::max_id));

            node_ofstream nw_b(zdd_b);
            nw_b << node(2, node::max_id, terminal_F, terminal_T)
                 << node(0,
                         node::max_id,
                         ptr_uint64(2, ptr_uint64::max_id),
                         ptr_uint64(2, ptr_uint64::max_id));
          }

          __zdd out = zdd_union(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

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
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes { {0}, {1,3}, {2,3}, {1} } U { {0,3}, {3} }", [&]() {
          /*
          //                1        1                        (1,1)         ---- x0
          //               / \      / \                       /   \
          //               2 T      | |                    (2,2)   \        ---- x1
          //              / \       | |                    /   \    \
          //              3  \      | |     ==>         (3,2)  |     \      ---- x2
          //              \\ /      \ /                 /   \  |     |
          //                4        2              (4,2)   (4,F)  (T,2)    ---- x3
          //               / \      / \              / \     / \    / \
          //               F T      F T              F T     T T    T T
          //
          //           The high arc on (2) and (3) on the left is shortcutting the
          //           second ZDD, to compensate for the omitted nodes.
          */
          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(3, node::max_id, terminal_F, terminal_T)
                 << node(2,
                         node::max_id,
                         ptr_uint64(3, ptr_uint64::max_id),
                         ptr_uint64(3, ptr_uint64::max_id))
                 << node(1,
                         node::max_id,
                         ptr_uint64(2, ptr_uint64::max_id),
                         ptr_uint64(3, ptr_uint64::max_id))
                 << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);

            node_ofstream nw_b(zdd_b);
            nw_b << node(3, node::max_id, terminal_F, terminal_T)
                 << node(0,
                         node::max_id,
                         ptr_uint64(3, ptr_uint64::max_id),
                         ptr_uint64(3, ptr_uint64::max_id));
          }

          __zdd out = zdd_union(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 1), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 2), false, terminal_T }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(4u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });
      });

      describe("zdd_intsec", [&]() {
        it("computes { {0} } ∩ { Ø }", [&]() {
          /*
          //           1       T              F       ---- x0
          //          / \           ==>
          //          F T
          */

          __zdd out = zdd_intsec(ep, zdd_x0, zdd_T);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes { Ø, {0} } ∩ { Ø }", [&]() {
          /*
          //           1       T              T       ---- x0
          //          / \           ==>
          //          T T
          */
          shared_levelized_file<zdd::node_type> zdd_a;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(0, node::max_id, terminal_T, terminal_T);
          }

          __zdd out = zdd_intsec(ep, zdd_a, zdd_T);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes { {0}, {1} } ∩ { Ø }", [&]() {
          /*
          //               1        T             F          ---- x0
          //              / \
          //              2 T             ==>                ---- x1
          //             / \
          //             F T
          */

          shared_levelized_file<zdd::node_type> zdd_a;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(1, node::max_id, terminal_F, terminal_T)
                 << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);
          }

          __zdd out = zdd_intsec(ep, zdd_a, zdd_T);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes (and shortcut) { {0,1}, {1} } ∩ { {0,1} }", [&]() {
          /*
          //            _1_        1            1        ---- x0
          //           /   \      / \          / \
          //           2   3      F 2    ==>   F 2       ---- x1
          //          / \ / \      / \          / \
          //          T T F T      F T          F T
          */

          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(1, node::max_id, terminal_F, terminal_T)
                 << node(1, node::max_id - 1, terminal_T, terminal_T)
                 << node(0,
                         node::max_id,
                         ptr_uint64(1, ptr_uint64::max_id - 1),
                         ptr_uint64(1, ptr_uint64::max_id));

            node_ofstream nw_b(zdd_b);
            nw_b << node(1, node::max_id, terminal_F, terminal_T)
                 << node(0, node::max_id, terminal_F, ptr_uint64(1, ptr_uint64::max_id));
          }

          __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));
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

        it("computes (and skip to terminal) { {0}, {1}, {0,1} } ∩ { Ø }", [&]() {
          __zdd out = zdd_intsec(ep, zdd_x0x1_x1, zdd_T);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes (and skip to terminal) { {0,2}, {0}, {2} } \\ { {1}, {2}, Ø }", [&]() {
          /*
          //                1                             F         ---- x0
          //               / \
          //              /   \         _1_                         ---- x1
          //              |   |        /   \       =>
          //              2   3        2   3                        ---- x2
          //             / \ / \      / \ / \
          //             F T T T      T F F T
          //
          //             Where (2) and (3) are swapped in order on the right. Notice,
          //             that (2) on the right technically is illegal, but it makes
          //             for a much simpler counter-example that catches
          //             prod_pq_1.peek() throwing an error on being empty.
          */

          shared_levelized_file<zdd::node_type> zdd_a;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(2, node::max_id, terminal_T, terminal_T)
                 << node(2, node::max_id - 1, terminal_F, terminal_T)
                 << node(0,
                         node::max_id,
                         ptr_uint64(2, ptr_uint64::max_id - 1),
                         ptr_uint64(2, ptr_uint64::max_id));
          }

          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_b(zdd_b);
            nw_b << node(2, node::max_id, terminal_T, terminal_F)
                 << node(2, node::max_id - 1, terminal_F, terminal_T)
                 << node(1,
                         node::max_id,
                         ptr_uint64(2, ptr_uint64::max_id),
                         ptr_uint64(2, ptr_uint64::max_id - 1));
          }

          __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes (and skips in) { {0,1,2}, {0,2}, {0}, {2} } } ∩ { {0,2}, {0}, {1}, {2} }",
           [&]() {
             /*
             //                1             1                 (1,1)      ---- x0
             //               / \           / \                /   \
             //               | _2_        2   \              /     \
             //               \/   \      / \  |     ==>      |     |
             //                3   4      3 T  4            (3,3) (3,4)
             //               / \ / \    / \  / \            / \   / \
             //               T T F T    F T  T T            F T   T T
             //
             //               where (3) and (4) are swapped in order on the left one.
             //
             //               (3,3) : ((2,1), (2,0))   ,   (3,4) : ((2,1), (2,1))
             //
             //               so (3,3) is forwarded while (3,4) is not and hence (3,3) is output
             first.
             */
             shared_levelized_file<zdd::node_type> zdd_a;

             { // Garbage collect writers early
               node_ofstream nw_a(zdd_a);
               nw_a << node(2, node::max_id, terminal_T, terminal_T)
                    << node(2, node::max_id - 1, terminal_F, terminal_T)
                    << node(1,
                            node::max_id,
                            ptr_uint64(2, ptr_uint64::max_id),
                            ptr_uint64(2, ptr_uint64::max_id - 1))
                    << node(0,
                            node::max_id,
                            ptr_uint64(2, ptr_uint64::max_id),
                            ptr_uint64(1, ptr_uint64::max_id));
             }

             shared_levelized_file<zdd::node_type> zdd_b;

             { // Garbage collect writers early
               node_ofstream nw_b(zdd_b);
               nw_b << node(2, node::max_id, terminal_T, terminal_T)
                    << node(2, node::max_id - 1, terminal_F, terminal_T)
                    << node(1, node::max_id, ptr_uint64(2, ptr_uint64::max_id - 1), terminal_T)
                    << node(0,
                            node::max_id,
                            ptr_uint64(1, ptr_uint64::max_id),
                            ptr_uint64(2, ptr_uint64::max_id));
             }

             __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(2, 0) }));
             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(2, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

             AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                        Is().GreaterThanOrEqualTo(2u));

             AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(1u));
             AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(3u));
           });

        it("computes (and skip) { {0}, {1} } ∩ { {0,1} }", [&]() {
          /*
          //            1          1                  (1,1)         ---- x0
          //           / \        / \                 /   \
          //           2 T        F 2                 F   F         ---- x1
          //          / \          / \       ==>
          //          F T          F T
          */

          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(1, node::max_id, terminal_F, terminal_T)
                 << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);

            node_ofstream nw_b(zdd_b);
            nw_b << node(1, node::max_id, terminal_F, terminal_T)
                 << node(0, node::max_id, terminal_F, ptr_uint64(1, ptr_uint64::max_id));
          }

          __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(0u));
        });

        it("computes (and skip) { {0}, {1}, {2}, {1,2}, {0,2} } ∩ { {0}, {2}, {0,2}, {0,1,2} }",
           [&]() {
             /*
             //              1          1                 (1,1)         ---- x0
             //             / \        / \                /   \
             //             2 |        | 2               /     \        ---- x1
             //            / \|        |/ \       ==>    |     |
             //            3  4        3  4            (3,3) (4,3)      ---- x2
             //           / \/ \      / \/ \            / \   / \
             //           F T  T      F T  T            F T   F T
             //
             //           Notice, how (2,3) and (4,2) was skipped on the low and high of (1,1)
             */

             shared_levelized_file<zdd::node_type> zdd_a;
             shared_levelized_file<zdd::node_type> zdd_b;

             { // Garbage collect writers early
               node_ofstream nw_a(zdd_a);
               nw_a << node(2, node::max_id, terminal_T, terminal_T)
                    << node(2, node::max_id - 1, terminal_F, terminal_T)
                    << node(1,
                            node::max_id,
                            ptr_uint64(2, ptr_uint64::max_id - 1),
                            ptr_uint64(2, ptr_uint64::max_id))
                    << node(0,
                            node::max_id,
                            ptr_uint64(1, ptr_uint64::max_id),
                            ptr_uint64(2, ptr_uint64::max_id));

               node_ofstream nw_b(zdd_b);
               nw_b << node(2, node::max_id, terminal_T, terminal_T)
                    << node(2, node::max_id - 1, terminal_F, terminal_T)
                    << node(1,
                            node::max_id,
                            ptr_uint64(2, ptr_uint64::max_id - 1),
                            ptr_uint64(2, ptr_uint64::max_id))
                    << node(0,
                            node::max_id,
                            ptr_uint64(2, ptr_uint64::max_id - 1),
                            ptr_uint64(1, ptr_uint64::max_id));
             }

             __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True());
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(2, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_F }));
             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().False());

             level_info_test_ifstream levels(out);

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

             AssertThat(levels.can_pull(), Is().True());
             AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

             AssertThat(levels.can_pull(), Is().False());

             AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

             AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                        Is().GreaterThanOrEqualTo(2u));

             AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(2u));
             AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(2u));
           });

        it("computes (and skip) { {0}, {1} } ∩ { {1}, {0,2} }", [&]() {
          /*
          //             1         1                (1,1)      ---- x0
          //            / \       / \               /   \
          //            2 T       2  \            (2,2) F      ---- x1
          //           / \       / \ |     =>      / \
          //           F T       F T 3             F T         ---- x2
          //                        / \
          //                        F T
          */

          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(1, node::max_id, terminal_F, terminal_T)
                 << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);

            node_ofstream nw_b(zdd_b);
            nw_b << node(2, node::max_id, terminal_F, terminal_T)
                 << node(1, node::max_id, terminal_F, terminal_T)
                 << node(0,
                         node::max_id,
                         ptr_uint64(1, ptr_uint64::max_id),
                         ptr_uint64(2, ptr_uint64::max_id));
          }

          __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));
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

        it("computes (and skip) { {0,2}, {1,2}, Ø } ∩ { {0,1}, {0}, {1} }", [&]() {
          /*
          //             1         1                    (1,1)    ---- x0
          //            / \       / \                   /   \
          //            2  \      2 T                 (2,2) |    ---- x1
          //           / \ /     / \       =>         /   \ |
          //           T  3      T T                  T   F F    ---- x2
          //             / \
          //             F T
          //
          //           This shortcuts the (3,T) tuple twice.
          */

          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(2, node::max_id, terminal_F, terminal_T)
                 << node(1, node::max_id, terminal_T, ptr_uint64(2, ptr_uint64::max_id))
                 << node(0,
                         node::max_id,
                         ptr_uint64(1, ptr_uint64::max_id),
                         ptr_uint64(2, ptr_uint64::max_id));

            node_ofstream nw_b(zdd_b);
            nw_b << node(1, node::max_id, terminal_T, terminal_T)
                 << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);
          }

          __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_T }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_F }));

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

        it("computes (and shortcut) { {0,2}, {1,2}, Ø } ∩ { {0,2}, {0} }", [&]() {
          /*
          //             1            1               (1,1)     ---- x0
          //            / \          / \               / \
          //            2  \         F |               F |      ---- x1
          //           / \ /           |     ==>         |
          //           T  3            2               (2,3)    ---- x2
          //             / \          / \               / \
          //             F T          T T               F T
          //
          //           This shortcuts the (3,T) tuple twice.
          */

          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(2, node::max_id, terminal_F, terminal_T)
                 << node(1, node::max_id, terminal_T, ptr_uint64(2, ptr_uint64::max_id))
                 << node(0,
                         node::max_id,
                         ptr_uint64(1, ptr_uint64::max_id),
                         ptr_uint64(2, ptr_uint64::max_id));

            node_ofstream nw_b(zdd_b);
            nw_b << node(2, node::max_id, terminal_T, terminal_T)
                 << node(0, node::max_id, terminal_F, ptr_uint64(2, ptr_uint64::max_id));
          }

          __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

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
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));
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
      });

      describe("zdd_diff", [&]() {
        it("computes { {Ø} } \\ { {0} }", [&]() {
          __zdd out = zdd_diff(ep, zdd_T, zdd_x0);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes { {0} } \\ { Ø }", [&]() {
          /*
          //           1      T            1       ---- x0
          //          / \          ==>    / \
          //          F T                 F T
          */
          __zdd out = zdd_diff(ep, zdd_x0, zdd_T);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));
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
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes { {0}, Ø } \\ { Ø }", [&]() {
          /*
          //           1      T            1       ---- x0
          //          / \          ==>    / \
          //          T T                 F T
          */
          shared_levelized_file<zdd::node_type> zdd_a;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(0, node::max_id, terminal_T, terminal_T);
          }

          __zdd out = zdd_diff(ep, zdd_a, zdd_T);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(0u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes (and skip) { {0}, {1} } \\ { {1} }", [&]() {
          /*
          //            1                       (1,1)       ---- x0
          //           / \                      /   \
          //           2 T      1     ==>       F   T       ---- x1
          //          / \      / \
          //          F T      F T
          */
          __zdd out = zdd_diff(ep, zdd_x0_x1, zdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

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
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes (and skip) { {1} } \\ { {0}, {1} }", [&]() {
          /*
          //                  1                        ---- x0
          //                 / \
          //         1       2 T     ==>       F       ---- x1
          //        / \     / \
          //        F T     F T
          */
          __zdd out = zdd_diff(ep, zdd_x1, zdd_x0_x1);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes (and skip) { {0,1}, {1} } \\ { {1}, Ø }", [&]() {
          /*
          //             1                      (1,1)       ---- x0
          //             ||                     /   \
          //             2      1     ==>    (2,1)  F       ---- x1
          //            / \    / \            / \
          //            F T    T T            F T
          */
          __zdd out = zdd_diff(ep, zdd_x0x1_x1, zdd_x1_T);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));
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

        it("computes { {0,1}, {1,2}, {1} } \\ { {1}, Ø }", [&]() {
          /*
          //             _1_                        (1,1)       ---- x0
          //            /   \                       /   \
          //            2   3      1     ==>    (3,1)   (2,F)    ---- x1
          //           / \ / \    / \           /   \    / \
          //           F 4 F T    T T           F (3,T)  F T    ---- x2
          //            / \                        / \
          //            T T                        F T
          */
          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            const node n4(2, node::max_id, terminal_T, terminal_T);
            const node n3(1, node::max_id, terminal_F, terminal_T);
            const node n2(1, node::max_id - 1, terminal_F, n4.uid());
            const node n1(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream nw(zdd_a);
            nw << n4 << n3 << n2 << n1;
          }

          { // Garbage collect writers early
            node_ofstream nw(zdd_b);
            nw << node(1, node::max_id, terminal_T, terminal_T);
          }

          __zdd out = zdd_diff(ep, zdd_a, zdd_b);

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

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), true, terminal_T }));

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
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(2u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });
      });
    });

    describe("access mode: random access", [&]() {
      const exec_policy ep = exec_policy::access::Random_Access;

      describe("zdd_union", [&]() {
        it("computes { {0} } U { Ø, {0} } (same level)", [&]() {
          shared_levelized_file<zdd::node_type> zdd_maybe_x0;
          /*
          //      { Ø, {0} }
          //
          //          1          ---- x0
          //          ||
          //          T
          */

          { // Garbage collect early and free write-lock
            node_ofstream nw(zdd_maybe_x0);
            nw << node(0, node::max_id, terminal_T, terminal_T);
          }

          /*
          // Result of { {0} } U { Ø, {0} }
          //
          //                      (1,1)                   ---- x0
          //                      /   \
          //                  (F,T)   (T,T)
          */

          __zdd out = zdd_union(ep, zdd_x0, zdd_maybe_x0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(0u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes { {0} } U { {1} } (different levels, random access for first level)", [&]() {
          /*
          //             1     ---- x0
          //            / \
          //            2 T    ---- x1
          //           / \
          //           F T
          */

          __zdd out = zdd_union(ep, zdd_x1, zdd_x0);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(1u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

        it("computes { {1} } U { {0} } (different levels, random access for second level)", [&]() {
          /*
          //             1     ---- x0
          //            / \
          //            2 T    ---- x1
          //           / \
          //           F T
          */

          __zdd out = zdd_union(ep, zdd_x0, zdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

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

        shared_levelized_file<zdd::node_type> zdd_thin;
        /*
        //    { {2}, {0, 1}, {0, 2}, {0, 1, 2} }
        //
        //                  1          ---- x0
        //                 / \
        //                2   3        ---- x1
        //                X_ / \
        //               /  X   |
        //               |_/ \__|
        //               4      5      ---- x2
        //              / \    / \
        //              F T    T T
        */
        { // Garbage collect early and free write-lock
          const node n5 = node(2, node::max_id, terminal_T, terminal_T);
          const node n4 = node(2, node::max_id - 1, terminal_F, terminal_T);
          const node n3 = node(1, node::max_id, n4, n5);
          const node n2 = node(1, node::max_id - 1, n5, n4);
          const node n1 = node(0, node::max_id, n2, n3);

          node_ofstream nw(zdd_thin);
          nw << n5 << n4 << n3 << n2 << n1;
        }

        // zdd_thin->canonical == true
        // zdd_thin->width == 2u

        shared_levelized_file<zdd::node_type> zdd_wide;
        /*
        //  { {0}, {2}, {0, 3}, {0, 1, 2}, {1, 2, 3} }
        //
        //                1         ---- x0
        //               / \
        //              2   3       ---- x1
        //              X   X
        //             / \ / \
        //            4   5   6     ---- x2
        //           / \ / \ / \
        //           F 7 F T T T    ---- x3
        //            / \
        //            F T
        */
        { // Garbage collect early and free write-lock
          const node n7 = node(3, node::max_id, terminal_F, terminal_T);
          const node n6 = node(2, node::max_id, terminal_T, terminal_T);
          const node n5 = node(2, node::max_id - 1, terminal_F, terminal_T);
          const node n4 = node(2, node::max_id - 2, terminal_F, n7);
          const node n3 = node(1, node::max_id, n6, n5);
          const node n2 = node(1, node::max_id - 1, n5, n4);
          const node n1 = node(0, node::max_id, n2, n3);

          node_ofstream nw(zdd_wide);
          nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        // zdd_wide->canonical == true
        // zdd_wide->width == 3u

        /*
        // Result of [thin] U [wide]
        //
        //                      (1,1)                   ---- x0
        //                      /   \
        //                  (2,2)   (3,3)               ---- x1
        //                    X       X
        //              _____/ \     / \_____
        //          (4,4)       (5,5)       (4,6)       ---- x2
        //          /   \       /   \       /   \
        //       (F,F) (T,7) (T,F) (T,T) (F,T) (T,T)    ---- x3
        //             /   \
        //          (T,F) (T,T)
        */

        it("should random access on the thinnest ([thin] U [wide])", [&]() {
          __zdd out = zdd_union(ep, zdd_thin, zdd_wide);

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
                     Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });

        it("should random access on the thinnest ([wide] U [thin])", [&]() {
          __zdd out = zdd_union(ep, zdd_thin, zdd_wide);

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
                     Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(6u));
        });

        shared_levelized_file<zdd::node_type> zdd_canon;
        /*
        //  { {0, 1}, {1, 2}, {2, 3}, {0, 1, 2}, {0, 2, 3} }
        //
        //                 1              ---- x0
        //                / \
        //               2   3            ---- x1
        //           ___/ \ / \___
        //          /    __X__    \
        //         4    5     6    7      ---- x2
        //        / \  / \   / \  / \
        //        F 8  F 9   F T  T T     ---- x3
        //         / \  / \
        //         F T  T T
        */

        { // Garbage collect early and free write-lock
          const node n9 = node(3, node::max_id, terminal_T, terminal_T);
          const node n8 = node(3, node::max_id - 1, terminal_F, terminal_T);
          const node n7 = node(2, node::max_id, terminal_T, terminal_T);
          const node n6 = node(2, node::max_id - 1, terminal_F, terminal_T);
          const node n5 = node(2, node::max_id - 2, terminal_F, n9);
          const node n4 = node(2, node::max_id - 3, terminal_F, n8);
          const node n3 = node(1, node::max_id, n5, n7);
          const node n2 = node(1, node::max_id - 1, n4, n6);
          const node n1 = node(0, node::max_id, n2, n3);

          node_ofstream nw(zdd_canon);
          nw << n9 << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        // zdd_canon->canonical == true
        // zdd_canon->width == 4u

        shared_levelized_file<zdd::node_type> zdd_indexable;
        /*
        //  { {0}, {2}, {0, 2}, {1, 3}, {1, 2}, {0, 1, 2} }
        //
        //                1            ---- x0
        //               / \
        //              2   3          ---- x1
        //             / \ _X_
        //             |__X   \
        //             4   5   6       ---- x2
        //            / \ / \ / \
        //            F T 7 T T T      ---- x3
        //               / \
        //               F T
        */

        { // Garbage collect early and free write-lock
          const node n7 = node(3, node::max_id, terminal_F, terminal_T);
          const node n6 = node(2, node::max_id, terminal_T, terminal_T);
          const node n5 = node(2, node::max_id - 1, n7, terminal_T);
          const node n4 = node(2, node::max_id - 2, terminal_F, terminal_T);
          const node n3 = node(1, node::max_id, n6, n4);
          const node n2 = node(1, node::max_id - 1, n4, n5);
          const node n1 = node(0, node::max_id, n2, n3);

          node_ofstream nw(zdd_indexable);
          nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        // zdd_indexable->canonical == false
        // zdd_indexable->width == 3u

        /*
        // Result of [canon] U [indexable]
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
        //    (F,F) (8,T) (T,F) (T,T) (F,7) (T,T) (F,T) (9,T)         ---- x3
        //          /   \             /   \             /   \
        //      (F,T)   (T,T)     (F,F)   (F,T)     (T,T)   (T,T)
        */

        it("should random access on non-canonical but still indexable ([canon] U [indexable])",
           [&]() {
             __zdd out = zdd_union(ep, zdd_canon, zdd_indexable);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (6,5)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 2) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (7,4)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(1, 1), true, ptr_uint64(2, 3) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (9,T)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 1), true, ptr_uint64(3, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (F,7)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 2), false, ptr_uint64(3, 2) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 2), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 3), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 3), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 1), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 1), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 2), false, terminal_F }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 2), true, terminal_T }));

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

             AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

             AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

             AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(2u));
             AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(9u));
           });

        it("should random access on non-canonical but still indexable ([indexable] U [canon])",
           [&]() {
             __zdd out = zdd_union(ep, zdd_indexable, zdd_canon);

             arc_test_ifstream arcs(out);

             AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (6,5)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 2) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (4,7)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(1, 1), true, ptr_uint64(2, 3) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (T,8)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 0) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (T,9)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 1), true, ptr_uint64(3, 1) }));

             AssertThat(arcs.can_pull_internal(), Is().True()); // (7,F)
             AssertThat(arcs.pull_internal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 2), false, ptr_uint64(3, 2) }));

             AssertThat(arcs.can_pull_internal(), Is().False());

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 2), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 3), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(2, 3), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 1), false, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 1), true, terminal_T }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 2), false, terminal_F }));

             AssertThat(arcs.can_pull_terminal(), Is().True());
             AssertThat(arcs.pull_terminal(),
                        Is().EqualTo(arc{ ptr_uint64(3, 2), true, terminal_T }));

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

             AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

             AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

             AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                        Is().EqualTo(2u));
             AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                        Is().EqualTo(9u));
           });

        shared_levelized_file<zdd::node_type> zdd_unindexable;
        /*
        // Same as `zdd_indexable` but the identifiers are messed up to not be indexable
        */

        { // Garbage collect early and free write-lock
          const node n7 = node(3, node::max_id, terminal_F, terminal_T);
          const node n6 = node(2, node::max_id, terminal_T, terminal_T);
          const node n5 = node(2, node::max_id - 1, n7, terminal_T);
          const node n4 = node(2, node::max_id - 2, terminal_F, terminal_T);
          const node n3 = node(1, node::max_id, n6, n4);
          const node n2 = node(1, node::max_id - 2, n4, n5); // bad index
          const node n1 = node(0, node::max_id - 1, n2, n3); // bad index

          node_ofstream nw(zdd_unindexable);
          nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        // zdd_unindexable->canonical == false
        // zdd_unindexable->width == 3u

        it("should random access on indexable ([canon] U [unindexable])", [&]() {
          __zdd out = zdd_union(ep, zdd_canon, zdd_unindexable);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), true, ptr_uint64(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (6,5)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (F,7)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 2), false, ptr_uint64(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (9,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 3), true, ptr_uint64(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 3), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 2), true, terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(9u));
        });

        it("should random access on indexable ([unindexable] U [canon])", [&]() {
          __zdd out = zdd_union(ep, zdd_unindexable, zdd_canon);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // (2,2)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (3,3)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (4,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (7,4)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), true, ptr_uint64(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (6,5)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (5,6)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 3) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (F,7)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 2), false, ptr_uint64(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (8,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // (9,T)
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 3), true, ptr_uint64(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 3), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 2), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 2), true, terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(4u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(9u));
        });

        it("computes { {0}, {1,3}, {2,3}, {1} } U { {0,3}, {3} } in different order", [&]() {
          /*
          //                1        1                        (1,1)        ---- x0
          //               / \      / \                       /   \
          //               2 T      | |                    (2,2)   \       ---- x1
          //              / \       | |                    /   \    |
          //              3  \      | |     ==>         (3,2)   \   |      ---- x2
          //              || |      | |                 /   \____\ /
          //              \\ /      \ /                /          X
          //                4        2              (4,2)    (T,2) (4,F)   ---- x3
          //               / \      / \              / \      / \   / \
          //               F T      F T              F T      T T   T T
          //
          //           The high arc on (2) and (3) on the left is shortcutting the
          //           second ZDD, to compensate for the omitted nodes.
          */

          shared_levelized_file<zdd::node_type> zdd_a;
          shared_levelized_file<zdd::node_type> zdd_b;

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << node(3, node::max_id, terminal_F, terminal_T)
                 << node(2,
                         node::max_id,
                         ptr_uint64(3, ptr_uint64::max_id),
                         ptr_uint64(3, ptr_uint64::max_id))
                 << node(1,
                         node::max_id,
                         ptr_uint64(2, ptr_uint64::max_id),
                         ptr_uint64(3, ptr_uint64::max_id))
                 << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T);

            node_ofstream nw_b(zdd_b);
            nw_b << node(3, node::max_id, terminal_F, terminal_T)
                 << node(0,
                         node::max_id,
                         ptr_uint64(3, ptr_uint64::max_id),
                         ptr_uint64(3, ptr_uint64::max_id));
          }

          // zdd_a->width == 1u
          // zdd_a->canonical == true

          // zdd_b->width == 1u
          // zdd_b->canonical == true

          __zdd out = zdd_union(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(3, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 2) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 1), false, terminal_T }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(3, 2), false, terminal_F }));
          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 2), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(3, 3u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(3u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                     Is().GreaterThanOrEqualTo(4u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });

        it("should correctly generate missing nodes (bottom layers)", [&]() {
          shared_levelized_file<zdd::node_type> zdd_a;
          /*
          //      { {0}, {1} }
          //
          //           1             ---- x0
          //          / \
          //         2   T           ---- x1
          //        / \
          //       F   T
          */

          node na_2 = node(1, node::max_id, terminal_F, terminal_T);
          node na_1 = node(0, node::max_id, na_2, terminal_T);

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << na_2 << na_1;
          }

          // zdd_a->width == 1u
          // zdd_a->canonical == true

          shared_levelized_file<zdd::node_type> zdd_b;
          /*
          //    { {0}, {1}, {0, 1} }
          //
          //           1             ---- x0
          //          / \
          //         2   3           ---- x1
          //        / \ / \
          //       F   T   T
          */

          node nb_3 = node(1, node::max_id, terminal_T, terminal_T);
          node nb_2 = node(1, node::max_id - 1, terminal_F, terminal_T);
          node nb_1 = node(0, node::max_id, nb_2, nb_3);

          { // Garbage collect writers early
            node_ofstream nw_b(zdd_b);
            nw_b << nb_3 << nb_2 << nb_1;
          }

          // zdd_b->width == 2u
          // zdd_b->canonical == true

          /*
          //    { {0}, {1} } U { {0}, {1}, {0, 1} }
          //
          //                 (1,1)              ---- x0
          //                 /   \
          //             (2,2)    (T,3)         ---- x1
          //             /   \    /   \
          //         (F,F)    (T,T)   (F,T)
          */

          __zdd out = zdd_union(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(3u));
        });

        it("should correctly generate missing nodes (intermediate layers)", [&]() {
          shared_levelized_file<zdd::node_type> zdd_a;
          /*
          //      { {0}, {1}, {0, 3} }
          //
          //           1             ---- x0
          //          / \
          //         2   \           ---- x1
          //        / \   \
          //       F   T   3         ---- x2
          //              / \
          //             T   T
          */

          node na_3 = node(2, node::max_id, terminal_T, terminal_T);
          node na_2 = node(1, node::max_id, terminal_F, terminal_T);
          node na_1 = node(0, node::max_id, na_2, na_3);

          { // Garbage collect writers early
            node_ofstream nw_a(zdd_a);
            nw_a << na_3 << na_2 << na_1;
          }

          // zdd_a->width == 1u
          // zdd_a->canonical == true

          shared_levelized_file<zdd::node_type> zdd_b;
          /*
          //    { {0}, {1}, {0, 1} }
          //
          //           1             ---- x0
          //          / \
          //         2   3           ---- x1
          //        / \ / \
          //       F   T   T
          */

          node nb_3 = node(1, node::max_id, terminal_T, terminal_T);
          node nb_2 = node(1, node::max_id - 1, terminal_F, terminal_T);
          node nb_1 = node(0, node::max_id, nb_2, nb_3);

          { // Garbage collect writers early
            node_ofstream nw_b(zdd_b);
            nw_b << nb_3 << nb_2 << nb_1;
          }

          // zdd_b->width == 2u
          // zdd_b->canonical == true

          /*
          //    { {0}, {1}, {0, 3} } U { {0}, {1}, {0, 1} }
          //
          //                 (1,1)               ---- x0
          //               __/   \__
          //              /         \
          //          (2,2)         (3,3)        ---- x1
          //          /   \         /   \
          //       (F,F) (T,T)   (3,T) (F,T)     ---- x2
          //                      | |
          //                     (T,T)
          */

          __zdd out = zdd_union(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

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

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(4u));
        });
      });

      describe("zdd_intsec", [&]() {
        it("should shortcircuit intermediate nodes", [&]() {
          shared_levelized_file<zdd::node_type> zdd_a;
          /*
          //    { {0, 1}, {0, 2}, {0, 1, 2} }
          //
          //              1          ---- x0
          //             / \
          //            F   2        ---- x1
          //               / \
          //              3   4      ---- x2
          //             / \ / \
          //             F T T T
          */

          node na_4 = node(2, node::max_id, terminal_T, terminal_T);
          node na_3 = node(2, node::max_id - 1, terminal_F, terminal_T);
          node na_2 = node(1, node::max_id, na_3, na_4);
          node na_1 = node(0, node::max_id, terminal_F, na_2);

          { // Garbage collect early and free write-lock
            node_ofstream nw_a(zdd_a);
            nw_a << na_4 << na_3 << na_2 << na_1;
          }

          // zdd_a->canonical == true
          // zdd_a->width == 2u

          shared_levelized_file<zdd::node_type> zdd_b;
          /*
          //     { {1, 2}, {0, 1, 2} }
          //
          //               1          ---- x0
          //               ||
          //               2          ---- x1
          //              / \
          //             F   3        ---- x2
          //                / \
          //               F   T
          */

          node nb_3 = node(2, node::max_id, terminal_F, terminal_T);
          node nb_2 = node(1, node::max_id, terminal_F, nb_3);
          node nb_1 = node(0, node::max_id, nb_2, nb_2);

          { // Garbage collect early and free write-lock
            node_ofstream nw_b(zdd_b);
            nw_b << nb_3 << nb_2 << nb_1;
          }

          // zdd_b->canonical == true
          // zdd_b->width == 1u

          /*
          // Result of { {0, 1}, {0, 2}, {0, 1, 2} } ∩ { {1, 2}, {0, 1, 2} }
          //
          //                  (1,1)               ---- x0
          //                  /   \
          //             (F,2)     (2,2)          ---- x1
          //                       /   \
          //                   (3,F)   (4,3)      ---- x2
          //                           /   \
          //                       (T,F)   (T,T)
          //
          // Shorts on (F,2) and (3,F)
          */

          __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

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

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(3u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("should suppress nodes", [&]() {
          shared_levelized_file<zdd::node_type> zdd_a;
          /*
          //        { {0}, {1} }
          //
          //              1          ---- x0
          //             / \
          //            2   T        ---- x1
          //           / \
          //           F T
          */

          node na_2 = node(1, node::max_id, terminal_F, terminal_T);
          node na_1 = node(0, node::max_id, na_2, terminal_T);

          { // Garbage collect early and free write-lock
            node_ofstream nw_a(zdd_a);
            nw_a << na_2 << na_1;
          }

          // zdd_a->canonical == true
          // zdd_a->width == 1u

          shared_levelized_file<zdd::node_type> zdd_b;
          /*
          //         { Ø, {0} }
          //
          //             1          ---- x0
          //             ||
          //             T
          */

          node nb_1 = node(0, node::max_id, terminal_T, terminal_T);

          { // Garbage collect early and free write-lock
            node_ofstream nw_b(zdd_b);
            nw_b << nb_1;
          }

          // zdd_b->canonical == true
          // zdd_b->width == 1u

          /*
          // Result of { {0}, {1} } ∩ { Ø, {0} }
          //
          //                  (1,1)               ---- x0
          //                  /   \
          //             (2,T)     (T,T)          ---- x1
          //             /   \
          //         (F,T)   (F,F)
          //
          // Suppresses node (2,T), as F ∩ F = F to
          //
          //                  (1,1)               ---- x0
          //                  /   \
          //             (F,T)     (T,T)
          */

          __zdd out = zdd_intsec(ep, zdd_a, zdd_b);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes (and skip to terminal) { {0}, {1}, {0,1} } ∩ { Ø }", [&]() {
          __zdd out = zdd_intsec(ep, zdd_x0x1_x1, zdd_T);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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
      });

      describe("zdd_diff", [&]() {
        it("computes (and skip) { {0}, {1} } \\ { {1} }", [&]() {
          /*
          //            1                       (1,1)       ---- x0
          //           / \                      /   \
          //           2 T      1     ==>       F   T       ---- x1
          //          / \      / \
          //          F T      F T
          */
          __zdd out = zdd_diff(ep, zdd_x0_x1, zdd_x1);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

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
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });

        it("computes (and skip) { {1} } \\ { {0}, {1} }", [&]() {
          /*
          //                  1                        ---- x0
          //                 / \
          //         1       2 T     ==>       F       ---- x1
          //        / \     / \
          //        F T     F T
          */
          __zdd out = zdd_diff(ep, zdd_x1, zdd_x0_x1);

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

          AssertThat(out_nodes.can_pull(), Is().False());

          AssertThat(out.get<shared_levelized_file<zdd::node_type>>()->levels(), Is().EqualTo(0u));

          AssertThat(
            out.get<shared_levelized_file<zdd::node_type>>()->max_1level_cut[cut::Internal],
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

        it("computes (and skip) { {0,1}, {1} } \\ { {1}, Ø }", [&]() {
          /*
          //             1                      (1,1)       ---- x0
          //             ||                     /   \
          //             2      1     ==>    (2,1)  F       ---- x1
          //            / \    / \            / \
          //            F T    T T            F T
          */
          __zdd out = zdd_diff(ep, zdd_x0x1_x1, zdd_x1_T);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));
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

        it("should flip non-commutative operator", [&]() {
          // Note, one of the inputs (`zdd_non_ra`) is not indexable (and hence not canonical).
          // Hence, random access should be done on the other one (`zdd_ra`) by flipping the
          // arguments. Therefore, the operator should be flipped too; otherwise the result would
          // not be correct. We can see this on the (non-commutative) '-' operator.

          shared_levelized_file<zdd::node_type> zdd_ra;
          /*
          //          { {0}, {1}, {0, 1} }
          //
          //                  1          ---- x0
          //                 / \
          //                2   3        ---- x1
          //               / \ / \
          //               F T T T
          */

          { // Garbage collect early and free write-lock
            const node n3 = node(1, node::max_id, terminal_T, terminal_T);
            const node n2 = node(1, node::max_id - 1, terminal_F, terminal_T);
            const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream nw(zdd_ra);
            nw << n3 << n2 << n1;
          }

          // zdd_a->canonical == false
          // zdd_a->width == 2u

          shared_levelized_file<zdd::node_type> zdd_non_ra;
          /*
          //         { Ø, {1}, {0, 1} }
          //
          //                  1          ---- x0
          //                 / \
          //                2   3        ---- x1
          //               / \ / \
          //               T T F T
          */

          { // Garbage collect early and free write-lock
            const node n3 = node(1, node::max_id, terminal_F, terminal_T);
            const node n2 = node(1, node::max_id - 1, terminal_T, terminal_T);
            const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream nw(zdd_non_ra);
            nw << n3 << n2 << n1;
          }

          // zdd_b->canonical == true
          // zdd_b->width == 2u

          /*
          // { {0}, {1}, {0, 1} } \ { Ø, {1}, {0, 1} } = { Ø, {0} }
          //
          //                      (1,1)               ---- x0
          //                    __/   \__
          //                   /         \
          //               (2,2)         (3,3)        ---- x1
          //               /   \         /   \
          //            (F,T) (T,T)   (T,F) (T,T)
          //
          // As T \ T = F, then layer x1 is skipped, to the following figure
          //
          //                      (1,1)               ---- x0
          //                      /   \
          //                      F   T
          */

          __zdd out = zdd_diff(ep, zdd_ra, zdd_non_ra);

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(0u));

          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(1u));
          AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(1u));
        });
      });
    });
  });
});
