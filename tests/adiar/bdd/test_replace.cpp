#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/replace.cpp", []() {
    using mapping_type = function<bdd::label_type(bdd::label_type)>;

    shared_levelized_file<bdd::node_type> bdd_F_nf;
    /*
    //        F
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(bdd_F_nf);
      nw << node(false);
    }
    const bdd bdd_F(bdd_F_nf);

    shared_levelized_file<bdd::node_type> bdd_T_nf;
    /*
    //        T
    */
    { // Garbage collect writers to free write-lock
      node_ofstream nw(bdd_T_nf);
      nw << node(true);
    }
    const bdd bdd_T(bdd_T_nf);

    const ptr_uint64 terminal_T = ptr_uint64(true);
    const ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<bdd::node_type> bdd_x0_nf;
    /*
    //          1        ---- x0
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x0_nf);
      nw << node(0, node::max_id, terminal_F, terminal_T);
    }
    const bdd bdd_x0(bdd_x0_nf);

    shared_levelized_file<bdd::node_type> bdd_x1_nf;
    /*
    //          1        ---- x1
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x1_nf);
      nw << node(1, node::max_id, terminal_F, terminal_T);
    }
    const bdd bdd_x1(bdd_x1_nf);

    shared_levelized_file<bdd::node_type> bdd_x2_nf;
    /*
    //          1        ---- x2
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x2_nf);
      nw << node(2, node::max_id, terminal_F, terminal_T);
    }
    const bdd bdd_x2(bdd_x2_nf);

    shared_levelized_file<bdd::node_type> bdd_1_nf;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x2
    //       |/ \
    //       3  T      ---- x4
    //      / \
    //      F T
    */
    { // Garbage collect early and free write-lock
      const node n3 = node(4, node::max_id, terminal_F, terminal_T);
      const node n2 = node(2, node::max_id, n3.uid(), terminal_T);
      const node n1 = node(0, node::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_1_nf);
      nw << n3 << n2 << n1;
    }
    const bdd bdd_1(bdd_1_nf);

    shared_levelized_file<bdd::node_type> bdd_2_nf;
    /*
    // NOTE: This BDD is on-purpose not canonical (to check whether it has been run through the
    //       Reduce algorithm or not).
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     F T T F
    */

    { // Garbage collect early and free write-lock
      const node n3 = node(1, node::max_id, terminal_T, terminal_F);
      const node n2 = node(1, node::max_id - 2, terminal_F, terminal_T);
      const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_2_nf);
      nw << n3 << n2 << n1;
    }
    const bdd bdd_2(bdd_2_nf);

    shared_levelized_file<bdd::node_type> bdd_3_nf;
    /*
    // NOTE: This BDD is on-purpose not canonical (to check whether it has been run through the
    //       Reduce algorithm or not)
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     | F F |
    //      \   /
    //       \ /
    //        4         ---- x2
    //       / \
    //       T F
    */

    { // Garbage collect early and free write-lock
      const node n4 = node(2, node::max_id, terminal_T, terminal_F);
      const node n3 = node(1, node::max_id, terminal_F, n4.uid());
      const node n2 = node(1, node::max_id - 1, n4.uid(), terminal_F);
      const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_3_nf);
      nw << n4 << n3 << n2 << n1;
    }
    const bdd bdd_3(bdd_3_nf);

    describe("bdd_replace(const bdd&, <...>)", [&]() {
      describe("<non-monotonic>", [&]() {
        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(bdd_F, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(bdd_T, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("preserves negation flag when returning original file", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(bdd_not(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().True());
        });

        it("identifies 'x(4-0)' as a mere shift on 'x0'", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(bdd_x0, m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(4));

          // Check it is read correctly
          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("throws exception if levels have to be swapped [bdd_1]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          AssertThrows(invalid_argument, bdd_replace(bdd_1, m));
        });

        it("throws exception if levels have to be swapped [bdd_2]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          AssertThrows(invalid_argument, bdd_replace(bdd_2, m));
        });

        // TODO: Add more complex inputs that test for all relevant behaviours of applying the
        //       Nested Sweeping framework to move levels.
      });

      describe("<monotonic>", [&]() {
        // NOTE: To future-proof these tests against the introduction of constant time 'Affine' or
        //       'Shift' variable replacement, we test with quadratic variable replacement (and
        //       similarly more complex functions).

        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_F, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_T, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("preserves negation flag when returning original file", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_not(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().True());
        });

        it("squares all variables in 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_1, m);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_1->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_1->indexable));

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(16, bdd::max_id, terminal_F, terminal_T)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(16, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(16, bdd::max_id),
                                       bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(16, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_1->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_1->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_1->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_1->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_1->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_1->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_1->number_of_terminals[true]));
        });

        it("bakes negation into output when squaring of variables in 'BDD 3'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_not(bdd_3), m);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_3->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_3->indexable));

          node_test_ifstream out_nodes(out);

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_T, bdd::pointer_type(4, bdd::max_id))));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id - 1, bdd::pointer_type(4, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(1, bdd::max_id - 1),
                                       bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_3->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_3->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_3->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_3->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_3->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_3->number_of_terminals[true]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_3->number_of_terminals[false]));
        });
      });

      describe("<affine>", [&]() {
        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return 2 * x; };
          const bdd out        = bdd_replace(bdd_F, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return 2 * x + 2; };
          const bdd out        = bdd_replace(bdd_T, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("preserves negation flag when returning original file", [&]() {
          const mapping_type m = [](const int x) { return 2 * x + 1; };
          const bdd out        = bdd_replace(bdd_not(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().True());
        });

        it("identifies 'x(2x+1)' as a mere shift for 'x0'", [&]() {
          const mapping_type m = [](const int x) { return 2 * x + 1; };
          const bdd out        = bdd_replace(bdd_x0, m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(1));

          // Check it is read correctly
          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("doubles variables in 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return 2 * x; };
          const bdd out        = bdd_replace(bdd_2, m);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id - 2, terminal_F, terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(2, bdd::max_id - 2),
                                       bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_2->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_2->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_2->number_of_terminals[true]));
        });
      });

      describe("<constant>", [&]() {
        it("shifts variables in 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x + 1; };
          const bdd out        = bdd_replace(bdd_1, m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_1_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(1));

          // Check it is read correctly
          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       bdd::max_id,
                                       bdd::pointer_type(5, bdd::max_id),
                                       bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("shifts variables in 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return x + 4; };
          const bdd out        = bdd_replace(bdd_2, m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_2_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(4));

          // Check it is read correctly
          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, bdd::max_id - 2, terminal_F, terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(4,
                                       bdd::max_id,
                                       bdd::pointer_type(5, bdd::max_id - 2),
                                       bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("shifts variables in 'BDD 3' multiple times [+3, +3]", [&]() {
          const mapping_type m = [](const int x) { return x + 3; };
          const bdd out        = bdd_replace(bdd_replace(bdd_3, m), m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_3_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(6));

          // Check it is read correctly
          node_test_ifstream out_nodes(out);

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, bdd::max_id, terminal_T, terminal_F)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(7, bdd::max_id, terminal_F, bdd::pointer_type(8, bdd::max_id))));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(7, bdd::max_id - 1, bdd::pointer_type(8, bdd::max_id), terminal_F)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(6,
                                       bdd::max_id,
                                       bdd::pointer_type(7, bdd::max_id - 1),
                                       bdd::pointer_type(7, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("shifts variables in 'BDD 1' multiple times [+2, -1]", [&]() {
          const bdd out = bdd_replace(bdd_replace(bdd_1, [](const int x) { return x + 2; }),
                                      [](const int x) { return x - 1; });

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_1_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(1));

          // Check it is read correctly
          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       bdd::max_id,
                                       bdd::pointer_type(5, bdd::max_id),
                                       bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        // TODO: Accumulation of shifts
      });

      describe("<identity>", [&]() {
        it("returns the original file for 'x0'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(bdd_x0, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'x1'", [&]() {
          const mapping_type m = [](const int x) { return x * x; };
          const bdd out        = bdd_replace(bdd_x1, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x1_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'BDD 1'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(bdd_1, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_1_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'BDD 2'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(bdd_2, m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_2_nf));
          AssertThat(out.is_negated(), Is().False());
        });
      });
    });

    describe("bdd_replace(const bdd&, <...>, replace_type)", [&]() {
      it("returns the original file for 'F'", [&]() {
        const mapping_type m = [](const int x) { return 4 - x; };
        const bdd out        = bdd_replace(bdd_F, m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'T'", [&]() {
        const mapping_type m = [](const int x) { return 4 - x; };
        const bdd out        = bdd_replace(bdd_T, m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      // TODO: Replace AssertThrows tests with checks for the output is correct (and reduced because
      //       it uses the most general algorithm).

      it("throws exception if 'replace_type' is 'Non_Monotone' [x0]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        const mapping_type m = [](const int x) { return x + 1; };

        AssertThrows(invalid_argument, bdd_replace(bdd_x0, m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [bdd_1]", [&]() {
        // NOTE: This mapping proves it can swap levels
        const mapping_type m = [](const int x) { return 4 - x; };
        AssertThrows(invalid_argument, bdd_replace(bdd_1, m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [bdd_2]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'; the BDD should end up reduced.
        const mapping_type m = [](const int x) { return x + 1; };

        AssertThrows(invalid_argument, bdd_replace(bdd_2, m, replace_type::Non_Monotone));
      });

      it("shifts variables in 'BDD 1' if 'replace_type' is 'Monotone'", [&]() {
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out        = bdd_replace(bdd_1, m, replace_type::Monotone);

        // Check it looks all right
        AssertThat(out->sorted, Is().EqualTo(bdd_1->sorted));
        AssertThat(out->indexable, Is().EqualTo(bdd_1->indexable));

        node_test_ifstream out_nodes(out);

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(
            1, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(3, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(bdd_1->width));

        AssertThat(out->max_1level_cut[cut::Internal],
                   Is().EqualTo(bdd_1->max_1level_cut[cut::Internal]));
        AssertThat(out->max_1level_cut[cut::Internal_False],
                   Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_False]));
        AssertThat(out->max_1level_cut[cut::Internal_True],
                   Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_True]));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_1->max_1level_cut[cut::All]));

        AssertThat(out->max_2level_cut[cut::Internal],
                   Is().EqualTo(bdd_1->max_2level_cut[cut::Internal]));
        AssertThat(out->max_2level_cut[cut::Internal_False],
                   Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_False]));
        AssertThat(out->max_2level_cut[cut::Internal_True],
                   Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_True]));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_1->max_2level_cut[cut::All]));

        AssertThat(out->number_of_terminals[false],
                   Is().EqualTo(bdd_1->number_of_terminals[false]));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_1->number_of_terminals[true]));
      });

      it("doubles variables in 'BDD 2' if 'replace_type' is 'Monotone'", [&]() {
        const mapping_type m = [](const int x) { return 2 * x; };
        const bdd out        = bdd_replace(bdd_2, m, replace_type::Monotone);

        // Check it looks all right
        AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
        AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

        node_test_ifstream out_nodes(out);

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2, bdd::max_id - 2, terminal_F, terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     bdd::max_id,
                                     bdd::pointer_type(2, bdd::max_id - 2),
                                     bdd::pointer_type(2, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(bdd_2->width));

        AssertThat(out->max_1level_cut[cut::Internal],
                   Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
        AssertThat(out->max_1level_cut[cut::Internal_False],
                   Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
        AssertThat(out->max_1level_cut[cut::Internal_True],
                   Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

        AssertThat(out->max_2level_cut[cut::Internal],
                   Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
        AssertThat(out->max_2level_cut[cut::Internal_False],
                   Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
        AssertThat(out->max_2level_cut[cut::Internal_True],
                   Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

        AssertThat(out->number_of_terminals[false],
                   Is().EqualTo(bdd_2->number_of_terminals[false]));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_2->number_of_terminals[true]));
      });

      it("shifts variables in 'BDD 1' if 'replace_type' is 'Shift'", [&]() {
        // NOTE: This function is in fact *not* a shift!
        const mapping_type m = [](const int x) { return 2 * x + 1; };
        const bdd out        = bdd_replace(bdd_1, m, replace_type::Shift);

        // Check it returns the same file but shifted
        AssertThat(out.file_ptr(), Is().EqualTo(bdd_1.file_ptr()));
        AssertThat(out.is_negated(), Is().False());
        AssertThat(out.shift(), Is().EqualTo(+1));

        // Check it is read correctly
        node_test_ifstream out_nodes(out);

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(
            1, bdd::max_id, bdd::pointer_type(5, bdd::max_id), bdd::pointer_type(3, bdd::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(bdd_1->width));

        AssertThat(out->max_1level_cut[cut::Internal],
                   Is().EqualTo(bdd_1->max_1level_cut[cut::Internal]));
        AssertThat(out->max_1level_cut[cut::Internal_False],
                   Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_False]));
        AssertThat(out->max_1level_cut[cut::Internal_True],
                   Is().EqualTo(bdd_1->max_1level_cut[cut::Internal_True]));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_1->max_1level_cut[cut::All]));

        AssertThat(out->max_2level_cut[cut::Internal],
                   Is().EqualTo(bdd_1->max_2level_cut[cut::Internal]));
        AssertThat(out->max_2level_cut[cut::Internal_False],
                   Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_False]));
        AssertThat(out->max_2level_cut[cut::Internal_True],
                   Is().EqualTo(bdd_1->max_2level_cut[cut::Internal_True]));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_1->max_2level_cut[cut::All]));

        AssertThat(out->number_of_terminals[false],
                   Is().EqualTo(bdd_1->number_of_terminals[false]));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(bdd_1->number_of_terminals[true]));
      });

      it("returns the original file if 'replace_type is 'Identity'", [&]() {
        // NOTE: This function is in fact *not* the identity!
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out        = bdd_replace(bdd_x0, m, replace_type::Identity);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
        AssertThat(out.is_negated(), Is().False());
      });
    });

    shared_levelized_file<arc> __bdd_x0;
    /*
    //          1      ---- x0
    //         / \
    //         F T
    */
    {
      const arc::pointer_type n1(0, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_x0);

      aw.push_terminal({ n1, false, terminal_F });
      aw.push_terminal({ n1, true, terminal_T });

      aw.push(level_info(0, 1u));

      __bdd_x0->max_1level_cut = 0;
    }

    shared_levelized_file<arc> __bdd_x0_unreduced;
    /*
    // NOTE: Due to the reduction rules, this BDD does not need Nested Sweeping to swap its levels.
    //       Yet, one cannot know this before having done the computation.
    //
    //            1      ---- x0
    //           / \
    //           F 2     ---- x2
    //            / \
    //            T T
    */
    {
      const arc::pointer_type n1(0, 0);
      const arc::pointer_type n2(2, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_x0_unreduced);

      aw.push_internal({ n1, true, n2 });

      aw.push_terminal({ n1, false, terminal_F });
      aw.push_terminal({ n2, false, terminal_T });
      aw.push_terminal({ n2, true, terminal_T });

      aw.push(level_info(0, 1u));
      aw.push(level_info(2, 1u));

      __bdd_x0_unreduced->max_1level_cut = 1;
    }

    shared_levelized_file<arc> __bdd_1;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x2
    //       |/ \
    //       3  T      ---- x4
    //      / \
    //      F T
    */
    {
      const arc::pointer_type n1(0, 0);
      const arc::pointer_type n2(2, 0);
      const arc::pointer_type n3(4, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_1);

      aw.push_internal({ n1, true, n2 });
      aw.push_internal({ n1, false, n3 });
      aw.push_internal({ n2, false, n3 });

      aw.push_terminal({ n2, true, terminal_T });
      aw.push_terminal({ n3, false, terminal_F });
      aw.push_terminal({ n3, true, terminal_T });

      aw.push(level_info(0, 1u));
      aw.push(level_info(2, 1u));
      aw.push(level_info(4, 1u));

      __bdd_1->max_1level_cut = 2;
    }

    shared_levelized_file<arc> __bdd_2;
    /*
    // NOTE: When reduced, the nodes (2) and (3) are swapped to make it canonical.
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     F T T F
    */
    {
      const arc::pointer_type n1(0, 0);
      const arc::pointer_type n2(1, 0);
      const arc::pointer_type n3(1, 1);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_2);

      aw.push_internal({ n1, false, n2 });
      aw.push_internal({ n1, true, n3 });

      aw.push_terminal({ n2, false, terminal_F });
      aw.push_terminal({ n2, true, terminal_T });
      aw.push_terminal({ n3, false, terminal_T });
      aw.push_terminal({ n3, true, terminal_F });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 2u));

      __bdd_2->max_1level_cut = 2;
    }

    shared_levelized_file<arc> __bdd_3;
    /*
    // NOTE: This BDD is on-purpose not canonical (to check whether it has been run through the
    //       Reduce algorithm or not)
    //
    //       _1_        ---- x0
    //      /   \
    //      2   3       ---- x1
    //     / \ / \
    //     | F F |
    //      \   /
    //       \ /
    //        4         ---- x2
    //       / \
    //       T F
    */
    {
      const arc::pointer_type n1(0, 0);
      const arc::pointer_type n2(1, 0);
      const arc::pointer_type n3(1, 1);
      const arc::pointer_type n4(2, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_3);

      aw.push_internal({ n1, false, n2 });
      aw.push_internal({ n1, true, n3 });
      aw.push_internal({ n2, false, n4 });
      aw.push_internal({ n3, true, n4 });

      aw.push_terminal({ n2, true, terminal_F });
      aw.push_terminal({ n3, false, terminal_F });
      aw.push_terminal({ n4, false, terminal_T });
      aw.push_terminal({ n4, true, terminal_F });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 2u));
      aw.push(level_info(2, 1u));

      __bdd_3->max_1level_cut = 2;
    }

    shared_levelized_file<arc> __bdd_3_unreduced;
    /*
    // NOTE: A more extreme version of '__bdd_3' with duplicate and redundant nodes.
    //
    //       __1__        ---- x0
    //      /     \
    //     _2_   _3_      ---- x1
    //    /   \ /   \
    //    4    5    6     ---- x2
    //   / \   ||  / \
    //   T F   F   T F
    */
    {
      const arc::pointer_type n6(2, 2);
      const arc::pointer_type n5(2, 1);
      const arc::pointer_type n4(2, 0);
      const arc::pointer_type n3(1, 1);
      const arc::pointer_type n2(1, 0);
      const arc::pointer_type n1(0, 0);

      // Garbage collect writer to free write-lock
      arc_ofstream aw(__bdd_3_unreduced);

      aw.push_internal({ n1, false, n2 });
      aw.push_internal({ n1, true, n3 });
      aw.push_internal({ n2, false, n4 });
      aw.push_internal({ n2, true, n5 });
      aw.push_internal({ n3, false, n5 });
      aw.push_internal({ n3, true, n6 });

      aw.push_terminal({ n4, false, terminal_T });
      aw.push_terminal({ n4, true, terminal_F });
      aw.push_terminal({ n5, false, terminal_F });
      aw.push_terminal({ n5, true, terminal_F });
      aw.push_terminal({ n6, false, terminal_T });
      aw.push_terminal({ n6, true, terminal_F });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 2u));
      aw.push(level_info(2, 3u));

      __bdd_3_unreduced->max_1level_cut = 4;
    }

    describe("bdd_replace(__bdd&&, <...>)", [&]() {
      describe("<non-monotonic>", [&]() {
        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(exec_policy(), __bdd(bdd_F), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(__bdd(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("identifies 'x(4-0)' as a mere shift on reduced 'x0' [bdd_x0]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(exec_policy(), __bdd(bdd_x0_nf), m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(4));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

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

        it("preserves negation flag when identiftying 'x(4-0)' is a mere shift [bdd_x0]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(__bdd(bdd(bdd_x0_nf, true)), m);

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().True());
          AssertThat(out.shift(), Is().EqualTo(4));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("reverses 'x0' into 'x4' [__bdd_x0]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out        = bdd_replace(__bdd(__bdd_x0, exec_policy()), m);

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

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

        it("throws exception if level-swapping is potentially necessary [__bdd_x0_unreduced]",
           [&]() {
             const mapping_type m = [](const int x) { return 4 - x; };
             AssertThrows(invalid_argument,
                          bdd_replace(__bdd(__bdd_x0_unreduced, exec_policy()), m));
           });

        it("throws exception if levels have to be swapped [bdd_1]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          AssertThrows(invalid_argument, bdd_replace(__bdd(__bdd_1, exec_policy()), m));
        });

        it("throws exception if levels have to be swapped [__bdd_1]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          AssertThrows(invalid_argument, bdd_replace(__bdd(__bdd_1, exec_policy()), m));
        });

        it("throws exception if levels have to be swapped [bdd_2]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          AssertThrows(invalid_argument, bdd_replace(__bdd(__bdd_2, exec_policy()), m));
        });

        it("throws exception if levels have to be swapped [__bdd_2]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          AssertThrows(invalid_argument, bdd_replace(__bdd(__bdd_2, exec_policy()), m));
        });
      });

      describe("<monotonic> / <affine> / <constant>", [&]() {
        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return 2 * x + 1; };
          const bdd out        = bdd_replace(__bdd(bdd_F), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return x + 42; };
          const bdd out        = bdd_replace(__bdd(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("doubles variables [bdd_2]", [&]() {
          const mapping_type m = [](const int x) { return 2 * x; };
          const bdd out        = bdd_replace(__bdd(bdd_2_nf), m);

          // Check it looks all right
          AssertThat(out->sorted, Is().EqualTo(bdd_2->sorted));
          AssertThat(out->indexable, Is().EqualTo(bdd_2->indexable));

          node_test_ifstream out_nodes(out);

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id - 2, terminal_F, terminal_T)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(2, bdd::max_id - 2),
                                       bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(bdd_2->width));

          AssertThat(out->max_1level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal]));
          AssertThat(out->max_1level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_False]));
          AssertThat(out->max_1level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_1level_cut[cut::Internal_True]));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(bdd_2->max_1level_cut[cut::All]));

          AssertThat(out->max_2level_cut[cut::Internal],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal]));
          AssertThat(out->max_2level_cut[cut::Internal_False],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_False]));
          AssertThat(out->max_2level_cut[cut::Internal_True],
                     Is().EqualTo(bdd_2->max_2level_cut[cut::Internal_True]));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(bdd_2->max_2level_cut[cut::All]));

          AssertThat(out->number_of_terminals[false],
                     Is().EqualTo(bdd_2->number_of_terminals[false]));
          AssertThat(out->number_of_terminals[true],
                     Is().EqualTo(bdd_2->number_of_terminals[true]));
        });

        it("returns shifted original file [bdd_x0]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x + 1;
          };
          const bdd out = bdd_replace(__bdd(bdd_x0), m);

          // Check only called once to determine type of mapping and then once to obtain the amount
          // to shift
          AssertThat(m_calls, Is().EqualTo(1 + 1));

          // Check it returns the same file but shifted
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
          AssertThat(out.shift(), Is().EqualTo(1));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("reduces and shifts variables [__bdd_x0]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x + 1;
          };
          const bdd out = bdd_replace(__bdd(__bdd_x0, exec_policy()), m);

          // Check only called once to determine type of mapping and then once to obtain the amount
          // to shift
          AssertThat(m_calls, Is().EqualTo(1 + 1));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

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

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("reduces and shifts variables at once [__bdd_x0_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x + 1;
          };
          const bdd out = bdd_replace(__bdd(__bdd_x0_unreduced, exec_policy()), m);

          // Check only called once to determine type of mapping and then for each level
          AssertThat(m_calls, Is().EqualTo(2 + 2));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, bdd::max_id, terminal_F, terminal_T)));

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

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("doubles variables [__bdd_2]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return 2 * x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_2, exec_policy()), m);

          // Check only called once to determine type of mapping and then for each level
          AssertThat(m_calls, Is().EqualTo(2 + 2));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, bdd::max_id - 1, terminal_T, terminal_F)));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(2, bdd::max_id),
                                       bdd::pointer_type(2, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("reduces and squares variables at once [__bdd_3_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x * x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m);

          // Check only called once to determine type of mapping and then for each level
          AssertThat(m_calls, Is().EqualTo(3 + 3));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n4 / n6
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id - 1, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(1, bdd::max_id),
                                       bdd::pointer_type(1, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          // Over-approximation, since (5) is removed?
          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(4u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("<identity>", [&]() {
        it("returns the original file for 'F'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(__bdd(bdd_F), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'T'", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(__bdd(bdd_T), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("returns the original file for 'x0' [bdd_x0]", [&]() {
          const mapping_type m = [](const int x) { return x; };
          const bdd out        = bdd_replace(__bdd(bdd_x0), m);

          AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
          AssertThat(out.is_negated(), Is().False());
        });

        it("reduces without any additional calls [__bdd_x0]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_x0, exec_policy()), m);

          // Check only called once (to determine type of mapping)
          AssertThat(m_calls, Is().EqualTo(1));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

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

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("reduces without any additional calls [__bdd_x0_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_x0_unreduced, exec_policy()), m);

          // Check only called once (to determine type of mapping)
          AssertThat(m_calls, Is().EqualTo(2));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

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

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("reduces without any additional calls [__bdd_3_unreduced]", [&]() {
          int m_calls          = 0;
          const mapping_type m = [&m_calls](const int x) {
            m_calls++;
            return x;
          };
          const bdd out = bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m);

          // Check only called once (to determine type of mapping)
          AssertThat(m_calls, Is().EqualTo(3));

          // Check it looks all right
          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());

          node_test_ifstream out_nodes(out);

          // n4 / n6
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          // n2
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id - 1, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(0,
                                       bdd::max_id,
                                       bdd::pointer_type(1, bdd::max_id),
                                       bdd::pointer_type(1, bdd::max_id - 1))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));
          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          // Over-approximation, since (5) is removed?
          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(4u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });
    });

    describe("bdd_replace(__bdd&&, <...>, replace_type)", [&]() {
      it("returns the original file for 'F'", [&]() {
        const mapping_type m = [](const int x) { return x + 2; };
        const bdd out = bdd_replace(exec_policy(), __bdd(bdd_F), m, replace_type::Non_Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_F_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("returns the original file for 'T'", [&]() {
        const mapping_type m = [](const int x) { return 2 * x; };
        const bdd out        = bdd_replace(__bdd(bdd_T), m, replace_type::Monotone);

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_T_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      // TODO: Replace AssertThrows tests with checks for the output is correct (and reduced because
      //       it uses the most general algorithm).

      it("throws exception if 'replace_type' is 'Non_Monotone' [bdd_x0]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        const mapping_type m = [](const int x) { return x + 1; };

        AssertThrows(invalid_argument,
                     bdd_replace(exec_policy(), __bdd(bdd_x0_nf), m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [__bdd_x0]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        const mapping_type m = [](const int x) { return x + 1; };

        AssertThrows(invalid_argument,
                     bdd_replace(__bdd(__bdd_x0, exec_policy()), m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [__bdd_x0_unreduced]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        const mapping_type m = [](const int x) { return x + 1; };

        AssertThrows(
          invalid_argument,
          bdd_replace(__bdd(__bdd_x0_unreduced, exec_policy()), m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [bdd_1]", [&]() {
        // NOTE: This mapping proves it can swap levels
        const mapping_type m = [](const int x) { return 4 - x; };
        AssertThrows(invalid_argument, bdd_replace(__bdd(bdd_1_nf), m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [__bdd_1]", [&]() {
        // NOTE: This mapping proves it can swap levels
        const mapping_type m = [](const int x) { return 4 - x; };
        AssertThrows(invalid_argument,
                     bdd_replace(__bdd(__bdd_1, exec_policy()), m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [bdd_3]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'; the BDD should end up reduced.
        const mapping_type m = [](const int x) { return x + 1; };

        AssertThrows(invalid_argument, bdd_replace(__bdd(bdd_3), m, replace_type::Non_Monotone));
      });

      it("throws exception if 'replace_type' is 'Non_Monotone' [__bdd_3_unreduced]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'; the BDD should end up reduced.
        const mapping_type m = [](const int x) { return x + 1; };

        AssertThrows(
          invalid_argument,
          bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m, replace_type::Non_Monotone));
      });

      it("reduces and affinely maps 'x0' if 'replace_type' is 'Monotone'", [&]() {
        int m_calls          = 0;
        const mapping_type m = [&m_calls](const int x) {
          m_calls++;
          return 3 * x + 42;
        };
        const bdd out = bdd_replace(
          exec_policy(), __bdd(__bdd_x0_unreduced, exec_policy()), m, replace_type::Monotone);

        // Check is only called for each level once
        AssertThat(m_calls, Is().EqualTo(2));

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());

        node_test_ifstream out_nodes(out);

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(42, bdd::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(42, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

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

      it("reduces and affinely maps variables in 'BDD 3' if 'replace_type' is 'Monotone' "
         "[__bdd_3_unreduced]",
         [&]() {
           int m_calls          = 0;
           const mapping_type m = [&m_calls](const int x) {
             m_calls++;
             return 2 * x + 1;
           };
           const bdd out =
             bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m, replace_type::Monotone);

           // Check only called once per level
           AssertThat(m_calls, Is().EqualTo(3));

           // Check it looks all right
           AssertThat(out->sorted, Is().True());
           AssertThat(out->indexable, Is().True());

           node_test_ifstream out_nodes(out);

           // n4 / n6
           AssertThat(out_nodes.can_pull(), Is().True());
           AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_T, terminal_F)));

           // n2
           AssertThat(out_nodes.can_pull(), Is().True());
           AssertThat(
             out_nodes.pull(),
             Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

           // n3
           AssertThat(out_nodes.can_pull(), Is().True());
           AssertThat(
             out_nodes.pull(),
             Is().EqualTo(node(3, bdd::max_id - 1, terminal_F, bdd::pointer_type(5, bdd::max_id))));

           // n1
           AssertThat(out_nodes.can_pull(), Is().True());
           AssertThat(out_nodes.pull(),
                      Is().EqualTo(node(1,
                                        bdd::max_id,
                                        bdd::pointer_type(3, bdd::max_id),
                                        bdd::pointer_type(3, bdd::max_id - 1))));

           AssertThat(out_nodes.can_pull(), Is().False());

           level_info_test_ifstream out_meta(out);

           AssertThat(out_meta.can_pull(), Is().True());
           AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));
           AssertThat(out_meta.can_pull(), Is().True());
           AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));
           AssertThat(out_meta.can_pull(), Is().True());
           AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

           AssertThat(out_meta.can_pull(), Is().False());

           AssertThat(out->width, Is().EqualTo(2u));

           // Over-approximation, since (5) is removed?
           AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
           AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
           AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
           AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
           AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
           AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(4u));

           AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
           AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
           AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
           AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
           AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
           AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
           AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

           AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
           AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
         });

      it("reduces and maps shifted of variables in 'BDD 2' if 'replace_type' is 'Monotone' "
         "[__bdd_2]",
         [&]() {
           int m_calls          = 0;
           const mapping_type m = [&m_calls](const int x) {
             m_calls++;
             return x + 1;
           };
           const bdd out = bdd_replace(__bdd(__bdd_2, exec_policy()), m, replace_type::Monotone);

           // Check only called once per level
           AssertThat(m_calls, Is().EqualTo(2));

           // Check it looks all right
           AssertThat(out->sorted, Is().True());
           AssertThat(out->indexable, Is().True());

           node_test_ifstream out_nodes(out);

           // n2
           AssertThat(out_nodes.can_pull(), Is().True());
           AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

           // n3
           AssertThat(out_nodes.can_pull(), Is().True());
           AssertThat(out_nodes.pull(),
                      Is().EqualTo(node(2, bdd::max_id - 1, terminal_T, terminal_F)));

           // n1
           AssertThat(out_nodes.can_pull(), Is().True());
           AssertThat(out_nodes.pull(),
                      Is().EqualTo(node(1,
                                        bdd::max_id,
                                        bdd::pointer_type(2, bdd::max_id),
                                        bdd::pointer_type(2, bdd::max_id - 1))));

           AssertThat(out_nodes.can_pull(), Is().False());

           level_info_test_ifstream out_meta(out);

           AssertThat(out_meta.can_pull(), Is().True());
           AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

           AssertThat(out_meta.can_pull(), Is().True());
           AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

           AssertThat(out_meta.can_pull(), Is().False());

           AssertThat(out->width, Is().EqualTo(2u));

           AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
           AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
           AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
           AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

           AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
           AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
           AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
           AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

           AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
           AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
         });

      it("reduces and shifts variables in 'BDD 2' if 'replace_type' is 'Shift' [__bdd_2]", [&]() {
        int m_calls          = 0;
        const mapping_type m = [&m_calls](const int x) {
          m_calls++;
          return x + 1;
        };
        const bdd out = bdd_replace(__bdd(__bdd_2, exec_policy()), m, replace_type::Shift);

        // Check is still called once per level
        AssertThat(m_calls, Is().EqualTo(2));

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());

        node_test_ifstream out_nodes(out);

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2, bdd::max_id - 1, terminal_T, terminal_F)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     bdd::max_id,
                                     bdd::pointer_type(2, bdd::max_id),
                                     bdd::pointer_type(2, bdd::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("returns the original file of 'x0' if 'replace_type' is 'Identity' with no calls", [&]() {
        // NOTE: This function is in fact *not* the identity!
        int m_calls          = 0;
        const mapping_type m = [&m_calls](const int x) {
          m_calls++;
          return x + 1;
        };
        const bdd out = bdd_replace(__bdd(bdd_x0_nf), m, replace_type::Identity);

        AssertThat(m_calls, Is().EqualTo(0));

        AssertThat(out.file_ptr(), Is().EqualTo(bdd_x0_nf));
        AssertThat(out.is_negated(), Is().False());
      });

      it("reduces 'x0' with no calls if 'replace_type' is 'Identity'", [&]() {
        // NOTE: This function is in fact *not* the identity!
        int m_calls          = 0;
        const mapping_type m = [&m_calls](const int x) {
          m_calls++;
          return x + 1;
        };
        const bdd out = bdd_replace(__bdd(__bdd_x0, exec_policy()), m, replace_type::Identity);

        // Check is never called
        AssertThat(m_calls, Is().EqualTo(0));

        // Check it looks all right
        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());

        node_test_ifstream out_nodes(out);

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

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

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });
    });
  });
});
