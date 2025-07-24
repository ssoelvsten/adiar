#include "../../test.h"
#include <iterator>

go_bandit([]() {
  describe("adiar/zdd/elem.cpp", [&]() {
    shared_levelized_file<zdd::node_type> zdd_F;
    shared_levelized_file<zdd::node_type> zdd_T;

    { // Garbage collect writers to free write-lock
      node_ofstream nw_F(zdd_F);
      nw_F << node(false);

      node_ofstream nw_T(zdd_T);
      nw_T << node(true);
    }

    const ptr_uint64 terminal_T = ptr_uint64(true);
    const ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<zdd::node_type> zdd_1;
    // { { 0 }, { 1 }, { 0,2 }, { 1,2 } }
    /*
    //      1     ---- x0
    //     / \
    //     2 |    ---- x1
    //    / \|
    //    F  3    ---- x2
    //      / \
    //      T T
    */

    {
      const node n3 = node(2, node::max_id, terminal_T, terminal_T);
      const node n2 = node(1, node::max_id, terminal_F, n3.uid());
      const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

      node_ofstream nw(zdd_1);
      nw << n3 << n2 << n1;
    }

    shared_levelized_file<zdd::node_type> zdd_2;
    // { Ø, { 1 }, { 2 }, { 2,3 } }
    /*
    //       1      ---- x1
    //      / \
    //      2 T     ---- x2
    //     / \
    //     T 3      ---- x3
    //      / \
    //      T T
     */

    {
      const node n3 = node(3, node::max_id, terminal_T, terminal_T);
      const node n2 = node(2, node::max_id, terminal_T, n3.uid());
      const node n1 = node(1, node::max_id, n2.uid(), terminal_T);

      node_ofstream nw(zdd_2);
      nw << n3 << n2 << n1;
    }

    shared_levelized_file<zdd::node_type> zdd_3;
    // { { 2,4 }, { 0,2 }, { 0,4 } }
    /*
    //      _1_      ---- x0
    //     /   \
    //     2   3     ---- x2
    //    / \ / \
    //    F  4  T    ---- x4
    //      / \
    //      F T
    */
    {
      const node n4 = node(4, node::max_id, terminal_F, terminal_T);
      const node n3 = node(2, node::max_id, n4.uid(), terminal_T);
      const node n2 = node(2, node::max_id - 1, terminal_F, n4.uid());
      const node n1 = node(1, node::max_id, n2.uid(), n3.uid());

      node_ofstream nw(zdd_3);
      nw << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<zdd::node_type> zdd_4;
    // { {1}, {0,1} }
    /*
    //      1      ---- x0
    //     | |
    //      2      ---- x1
    //     / \
    //     F T
    */
    {
      const node n2 = node(1, node::max_id, terminal_F, terminal_T);
      const node n1 = node(0, node::max_id, n2.uid(), n2.uid());

      node_ofstream nw(zdd_4);
      nw << n2 << n1;
    }

    describe("zdd_minelem(A)", [&]() {
      it("finds Ø on Ø", [&]() {
        zdd out = zdd_minelem(zdd_F);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_type(true)));

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

      it("finds { Ø } set on { Ø }", [&]() {
        zdd out = zdd_minelem(zdd_T);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_type(true)));

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

      it("finds { {1} } on [1]", [&]() {
        zdd out = zdd_minelem(zdd_1);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(1, zdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

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

      it("finds { Ø } on [2]", [&]() {
        zdd out = zdd_minelem(zdd_2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_type(true)));

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

      it("finds {2,4} on [3]", [&]() {
        zdd out = zdd_minelem(zdd_3);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(4, zdd::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(
                     2, zdd::max_id, terminal_F, zdd::pointer_type(4, zdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

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

      it("finds {1} on [4]", [&]() {
        zdd out = zdd_minelem(zdd_4);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(1, zdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

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

    describe("zdd_minelem(A, c)", [&]() {
      it("makes no calls for { Ø } on { Ø }", [&]() {
        size_t calls = 0u;
        const auto c = [&calls](zdd::label_type) { calls++; };

        zdd_minelem(zdd_T, c);
        AssertThat(calls, Is().EqualTo(0u));
      });

      it("calls with 1 on [1]", [&]() {
        size_t calls = 0u;
        std::vector<zdd::label_type> expected{ 1 };

        const auto c = [&calls, &expected](zdd::label_type x) {
          AssertThat(x, Is().EqualTo(expected.at(calls)));
          calls++;
        };

        zdd_minelem(zdd_1, c);
        AssertThat(calls, Is().EqualTo(1u));
      });

      it("makes no calls for { Ø } on [2]", [&]() {
        size_t calls = 0u;
        const auto c = [&calls](zdd::label_type) { calls++; };

        zdd_minelem(zdd_2, c);
        AssertThat(calls, Is().EqualTo(0u));
      });

      it("calls with 2,4 on [3]", [&]() {
        size_t calls = 0u;
        std::vector<zdd::label_type> expected{ 2, 4 };

        const auto c = [&calls, &expected](zdd::label_type x) {
          AssertThat(x, Is().EqualTo(expected.at(calls)));
          calls++;
        };

        zdd_minelem(zdd_3, c);
        AssertThat(calls, Is().EqualTo(2u));
      });

      it("calls with 1 on [4]", [&]() {
        size_t calls = 0u;
        std::vector<zdd::label_type> expected{ 1 };

        const auto c = [&calls, &expected](zdd::label_type x) {
          AssertThat(x, Is().EqualTo(expected.at(calls)));
          calls++;
        };

        zdd_minelem(zdd_4, c);
        AssertThat(calls, Is().EqualTo(1u));
      });
    });

    describe("zdd_minelem(A, iter)", [&]() {
      using output_type = std::vector<zdd::label_type>;

      it("outputs {} for { Ø }", [&]() {
        output_type out;
        auto iter = zdd_minelem(zdd_T, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(0u));

        // Check state of 'iter'
        iter = 21;
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(21u));
      });

      it("outputs {1} in buffer for [1]", [&]() {
        output_type out;
        auto iter = zdd_minelem(zdd_1, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(1u));

        // Check state of 'iter'
        iter = 2;
        AssertThat(out.size(), Is().EqualTo(2u));
        AssertThat(out.at(0), Is().EqualTo(1u));
        AssertThat(out.at(1), Is().EqualTo(2u));
      });

      it("outputs { } in buffer for [2]", [&]() {
        output_type out;
        auto iter = zdd_minelem(zdd_2, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(0u));

        // Check state of 'iter'
        iter = 4;
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(4u));
      });

      it("outputs { 2,4 } in buffer for [3]", [&]() {
        output_type out;
        auto iter = zdd_minelem(zdd_3, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(2u));
        AssertThat(out.at(0), Is().EqualTo(2u));
        AssertThat(out.at(1), Is().EqualTo(4u));

        // Check state of 'iter'
        iter = 6;
        AssertThat(out.size(), Is().EqualTo(3u));
        AssertThat(out.at(1), Is().EqualTo(4u));
        AssertThat(out.at(2), Is().EqualTo(6u));
      });

      it("outputs { 1 } in buffer for [4]", [&]() {
        output_type out;
        auto iter = zdd_minelem(zdd_4, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(1u));

        // Check state of 'iter'
        iter = 2;
        AssertThat(out.size(), Is().EqualTo(2u));
        AssertThat(out.at(0), Is().EqualTo(1u));
        AssertThat(out.at(1), Is().EqualTo(2u));
      });
    });

    describe("zdd_maxelem(A)", [&]() {
      it("finds Ø on Ø", [&]() {
        zdd out = zdd_maxelem(zdd_F);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_type(true)));

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

      it("finds { Ø } on { Ø }", [&]() {
        zdd out = zdd_maxelem(zdd_T);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(zdd::node_type(true)));

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

      it("finds {0,2} on [1]", [&]() {
        zdd out = zdd_maxelem(zdd_1);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(2, zdd::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(
                     0, zdd::max_id, terminal_F, zdd::pointer_type(2, zdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

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

      it("finds {1} on [2]", [&]() {
        zdd out = zdd_maxelem(zdd_2);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(1, zdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

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

      it("finds {0,1} on [4]", [&]() {
        zdd out = zdd_maxelem(zdd_4);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(1, zdd::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(zdd::node_type(
                     0, zdd::max_id, terminal_F, zdd::pointer_type(1, zdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

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
    });

    describe("zdd_maxelem(A, c)", [&]() {
      it("makes no calls for { Ø } on { Ø }", [&]() {
        size_t calls = 0u;
        const auto c = [&calls](zdd::label_type) { calls++; };

        zdd_maxelem(zdd_T, c);
        AssertThat(calls, Is().EqualTo(0u));

        zdd out = zdd_maxelem(zdd_T);
      });

      it("calls 0,2 on [1]", [&]() {
        size_t calls = 0u;
        std::vector<zdd::label_type> expected{ 0, 2 };

        const auto c = [&calls, &expected](zdd::label_type x) {
          AssertThat(x, Is().EqualTo(expected.at(calls)));
          calls++;
        };

        zdd_maxelem(zdd_1, c);
        AssertThat(calls, Is().EqualTo(2u));
      });

      it("calls with 1 on [2]", [&]() {
        size_t calls = 0u;
        std::vector<zdd::label_type> expected{ 1 };

        const auto c = [&calls, &expected](zdd::label_type x) {
          AssertThat(x, Is().EqualTo(expected.at(calls)));
          calls++;
        };

        zdd_maxelem(zdd_2, c);
        AssertThat(calls, Is().EqualTo(1u));
      });

      it("calls with 0,1 on [4]", [&]() {
        size_t calls = 0u;
        std::vector<zdd::label_type> expected{ 0, 1 };

        const auto c = [&calls, &expected](zdd::label_type x) {
          AssertThat(x, Is().EqualTo(expected.at(calls)));
          calls++;
        };

        zdd_maxelem(zdd_4, c);
        AssertThat(calls, Is().EqualTo(2u));
      });
    });

    describe("zdd_maxelem(A, iter)", [&]() {
      using output_type = std::vector<zdd::label_type>;

      it("outputs { } in buffer for { Ø }", [&]() {
        output_type out;
        auto iter = zdd_maxelem(zdd_T, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(0u));

        // Check state of 'iter'
        iter = 2;
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(2u));
      });

      it("outputs { 0,2 } in buffer for [1]", [&]() {
        output_type out;
        auto iter = zdd_maxelem(zdd_1, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(2u));
        AssertThat(out.at(0), Is().EqualTo(0u));
        AssertThat(out.at(1), Is().EqualTo(2u));

        // Check state of 'iter'
        iter = 4;
        AssertThat(out.size(), Is().EqualTo(3u));
        AssertThat(out.at(1), Is().EqualTo(2u));
        AssertThat(out.at(2), Is().EqualTo(4u));
      });

      it("outputs { 1 } in buffer for [2]", [&]() {
        output_type out;
        auto iter = zdd_maxelem(zdd_2, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(1u));

        // Check state of 'iter'
        iter = 42;
        AssertThat(out.size(), Is().EqualTo(2u));
        AssertThat(out.at(0), Is().EqualTo(1u));
        AssertThat(out.at(1), Is().EqualTo(42u));
      });

      it("outputs { 0,1 } in buffer for [4]", [&]() {
        output_type out;
        auto iter = zdd_maxelem(zdd_4, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(2u));
        AssertThat(out.at(0), Is().EqualTo(0u));
        AssertThat(out.at(1), Is().EqualTo(1u));

        // Check state of 'iter'
        iter = 2;
        AssertThat(out.size(), Is().EqualTo(3u));
        AssertThat(out.at(1), Is().EqualTo(1u));
        AssertThat(out.at(2), Is().EqualTo(2u));
      });
    });
  });
});
