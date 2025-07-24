#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/optmin.cpp", []() {
    const bdd::pointer_type terminal_T(true);
    const bdd::pointer_type terminal_F(false);

    shared_levelized_file<bdd::node_type> bdd_F;
    /*
    //    F
    */
    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_F);
      nw << node(false);
    }

    shared_levelized_file<bdd::node_type> bdd_T;
    /*
    //    T
    */
    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_T);
      nw << node(true);
    }

    shared_levelized_file<bdd::node_type> bdd_x1;
    /*
    //     1         ---- x1
    //    / \
    //    F T
    */
    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_x1);
      nw << node(1, bdd::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_x1__neg;
    /*
    //     1         ---- x1
    //    / \
    //    T F
    */
    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_x1__neg);
      nw << node(1, bdd::max_id, terminal_T, terminal_F);
    }

    shared_levelized_file<bdd::node_type> bdd_fig4;
    /* Figure 4 in "Solving Optimisation Problemns I/O efficiently: An extension to Adiar"
    // - MSc Course Project Report, Erik Funder Carstensen
    //
    //       1        ---- x0
    //      / \
    //      2 |       ---- x1
    //     / \|
    //     T  3       ---- x2
    //       / \
    //       T F
    */
    { // Garbage collect writer to free write-lock
      const bdd::node_type n3(3, bdd::max_id, terminal_T, terminal_F);
      const bdd::node_type n2(2, bdd::max_id, terminal_T, n3.uid());
      const bdd::node_type n1(1, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_fig4);
      nw << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_1;
    /* Example with lots of early T terminals to test for pruning
    //
    //         1       ---- x0
    //        / \
    //        2 T      ---- x1
    //       / \
    //       3 |       ---- x2
    //      / \|
    //      4  5       ---- x3
    //     / \/ \
    //     F 6  T      ---- x4
    //      / \
    //      F T
    */
    { // Garbage collect writer to free write-lock
      const bdd::node_type n6(4, bdd::max_id, terminal_F, terminal_T);
      const bdd::node_type n5(3, bdd::max_id, n6.uid(), terminal_T);
      const bdd::node_type n4(3, bdd::max_id - 1, terminal_F, n6.uid());
      const bdd::node_type n3(2, bdd::max_id, n4.uid(), n5.uid());
      const bdd::node_type n2(1, bdd::max_id, n3.uid(), n5.uid());
      const bdd::node_type n1(0, bdd::max_id, n2.uid(), terminal_T);

      node_ofstream nw(bdd_1);
      nw << n6 << n5 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_2;
    /* More aggressive example for pruning
    //
    //         1         ---- x0
    //        / \
    //        2 T        ---- x2
    //       / \
    //       F 3         ---- x4
    //        / \
    //       4   5       ---- x6
    //      / \ / \
    //      F  6  T      ---- x8
    //        / \
    //        F T
    */
    { // Garbage collect writer to free write-lock
      const bdd::node_type n6(8, bdd::max_id, terminal_F, terminal_T);
      const bdd::node_type n5(6, bdd::max_id, n6.uid(), terminal_T);
      const bdd::node_type n4(6, bdd::max_id - 1, terminal_F, n6.uid());
      const bdd::node_type n3(4, bdd::max_id, n4.uid(), n5.uid());
      const bdd::node_type n2(2, bdd::max_id, terminal_F, n3.uid());
      const bdd::node_type n1(0, bdd::max_id, n2.uid(), terminal_T);

      node_ofstream nw(bdd_2);
      nw << n6 << n5 << n4 << n3 << n2 << n1;
    }

    describe("optmin(bdd, cost<...>, consumer<...>)", [&]() {
      it("returns [T, 0] for true terminal [(x) -> 0]", [&]() {
        const auto [cube, cost] = bdd_optmin(bdd_T, [](int) -> double { return 0.0; });

        // Check BDD is T
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(0u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(1u));

        // Check cost
        AssertThat(cost, Is().EqualTo(0.0));
      });

      it("returns [T, 0] for true terminal [(x) -> 1]", [&]() {
        const auto [cube, cost] = bdd_optmin(bdd_T, [](int) -> double { return 1.0; });

        // Check BDD is T
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(0u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(1u));

        // Check cost
        AssertThat(cost, Is().EqualTo(0.0));
      });

      it("returns [T, NaN] for false terminal [(x) -> 0]", [&]() {
        const auto [cube, cost] = bdd_optmin(bdd_F, [](int) -> double { return 0.0; });

        // Check BDD is F
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(0u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(0u));

        // Check cost is NaN. This is only the case, if it is not equals to itself
        AssertThat(cost == cost, Is().False());
      });

      it("returns [T, NaN] for false terminal [(x) -> 1]", [&]() {
        const auto [cube, cost] = bdd_optmin(bdd_F, [](int) -> double { return 1.0; });

        // Check BDD is F
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(0u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(0u));

        // Check cost is NaN. This is only the case, if it is not equals to itself
        AssertThat(cost == cost, Is().False());
      });

      it("returns [~1 & ~2, 0] for Fig. 4 [(x) -> 1]", [&]() {
        const auto [cube, cost] = bdd_optmin(bdd_fig4, [](int) -> double { return 1.0; });

        // Check BDD is expected cube
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(2, node::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(1u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(1u));

        // Check cost is 0
        AssertThat(cost, Is().EqualTo(0.0));
      });

      it("returns [~1 & ~2, 0] for Fig. 4 [(x) -> 1]", [&]() {
        const auto [cube, cost] = bdd_optmin(bdd_fig4, [](int) -> double { return 1.0; });

        // Check BDD is expected cube
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(2, node::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(1u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(1u));

        // Check cost
        AssertThat(cost, Is().EqualTo(0.0));
      });

      it("returns for [0, 1] for BDD 1 with [(x) -> 1]", [&]() {
        const auto [cube, cost] = bdd_optmin(bdd_1, [](int) -> double { return 1.0; });

        // Check BDD is expected cube
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(1u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(1u));

        // Check cost
        AssertThat(cost, Is().EqualTo(1.0));
      });

      it("returns [~0 & 1 & 3, 2] for BDD 1 with [(x) -> 1 + 2*(1-x%2)]", [&]() {
        const auto [cube, cost] =
          bdd_optmin(bdd_1, [](int x) -> double { return 1.0 + 2 * (1 - x % 2); });

        // Check BDD is expected cube
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, node::max_id, terminal_F, node::pointer_type(3, node::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(0, node::max_id, node::pointer_type(1, node::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(1u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(1u));

        // Check cost
        AssertThat(cost, Is().EqualTo(2.0));
      });

      it("returns [~0 & 1 & ~3 & 4, 1] for BDD 1 with [(x) -> 0.5 + a lot for x0 and x3]", [&]() {
        const auto [cube, cost] =
          bdd_optmin(bdd_1, [](int x) -> double { return 0.5 + (x == 0 || x == 3 ? 42.0 : 0.0); });

        // Check BDD is expected cube
        node_test_ifstream ns(cube);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, node::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(3, node::max_id, node::pointer_type(4, node::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, node::max_id, terminal_F, node::pointer_type(3, node::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(0, node::max_id, node::pointer_type(1, node::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(cube);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(cube->width, Is().EqualTo(1u));

        AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(cube->number_of_terminals[false], Is().EqualTo(4u));
        AssertThat(cube->number_of_terminals[true], Is().EqualTo(1u));

        // Check cost
        AssertThat(cost, Is().EqualTo(1.0));
      });

      it("returns [~0 & ~1 & 2 & ~3 & 4, 4.5] for BDD 1 with [(x) -> 2.25 + a lot for x0, x1, and "
         "x3]",
         [&]() {
           const auto [cube, cost] = bdd_optmin(bdd_1, [](int x) -> double {
             return 2.25 + (x == 0 || x == 1 || x == 3 ? 42.0 : 0.0);
           });

           // Check BDD is expected cube
           node_test_ifstream ns(cube);

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(ns.pull(), Is().EqualTo(node(4, node::max_id, terminal_F, terminal_T)));

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(
             ns.pull(),
             Is().EqualTo(node(3, node::max_id, node::pointer_type(4, node::max_id), terminal_F)));

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(
             ns.pull(),
             Is().EqualTo(node(2, node::max_id, terminal_F, node::pointer_type(3, node::max_id))));

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(
             ns.pull(),
             Is().EqualTo(node(1, node::max_id, node::pointer_type(2, node::max_id), terminal_F)));

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(
             ns.pull(),
             Is().EqualTo(node(0, node::max_id, node::pointer_type(1, node::max_id), terminal_F)));

           AssertThat(ns.can_pull(), Is().False());

           level_info_test_ifstream ms(cube);

           AssertThat(ms.can_pull(), Is().True());
           AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

           AssertThat(ms.can_pull(), Is().True());
           AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

           AssertThat(ms.can_pull(), Is().True());
           AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

           AssertThat(ms.can_pull(), Is().True());
           AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

           AssertThat(ms.can_pull(), Is().True());
           AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

           AssertThat(ms.can_pull(), Is().False());

           AssertThat(cube->width, Is().EqualTo(1u));

           AssertThat(cube->max_1level_cut[cut::Internal], Is().EqualTo(1u));
           AssertThat(cube->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
           AssertThat(cube->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
           AssertThat(cube->max_1level_cut[cut::All], Is().EqualTo(6u));

           AssertThat(cube->max_2level_cut[cut::Internal], Is().EqualTo(1u));
           AssertThat(cube->max_2level_cut[cut::Internal_False], Is().EqualTo(5u));
           AssertThat(cube->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
           AssertThat(cube->max_2level_cut[cut::All], Is().EqualTo(6u));

           AssertThat(cube->number_of_terminals[false], Is().EqualTo(5u));
           AssertThat(cube->number_of_terminals[true], Is().EqualTo(1u));

           // Check cost
           AssertThat(cost, Is().EqualTo(4.5));
         });
    });

    describe("optmin(bdd, cost<...>, consumer<...>)", [&]() {
      // Store result in `res` and call history in `call_history`
      std::vector<int> call_history;
      std::vector<pair<int, bool>> res;

      before_each([&]() {
        call_history.clear();
        res.clear();
      });

      const auto c = [&res](const pair<int, bool>& xa) -> void { res.push_back(xa); };

      it("never invokes callback for false terminal", [&]() {
        const double cost = bdd_optmin(
          bdd_F,
          [&](int x) -> double {
            call_history.push_back(x);
            return 1.0;
          },
          c);

        // Check cost is NaN
        AssertThat(cost == cost, Is().False());

        // Check result
        AssertThat(res.size(), Is().EqualTo(0u));

        // Check call history
        AssertThat(call_history.size(), Is().EqualTo(0u));
      });

      it("never invokes callback for true terminal", [&]() {
        const double cost = bdd_optmin(
          bdd_T,
          [&](int x) -> double {
            call_history.push_back(x);
            return 1.0;
          },
          c);

        // Check cost
        AssertThat(cost, Is().EqualTo(0.0));

        // Check result
        AssertThat(res.size(), Is().EqualTo(0u));

        // Check call history
        AssertThat(call_history.size(), Is().EqualTo(0u));
      });

      it("only invokes cost function for existing levels BDD 2 [(x) -> (8-x)^2]", [&]() {
        const double cost = bdd_optmin(
          bdd_2,
          [&](int x) -> double {
            call_history.push_back(x);
            return (8 - x) * (8 - x);
          },
          c);

        // Check cost
        AssertThat(cost, Is().EqualTo(40.0));

        // Check result
        AssertThat(res.size(), Is().EqualTo(5u));

        AssertThat(res.at(0), Is().EqualTo(make_pair(8, true)));
        AssertThat(res.at(1), Is().EqualTo(make_pair(6, true)));
        AssertThat(res.at(2), Is().EqualTo(make_pair(4, false)));
        AssertThat(res.at(3), Is().EqualTo(make_pair(2, true)));
        AssertThat(res.at(4), Is().EqualTo(make_pair(0, false)));

        // Check call history
        AssertThat(call_history.size(), Is().EqualTo(5u));

        AssertThat(call_history.at(0), Is().EqualTo(0));
        AssertThat(call_history.at(1), Is().EqualTo(2));
        AssertThat(call_history.at(2), Is().EqualTo(4));
        AssertThat(call_history.at(3), Is().EqualTo(6));
        AssertThat(call_history.at(4), Is().EqualTo(8));
      });

      it("stops invoking the cost function early for BDD 1 [(x) -> 1]", [&]() {
        const double cost = bdd_optmin(
          bdd_1,
          [&](int x) -> double {
            call_history.push_back(x);
            return 1.0;
          },
          c);

        // Check cost
        AssertThat(cost, Is().EqualTo(1.0));

        // Check result
        AssertThat(res.size(), Is().EqualTo(1u));

        AssertThat(res.at(0), Is().EqualTo(make_pair(0, true)));

        // Check call history
        AssertThat(call_history.size(), Is().EqualTo(4u));

        AssertThat(call_history.at(0), Is().EqualTo(0));
        AssertThat(call_history.at(1), Is().EqualTo(1));
        AssertThat(call_history.at(2), Is().EqualTo(2));
        AssertThat(call_history.at(3), Is().EqualTo(3));
      });

      it("stops invoking the cost function early for BDD 2 [(x) -> 4.2]", [&]() {
        const double cost = bdd_optmin(
          bdd_2,
          [&](int x) -> double {
            call_history.push_back(x);
            return 4.2;
          },
          c);

        // Check cost
        AssertThat(cost, Is().EqualTo(4.2));

        // Check result
        AssertThat(res.size(), Is().EqualTo(1u));

        AssertThat(res.at(0), Is().EqualTo(make_pair(0, true)));

        // Check call history
        AssertThat(call_history.size(), Is().EqualTo(2u));

        AssertThat(call_history.at(0), Is().EqualTo(0));
        AssertThat(call_history.at(1), Is().EqualTo(2));
      });
    });
  });
});
