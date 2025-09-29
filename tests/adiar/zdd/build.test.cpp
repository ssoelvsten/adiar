#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/build.cpp", []() {
    zdd::pointer_type terminal_T = zdd::pointer_type(true);
    zdd::pointer_type terminal_F = zdd::pointer_type(false);

    describe("zdd_terminal(v)", [&]() {
      it("can create { Ø } [zdd_terminal]", [&]() {
        zdd res = zdd_terminal(true);
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create { Ø } [zdd_null]", [&]() {
        zdd res = zdd_null();
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create Ø [zdd_terminal]", [&]() {
        zdd res = zdd_terminal(false);
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("can create Ø [zdd_empty]", [&]() {
        zdd res = zdd_empty();
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });
    });

    describe("zdd_ithvar(i, dom)", [&]() {
      // TODO
    });

    describe("zdd_ithvar(i, begin, end)", [&]() {
      it("creates Ø when dom is empty", [&]() {
        std::vector<int> dom;

        zdd res = zdd_ithvar(42, dom.rbegin(), dom.rend());

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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("creates { { 42 } } for i = 42, dom = {42}", [&]() {
        std::vector<int> dom = { 42 };

        zdd res = zdd_ithvar(42, dom.rbegin(), dom.rend());

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(
                     node(42, zdd::max_id, node::pointer_type(false), node::pointer_type(true))));
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("creates { { 7 } } for i = 7, dom = {7}", [&]() {
        std::vector<int> dom = { 7 };

        zdd res = zdd_ithvar(7, dom.rbegin(), dom.rend());

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(7, zdd::max_id, node::pointer_type(false), node::pointer_type(true))));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(7, 1u)));

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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("creates { { 21 }, { 42,21 } } for i = 21, dom = {21,42}", [&]() {
        std::vector<int> dom = { 21, 42 };

        zdd res = zdd_ithvar(21, dom.rbegin(), dom.rend());

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(42, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(
            21, zdd::max_id, node::pointer_type(false), node::pointer_type(42, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(21, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("creates { { 42 }, { 42,21 } } for i = 42, dom = {21,42}", [&]() {
        std::vector<int> dom = { 21, 42 };

        zdd res = zdd_ithvar(42, dom.rbegin(), dom.rend());

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(
                     node(42, zdd::max_id, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(21,
                                     zdd::max_id,
                                     node::pointer_type(42, zdd::max_id),
                                     node::pointer_type(42, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(21, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("creates { { 21 }, { 42,21 }, { 21,10 }, { 42,21,10 } } for i = 21, dom = {10,21,42}",
         [&]() {
           std::vector<int> dom = { 10, 21, 42 };

           zdd res = zdd_ithvar(21, dom.rbegin(), dom.rend());

           node_test_ifstream ns(res);

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(ns.pull(),
                      Is().EqualTo(node(
                        42, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(
             ns.pull(),
             Is().EqualTo(node(
               21, zdd::max_id, node::pointer_type(false), node::pointer_type(42, zdd::max_id))));

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(ns.pull(),
                      Is().EqualTo(node(10,
                                        zdd::max_id,
                                        node::pointer_type(21, zdd::max_id),
                                        node::pointer_type(21, zdd::max_id))));

           AssertThat(ns.can_pull(), Is().False());

           level_info_test_ifstream ms(res);

           AssertThat(ms.can_pull(), Is().True());
           AssertThat(ms.pull(), Is().EqualTo(level_info(42, 1u)));
           AssertThat(ms.can_pull(), Is().True());
           AssertThat(ms.pull(), Is().EqualTo(level_info(21, 1u)));
           AssertThat(ms.can_pull(), Is().True());
           AssertThat(ms.pull(), Is().EqualTo(level_info(10, 1u)));
           AssertThat(ms.can_pull(), Is().False());

           AssertThat(res->width, Is().EqualTo(1u));

           AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
           AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
           AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
           AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(3u));

           AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
           AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
           AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
           AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(3u));

           AssertThat(res->sorted, Is().True());
           AssertThat(res->indexable, Is().True());
           AssertThat(zdd_iscanonical(res), Is().True());

           AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
           AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
         });

      it("throws exception when domain is not in descending order", [&]() {
        std::vector<int> dom = { 3, 2, 1, 0 };
        AssertThrows(invalid_argument, zdd_ithvar(2, dom.rbegin(), dom.rend()));
      });
    });

    describe("zdd_ithvar(i)", [&]() {
      {
        std::vector<int> dom = { 0, 1, 2, 3 };
        domain_set(dom.begin(), dom.end());
      }

      it("constructs chain for i = 1 global dom = {0,1,2,3}", [&]() {
        zdd res = zdd_ithvar(1);

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(3, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2,
                                     zdd::max_id,
                                     node::pointer_type(3, zdd::max_id),
                                     node::pointer_type(3, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(
            node(1, zdd::max_id, node::pointer_type(false), node::pointer_type(2, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0,
                                     zdd::max_id,
                                     node::pointer_type(1, zdd::max_id),
                                     node::pointer_type(1, zdd::max_id))));

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

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });
    });

    describe("zdd_nithvar(i, dom)", [&]() {
      // TODO
    });

    describe("zdd_nithvar(i, begin, end)", [&]() {
      it("creates { Ø } when dom is empty", [&]() {
        std::vector<int> dom;

        zdd res = zdd_nithvar(42, dom.rbegin(), dom.rend());

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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("creates { Ø } for i = 42, dom = {42}", [&]() {
        std::vector<int> dom = { 42 };

        zdd res = zdd_nithvar(42, dom.rbegin(), dom.rend());

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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("creates { Ø, { 21 } } for i = 42, dom = {21,42}", [&]() {
        std::vector<int> dom = { 21, 42 };

        zdd res = zdd_nithvar(42, dom.rbegin(), dom.rend());

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(21, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(21, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("creates { Ø, { 42 } } for i = 21, dom = {21,42}", [&]() {
        std::vector<int> dom = { 21, 42 };

        zdd res = zdd_nithvar(21, dom.rbegin(), dom.rend());

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(42, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("creates { Ø, { 0 }, { 2 }, { 0,2 } } for i = 1, dom = {0,1,2}", [&]() {
        std::vector<int> dom = { 0, 1, 2 };

        zdd res = zdd_nithvar(1, dom.rbegin(), dom.rend());

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0,
                                     zdd::max_id,
                                     node::pointer_type(2, zdd::max_id),
                                     node::pointer_type(2, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("throws exception when domain is not in ascending order", [&]() {
        std::vector<int> dom = { 3, 2, 1, 0 };

        AssertThrows(invalid_argument, zdd_nithvar(2, dom.rbegin(), dom.rend()));
      });
    });

    describe("zdd_nithvar(i)", [&]() {
      {
        std::vector<int> dom = { 0, 1, 2, 3 };
        domain_set(dom.begin(), dom.end());
      }

      it("constructs chain for i = 2 global dom = {0,1,2,3}", [&]() {
        zdd res = zdd_nithvar(2);

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(3, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1,
                                     zdd::max_id,
                                     node::pointer_type(3, zdd::max_id),
                                     node::pointer_type(3, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0,
                                     zdd::max_id,
                                     node::pointer_type(1, zdd::max_id),
                                     node::pointer_type(1, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("constructs chain for i = 3 global dom = {0,1,2,3}", [&]() {
        zdd res = zdd_nithvar(3);

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1,
                                     zdd::max_id,
                                     node::pointer_type(2, zdd::max_id),
                                     node::pointer_type(2, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0,
                                     zdd::max_id,
                                     node::pointer_type(1, zdd::max_id),
                                     node::pointer_type(1, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("constructs chain for i = 0 global dom = {0,1,2,3}", [&]() {
        zdd res = zdd_nithvar(0);

        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(3, zdd::max_id, node::pointer_type(true), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2,
                                     zdd::max_id,
                                     node::pointer_type(3, zdd::max_id),
                                     node::pointer_type(3, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1,
                                     zdd::max_id,
                                     node::pointer_type(2, zdd::max_id),
                                     node::pointer_type(2, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(3, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });
    });

    describe("zdd_vars(vars)", [&]() {
      // TODO: tests independent of std::vector<int>
    });

    describe("zdd_vars(begin, end)", [&]() {
      it("can create { Ø } on empty list", [&]() {
        std::vector<int> vars;

        zdd res = zdd_vars(vars.rbegin(), vars.rend());

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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create { {42} }", [&]() {
        std::vector<int> vars = { 42 };

        zdd res = zdd_vars(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, zdd::max_id, terminal_F, terminal_T)));
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create { {1,2,5} }", [&]() {
        std::vector<int> vars = { 1, 2, 5 };

        zdd res = zdd_vars(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, zdd::max_id, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, zdd::max_id, terminal_F, zdd::pointer_type(5, zdd::max_id))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, zdd::max_id, terminal_F, zdd::pointer_type(2, zdd::max_id))));

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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("throws exception when domain is not in ascending order", [&]() {
        std::vector<int> vars = { 3, 2, 1, 0 };

        AssertThrows(invalid_argument, zdd_vars(vars.rbegin(), vars.rend()));
      });
    });

    describe("zdd_point(vars)", [&]() {
      it("can create { {3} }", [&]() {
        zdd res = zdd_point(make_generator(3));
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(3, zdd::max_id, terminal_F, terminal_T)));
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      // TODO: more tests independent of std::vector<int>
    });

    describe("zdd_point(begin, end)", [&]() {
      it("can create { Ø } on empty list", [&]() {
        std::vector<int> vars;

        zdd res = zdd_point(vars.rbegin(), vars.rend());

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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create { {4} }", [&]() {
        std::vector<int> vars = { 4 };

        zdd res = zdd_point(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, zdd::max_id, terminal_F, terminal_T)));
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create { {0,2,4} }", [&]() {
        std::vector<int> vars = { 0, 2, 4 };

        zdd res = zdd_point(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, zdd::max_id, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, zdd::max_id, terminal_F, zdd::pointer_type(4, zdd::max_id))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(0, zdd::max_id, terminal_F, zdd::pointer_type(2, zdd::max_id))));

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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("throws exception when domain is not in ascending order", [&]() {
        std::vector<int> vars = { 6, 4, 2, 0 };

        AssertThrows(invalid_argument, zdd_point(vars.rbegin(), vars.rend()));
      });
    });

    describe("zdd_singleton(i)", [&]() {
      it("can create { {0} }", [&]() {
        zdd res = zdd_singleton(0);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, zdd::max_id, terminal_F, terminal_T)));
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create { {42} }", [&]() {
        zdd res = zdd_singleton(42);
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, zdd::max_id, terminal_F, terminal_T)));
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("throws exception if the label is too large",
         [&]() { AssertThrows(invalid_argument, zdd_singleton(zdd::max_label + 1)); });
    });

    describe("zdd_singletons(vars)", [&]() {
      // TODO: tests independent of std::vector<int>
    });

    describe("zdd_singletons(begin, end)", [&]() {
      it("can create Ø on empty list", [&]() {
        std::vector<int> vars;

        zdd res = zdd_singletons(vars.rbegin(), vars.rend());
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("can create { {42} }", [&]() {
        std::vector<int> vars = { 42 };

        zdd res = zdd_singletons(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, zdd::max_id, terminal_F, terminal_T)));
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create { {1}, {2}, {5} }", [&]() {
        std::vector<int> vars = { 1, 2, 5 };

        zdd res = zdd_singletons(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, zdd::max_id, terminal_F, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(2, zdd::max_id, zdd::pointer_type(5, zdd::max_id), terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(
          ns.pull(),
          Is().EqualTo(node(1, zdd::max_id, zdd::pointer_type(2, zdd::max_id), terminal_T)));

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
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("throws exception when domain is not in ascending order", [&]() {
        std::vector<int> vars = { 3, 2, 1, 0 };

        AssertThrows(invalid_argument, zdd_singletons(vars.rbegin(), vars.rend()));
      });
    });

    describe("zdd_powerset(vars)", [&]() {
      // TODO
    });

    describe("zdd_powerset(begin, end)", [&]() {
      it("can create { Ø } on empty list", [&]() {
        std::vector<int> vars;

        zdd res = zdd_powerset(vars.rbegin(), vars.rend());
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

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can create { Ø, {42} }", [&]() {
        std::vector<int> vars = { 42 };

        zdd res = zdd_powerset(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(42, zdd::max_id, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(42, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("can create { Ø, {1}, {2}, {5}, {1,2}, {1,5}, {2,5}, {1,2,5} }", [&]() {
        std::vector<int> vars = { 1, 2, 5 };

        zdd res = zdd_powerset(vars.rbegin(), vars.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(5, zdd::max_id, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2,
                                     zdd::max_id,
                                     zdd::pointer_type(5, zdd::max_id),
                                     zdd::pointer_type(5, zdd::max_id))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1,
                                     zdd::max_id,
                                     zdd::pointer_type(2, zdd::max_id),
                                     zdd::pointer_type(2, zdd::max_id))));

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

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("throws exception when domain is not in ascending order", [&]() {
        std::vector<int> vars = { 3, 2, 1, 0 };

        AssertThrows(invalid_argument, zdd_powerset(vars.rbegin(), vars.rend()));
      });
    });

    describe("zdd_bot(...)", [&]() {
      it("is Ø when given generator", [&]() {
        const auto dom = []() -> int { throw std::logic_error("Called generator"); };

        zdd res = zdd_bot(dom);
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("is Ø when given iterators", [&]() {
        std::vector<int> dom = { 42 };

        zdd res = zdd_bot(dom.begin(), dom.end());
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("is Ø when there is no global domain", [&]() {
        domain_unset();
        AssertThat(domain_isset(), Is().False());

        zdd res = zdd_bot();
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("is Ø when there is a global domain", [&]() {
        domain_set(3);
        AssertThat(domain_isset(), Is().True());

        zdd res = zdd_bot();
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(0u));
      });
    });

    describe("zdd_top(...)", [&]() {
      it("is { Ø } with empty generator", [&]() {
        const auto dom = []() { return make_optional<zdd::label_type>(); };

        zdd res = zdd_top(dom);
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("creates { Ø, {0}, ..., {3}, {0,1}, ..., {2,3}, {0,1,2}, ..., {0,1,2,3} } from generator",
         [&]() {
           int x          = 3;
           const auto dom = [&x]() -> optional<zdd::label_type> {
             if (x < 0) { return make_optional<zdd::label_type>(); }
             return x--;
           };

           zdd res = zdd_top(dom);
           node_test_ifstream ns(res);

           AssertThat(ns.can_pull(), Is().True());
           AssertThat(ns.pull(), Is().EqualTo(node(3, zdd::max_id, terminal_T, terminal_T)));
           AssertThat(ns.can_pull(), Is().True());
           AssertThat(ns.pull(),
                      Is().EqualTo(node(2,
                                        zdd::max_id,
                                        zdd::pointer_type(3, zdd::max_id),
                                        zdd::pointer_type(3, zdd::max_id))));
           AssertThat(ns.can_pull(), Is().True());
           AssertThat(ns.pull(),
                      Is().EqualTo(node(1,
                                        zdd::max_id,
                                        zdd::pointer_type(2, zdd::max_id),
                                        zdd::pointer_type(2, zdd::max_id))));
           AssertThat(ns.can_pull(), Is().True());
           AssertThat(ns.pull(),
                      Is().EqualTo(node(0,
                                        zdd::max_id,
                                        zdd::pointer_type(1, zdd::max_id),
                                        zdd::pointer_type(1, zdd::max_id))));

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

           AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
           AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
           AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
           AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

           AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
           AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
           AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
           AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

           AssertThat(res->sorted, Is().True());
           AssertThat(res->indexable, Is().True());
           AssertThat(zdd_iscanonical(res), Is().True());

           AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
           AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
         });

      it("is { Ø } with empty iterator", [&]() {
        const std::vector<int> dom = {};

        zdd res = zdd_top(dom.rbegin(), dom.rend());
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("creates { Ø, {0}, {2}, {4}, {0,2}, {0,4}, {0,4}, {0,2,4} } from iterator", [&]() {
        const std::vector<int> dom = { 0, 2, 4 };

        zdd res = zdd_top(dom.rbegin(), dom.rend());
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(4, zdd::max_id, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2,
                                     zdd::max_id,
                                     zdd::pointer_type(4, zdd::max_id),
                                     zdd::pointer_type(4, zdd::max_id))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0,
                                     zdd::max_id,
                                     zdd::pointer_type(2, zdd::max_id),
                                     zdd::pointer_type(2, zdd::max_id))));

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

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("is { Ø } terminal when there is no global domain", [&]() {
        domain_unset();
        AssertThat(domain_isset(), Is().False());

        zdd res = zdd_top();
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
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("creates { Ø, {0}, {1}, {0,1}, {0,1} } from global domain", [&]() {
        domain_set(2);
        AssertThat(domain_isset(), Is().True());

        zdd res = zdd_top();
        node_test_ifstream ns(res);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, zdd::max_id, terminal_T, terminal_T)));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0,
                                     zdd::max_id,
                                     zdd::pointer_type(1, zdd::max_id),
                                     zdd::pointer_type(1, zdd::max_id))));

        AssertThat(ns.can_pull(), Is().False());

        level_info_test_ifstream ms(res);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));
        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));
        AssertThat(ms.can_pull(), Is().False());

        AssertThat(res->width, Is().EqualTo(1u));

        AssertThat(res->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(res->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(res->sorted, Is().True());
        AssertThat(res->indexable, Is().True());
        AssertThat(zdd_iscanonical(res), Is().True());

        AssertThat(res->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(res->number_of_terminals[true], Is().EqualTo(2u));
      });
    });
  });
});
