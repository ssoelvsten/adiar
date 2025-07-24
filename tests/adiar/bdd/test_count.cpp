#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/count.cpp", []() {
    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<bdd::node_type> bdd_1;
    /*
    //       1       ---- x0
    //      / \
    //      | 2      ---- x1
    //      |/ \
    //      3  |     ---- x2
    //     / \ /
    //     F  4      ---- x3
    //       / \
    //       F T
    */

    { // Garbage collect writer to free write-lock
      node n4 = node(3, 0, terminal_F, terminal_T);
      node n3 = node(2, 0, terminal_F, n4.uid());
      node n2 = node(1, 0, n3.uid(), n4.uid());
      node n1 = node(0, 0, n3.uid(), n2.uid());

      node_ofstream nw_1(bdd_1);
      nw_1 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_2;
    /*
    //               ---- x0
    //
    //        1      ---- x1
    //       / \
    //      2  |     ---- x2
    //     / \ /
    //     F  T
    */

    { // Garbage collect writer to free write-lock
      node n2 = node(2, 0, terminal_F, terminal_T);
      node n1 = node(1, 0, n2.uid(), terminal_T);

      node_ofstream nw_2(bdd_2);
      nw_2 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_3;
    /*
    //              ---- x0
    //
    //       1      ---- x1
    //      / \
    //     2  3     ---- x2
    //    / \/ \
    //    F T  F
    */

    { // Garbage collect writer to free write-lock
      node n3 = node(2, 1, terminal_T, terminal_F);
      node n2 = node(2, 0, terminal_F, terminal_T);
      node n1 = node(1, 0, n2.uid(), n3.uid());

      node_ofstream nw_3(bdd_3);
      nw_3 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_4;
    /*
    //              __1__      ---- x0
    //             /     \
    //            _2_   _3_    ---- x2
    //           /   \ /   \
    //           \    4    /   ---- x4
    //            \  / \  /
    //             \/   \/
    //             5    6      ---- x6
    //            / \  / \
    //            T F  F T
    */

    { // Garbage collect writer to free write-lock
      node n6 = node(6, 1, terminal_F, terminal_T);
      node n5 = node(6, 0, terminal_T, terminal_F);
      node n4 = node(4, 0, n5.uid(), n6.uid());
      node n3 = node(2, 1, n5.uid(), n4.uid());
      node n2 = node(2, 0, n4.uid(), n6.uid());
      node n1 = node(0, 0, n2.uid(), n3.uid());

      node_ofstream nw_4(bdd_4);
      nw_4 << n6 << n5 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_T;
    /*
    //        T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw_T(bdd_T);
      nw_T << node(true);
    }

    shared_levelized_file<bdd::node_type> bdd_F;
    /*
    //        F
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw_F(bdd_F);
      nw_F << node(false);
    }

    shared_levelized_file<bdd::node_type> bdd_root_1;
    /*
    //           1    ---- x1
    //          / \
    //          F T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw_root_1(bdd_root_1);
      nw_root_1 << node(1, 0, terminal_F, terminal_T);
    }

    // Set domain to be empty
    shared_file<bdd::label_type> empty_dom;
    domain_set(empty_dom);

    describe("bdd_nodecount", [&]() {
      it("can count number of nodes", [&]() {
        AssertThat(bdd_nodecount(bdd_1), Is().EqualTo(4u));
        AssertThat(bdd_nodecount(bdd_2), Is().EqualTo(2u));
        AssertThat(bdd_nodecount(bdd_3), Is().EqualTo(3u));
        AssertThat(bdd_nodecount(bdd_4), Is().EqualTo(6u));
        AssertThat(bdd_nodecount(bdd_T), Is().EqualTo(0u));
        AssertThat(bdd_nodecount(bdd_F), Is().EqualTo(0u));
        AssertThat(bdd_nodecount(bdd_root_1), Is().EqualTo(1u));
      });
    });

    describe("bdd_varcount", [&]() {
      it("can count number of variables", [&]() {
        AssertThat(bdd_varcount(bdd_1), Is().EqualTo(4u));
        AssertThat(bdd_varcount(bdd_2), Is().EqualTo(2u));
        AssertThat(bdd_varcount(bdd_3), Is().EqualTo(2u));
        AssertThat(bdd_varcount(bdd_4), Is().EqualTo(4u));
        AssertThat(bdd_varcount(bdd_T), Is().EqualTo(0u));
        AssertThat(bdd_varcount(bdd_F), Is().EqualTo(0u));
        AssertThat(bdd_varcount(bdd_root_1), Is().EqualTo(1u));
      });
    });

    describe("bdd_pathcount", [&]() {
      it("can count paths leading to T terminals [1]",
         [&]() { AssertThat(bdd_pathcount(bdd_1), Is().EqualTo(3u)); });

      it("can count paths leading to T terminals [2]",
         [&]() { AssertThat(bdd_pathcount(bdd_2), Is().EqualTo(2u)); });

      it("can count paths leading to T terminals [3]",
         [&]() { AssertThat(bdd_pathcount(bdd_3), Is().EqualTo(2u)); });

      it("can count paths leading to T terminals [4]",
         [&]() { AssertThat(bdd_pathcount(bdd_4), Is().EqualTo(6u)); });

      it("can count paths leading to F terminals [1]",
         [&]() { AssertThat(bdd_pathcount(bdd_not(bdd_1)), Is().EqualTo(5u)); });

      it("can count paths leading to F terminals [2]",
         [&]() { AssertThat(bdd_pathcount(bdd_not(bdd_2)), Is().EqualTo(1u)); });

      it("should count no paths in a true terminal-only BDD",
         [&]() { AssertThat(bdd_pathcount(bdd_T), Is().EqualTo(0u)); });

      it("should count no paths in a false terminal-only BDD",
         [&]() { AssertThat(bdd_pathcount(bdd_F), Is().EqualTo(0u)); });

      it("should count paths of a root-only BDD [1]",
         [&]() { AssertThat(bdd_pathcount(bdd_root_1), Is().EqualTo(1u)); });
    });

    describe("bdd_satcount(f, varcount)", [&]() {
      it("can count assignments leading to T terminals [1]", [&]() {
        AssertThat(bdd_satcount(bdd_1, 4), Is().EqualTo(5u));
        AssertThat(bdd_satcount(bdd_1, 5), Is().EqualTo(2 * 5u));
        AssertThat(bdd_satcount(bdd_1, 6), Is().EqualTo(2 * 2 * 5u));
      });

      it("can count assignments leading to T terminals [2]", [&]() {
        AssertThat(bdd_satcount(bdd_2, 2), Is().EqualTo(3u));
        AssertThat(bdd_satcount(bdd_2, 3), Is().EqualTo(2 * 3u));
        AssertThat(bdd_satcount(bdd_2, 5), Is().EqualTo(2 * 2 * 2 * 3u));
      });

      it("can count assignments leading to T terminals [3]", [&]() {
        AssertThat(bdd_satcount(bdd_3, 2), Is().EqualTo(2u));
        AssertThat(bdd_satcount(bdd_3, 3), Is().EqualTo(2 * 2u));
        AssertThat(bdd_satcount(bdd_3, 5), Is().EqualTo(2 * 2 * 2 * 2u));
      });

      it("can count assignments leading to T terminals [4]", [&]() {
        AssertThat(bdd_satcount(bdd_4, 4), Is().EqualTo(8u));
        AssertThat(bdd_satcount(bdd_4, 5), Is().EqualTo(2 * 8u));
        AssertThat(bdd_satcount(bdd_4, 8), Is().EqualTo(2 * 2 * 2 * 2 * 8u));
      });

      it("can count assignments leading to F terminals [1]", [&]() {
        AssertThat(bdd_satcount(bdd_not(bdd_1), 4), Is().EqualTo(11u));
        AssertThat(bdd_satcount(bdd_not(bdd_1), 5), Is().EqualTo(2 * 11u));
        AssertThat(bdd_satcount(bdd_not(bdd_1), 6), Is().EqualTo(2 * 2 * 11u));
      });

      it("can count assignments leading to F terminals [2]", [&]() {
        AssertThat(bdd_satcount(bdd_not(bdd_2), 2), Is().EqualTo(1u));
        AssertThat(bdd_satcount(bdd_not(bdd_2), 3), Is().EqualTo(2 * 1u));
        AssertThat(bdd_satcount(bdd_not(bdd_2), 5), Is().EqualTo(2 * 2 * 2 * 1u));
      });

      it("should count no assignments to the wrong terminal-only BDD", [&]() {
        AssertThat(bdd_satcount(bdd_not(bdd_T), 5), Is().EqualTo(0u));
        AssertThat(bdd_satcount(bdd_not(bdd_T), 4), Is().EqualTo(0u));
        AssertThat(bdd_satcount(bdd_F, 3), Is().EqualTo(0u));
        AssertThat(bdd_satcount(bdd_F, 2), Is().EqualTo(0u));
      });

      it("should count all assignments to the desired terminal-only BDD", [&]() {
        AssertThat(bdd_satcount(bdd_T, 5), Is().EqualTo(32u));
        AssertThat(bdd_satcount(bdd_T, 4), Is().EqualTo(16u));
        AssertThat(bdd_satcount(bdd_not(bdd_F), 3), Is().EqualTo(8u));
        AssertThat(bdd_satcount(bdd_not(bdd_F), 2), Is().EqualTo(4u));
      });

      it("throws exception on varcount being smaller than the number of levels [1]",
         [&]() { AssertThrows(invalid_argument, bdd_satcount(bdd_1, 3)); });

      it("throws exception on varcount being smaller than the number of levels [2]",
         [&]() { AssertThrows(invalid_argument, bdd_satcount(bdd_2, 1)); });

      it("throws exception on varcount being smaller than the number of levels [3]",
         [&]() { AssertThrows(invalid_argument, bdd_satcount(bdd_3, 1)); });

      it("throws exception on varcount being smaller than the number of levels [4]",
         [&]() { AssertThrows(invalid_argument, bdd_satcount(bdd_4, 3)); });
    });

    describe("bdd_satcount(f) [empty dom]", [&]() {
      it("can count assignments leading to T terminals [1]",
         [&]() { AssertThat(bdd_satcount(bdd_1), Is().EqualTo(5u)); });

      it("can count assignments leading to T terminals [2]",
         [&]() { AssertThat(bdd_satcount(bdd_2), Is().EqualTo(3u)); });

      it("can count assignments leading to F terminals [1]",
         [&]() { AssertThat(bdd_satcount(bdd_not(bdd_1)), Is().EqualTo(11u)); });

      it("can count assignments leading to F terminals [2]",
         [&]() { AssertThat(bdd_satcount(bdd_not(bdd_2)), Is().EqualTo(1u)); });

      it("should count no assignments to the true terminal-only BDD",
         [&]() { AssertThat(bdd_satcount(bdd_T), Is().EqualTo(0u)); });

      it("should count no assignments in a false terminal-only BDD", [&]() {
        AssertThat(bdd_satcount(bdd_not(bdd_F)), Is().EqualTo(0u));
        AssertThat(bdd_satcount(bdd_F), Is().EqualTo(0u));
      });

      it("should count assignments of a root-only BDD [1]", [&]() {
        AssertThat(bdd_satcount(bdd_not(bdd_root_1)), Is().EqualTo(1u));
        AssertThat(bdd_satcount(bdd_root_1), Is().EqualTo(1u));
      });
    });

    describe("bdd_satcount(f) [non-empty dom]", [&]() {
      shared_file<bdd::label_type> dom;
      {
        ofstream<ptr_uint64::label_type> lw(dom);
        lw << 0 << 1 << 2 << 3 << 4 << 5 << 6;
      }
      domain_set(dom);

      it("can count assignments leading to T terminals [1]",
         [&]() { AssertThat(bdd_satcount(bdd_1), Is().EqualTo(8u * 5u)); });

      it("can count assignments leading to T terminals [2]",
         [&]() { AssertThat(bdd_satcount(bdd_2), Is().EqualTo(32u * 3u)); });

      it("can count assignments leading to F terminals [1]",
         [&]() { AssertThat(bdd_satcount(bdd_not(bdd_1)), Is().EqualTo(8u * 11u)); });

      it("can count assignments leading to F terminals [2]",
         [&]() { AssertThat(bdd_satcount(bdd_not(bdd_2)), Is().EqualTo(32u * 1u)); });

      it("should count assignments to the true terminal-only BDD", [&]() {
        AssertThat(bdd_satcount(bdd_T), Is().EqualTo(128u * 1u));
        AssertThat(bdd_satcount(bdd_not(bdd_F)), Is().EqualTo(128u * 1u));
      });

      it("should count no assignments in a false terminal-only BDD",
         [&]() { AssertThat(bdd_satcount(bdd_F), Is().EqualTo(0u)); });

      it("should count assignments of a root-only BDD [1]", [&]() {
        AssertThat(bdd_satcount(bdd_not(bdd_root_1)), Is().EqualTo(64u * 1u));
        AssertThat(bdd_satcount(bdd_root_1), Is().EqualTo(64u * 1u));
      });
    });
  });
});
