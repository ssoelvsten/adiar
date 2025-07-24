#include "../test.h"

go_bandit([]() {
  describe("adiar/builder.h", []() {
    const ptr_uint64 terminal_T = ptr_uint64(true);
    const ptr_uint64 terminal_F = ptr_uint64(false);

    describe("builder_ptr", [&]() {
      it("supports copy-construction", [&]() {
        bdd_builder b;

        const bdd_ptr p1 = b.add_node(1, false, true);
        const bdd_ptr p2 = p1;

        const bdd_ptr p3 = b.add_node(0, p2, true);

        bdd out = b.build();
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());
      });

      it("supports move-semantics", [&]() {
        bdd_builder b;

        b.add_node(0, b.add_node(1, false, true), true);

        bdd out = b.build();
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());
      });
    });

    it("can create a false terminal-only BDD", [&]() {
      bdd_builder b;

      b.add_node(false);

      bdd out = b.build();

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

    it("can create a true terminal-only BDD", [&]() {
      bdd_builder b;

      b.add_node(true);

      bdd out = b.build();

      // Check it looks all right
      node_test_ifstream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_ifstream out_meta(out);

      AssertThat(out_meta.can_pull(), Is().False());

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

    it("creates the last pushed terminal if no nodes are pushed", [&]() {
      bdd_builder b;

      b.add_node(true);

      b.add_node(false);

      bdd out = b.build();

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

    it("throws an exception when create is called on an empty file", [&]() {
      bdd_builder b;

      AssertThrows(domain_error, b.build());
    });

    it("throws an exception when calling create a second time with no new nodes in between", [&]() {
      bdd_builder b;

      b.add_node(0, false, true);
      b.build();

      AssertThrows(domain_error, b.build());
    });

    it("can create a single-node BDD", [&]() {
      /*
      //         1     ---- x0
      //        / \
      //        F T
       */

      bdd_builder b;

      b.add_node(0, false, true);

      bdd out = b.build();

      // Check it looks all right
      node_test_ifstream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id, terminal_F, terminal_T)));
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

      AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

      AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
    });

    it("throws an exception if pointers are used from a different builder [1]", [&]() {
      bdd_builder b1;
      bdd_builder b2;

      builder_ptr p = b1.add_node(true);

      AssertThrows(invalid_argument, b2.add_node(0, false, p));
    });

    it("throws an exception if pointers are used from a different builder [2]", [&]() {
      bdd_builder b1;
      bdd_builder b2;

      builder_ptr p = b1.add_node(1, true, false);

      AssertThrows(invalid_argument, b2.add_node(0, p, false));
    });

    it("throws an exception if pointers are used after reset", [&]() {
      bdd_builder b;

      builder_ptr p = b.add_node(1, true, false);
      b.clear();

      AssertThrows(invalid_argument, b.add_node(0, p, false));
    });

    it("throws an exception if pointers are used after create", [&]() {
      bdd_builder b;

      builder_ptr p = b.add_node(1, true, false);
      b.build();

      AssertThrows(invalid_argument, b.add_node(0, p, false));
    });

    it("throws an exception when label > max_label", [&]() {
      bdd_builder b;

      AssertThrows(invalid_argument, b.add_node(node::max_label + 1, false, true));
    });

    it("throws an exception when label > last label", [&]() {
      bdd_builder b;

      b.add_node(0, false, true);

      AssertThrows(invalid_argument, b.add_node(1, false, true));
    });

    it("throws an exception when low.label() >= label [1]", [&]() {
      bdd_builder b;

      const bdd_ptr p = b.add_node(0, false, true);

      AssertThrows(invalid_argument, b.add_node(0, p, true));
    });

    it("throws an exception when low.label() >= label [2]", [&]() {
      bdd_builder b;

      const bdd_ptr p = b.add_node(3, false, true);

      AssertThrows(invalid_argument, b.add_node(3, p, true));
    });

    it("throws an exception when high.label() >= label [1]", [&]() {
      bdd_builder b;

      const bdd_ptr p = b.add_node(0, false, true);

      AssertThrows(invalid_argument, b.add_node(0, false, p));
    });

    it("throws an exception when high.label() >= label [2]", [&]() {
      bdd_builder b;

      const bdd_ptr p = b.add_node(6, false, true);

      AssertThrows(invalid_argument, b.add_node(6, false, p));
    });

    it("can create nodes on different levels", [&]() {
      /*
      //           1      ---- x0
      //          / \
      //          | 2     ---- x1
      //          |/ \
      //          3  T    ---- x2
      //          / \
      //          F T
      */

      bdd_builder b;

      const bdd_ptr p3 = b.add_node(2, false, true);
      const bdd_ptr p2 = b.add_node(1, p3, true);
      const bdd_ptr p1 = b.add_node(0, p3, p2);

      bdd out = b.build();

      // Check it looks all right
      node_test_ifstream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      // n3
      AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n2
      AssertThat(
        out_nodes.pull(),
        Is().EqualTo(node(1, node::max_id, ptr_uint64(2, ptr_uint64::max_id), terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n1
      AssertThat(
        out_nodes.pull(),
        Is().EqualTo(node(
          0, node::max_id, ptr_uint64(2, ptr_uint64::max_id), ptr_uint64(1, ptr_uint64::max_id))));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_ifstream out_meta(out);

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->width, Is().EqualTo(1u));

      AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
      AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
      AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
      AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

      AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

      AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
    });

    it("can create several nodes on the same level", [&]() {
      /*
      //                1       ---- x0
      //               / \
      //               2  3      ---- x1
      //              / \/ \
      //              4  5  T   ---- x2
      //             / \/ \
      //             F  T  F
      */

      bdd_builder b;

      const bdd_ptr p5 = b.add_node(2, true, false);
      const bdd_ptr p4 = b.add_node(2, false, true);
      const bdd_ptr p3 = b.add_node(1, p5, true);
      const bdd_ptr p2 = b.add_node(1, p4, p5);
      const bdd_ptr p1 = b.add_node(0, p2, p3);

      bdd out = b.build();

      // Check it looks all right
      node_test_ifstream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      // n5
      AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_F)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n4
      AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id - 1, terminal_F, terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n3
      AssertThat(
        out_nodes.pull(),
        Is().EqualTo(node(1, node::max_id, ptr_uint64(2, ptr_uint64::max_id), terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n2
      AssertThat(out_nodes.pull(),
                 Is().EqualTo(node(1,
                                   node::max_id - 1,
                                   ptr_uint64(2, ptr_uint64::max_id - 1),
                                   ptr_uint64(2, ptr_uint64::max_id))));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n1
      AssertThat(out_nodes.pull(),
                 Is().EqualTo(node(0,
                                   node::max_id,
                                   ptr_uint64(1, ptr_uint64::max_id - 1),
                                   ptr_uint64(1, ptr_uint64::max_id))));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_ifstream out_meta(out);

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->width, Is().EqualTo(2u));

      AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));
      AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
      AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));
      AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

      AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(6u));
      AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(8u));

      AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
      AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
    });

    it("can reset and create new nodes", [&]() {
      bdd_builder b;

      b.add_node(0, false, true);

      b.clear();

      b.add_node(1, true, false);

      bdd out = b.build();

      // Check it looks all right
      node_test_ifstream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_T, terminal_F)));
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

      AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

      AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
      AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
    });

    it("is empty after reset", [&]() {
      bdd_builder b;

      b.add_node(0, false, true);

      b.clear();

      AssertThrows(domain_error, b.build());
    });

    it("can create two different BDDs", [&]() {
      bdd_builder b;

      { // FIRST
        b.add_node(0, false, true);
        bdd out = b.build();

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id, terminal_F, terminal_T)));
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

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      }

      { // SECOND
        b.add_node(1, true, false);

        bdd out = b.build();

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_T, terminal_F)));
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

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      }
    });

    it("throws an exception when there is more than one root [1]", [&]() {
      bdd_builder b;

      b.add_node(0, false, true);
      b.add_node(0, true, false);

      AssertThrows(domain_error, b.build());
    });

    it("throws an exception when there is more than one root [2]", [&]() {
      bdd_builder b;

      b.add_node(4, false, true);
      b.add_node(2, true, false);

      AssertThrows(domain_error, b.build());
    });

    it("throws an exception when there is more than one root [3]", [&]() {
      bdd_builder b;

      const bdd_ptr p5 = b.add_node(5, true, false);
      const bdd_ptr p4 = b.add_node(4, false, true);
      const bdd_ptr p3 = b.add_node(4, p5, true);
      const bdd_ptr p2 = b.add_node(2, p3, p4);
      const bdd_ptr p1 = b.add_node(1, p3, p5);

      AssertThrows(domain_error, b.build());
    });

    it("recognizes copies of nodes", [&]() {
      /*
                         1        ---- x2
                        / \
                        3  4      ---- x4
                       / \/ \
                       5  T F    ---- x5
                      / \
                      T F
      */

      bdd_builder b;

      const bdd_ptr p5 = b.add_node(5, true, false);
      const bdd_ptr p4 = b.add_node(4, true, false);
      const bdd_ptr p3 = b.add_node(4, p5, true);
      const bdd_ptr p2 = p3;
      const bdd_ptr p1 = b.add_node(2, p2, p4);

      bdd out = b.build();

      // Check it looks all right
      node_test_ifstream out_nodes(out);

      AssertThat(out_nodes.can_pull(), Is().True());

      // n5
      AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_T, terminal_F)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n4
      AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_F)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n3
      AssertThat(
        out_nodes.pull(),
        Is().EqualTo(node(4, node::max_id - 1, ptr_uint64(5, ptr_uint64::max_id), terminal_T)));
      AssertThat(out_nodes.can_pull(), Is().True());

      // n1
      AssertThat(out_nodes.pull(),
                 Is().EqualTo(node(2,
                                   node::max_id,
                                   ptr_uint64(4, ptr_uint64::max_id - 1),
                                   ptr_uint64(4, ptr_uint64::max_id))));
      AssertThat(out_nodes.can_pull(), Is().False());

      level_info_test_ifstream out_meta(out);

      AssertThat(out->width, Is().EqualTo(2u));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 2u)));

      AssertThat(out_meta.can_pull(), Is().True());
      AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

      AssertThat(out_meta.can_pull(), Is().False());

      AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
      AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
      AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));
      AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

      AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
      AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
      AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
      AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(6u));
      AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(5u));
      AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(8u));

      AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
      AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
    });

    describe("bdd_builder", [&]() {
      it("uses the BDD reduction rule", [&]() {
        bdd_builder b;

        const bdd_ptr p = b.add_node(1, false, true);

        b.add_node(0, p, p);

        bdd out = b.build();

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));
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

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can collapse BDD reduction rule to a false terminal", [&]() {
        bdd_builder b;

        const bdd_ptr p2 = b.add_node(2, false, false);
        const bdd_ptr p1 = b.add_node(1, false, p2);

        bdd out = b.build();

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

      it("can collapse BDD reduction rule to a true terminal [1]", [&]() {
        bdd_builder b;

        const bdd_ptr p2 = b.add_node(2, true, true);
        const bdd_ptr p1 = b.add_node(1, p2, p2);

        bdd out = b.build();

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

      it("can collapse BDD reduction rule to a true terminal [2]", [&]() {
        bdd_builder b;

        const bdd_ptr p2 = b.add_node(2, true, true);
        const bdd_ptr p1 = b.add_node(1, p2, true);

        bdd out = b.build();

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

      it("does not decrement 'id' when applying the BDD reduction rule", [&]() {
        bdd_builder b;

        /*
                 _1_      ---- x0
                /   \
                2   3     ---- x1
               / \ //
               F  4       ---- x2
                 / \
                 F T
         */
        const bdd_ptr p4 = b.add_node(2, false, true);
        const bdd_ptr p3 = b.add_node(1, p4, p4);
        const bdd_ptr p2 = b.add_node(1, false, p4);
        const bdd_ptr p1 = b.add_node(0, p3, p2);

        bdd out = b.build();

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, terminal_F, ptr_uint64(2, ptr_uint64::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     ptr_uint64(2, ptr_uint64::max_id),
                                     ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(4u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("uses the BDD reduction rule with copies of nodes", [&]() {
        bdd_builder b;

        const bdd_ptr p1 = b.add_node(1, false, true);
        const bdd_ptr p2 = p1;

        b.add_node(0, p1, p2);

        bdd out = b.build();

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));
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

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("does not allow multiple roots when using BDD reduction rule", [&]() {
        bdd_builder b;

        const bdd_ptr p4 = b.add_node(2, true, false);
        const bdd_ptr p3 = b.add_node(2, false, true);
        const bdd_ptr p2 = b.add_node(1, p4, true);
        const bdd_ptr p1 = b.add_node(1, p3, p4); // root
        const bdd_ptr p0 = b.add_node(0, p2, p2); // root

        AssertThrows(domain_error, b.build());
      });
    });

    describe("zdd_builder", [&]() {
      it("uses the ZDD reduction rule", [&]() {
        zdd_builder b;

        const zdd_ptr p = b.add_node(1, false, true);

        b.add_node(0, p, false);

        zdd out = b.build();

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));
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

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can collapse ZDD reduction rule to a false terminal", [&]() {
        zdd_builder b;

        const zdd_ptr p2 = b.add_node(2, false, false);
        const zdd_ptr p1 = b.add_node(1, p2, false);

        zdd out = b.build();

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

      it("can collapse ZDD reduction rule to a true terminal", [&]() {
        zdd_builder b;

        const zdd_ptr p2 = b.add_node(2, true, false);
        const zdd_ptr p1 = b.add_node(1, p2, false);

        zdd out = b.build();

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

      it("does not decrement 'id' when applying the ZDD reduction rule", [&]() {
        zdd_builder b;

        /*
                 _1_      ---- x0
                /   \
                2   3     ---- x1
                \\ / \
                  4  F    ---- x2
                 / \
                 F T
         */
        const zdd_ptr p4 = b.add_node(2, false, true);
        const zdd_ptr p3 = b.add_node(1, p4, false);
        const zdd_ptr p2 = b.add_node(1, p4, p4);
        const zdd_ptr p1 = b.add_node(0, p3, p2);

        zdd out = b.build();

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     ptr_uint64(2, ptr_uint64::max_id),
                                     ptr_uint64(2, ptr_uint64::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True());

        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     ptr_uint64(2, ptr_uint64::max_id),
                                     ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(4u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("does not allow multiple roots when using ZDD reduction rule", [&]() {
        zdd_builder b;

        const zdd_ptr p4 = b.add_node(2, true, false);
        const zdd_ptr p3 = b.add_node(2, false, true);
        const zdd_ptr p2 = b.add_node(1, p4, true);
        const zdd_ptr p1 = b.add_node(1, p3, p4);    // root
        const zdd_ptr p0 = b.add_node(0, p2, false); // root

        AssertThrows(domain_error, b.build());
      });
    });
  });
});
