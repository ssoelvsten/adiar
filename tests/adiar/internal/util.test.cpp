#include "../../test.h"
#include <adiar/internal/util.h>

go_bandit([]() {
  describe("adiar/internal/util.h", []() {
    describe("transpose", []() {
      const ptr_uint64 false_ptr = ptr_uint64(false);
      const ptr_uint64 true_ptr  = ptr_uint64(true);

      it("computes the semi-transposition of a BDD [1]", [&]() {
        /*
        //                 1        ---- x0
        //                / \
        //                2 |       ---- x1
        //               / \|
        //               T  3       ---- x2
        //                 / \
        //                 F T
         */

        const node n3 = node(2, node::max_id, false_ptr, true_ptr);
        const node n2 = node(1, node::max_id, true_ptr, n3.uid());
        const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

        shared_levelized_file<bdd::node_type> in;
        {
          node_ofstream writer(in);
          writer << n3 << n2 << n1;
        }

        shared_levelized_file<arc> out = transpose(bdd(in));

        AssertThat(out->size(0), Is().EqualTo(3u));
        AssertThat(out->size(1), Is().EqualTo(3u));
        AssertThat(out->size(2), Is().EqualTo(0u));

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // n2
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), false, n2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // n3
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), true, n3.uid() }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2.uid(), true, n3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // n2
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), false, true_ptr }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // n3
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3.uid(), false, false_ptr }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3.uid(), true, true_ptr }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out->max_1level_cut, Is().EqualTo(in->max_1level_cut[cut::Internal]));
        AssertThat(out->number_of_terminals[false], Is().EqualTo(in->number_of_terminals[false]));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(in->number_of_terminals[true]));
      });

      it("computes the semi-transposition of a BDD [2]", [&]() {
        /*
        //             1         ---- x0
        //            / \
        //            | 2        ---- x1
        //            |/ \
        //            3  4       ---- x2
        //           / \/ \
        //           F 5  T      ---- x3
        //            / \
        //            F T
         */

        const node n5 = node(3, node::max_id, false_ptr, true_ptr);
        const node n4 = node(2, node::max_id, n5.uid(), true_ptr);
        const node n3 = node(2, node::max_id - 1, false_ptr, n5.uid());
        const node n2 = node(1, node::max_id, n3.uid(), n4.uid());
        const node n1 = node(0, node::max_id, n3.uid(), n2.uid());

        shared_levelized_file<bdd::node_type> in;
        {
          node_ofstream writer(in);
          writer << n5 << n4 << n3 << n2 << n1;
        }

        shared_levelized_file<arc> out = transpose(bdd(in));

        AssertThat(out->size(0), Is().EqualTo(6u));
        AssertThat(out->size(1), Is().EqualTo(4u));
        AssertThat(out->size(2), Is().EqualTo(0u));

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // n2
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), true, n2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // n3
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), false, n3.uid() }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2.uid(), false, n3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // n4
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2.uid(), true, n4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // n5
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n3.uid(), true, n5.uid() }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n4.uid(), false, n5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // n3
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3.uid(), false, false_ptr }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // n4
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4.uid(), true, true_ptr }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // n5
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5.uid(), false, false_ptr }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5.uid(), true, true_ptr }));

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

        AssertThat(out->max_1level_cut, Is().EqualTo(in->max_1level_cut[cut::Internal]));
        AssertThat(out->number_of_terminals[false], Is().EqualTo(in->number_of_terminals[false]));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(in->number_of_terminals[true]));
      });

      it("computes the semi-transposition of a ZDD", [&]() {
        /*
        //            _1_       ---- x0
        //           /   \
        //           2   3      ---- x1
        //          / \ /|
        //          F _X /
        //           /__X_
        //          //    \
        //          4     5     ---- x2
        //         / \   / \
        //         F T   T T
         */

        const node n5 = node(2, node::max_id, true_ptr, true_ptr);
        const node n4 = node(2, node::max_id - 1, false_ptr, true_ptr);
        const node n3 = node(1, node::max_id, n4.uid(), n4.uid());
        const node n2 = node(1, node::max_id - 1, false_ptr, n5.uid());
        const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

        shared_levelized_file<bdd::node_type> in;
        {
          node_ofstream writer(in);
          writer << n5 << n4 << n3 << n2 << n1;
        }

        shared_levelized_file<arc> out = transpose(zdd(in));

        AssertThat(out->size(0), Is().EqualTo(5u));
        AssertThat(out->size(1), Is().EqualTo(5u));
        AssertThat(out->size(2), Is().EqualTo(0u));

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // n2
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), false, n2.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // n3
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n1.uid(), true, n3.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // n4
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n3.uid(), false, n4.uid() }));
        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n3.uid(), true, n4.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // n5
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n2.uid(), true, n5.uid() }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // n2
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), false, false_ptr }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // n4
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4.uid(), false, false_ptr }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n4.uid(), true, true_ptr }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // n5
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5.uid(), false, true_ptr }));
        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5.uid(), true, true_ptr }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out->max_1level_cut, Is().EqualTo(in->max_1level_cut[cut::Internal]));
        AssertThat(out->number_of_terminals[false], Is().EqualTo(in->number_of_terminals[false]));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(in->number_of_terminals[true]));
      });
    });
  });
});
