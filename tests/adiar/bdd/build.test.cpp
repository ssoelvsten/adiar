#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/build.cpp", []() {
    bdd::pointer_type terminal_T = bdd::pointer_type(true);
    bdd::pointer_type terminal_F = bdd::pointer_type(false);

    describe("bdd_terminal(v)", [&]() {
      it("can create true terminal [bdd_const]", [&]() {
        bdd res = bdd_const(true);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create true terminal [bdd_terminal]", [&]() {
        bdd res = bdd_terminal(true);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create true terminal [bdd_true]", [&]() {
        bdd res = bdd_true();
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create true terminal [bdd_top]", [&]() {
        bdd res = bdd_top();
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create false terminal [bdd_const]", [&]() {
        bdd res = bdd_const(false);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("can create false terminal [bdd_terminal]", [&]() {
        bdd res = bdd_terminal(false);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("can create false terminal [bdd_false]", [&]() {
        bdd res = bdd_false();
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("can create false terminal [bdd_bot]", [&]() {
        bdd res = bdd_bot();
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });
    });

    describe("bdd_ithvar(i)", [&]() {
      it("can create x0", [&]() {
        bdd res = bdd_ithvar(0);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create x42", [&]() {
        bdd res = bdd_ithvar(42);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, bdd::max_id, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("throws exception for a too large label",
         []() { AssertThrows(invalid_argument, bdd_nithvar(bdd::max_label + 1)); });
    });

    describe("bdd_nithvar(i)", [&]() {
      it("can create !x1", [&]() {
        bdd res = bdd_nithvar(1);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create !x3", [&]() {
        bdd res = bdd_nithvar(3);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("throws exception for a too large label",
         []() { AssertThrows(invalid_argument, bdd_nithvar(bdd::max_label + 1)); });
    });

    describe("bdd_and(generator<pair<bdd::label_type, bool>>)", [&]() {
      it("creates T from empty generator", [&]() {
        const auto gen = []() { return make_optional<pair<bdd::label_type, bool>>(); };

        bdd res = bdd_and(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create x1", [&]() {
        int calls = 0;

        const auto gen = [&calls]() -> optional<pair<bdd::label_type, bool>> {
          if (calls++ > 0) { return make_optional<pair<bdd::label_type, bool>>(); }
          return make_pair(1u, false);
        };

        bdd res = bdd_and(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create -x1", [&]() {
        int calls = 0;

        const auto gen = [&calls]() -> optional<pair<bdd::label_type, bool>> {
          if (calls++ > 0) { return make_optional<pair<bdd::label_type, bool>>(); }
          return make_pair(1u, true);
        };

        bdd res = bdd_and(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      // TODO: more tests independent of iterators
    });

    describe("bdd_and(generator<int>)", [&]() {
      it("creates T from empty generator", [&]() {
        const auto gen = []() { return make_optional<int>(); };

        bdd res = bdd_and(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create x1", [&]() {
        int calls = 0;

        const auto gen = [&calls]() {
          return calls++ == 0 ? make_optional<int>(1) : make_optional<int>();
        };

        bdd res = bdd_and(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create -x1", [&]() {
        int calls = 0;

        const auto gen = [&calls]() {
          return calls++ == 0 ? make_optional<int>(-1) : make_optional<int>();
        };

        bdd res = bdd_and(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      // TODO: more tests independent of iterators
    });

    describe("bdd_and(begin, end)", [&]() {
      it("can create {} as trivially true", [&]() {
        std::vector<int> vars;

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {x1}", [&]() {
        std::vector<int> vars = { 1 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {-x1}", [&]() {
        std::vector<int> vars = { -1 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {x1, x2, x5}", [&]() {
        std::vector<int> vars = { 1, 2, 5 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(5, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {-x1, -x2, x5}", [&]() {
        std::vector<int> vars = { -1, -2, 5 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {x1, -x2}", [&]() {
        std::vector<int> vars = { 1, -2 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("skips duplicates", [&]() {
        std::vector<int> vars = { 1, 2, 2, 5 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(5, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("works with ForwardIt::value_type == 'uint64_t'", [&]() {
        std::vector<uint64_t> vars = { 1, 3 };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("works with ForwardIt::value_type == 'char'", [&]() {
        std::vector<char> vars = { 'a', 'b', 'c', 'd' };

        bdd res = bdd_and(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node('d', bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node('c', bdd::max_id, terminal_F, bdd::pointer_type('d', bdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node('b', bdd::max_id, terminal_F, bdd::pointer_type('c', bdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node('a', bdd::max_id, terminal_F, bdd::pointer_type('b', bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info('d', 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info('c', 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info('b', 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info('a', 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(4u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("throws exception for non-ascending list", []() {
        std::vector<int> vars = { 3, 2 };

        AssertThrows(invalid_argument, bdd_and(vars.rbegin(), vars.rend()));
      });
    });

    describe("bdd_or(generator<pair<bdd::label_type, bool>>)", [&]() {
      it("creates F from empty generator", [&]() {
        const auto gen = []() { return make_optional<pair<bdd::label_type, bool>>(); };

        bdd res = bdd_or(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(0u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(0u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(1u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(0u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(0u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(0u));
      });

      it("can create x1", [&]() {
        int calls = 0;

        const auto gen = [&calls]() -> optional<pair<bdd::label_type, bool>> {
          if (calls++ > 0) { return make_optional<pair<bdd::label_type, bool>>(); }
          return make_pair(1u, false);
        };

        bdd res = bdd_or(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(2u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(1u));
      });

      it("can create -x1", [&]() {
        int calls = 0;

        const auto gen = [&calls]() -> optional<pair<bdd::label_type, bool>> {
          if (calls++ > 0) { return make_optional<pair<bdd::label_type, bool>>(); }
          return make_pair(1u, true);
        };

        bdd res = bdd_or(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(2u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(1u));
      });

      // TODO: more tests independent of iterators (and differentiating it from bdd_and)
    });

    describe("bdd_or(generator<int>)", [&]() {
      it("creates F from empty generator", [&]() {
        const auto gen = []() { return make_optional<int>(); };

        bdd res = bdd_or(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(0u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(0u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(1u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(0u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(0u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(0u));
      });

      it("can create x1", [&]() {
        int calls = 0;

        const auto gen = [&calls]() {
          return calls++ == 0 ? make_optional<int>(1) : make_optional<int>();
        };

        bdd res = bdd_or(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(2u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(1u));
      });

      it("can create -x1", [&]() {
        int calls = 0;

        const auto gen = [&calls]() {
          return calls++ == 0 ? make_optional<int>(-1) : make_optional<int>();
        };

        bdd res = bdd_or(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(2u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(1u));
      });

      // TODO: more tests independent of iterators (and differentiating it from bdd_and)
    });

    describe("bdd_or(begin, end)", [&]() {
      it("can create {x1, x2, x5}", [&]() {
        std::vector<int> vars = { 1, 2, 5 };

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(3u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(4u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(3u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(3u));
      });

      it("can create {} as trivially false", [&]() {
        std::vector<int> vars;

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(0u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(0u));
      });

      it("can create {x1, -x3}", [&]() {
        std::vector<int> vars = { 1, -3 };

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(2u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(3u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(2u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(3u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(2u));
      });

      it("can create {-x2, x4}", [&]() {
        std::vector<int> vars = { -2, 4 };

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_T, bdd::pointer_type(4, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(2u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(3u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(2u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(3u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(2u));
      });

      it("skips duplicates", [&]() {
        std::vector<int> vars = { 1, 2, 2, 5 };

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(3u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(4u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(3u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(3u));
      });

      it("works with ForwardIt::value_type == 'int64_t'", [&]() {
        std::vector<int64_t> vars = { 1, -2, 3, 5 };

        bdd res = bdd_or(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_T, bdd::pointer_type(3, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res.max_1level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_1level_cut(cut::Internal_True), Is().EqualTo(4u));
        AssertThat(res.max_1level_cut(cut::All), Is().EqualTo(5u));

        AssertThat(res.max_2level_cut(cut::Internal), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_False), Is().EqualTo(1u));
        AssertThat(res.max_2level_cut(cut::Internal_True), Is().EqualTo(4u));
        AssertThat(res.max_2level_cut(cut::All), Is().EqualTo(5u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res.number_of_terminals(false), Is().EqualTo(1u));
        AssertThat(res.number_of_terminals(true), Is().EqualTo(4u));
      });

      it("throws exception for non-ascending list", []() {
        std::vector<int> vars = { 3, 2 };

        AssertThrows(invalid_argument, bdd_or(vars.rbegin(), vars.rend()));
      });
    });

    describe("bdd_cube(generator<int>)", [&]() {
      it("creates T from empty generator", [&]() {
        const auto gen = []() { return make_optional<int>(); };

        bdd res = bdd_cube(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create x0", [&]() {
        int calls = 0;

        const auto gen = [&calls]() -> optional<int> {
          if (calls++ > 0) { return make_optional<int>(); }
          return 0;
        };

        bdd res = bdd_cube(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create -x3 & x2 & -x1 & x0", [&]() {
        int calls = 0;

        const auto gen = [&calls]() -> optional<int> {
          if (calls > 3) { return make_optional<int>(); }

          const bdd::label_type x = 3 - calls;
          const bool negate       = calls % 2 == 0;

          calls++;

          return (negate ? -1 : 1) * x;
        };

        bdd res = bdd_cube(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(4u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });
    });

    describe("bdd_cube(generator<pair<bdd::label_type, bool>>)", [&]() {
      it("creates T from empty generator", [&]() {
        const auto gen = []() { return make_optional<pair<bdd::label_type, bool>>(); };

        bdd res = bdd_cube(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create x0", [&]() {
        int calls = 0;

        const auto gen = [&calls]() -> optional<pair<bdd::label_type, bool>> {
          if (calls++ > 0) { return make_optional<pair<bdd::label_type, bool>>(); }
          return make_pair(0u, false);
        };

        bdd res = bdd_cube(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create x3 & ~x2 & x1 & ~x0", [&]() {
        int calls = 0;

        const auto gen = [&calls]() -> optional<pair<bdd::label_type, bool>> {
          if (calls > 3) { return make_optional<pair<bdd::label_type, bool>>(); }

          const bdd::label_type x = 3 - calls;
          const bool negate       = calls % 2 == 1;

          calls++;

          return make_pair(x, negate);
        };

        bdd res = bdd_cube(gen);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(4u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });
    });

    describe("bdd_cube(ForwardIt, ForwardIt)", [&]() {
      it("can create {} as trivially true", [&]() {
        std::vector<int> vars;

        bdd res = bdd_cube(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(0u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {x42}", [&]() {
        std::vector<int> vars = { 42 };

        bdd res = bdd_cube(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {~x4}", [&]() {
        std::vector<int> vars = { -4 };

        bdd res = bdd_cube(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {x1, ~x3, x5}", [&]() {
        std::vector<int> vars = { 1, -3, 5 };

        bdd res = bdd_cube(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {x0, -x2, -x4}", [&]() {
        std::vector<int> vars = { -0, -2, -4 };

        bdd res = bdd_cube(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create {~x0, ~x2, ~x4}", [&]() {
        std::vector<pair<bdd::label_type, bool>> vars = { { 0, true }, { 2, true }, { 4, true } };

        bdd res = bdd_cube(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(bdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });
    });
  });
});
