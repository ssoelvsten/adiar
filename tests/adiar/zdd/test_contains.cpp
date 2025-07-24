#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/contains.cpp", [&]() {
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
    // { { 0 } }
    /*
    //      1        ---- x0
    //     / \
    //     F T
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(zdd_x0);
      nw << node(0, node::max_id, terminal_F, terminal_T);
    }

    // TODO: tests...

    shared_levelized_file<zdd::node_type> zdd_x1_null;
    // { Ø, { 1 } }
    /*
    //      1         ---- x1
    //     / \
    //     T T
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(zdd_x1_null);
      nw << node(1, node::max_id, terminal_T, terminal_T);
    }

    // TODO: tests...

    shared_levelized_file<zdd::node_type> zdd_1;
    // { Ø, { 0,2 }, { 0,3 } { 1,2 }, { 1,3 }, { 1,2,3 }, { 0,2,3 } }
    /*
    //          1      ---- x0
    //         / \
    //         2  \    ---- x1
    //        / \ /
    //        T  3     ---- x2
    //          / \
    //         4  5    ---- x3
    //        / \/ \
    //        F T  T
    */
    const node n1_5 = node(3, node::max_id, terminal_T, terminal_T);
    const node n1_4 = node(3, node::max_id - 1, terminal_F, terminal_T);
    const node n1_3 = node(2, node::max_id, n1_4.uid(), n1_5.uid());
    const node n1_2 = node(1, node::max_id, terminal_T, n1_3.uid());
    const node n1_1 = node(0, node::max_id, n1_2.uid(), n1_3.uid());

    { // Garbage collect writers to free write-lock
      node_ofstream nw(zdd_1);
      nw << n1_5 << n1_4 << n1_3 << n1_2 << n1_1;
    }

    shared_levelized_file<zdd::node_type> zdd_2;
    // { { 6 }, { 2,4 }, { 2,6 }, { 2,4,6 } }
    /*
    //         1       ---- x2
    //        / \
    //        | 2      ---- x4
    //        |/ \
    //        3  4     ---- x6
    //       / \/ \
    //       F T  T
    */
    const node n2_4 = node(6, node::max_id, terminal_T, terminal_T);
    const node n2_3 = node(6, node::max_id - 1, terminal_F, terminal_T);
    const node n2_2 = node(4, node::max_id, n2_3.uid(), n2_4.uid());
    const node n2_1 = node(2, node::max_id, n2_3.uid(), n2_2.uid());

    { // Garbage collect writers to free write-lock
      node_ofstream nw(zdd_2);
      nw << n2_4 << n2_3 << n2_2 << n2_1;
    }

    describe("zdd_contains(A, a_gen)", [&]() {
      // TODO
    });

    describe("zdd_contains(A, begin, end)", [&]() {
      it("returns false for Ø on Ø", [&]() {
        const std::vector<int> a = {};

        AssertThat(zdd_contains(zdd_F, a.begin(), a.end()), Is().False());
      });

      it("returns true for { Ø } on Ø", [&]() {
        const std::vector<int> a = {};

        AssertThat(zdd_contains(zdd_T, a.begin(), a.end()), Is().True());
      });

      it("returns false for { Ø } on { 1, 42 }", [&]() {
        const std::vector<int> a = { 1, 42 };

        AssertThat(zdd_contains(zdd_T, a.begin(), a.end()), Is().False());
      });

      it("returns visited root for [1] on Ø", [&]() {
        const std::vector<int> a = {};

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().True());
      });

      it("returns visited terminal for [1] on { 0 }", [&]() {
        const std::vector<int> a = { 0 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().False());
      });

      it("returns visited terminal for [1] on { 1 }", [&]() {
        const std::vector<int> a = { 1 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().False());
      });

      it("returns visited terminal for [1] on { 0, 2 }", [&]() {
        const std::vector<int> a = { 0, 2 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().True());
      });

      it("returns visited terminal for [1] on { 1, 3 }", [&]() {
        const std::vector<int> a = { 1, 3 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().True());
      });

      it("returns visited terminal for [1] on { 0, 2, 3 }", [&]() {
        const std::vector<int> a = { 0, 2, 3 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().True());
      });

      it("fails on missed label for [1] on { 0, 1, 2 }", [&]() {
        const std::vector<int> a = { 0, 1, 2 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().False());
      });

      it("fails on terminal with unread labels for [1] on { 2 }", [&]() {
        const std::vector<int> a = { 2 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().False());
      });

      it("fails on terminal with unread labels for [1] on { 0, 2, 4 }", [&]() {
        const std::vector<int> a = { 0, 2, 4 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().False());
      });

      it("fails on terminal with unread labels for [1] on { 0, 2, 3, 4 }", [&]() {
        const std::vector<int> a = { 0, 2, 3, 4 };

        AssertThat(zdd_contains(zdd_1, a.begin(), a.end()), Is().False());
      });

      it("returns visited root for [2] on Ø", [&]() {
        const std::vector<int> a = {};

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().False());
      });

      it("returns visited terminal for [2] on { 2 }", [&]() {
        const std::vector<int> a = { 2 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().False());
      });

      it("returns visited terminal for [2] on { 6 }", [&]() {
        const std::vector<int> a = { 6 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().True());
      });

      it("returns visited terminal for [2] on { 2, 4 }", [&]() {
        const std::vector<int> a = { 2, 4 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().True());
      });

      it("returns visited terminal for [2] on { 2, 4, 6 }", [&]() {
        const std::vector<int> a = { 2, 4, 6 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().True());
      });

      it("fails on missed label for [2] on { 4, 6 }", [&]() {
        const std::vector<int> a = { 4, 6 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().False());
      });

      it("fails on missed label for [2] on { 2, 3, 4 }", [&]() {
        const std::vector<int> a = { 2, 3, 4 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().False());
      });

      it("fails on missed label for [2] on { 2, 4, 6, 8 }", [&]() {
        const std::vector<int> a = { 2, 4, 6, 8 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().False());
      });

      it("fails on label before root for [2] on { 0, 2, 4 }", [&]() {
        const std::vector<int> a = { 0, 2, 4 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().False());
      });

      it("fails on labels before root for [2] on { 0, 1, 2, 4 }", [&]() {
        const std::vector<int> a = { 0, 1, 2, 4 };

        AssertThat(zdd_contains(zdd_2, a.begin(), a.end()), Is().False());
      });
    });
  });
});
