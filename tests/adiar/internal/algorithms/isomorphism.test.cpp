#include "../../../test.h"

#include <adiar/internal/algorithms/pred.h>

go_bandit([]() {
  describe("adiar/internal/algoritms/pred.h", []() {
    describe("is_isomorphic", []() {
      shared_levelized_file<dd::node_type> F;
      /*
      //           F
      */
      {
        node_ofstream w(F);
        w << node(false);
      }

      shared_levelized_file<dd::node_type> T;
      /*
      //           T
      */
      {
        node_ofstream w(T);
        w << node(true);
      }

      shared_levelized_file<dd::node_type> x42;
      /*
      //           1      ---- x42
      //          / \
      //          F T
      */
      {
        node_ofstream w(x42);
        w << node(42, node::max_id, node::pointer_type(false), node::pointer_type(true));
      }

      describe("Case: File Pointer Address", [&]() {
        it("accepts F when negation flags match", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(F, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(F, true), dd(F, true)), Is().True());
        });

        it("accepts T when negation flags match", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(F, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(F, true), dd(F, true)), Is().True());
        });

        it("accepts x42 when negation flags match", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false), dd(x42, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(x42, true)), Is().True());
        });

        it("rejects F when negation flags mismatch", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(F, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(F, true), dd(F, false)), Is().False());
        });

        it("rejects T when negation flags mismatch", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(F, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(F, true), dd(F, false)), Is().False());
        });

        it("rejects x42 when negation flags mismatch", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false), dd(x42, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(x42, false)), Is().False());
        });

        it("accepts x42 when shift of +0 matches", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false, +0), dd(x42, false, +0)),
                     Is().True());
        });

        it("accepts x42 when shift of +1 matches", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false, +1), dd(x42, false, +1)),
                     Is().True());
        });

        it("accepts x42 when shift of -21 matches", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false, -21), dd(x42, false, -21)),
                     Is().True());
        });

        it("rejects x42 when shift mismatches", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false, +0), dd(x42, false, +1)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false, +1), dd(x42, false, +0)),
                     Is().False());
        });

        it("rejects x42 when shift and negation flags mismatches", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false, +0), dd(x42, true, +1)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false, +1), dd(x42, true, +0)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true, +1), dd(x42, false, +2)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true, +2), dd(x42, false, +1)),
                     Is().False());
        });
      });

      shared_levelized_file<dd::node_type> x21;
      /*
      //         1      ---- x21
      //        / \
      //        F T
      */
      {
        node_ofstream w(x21);
        w << node(21, node::max_id, node::pointer_type(false), node::pointer_type(true));
      }

      shared_levelized_file<dd::node_type> x21_and_x42;
      /*
      //          1      ---- x21
      //         / \
      //         F 2     ---- x42
      //          / \
      //          F T
      */
      {
        const node n2(42, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n1(21, node::max_id, node::pointer_type(false), n2.uid());

        node_ofstream w(x21_and_x42);
        w << n2 << n1;
      }

      shared_levelized_file<dd::node_type> x21_xor_x42;
      /*
      //          _1_      ---- x21
      //         /   \
      //         2   3     ---- x42
      //        / \ / \
      //        T F F T
      */
      {
        const node n3(42, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n2(42, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n1(21, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(x21_xor_x42);
        w << n3 << n2 << n1;
      }

      describe("Case: #Nodes", [&]() {
        it("rejects F (#nodes = 0) vs. x21 (#nodes = 2)", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(x21, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(x21, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21, true), dd(F, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21, true), dd(F, true)), Is().False());
        });

        it("rejects x21 (#nodes = 1) vs. x21 & x42 (#nodes = 2)", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false), dd(x21_and_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false), dd(x21_and_x42, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(x21, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(x21, true)),
                     Is().False());
        });

        it("rejects x42 (#nodes = 1) vs. x21 & x42 (#nodes = 2)", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false), dd(x21_and_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false), dd(x21_and_x42, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(x42, true)),
                     Is().False());
        });

        it("rejects x21 (#nodes = 1) vs. x21 ^ x42 (#nodes = 3)", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false), dd(x21_xor_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false), dd(x21_xor_x42, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_xor_x42, true), dd(x21, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_xor_x42, true), dd(x21, true)),
                     Is().False());
        });

        it("rejects x21 & x42 (#nodes = 2) vs. x21 ^ x42 (#nodes = 3)", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x21_xor_x42, false), dd(x21_and_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_xor_x42, false), dd(x21_and_x42, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(x21_xor_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(x21_xor_x42, true)),
                     Is().False());
        });
      });

      shared_levelized_file<node> dd_4;
      /*
      // Similar to dd_3 (see further below) but mirrored horisontally
      //
      //           1     ---- x0
      //          / \
      //         2  |    ---- x1
      //        / \ /
      //        F  3     ---- x2
      //          / \
      //          4 T    ---- x3
      //         / \
      //         T F
      */
      {
        const node n4(3, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n3(2, node::max_id, n4.uid(), node::pointer_type(true));
        const node n2(1, node::max_id, node::pointer_type(false), n3.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_4);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_5;
      /*
      // Similar to dd_4 but (2) goes to a terminal instead of (3)
      //
      //          1        ---- x0
      //         / \
      //         2  \      ---- x1
      //        / \  \
      //        F T  3     ---- x2
      //            / \
      //            4 T    ---- x3
      //           / \
      //           T F
      */
      {
        const node n4(3, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n3(2, node::max_id, n4.uid(), node::pointer_type(true));
        const node n2(1, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_5);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_6;
      /*
      // Similar to dd_5 but (3) goes to F rather than T
      //
      //          1       ---- x0
      //         / \
      //         2  \     ---- x1
      //        / \  \
      //        F T  3    ---- x2
      //            / \
      //            4 F   ---- x3
      //           / \
      //           T F
      */
      {
        const node n4(3, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n3(2, node::max_id, n4.uid(), node::pointer_type(false));
        const node n2(1, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_6);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<dd::node_type> x21_x22_x42_chain;
      /*
      // This matches 'x21_xor_x42' above in the number of nodes and the number
      // of edges to each terminal.
      //
      // Hence, it only mismatches on width and the number of levels.
      //
      //          1      ---- x21
      //         / \
      //         F 2     ---- x22
      //          / \
      //          T 3    ---- x42
      //           / \
      //           F T
      */
      {
        const node n3(42, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n2(22, node::max_id, node::pointer_type(true), n3.uid());
        const node n1(21, node::max_id, node::pointer_type(false), n2.uid());

        node_ofstream w(x21_x22_x42_chain);
        w << n3 << n2 << n1;
      }

      describe("Case: Width", [&]() {
        it("rejects x21 ^ x42 (#levels = 2) vs. (x21 & ~x22) | (x21 & x42) (#levels = 3)", [&]() {
          AssertThat(
            is_isomorphic(exec_policy(), dd(x21_x22_x42_chain, false), dd(x21_xor_x42, false)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(x21_x22_x42_chain, false), dd(x21_xor_x42, true)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(x21_xor_x42, true), dd(x21_x22_x42_chain, false)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(x21_xor_x42, true), dd(x21_x22_x42_chain, true)),
            Is().False());
        });

        it("rejects due to different width", []() {
          // Create two BDDs with 7 nodes each on variables x0, x1, x2,
          // and x3 (i.e. the same number of nodes and levels). One with 3
          // nodes for x2 and one node for x3 and one with 2 nodes for x2 and x3.
          shared_levelized_file<dd::node_type> in_a;
          /*
          //                _1_        ---- x0
          //               /   \
          //             _2_   _3_     ---- x1
          //            /   \ /   \
          //            4    5    6    ---- x2
          //           / \  / \  / \
          //           F  \ T |  F T
          //               \ /
          //                7          ---- x3
          //               / \
          //               F T
          */
          { // Garbage collect writers to free write-lock
            const node n7(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n6(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n5(2, node::max_id - 1, node::pointer_type(true), n7.uid());
            const node n4(2, node::max_id - 2, node::pointer_type(false), n7.uid());
            const node n3(1, node::max_id, n5.uid(), n6.uid());
            const node n2(1, node::max_id - 1, n4.uid(), n5.uid());
            const node n1(0, node::max_id, n2.uid(), n2.uid());

            node_ofstream w(in_a);
            w << n7 << n6 << n5 << n4 << n3 << n2 << n1;
          }

          shared_levelized_file<dd::node_type> in_b;
          /*
          //              1         ---- x0
          //             / \
          //             2 3        ---- x1
          //             |X|
          //            _4 5_       ---- x2
          //           / | | \
          //           F 6 7 T      ---- x3
          //            /| |\
          //            TF FT
          */
          { // Garbage collect writers to free write-lock
            const node n7(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n6(3, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
            const node n5(2, node::max_id, n7.uid(), node::pointer_type(true));
            const node n4(2, node::max_id - 1, node::pointer_type(false), n6.uid());
            const node n3(1, node::max_id, n4.uid(), n5.uid());
            const node n2(1, node::max_id - 1, n5.uid(), n4.uid());
            const node n1(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream w(in_b);
            w << n7 << n6 << n5 << n4 << n3 << n2 << n1;
          }

          AssertThat(is_isomorphic(exec_policy(), dd(in_a, false), dd(in_b, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(in_a, false), dd(in_b, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(in_b, true), dd(in_a, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(in_b, true), dd(in_a, true)), Is().False());
        });
      });

      describe("Case: #Terminal Arcs", [&]() {
        it("rejects F (̈́[1,0]) vs. T ([0,1])", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(T, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(T, true), dd(F, true)), Is().False());
        });

        it("rejects F (̈́[1,0]) vs. ~F ([0,1])", [&]() {
          const auto F_copy = shared_levelized_file<dd::node_type>::copy(F);

          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(F_copy, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(F_copy, true), dd(F, false)), Is().False());
        });

        it("rejects F ([1,0]) vs. x42 ([1,1])", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(x42, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(F, false), dd(x42, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(F, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(F, true)), Is().False());
        });

        it("rejects T ([0,1]) vs. x21 & x42 ([2,1])", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(T, false), dd(x21_and_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(T, false), dd(x21_and_x42, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(T, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(T, true)),
                     Is().False());
        });

        it("rejects x21 & x42 (̈́[1,2]) vs. ~x21 | (x21 & x42) ([2,1])", [&]() {
          shared_levelized_file<dd::node_type> a = x21_and_x42;

          shared_levelized_file<dd::node_type> b;
          /*
          //          1      ---- x21
          //         / \
          //         T 2     ---- x42
          //          / \
          //          F T
          */
          {
            const node n2(42, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n1(21, node::max_id, node::pointer_type(true), n2.uid());

            node_ofstream w(b);
            w << n2 << n1;
          }

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(b, false), dd(a, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(b, true), dd(a, true)), Is().False());
        });

        it("rejects x21 & x42 (̈́[1,2]) vs. ~(x21 & ~x42) ([2,1])", [&]() {
          shared_levelized_file<dd::node_type> a = x21_and_x42;

          shared_levelized_file<dd::node_type> b;
          /*
          //          1      ---- x21
          //         / \
          //         F 2     ---- x42
          //          / \
          //          T F
          */
          {
            const node n2(42, node::max_id, node::pointer_type(true), node::pointer_type(false));
            const node n1(21, node::max_id, node::pointer_type(false), n2.uid());

            node_ofstream w(b);
            w << n2 << n1;
          }

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(b, true), dd(a, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(b, false), dd(a, true)), Is().False());
        });

        it("rejects x21 ([1,1]) vs. x21 & x42 ([2,1])", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false), dd(x21_and_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false), dd(x21_and_x42, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(x21, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, true), dd(x21, true)),
                     Is().False());
        });

        it("rejects [4] ([2,2]) vs. ~[5] ([3,2]) due to one extra F terminal", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4, false), dd(dd_5, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4, false), dd(dd_5, true)), Is().False());
        });

        it("rejects [4] ([2,2]) vs. [5] ([2,3]) due to one extra T terminal", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4, false), dd(dd_5, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_5, false), dd(dd_4, false)), Is().False());
        });

        it("rejects [5] ([2,3]) vs. [6] ([3,2])", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_5, false), dd(dd_6, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_5, true), dd(dd_6, true)), Is().False());
        });
      });

      shared_levelized_file<node> dd_8;
      /*
      //           __1__     ---- x0
      //          /     \
      //          2     3    ---- x1
      //         / \   / \
      //         F |   4 T   ---- x2
      //           |  / \
      //           5  T F    ---- x3
      //          / \
      //          F T
      */
      {
        const node n5(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, n4.uid(), node::pointer_type(true));
        const node n2(1, node::max_id - 1, node::pointer_type(false), n5.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_8);
        w << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_9;
      /*
      // Similar to dd_8, but (5) is moved up to the same level as (4) which
      // does not change the width, the number of nodes, nor the number of
      // terminal arcs.
      //
      //           __1__      ---- x0
      //          /     \
      //          2     3     ---- x1
      //         / \   / \
      //         F 4   5 T    ---- x2
      //          / \ / \
      //          T F F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, n4.uid(), node::pointer_type(true));
        const node n2(1, node::max_id - 1, node::pointer_type(false), n5.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_9);
        w << n5 << n4 << n3 << n2 << n1;
      }

      describe("Case: #Levels", [&]() {
        it("rejects [8] vs. [9] on the number of levels", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_8, false), dd(dd_9, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_8, false), dd(dd_9, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_9, true), dd(dd_8, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_9, true), dd(dd_8, true)), Is().False());
        });
      });

      shared_levelized_file<dd::node_type> x21_and_x22;
      /*
      //          1      ---- x21
      //         / \
      //         F 2     ---- x42
      //          / \
      //          F T
      */
      {
        const node n2(22, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n1(21, node::max_id, node::pointer_type(false), n2.uid());

        node_ofstream w(x21_and_x22);
        w << n2 << n1;
      }

      describe("Case: Individual Level's Label and Width)", [&]() {
        it("rejects x21 vs x42 on the label", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false), dd(x42, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false), dd(x42, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(x21, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(x21, true)), Is().False());
        });

        it("rejects x21 & x42 vs x21 & x22 on the label", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, false), dd(x21_and_x22, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x42, false), dd(x21_and_x22, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x22, true), dd(x21_and_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21_and_x22, true), dd(x21_and_x42, true)),
                     Is().False());
        });

        it("rejects shifted x42 on the label", [&]() {
          const auto a = x42;
          const auto b = shared_levelized_file<node>::copy(x42);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false, +1), dd(b, false, +0)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(a, false, +0), dd(b, false, +1)),
                     Is().False());
        });

        it("rejects shifted x21 vs x42 on the label", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false, +10), dd(x42, false, -10)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false, +10), dd(x42, true, -10)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true, -10), dd(x21, false, +10)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true, -10), dd(x21, true, +10)),
                     Is().False());
        });

        it("rejects due to levels have mismatching width", []() {
          shared_levelized_file<dd::node_type> in_a;
          /*
          //               _1_        ---- x0
          //              /   \
          //              2   3       ---- x1
          //             / \ / \
          //             |  4  |      ---- x2
          //             \ / \ /
          //              5   6       ---- x3
          //             / \ / \
          //             T F F T
          */
          { // Garbage collect writers to free write-lock
            const node n6(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n5(3, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
            const node n4(2, node::max_id, n5.uid(), n6.uid());
            const node n3(1, node::max_id, n4.uid(), n6.uid());
            const node n2(1, node::max_id - 1, n5.uid(), n4.uid());
            const node n1(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream w(in_a);
            w << n6 << n5 << n4 << n3 << n2 << n1;
          }

          shared_levelized_file<dd::node_type> in_b;
          /*
          //                 _1_       ---- x0
          //                /   \
          //               _2_   3     ---- x1
          //              /   \ / \
          //              |    4  5    ---- x2
          //              \   /| / \
          //               \  |F | T
          //                \ | /
          //                 \|/
          //                  6        ---- x3
          //                 / \
          //                 F T
          */
          { // Garbage collect writers to free write-lock
            const node n6(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n5(2, node::max_id, n6.uid(), node::pointer_type(true));
            const node n4(2, node::max_id, n6.uid(), node::pointer_type(false));
            const node n3(1, node::max_id, n4.uid(), n5.uid());
            const node n2(1, node::max_id - 1, n6.uid(), n4.uid());
            const node n1(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream w(in_b);
            w << n6 << n5 << n4 << n3 << n2 << n1;
          }

          AssertThat(is_isomorphic(exec_policy(), dd(in_a, false), dd(in_b, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(in_a, false), dd(in_b, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(in_b, true), dd(in_a, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(in_b, true), dd(in_a, true)), Is().False());
        });
      });

      shared_levelized_file<dd::node_type> not_x42;
      /*
      //         1      ---- x42
      //        / \
      //        T F
      */

      { // Garbage collect writers to free write-lock
        node_ofstream w(not_x42);
        w << node(42, node::max_id, node::pointer_type(true), node::pointer_type(false));
      }

      shared_levelized_file<dd::node_type> trivial_x69;
      /*
      //          1       ---- x69
      //         / \
      //         T T
      */

      { // Garbage collect writers to free write-lock
        node_ofstream w(trivial_x69);
        w << node(60, node::max_id, node::pointer_type(true), node::pointer_type(true));
      }

      shared_levelized_file<dd::node_type> not_trivial_x69;
      /*
      //          1       ---- x69
      //         / \
      //         F F
      */

      { // Garbage collect writers to free write-lock
        node_ofstream w(not_trivial_x69);
        w << node(60, node::max_id, node::pointer_type(false), node::pointer_type(false));
      }

      shared_levelized_file<node> dd_1;
      /*
      //      _1_     ---- x0
      //     /   \
      //     2   3    ---- x1
      //    / \ / \
      //    T  4  F   ---- x2
      //      / \
      //      F T
      */
      {
        const node n4(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n3(1, node::max_id, n4.uid(), node::pointer_type(false));
        const node n2(1, node::max_id - 1, node::pointer_type(true), n4.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_1);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_1n;
      /*
      // All terminals flipped.
      //
      //      _1_     ---- x0
      //     /   \
      //     2   3    ---- x1
      //    / \ / \
      //    F  4  T   ---- x2
      //      / \
      //      T F
      */
      {
        const node n4(2, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, n4.uid(), node::pointer_type(true));
        const node n2(1, node::max_id - 1, node::pointer_type(false), n4.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_1n);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_1_low_leaf;
      /*
      // Low terminals of (2) and (4) flipped.
      //
      //      _1_     ---- x0
      //     /   \
      //     2   3    ---- x1
      //    / \ / \
      //    F  4  F   ---- x2
      //      / \
      //      T T
      */
      {
        const node n4(2, node::max_id, node::pointer_type(true), node::pointer_type(true));
        const node n3(1, node::max_id, n4.uid(), node::pointer_type(false));
        const node n2(1, node::max_id - 1, node::pointer_type(false), n4.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_1_low_leaf);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_1_high_leaf;
      /*
      // High terminals of (3) and (4) flipped.
      //
      //
      //      _1_         ---- x0
      //     /   \
      //     2   3        ---- x1
      //    / \ / \
      //    T  4  T       ---- x2
      //      / \
      //      F F
      */
      {
        const node n4(2, node::max_id, node::pointer_type(false), node::pointer_type(false));
        const node n3(1, node::max_id, n4.uid(), node::pointer_type(true));
        const node n2(1, node::max_id - 1, node::pointer_type(true), n4.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_1_high_leaf);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_2;
      /*
      //        _1_          ---- x0
      //       /   \
      //      _2_ _3_        ---- x1
      //     /   X   \
      //     F  / \  F
      //       4   5         ---- x2
      //      / \ / \
      //      T F F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, n4.uid(), node::pointer_type(false));
        const node n2(1, node::max_id - 1, node::pointer_type(false), n5.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_2);
        w << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_2n;
      /*
      // Negation of the above (by flipping terminals of (2) and (3) and
      // swapping their references to n4 and n5 to keep the diagram canonical)
      //
      //        __1__        ---- x0
      //       /     \
      //       2     3       ---- x1
      //      / \   / \
      //      T 4   5 T      ---- x2
      //       / \ / \
      //       T F F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, n5.uid(), node::pointer_type(true));
        const node n2(1, node::max_id - 1, node::pointer_type(true), n4.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_2n);
        w << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_2_low_child;
      /*
      // During traversal this look similar to dd_2 until (2) on level x1
      //
      //          __1__        ---- x0
      //         /     \
      //        _2_    3       ---- x1
      //       /   \  / \
      //       4   5  T F      ---- x2
      //      / \ / \
      //      T F F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n2(1, node::max_id - 1, n4.uid(), n5.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_2_low_child);
        w << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_2_low_child2;
      /*
      // During traversal this look similar to dd_2_low_child until (2) on level x1
      //
      //          __1__        ---- x0
      //         /     \
      //         2     3       ---- x1
      //        _X_   / \
      //       4   5  T F      ---- x2
      //      / \ / \
      //      T F F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n2(1, node::max_id - 1, n5.uid(), n4.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_2_low_child2);
        w << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_2_high_child;
      /*
      // During traversal this look similar to dd_2 until (2) on level x1
      //
      //        __1__        ---- x0
      //       /     \
      //       2     3       ---- x1
      //      / \   / \
      //      F 4   5 F      ---- x2
      //       / \ / \
      //       T F F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, n5.uid(), node::pointer_type(false));
        const node n2(1, node::max_id - 1, node::pointer_type(false), n4.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_2_high_child);
        w << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_3;
      /*
      //         1      ---- x0
      //        / \
      //        |  2    ---- x1
      //        \ / \
      //         3  F   ---- x2
      //        / \
      //        T 4     ---- x3
      //         / \
      //         F T
      */
      {
        const node n4(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n3(2, node::max_id, node::pointer_type(true), n4.uid());
        const node n2(1, node::max_id, n3.uid(), node::pointer_type(false));
        const node n1(0, node::max_id, n3.uid(), n2.uid());

        node_ofstream w(dd_3);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_3n;
      /*
      // Same as dd_3 but with all terminals flipped
      //
      //         1      ---- x0
      //        / \
      //        |  2    ---- x1
      //        \ / \
      //         3  T  ---- x2
      //        / \
      //        F 4     ---- x3
      //         / \
      //         T F
      */
      {
        const node n4(3, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n3(2, node::max_id, node::pointer_type(false), n4.uid());
        const node n2(1, node::max_id, n3.uid(), node::pointer_type(true));
        const node n1(0, node::max_id, n3.uid(), n2.uid());

        node_ofstream w(dd_3n);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_3_low_child;
      /*
      // Same as dd_3 but with (2) directly going to (4) on low
      //
      //         _1_      ---- x0
      //        /   \
      //        |   2     ---- x1
      //        |  / \
      //        3 /  F    ---- x2
      //       / \|
      //       T  4       ---- x3
      //         / \
      //         F T
      */
      {
        const node n4(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n3(2, node::max_id, node::pointer_type(true), n4.uid());
        const node n2(1, node::max_id, n4.uid(), node::pointer_type(false));
        const node n1(0, node::max_id, n3.uid(), n2.uid());

        node_ofstream w(dd_3_low_child);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_4n;
      /*
      // Similar to dd_4 but with terminals flipped.
      //
      //           1     ---- x0
      //          / \
      //         2  |    ---- x1
      //        / \ /
      //        T  3     ---- x2
      //          / \
      //          4 F    ---- x3
      //         / \
      //         F T
      */
      {
        const node n4(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n3(2, node::max_id, n4.uid(), node::pointer_type(false));
        const node n2(1, node::max_id, node::pointer_type(true), n3.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_4n);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_4_high_child;
      /*
      // Same as dd_4 but with (2) directly going to (4) on the high
      //
      //           _1_           ---- x0
      //          /   \
      //          2   |          ---- x1
      //         / \  |
      //         F  \ 3          ---- x2
      //            |/ \
      //            4  T         ---- x3
      //           / \
      //           T F
      */
      {
        const node n4(3, node::max_id, node::pointer_type(true), node::pointer_type(false));
        const node n3(2, node::max_id, n4.uid(), node::pointer_type(true));
        const node n2(1, node::max_id, node::pointer_type(false), n4.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_4_high_child);
        w << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_7;
      /*
      //         __1__       ---- x0
      //        /     \
      //        2     3      ---- x1
      //       / \   / \
      //       4 T   5 T     ---- x2
      //      / \   / \
      //      T F   F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, n5.uid(), node::pointer_type(true));
        const node n2(1, node::max_id - 1, n4.uid(), node::pointer_type(true));
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_7);
        w << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_7n;
      /*
      // Same as dd_7 but with terminals negated (and sorted to keep it
      // canonical)
      //
      //           1         ---- x0
      //         __X__
      //        2     3      ---- x1
      //       / \   / \
      //       4 F   5 F     ---- x2
      //      / \   / \
      //      T F   F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, n5.uid(), node::pointer_type(false));
        const node n2(1, node::max_id - 1, n4.uid(), node::pointer_type(false));
        const node n1(0, node::max_id, n3.uid(), n2.uid());

        node_ofstream w(dd_7n);
        w << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<node> dd_7_high_child;
      /*
      // During traversal this look similar to dd_5 until (3) on level x1
      //
      //         __1__        ---- x0
      //        /     \
      //       _2_     3      ---- x1
      //      /   \    ||
      //      4   5    T      ---- x2
      //     / \ / \
      //     T F F T
      */
      {
        const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
        const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
        const node n3(1, node::max_id, node::pointer_type(true), node::pointer_type(true));
        const node n2(1, node::max_id - 1, n4.uid(), n5.uid());
        const node n1(0, node::max_id, n2.uid(), n3.uid());

        node_ofstream w(dd_7_high_child);
        w << n5 << n4 << n3 << n2 << n1;
      }

      describe("Case: N/B Comparison Check", [&]() {
        // NOTE: The fast checks require the BDDs to be on 'canonical' form,
        // which makes two isomorphic BDDs truly identical.

        it("accepts F", [&]() {
          const auto a = F;
          const auto b = shared_levelized_file<node>::copy(F);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("accepts T", [&]() {
          const auto a = T;
          const auto b = shared_levelized_file<node>::copy(T);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("accepts x42", [&]() {
          const auto a = x42;
          const auto b = shared_levelized_file<node>::copy(x42);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("accepts x42 and x21 [+21 shift]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false), dd(x21, false, +21)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(x21, true, +21)), Is().True());
        });

        it("accepts ~x42", [&]() {
          const auto a = not_x42;
          const auto b = shared_levelized_file<node>::copy(not_x42);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("rejects on child mismatch [terminal value]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(not_x42, false), dd(x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false), dd(not_x42, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(not_x42, true), dd(x42, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(not_x42, true)), Is().False());
        });

        it("accepts [trivial_x69]", [&]() {
          const auto a = trivial_x69;
          const auto b = shared_levelized_file<node>::copy(trivial_x69);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("rejects on child mismatch [terminal value]", [&]() {
          AssertThat(
            is_isomorphic(exec_policy(), dd(trivial_x69, false), dd(not_trivial_x69, false)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(not_trivial_x69, false), dd(trivial_x69, false)),
            Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(trivial_x69, true), dd(not_trivial_x69, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(not_trivial_x69, true), dd(trivial_x69, true)),
                     Is().False());
        });

        it("accepts [1]", [&]() {
          const auto a = dd_1;
          const auto b = shared_levelized_file<node>::copy(dd_1);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("accepts shifted [1]", [&]() {
          shared_levelized_file<node> dd_1_shift;
          /*
          //      _1_     ---- x1
          //     /   \
          //     2   3    ---- x2
          //    / \ / \
          //    T  4  F   ---- x3
          //      / \
          //      F T
          */
          {
            const node n4(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n3(2, node::max_id, n4.uid(), node::pointer_type(false));
            const node n2(2, node::max_id - 1, node::pointer_type(true), n4.uid());
            const node n1(1, node::max_id, n2.uid(), n3.uid());

            node_ofstream w(dd_1_shift);
            w << n4 << n3 << n2 << n1;
          }

          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, false, +1), dd(dd_1_shift, false)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_shift, false), dd(dd_1, false, +1)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, true, +1), dd(dd_1_shift, true)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_shift, true), dd(dd_1, true, +1)),
                     Is().True());
        });

        it("rejects on low child mismatch [terminal value]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, false), dd(dd_1n, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n, false), dd(dd_1, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, true), dd(dd_1n, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n, true), dd(dd_1, true)), Is().False());
        });

        it("rejects on low child mismatch [terminal value]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, false), dd(dd_1_low_leaf, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_low_leaf, false), dd(dd_1, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, true), dd(dd_1_low_leaf, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_low_leaf, true), dd(dd_1, true)),
                     Is().False());
        });

        it("rejects on high child mismatch [terminal value]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, false), dd(dd_1_high_leaf, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_high_leaf, false), dd(dd_1, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, true), dd(dd_1_high_leaf, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_high_leaf, true), dd(dd_1, true)),
                     Is().False());
        });

        it("accepts [2]", [&]() {
          const auto a = dd_2;
          const auto b = shared_levelized_file<node>::copy(dd_2);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("rejects on low child mismatch [terminal value]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, false), dd(dd_2n, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, false), dd(dd_2, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, true), dd(dd_2n, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, true), dd(dd_2, true)), Is().False());
        });

        it("rejects on low child mismatch [internal vs. terminal]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, false), dd(dd_2_low_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2_low_child, false), dd(dd_2, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, true), dd(dd_2_low_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2_low_child, true), dd(dd_2, true)),
                     Is().False());
        });

        it("rejects on low child mismatch [node id]", [&]() {
          AssertThat(
            is_isomorphic(exec_policy(), dd(dd_2_low_child, false), dd(dd_2_low_child2, false)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(dd_2_low_child2, false), dd(dd_2_low_child, false)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(dd_2_low_child, true), dd(dd_2_low_child2, true)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(dd_2_low_child2, true), dd(dd_2_low_child, true)),
            Is().False());
        });

        it("rejects on high child mismatch [node id]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, false), dd(dd_2_high_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2_high_child, false), dd(dd_2, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, true), dd(dd_2_high_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2_high_child, true), dd(dd_2, true)),
                     Is().False());
        });

        it("accepts [3]", [&]() {
          const auto a = dd_3;
          const auto b = shared_levelized_file<node>::copy(dd_3);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("rejects on low child mismatch [node label]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3, false), dd(dd_3_low_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3_low_child, false), dd(dd_3, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3, true), dd(dd_3_low_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3_low_child, true), dd(dd_3, true)),
                     Is().False());
        });

        it("accepts [4]", [&]() {
          const auto a = dd_4;
          const auto b = shared_levelized_file<node>::copy(dd_4);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("rejects on high child mismatch [node label]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4, false), dd(dd_4_high_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4_high_child, false), dd(dd_4, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4, true), dd(dd_4_high_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4_high_child, true), dd(dd_4, true)),
                     Is().False());
        });

        it("accepts [7]", [&]() {
          const auto a = dd_7;
          const auto b = shared_levelized_file<node>::copy(dd_7);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, true)), Is().True());
        });

        it("rejects on root's children flipped [node id]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7, false), dd(dd_7n, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7n, false), dd(dd_7, false)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7, true), dd(dd_7n, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7n, true), dd(dd_7, true)), Is().False());
        });

        it("rejects on high child mismatch [internal vs. terminal]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7, false), dd(dd_7_high_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7_high_child, false), dd(dd_7, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7, true), dd(dd_7_high_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7_high_child, true), dd(dd_7, true)),
                     Is().False());
        });
      });

      describe("Case: O(sort(N)) Comparison Check", [&]() {
        it("accepts ~F and T", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(T, false), dd(F, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(F, true), dd(T, false)), Is().True());
        });

        it("accepts F and ~T", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(T, false), dd(F, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(F, true), dd(T, false)), Is().True());
        });

        it("rejects x42 and ~x42", [&]() {
          const auto a = x42;
          const auto b = shared_levelized_file<node>::copy(x42);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, false)), Is().False());
        });

        it("accepts x42 and negated ~x42", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x42, false), dd(not_x42, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(x42, true), dd(not_x42, false)), Is().True());
        });

        it("accepts negated ~x42 and shifted x21", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(x21, false, +21), dd(not_x42, true)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(not_x42, true), dd(x21, false, +21)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(x21, true, +21), dd(not_x42, false)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(not_x42, false), dd(x21, true, +21)),
                     Is().True());
        });

        it("accepts [1] and negated [~1]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, false), dd(dd_1n, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, true), dd(dd_1n, false)), Is().True());
        });

        it("accepts [1] and shifted negated [~1]", [&]() {
          shared_levelized_file<node> dd_1n_shift;
          /*
          //      _1_     ---- x1
          //     /   \
          //     2   3    ---- x2
          //    / \ / \
          //    F  4  T   ---- x3
          //      / \
          //      T F
          */
          {
            const node n4(3, node::max_id, node::pointer_type(true), node::pointer_type(false));
            const node n3(2, node::max_id, n4.uid(), node::pointer_type(true));
            const node n2(2, node::max_id - 1, node::pointer_type(false), n4.uid());
            const node n1(1, node::max_id, n2.uid(), n3.uid());

            node_ofstream w(dd_1n_shift);
            w << n4 << n3 << n2 << n1;
          }

          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, false, +1), dd(dd_1n_shift, true)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n_shift, false), dd(dd_1, true, +1)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, true, +1), dd(dd_1n_shift, false)),
                     Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n_shift, true), dd(dd_1, false, +1)),
                     Is().True());
        });

        it("rejects on low child mismatch [terminal value]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n, true), dd(dd_1_low_leaf, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_low_leaf, false), dd(dd_1n, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n, false), dd(dd_1_low_leaf, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_low_leaf, true), dd(dd_1n, false)),
                     Is().False());
        });

        it("rejects on high child mismatch [terminal value]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n, true), dd(dd_1_high_leaf, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_high_leaf, false), dd(dd_1n, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n, false), dd(dd_1_high_leaf, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1_high_leaf, true), dd(dd_1n, false)),
                     Is().False());
        });

        it("accepts [2] and negated [~2]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, false), dd(dd_2n, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, true), dd(dd_2, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, true), dd(dd_2n, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, false), dd(dd_2, true)), Is().True());
        });

        it("rejects on low child mismatch [internal vs. terminal]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, true), dd(dd_2_low_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2_low_child, false), dd(dd_2n, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, false), dd(dd_2_low_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2_low_child, true), dd(dd_2n, false)),
                     Is().False());
        });

        it("rejects on low child mismatch [node id]", [&]() {
          shared_levelized_file<node> dd_2_low_childn;
          /*
          // Same as dd_2_low_child but all terminals flipped (by swapping
          // children of (2) and (3) to keep it canonical)
          //
          //          __1__        ---- x0
          //         /     \
          //         2     3       ---- x1
          //        _X_   / \
          //       4   5  F T      ---- x2
          //      / \ / \
          //      T F F T
          */
          {
            const node n5(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n4(2, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
            const node n3(1, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n2(1, node::max_id - 1, n5.uid(), n4.uid());
            const node n1(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream w(dd_2_low_childn);
            w << n5 << n4 << n3 << n2 << n1;
          }

          AssertThat(
            is_isomorphic(exec_policy(), dd(dd_2_low_childn, true), dd(dd_2_low_child2, false)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(dd_2_low_child2, false), dd(dd_2_low_childn, true)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(dd_2_low_childn, false), dd(dd_2_low_child2, true)),
            Is().False());
          AssertThat(
            is_isomorphic(exec_policy(), dd(dd_2_low_child2, true), dd(dd_2_low_childn, false)),
            Is().False());
        });

        it("rejects on high child mismatch [node id]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, true), dd(dd_2_high_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2_high_child, false), dd(dd_2n, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, false), dd(dd_2_high_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2_high_child, true), dd(dd_2n, false)),
                     Is().False());
        });

        it("accepts [3] and negated [~3]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3, false), dd(dd_3n, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3n, true), dd(dd_3, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3, true), dd(dd_3n, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3n, false), dd(dd_3, true)), Is().True());
        });

        it("rejects on low child mismatch [node label]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3n, true), dd(dd_3_low_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3_low_child, false), dd(dd_3n, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3n, false), dd(dd_3_low_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_3_low_child, true), dd(dd_3n, false)),
                     Is().False());
        });

        it("accepts [4] and negated [~4]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4, false), dd(dd_4n, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4n, false), dd(dd_4, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4, true), dd(dd_4n, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4n, true), dd(dd_4, false)), Is().True());
        });

        it("rejects on high child mismatch [node label]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4n, true), dd(dd_4_high_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4_high_child, false), dd(dd_4n, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4n, false), dd(dd_4_high_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_4_high_child, true), dd(dd_4n, false)),
                     Is().False());
        });

        it("accepts [7] and negated [~7]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7, false), dd(dd_7n, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7n, false), dd(dd_7, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7, true), dd(dd_7n, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7n, true), dd(dd_7, false)), Is().True());
        });

        it("rejects on high child mismatch [internal vs. terminal]", [&]() {
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7n, true), dd(dd_7_high_child, false)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7_high_child, false), dd(dd_7n, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7n, false), dd(dd_7_high_child, true)),
                     Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_7_high_child, true), dd(dd_7n, false)),
                     Is().False());
        });

        it("rejects on root's children mismatching", [&]() {
          shared_levelized_file<node> a;
          /*
          //          1       ---- x1
          //         / \
          //         2 F      ---- x0
          //        / \
          //        F T
          */
          {
            const node n2(1, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n1(0, node::max_id, n2.uid(), node::pointer_type(false));

            node_ofstream w(a);
            w << n2 << n1;
          }

          shared_levelized_file<node> b;
          /*
          //          1     ---- x1
          //         / \
          //         F 2    ---- x0
          //          / \
          //          F T
          */
          {
            const node n2(1, node::max_id, node::pointer_type(true), node::pointer_type(false));
            const node n1(0, node::max_id, node::pointer_type(true), n2.uid());

            node_ofstream w(b);
            w << n2 << n1;
          }

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, false)), Is().False());
        });

        it("rejects on root's low child mismatches [terminal value]", [&]() {
          shared_levelized_file<node> a;
          /*
          //            1       ---- x0
          //           / \
          //           T 2      ---- x1
          //            / \
          //            3 F     ---- x2
          //           / \
          //           F T
          */
          {
            const node n3(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n2(1, node::max_id, n3.uid(), node::pointer_type(false));
            const node n1(0, node::max_id, node::pointer_type(true), n2.uid());

            node_ofstream w(a);
            w << n3 << n2 << n1;
          }

          const auto b = shared_levelized_file<node>::copy(a);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, false)), Is().False());
        });

        it("rejects on root's high child mismatches [terminal value]", [&]() {
          shared_levelized_file<node> a;
          /*
          //            1       ---- x0
          //           / \
          //           2 T      ---- x1
          //          / \
          //          3 F       ---- x2
          //         / \
          //         F T
          */
          {
            const node n3(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n2(1, node::max_id, n3.uid(), node::pointer_type(false));
            const node n1(0, node::max_id, n2.uid(), node::pointer_type(true));

            node_ofstream w(a);
            w << n3 << n2 << n1;
          }

          const auto b = shared_levelized_file<node>::copy(a);

          AssertThat(is_isomorphic(exec_policy(), dd(a, false), dd(b, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, false)), Is().False());
        });

        it("accepts with nodes swapped [1]", [&]() {
          /*
          // dd_1 with nodes (2) and (3) swapped (and hence non-canonical)
          //
          //       1     ---- x0
          //      _X_
          //     2   3    ---- x1
          //    / \ / \
          //    | F T |   ---- x2
          //     \   /
          //      \ /
          //       4      ---- x3
          //      / \
          //      F T
          */
          shared_levelized_file<node> dd_1b;
          { // Garbage collect writers to free write-lock
            const node n4(2, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n3(1, node::max_id, node::pointer_type(true), n4.uid());
            const node n2(1, node::max_id - 1, n4.uid(), node::pointer_type(false));
            const node n1(0, node::max_id, n3.uid(), n2.uid());

            node_ofstream w(dd_1b);
            w << n4 << n3 << n2 << n1;
          }

          adiar_assert(dd_1b->is_canonical() == false);

          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, false), dd(dd_1b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1b, false), dd(dd_1, false)), Is().True());

          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n, true), dd(dd_1b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1b, false), dd(dd_1n, true)), Is().True());

          AssertThat(is_isomorphic(exec_policy(), dd(dd_1n, false), dd(dd_1b, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1b, true), dd(dd_1n, false)), Is().True());

          AssertThat(is_isomorphic(exec_policy(), dd(dd_1, true), dd(dd_1b, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_1b, true), dd(dd_1, true)), Is().True());
        });

        it("accepts with nodes swapped [2]", [&]() {
          /*
          // dd_2 with nodes (4) and (5) swapped (and hence non-canonical)
          //
          //        _1_          ---- x0
          //       /   \
          //      2     3         ---- x1
          //     / \   / \
          //     F 4   5 F        ---- x2
          //      / \ / \
          //      F T T F
          */
          shared_levelized_file<node> dd_2b;
          { // Garbage collect writers to free write-lock
            const node n5(2, node::max_id, node::pointer_type(true), node::pointer_type(false));
            const node n4(2, node::max_id - 1, node::pointer_type(false), node::pointer_type(true));
            const node n3(1, node::max_id, n5.uid(), node::pointer_type(false));
            const node n2(1, node::max_id - 1, node::pointer_type(false), n4.uid());
            const node n1(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream w(dd_2b);
            w << n5 << n4 << n3 << n2 << n1;
          }

          adiar_assert(dd_2b->is_canonical() == false);

          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, false), dd(dd_2b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2b, false), dd(dd_2, false)), Is().True());

          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, true), dd(dd_2b, false)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2b, false), dd(dd_2n, true)), Is().True());

          AssertThat(is_isomorphic(exec_policy(), dd(dd_2n, false), dd(dd_2b, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2b, true), dd(dd_2n, false)), Is().True());

          AssertThat(is_isomorphic(exec_policy(), dd(dd_2, true), dd(dd_2b, true)), Is().True());
          AssertThat(is_isomorphic(exec_policy(), dd(dd_2b, true), dd(dd_2, true)), Is().True());
        });

        it("rejects when number of requests exceed the input's width", [&]() {
          /*
          //             1          ---- x0
          //            / \
          //            | 2         ---- x1
          //            |/ \
          //            3   4       ---- x2
          //           / \ / \
          //           .  .  .
          //
          //           (x3 level omitted from drawing)
          //
          //         We can then create another version where the low and high
          //         of (2) has been swapped. This can be seen on level x2
          //         (despite all the children are to something with x3) since
          //         the unique requests are:
          //
          //         [ (3,3), (3,4), (4,3) ]
          //
          //         which means that (3) in the first one has been related to
          //         both (3) and (4) in the other one. Hence, they cannot be
          //         isomorphic.
          */
          shared_levelized_file<node> a;
          { // Garbage collect writers to free write-lock
            const node n6(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n5(3, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
            const node n4(2, node::max_id, n5.uid(), n6.uid());
            const node n3(2, node::max_id - 1, n6.uid(), n5.uid());
            const node n2(1, node::max_id, n3.uid(), n4.uid());
            const node n1(0, node::max_id, n3.uid(), n2.uid());

            node_ofstream w(a);
            w << n6 << n5 << n4 << n3 << n2 << n1;
          }

          shared_levelized_file<node> b;
          /*
          //         1         ---- x0
          //        / \
          //        | 2        ---- x1
          //        \ X
          //         3 4       ---- x2
          //         . .
          */
          { // Garbage collect writers to free write-lock
            const node n6(3, node::max_id, node::pointer_type(false), node::pointer_type(true));
            const node n5(3, node::max_id - 1, node::pointer_type(true), node::pointer_type(false));
            const node n4(2, node::max_id, n5.uid(), n6.uid());
            const node n3(2, node::max_id - 1, n6.uid(), n5.uid());
            const node n2(1, node::max_id, n4.uid(), n3.uid());
            const node n1(0, node::max_id, n3.uid(), n2.uid());

            node_ofstream w(b);
            w << n6 << n5 << n4 << n3 << n2 << n1;
          }

          AssertThat(is_isomorphic(exec_policy(), dd(b, false), dd(a, true)), Is().False());
          AssertThat(is_isomorphic(exec_policy(), dd(a, true), dd(b, false)), Is().False());
        });
      });
    });
  });
});
