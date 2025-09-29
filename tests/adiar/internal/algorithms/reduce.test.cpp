#include "../../../test.h"

go_bandit([]() {
  describe("adiar/internal/algorithms/reduce.h", [&]() {
    // The reduce<dd_policy> function is used within the constructors of the BDD
    // and ZDD classes.

    const arc::uid_type terminal_F(false);
    const arc::uid_type terminal_T(true);

    shared_levelized_file<node> x0x1_node_file;

    { // Garbage collect writer to free write-lock
      node_ofstream nw(x0x1_node_file);

      nw << node(1, node::max_id, terminal_F, terminal_T)
         << node(0, node::max_id, terminal_F, node::pointer_type(1, node::max_id));
    }

    it("preserves negation flag on reduced input [1]", [&]() {
      /*
      //         1                  1      ---- x0
      //        / \                / \
      //        F 2        =>      F 2     ---- x1
      //         / \                / \
      //         F T                F T
      */

      // Use a __bdd that stores the node file from a bdd.
      bdd out(__bdd(bdd(x0x1_node_file, false)));

      AssertThat(out.file_ptr(), Is().EqualTo(x0x1_node_file));
      AssertThat(out.is_negated(), Is().False());
      AssertThat(out.shift(), Is().EqualTo(0));
    });

    it("preserves negation flag on reduced input [2]", [&]() {
      /*
      //         1                  1      ---- x0
      //        / \                / \
      //        F 2      = ~ =>    T 2     ---- x1
      //         / \                / \
      //         F T                T F
      */

      // Use a __bdd that stores the node file from a bdd.
      bdd out(__bdd(bdd(x0x1_node_file, true)));

      AssertThat(out.file_ptr(), Is().EqualTo(x0x1_node_file));
      AssertThat(out.is_negated(), Is().True());
      AssertThat(out.shift(), Is().EqualTo(0));
    });

    it("preserves shift on reduced input [1]", [&]() {
      /*
      //         1                  1      ---- x0
      //        / \                / \
      //        F 2        =>      F 2     ---- x1
      //         / \                / \
      //         F T                F T
      */

      // Use a __bdd that stores the node file from a bdd.
      bdd out(__bdd(bdd(x0x1_node_file, false, +1)));

      AssertThat(out.file_ptr(), Is().EqualTo(x0x1_node_file));
      AssertThat(out.is_negated(), Is().False());
      AssertThat(out.shift(), Is().EqualTo(+1));
    });

    describe("Reduction Rule 2", [&]() {
      it("applies to terminal arcs [1]", [&]() {
        /*
        //           1                  1      ---- x0
        //          / \                / \
        //          | 2_               | 2     ---- x1
        //          | | \      =>      |/ \
        //          3 4 T              4  T    ---- x2
        //          |X|               / \
        //          F T               F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(2, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n1, false, n3 });
          aw.push_internal({ n2, false, n4 });

          aw.push_terminal({ n2, true, terminal_T });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(2, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(1, node::max_id))));
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
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(4u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("applies to terminal arcs [2]", [&]() {
        /*
        //            1      ---- x0            1
        //           / \                       / \
        //           | |     ---- x1           | |
        //           | |                       | |
        //           2 3     ---- x2    ==>    2 3
        //          / X \                     /\ /\
        //          F/ \T                     F | T
        //           4 5     ---- x3            4
        //          /| |\                      / \
        //          FT FT                      F T
        */
        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(2, 1);
        const arc::uid_type n4(3, 0);
        const arc::uid_type n5(3, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n2, true, n5 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 2u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, node::max_id, node::pointer_type(3, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, node::max_id - 1, terminal_F, node::pointer_type(3, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id - 1),
                                     node::pointer_type(2, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("applies to node arcs [1]", [&]() {
        /*
        //            1                  1       ---- x0
        //           / \                / \
        //           | 2_               | 2      ---- x1
        //           | | \      =>      |/ \
        //           3 4 T              4  T     ---- x2
        //           |X|               / \
        //           5 6               5  6      ---- x3
        //          / \ \\            / \/ \
        //          F T T F           F  T F
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(2, 1);
        const arc::uid_type n5(3, 0);
        const arc::uid_type n6(3, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n1, false, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n4, false, n5 });
          aw.push_internal({ n3, true, n6 });
          aw.push_internal({ n4, true, n6 });

          aw.push_terminal({ n2, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });
          aw.push_terminal({ n6, false, terminal_T });
          aw.push_terminal({ n6, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 2u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n6
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(3, node::max_id - 1, terminal_T, terminal_F)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id),
                                     node::pointer_type(3, node::max_id - 1))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(2, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(1, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(6u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("applies to both node and terminal arcs", [&]() {
        /*
        //            1                  1     ---- x0
        //           / \                / \
        //           | 2_               | 2    ---- x1
        //           | | \      =>      |/ \
        //           3 4 T              4  T   ---- x2
        //           |X|               / \
        //           5 T               5 T     ---- x3
        //          / \               / \
        //          F T               F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(2, 1);
        const arc::uid_type n5(3, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n1, false, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n4, false, n5 });

          aw.push_terminal({ n2, true, terminal_T });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 1u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, node::max_id, node::pointer_type(3, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(2, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(1, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

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

        // Over-approximation, since T-terminal from level (3) is removed
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));

        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("applies to 'disjoint' branches", [&]() {
        /*
        //               1                        1         ---- x0
        //              / \                      / \
        //             2   3                     2  3       ---- x1
        //            / \ / \                    \\/ \
        //           /   |   \                    X___\     (2.low() goes to 6)
        //           4   5   6        =>          5   6     ---- x2
        //          / \ / \ / \                  / \ / \
        //          F T 7 T F T                  7 T F T    ---- x3
        //             / \                      / \
        //             F T                      F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(2, 2);
        const arc::uid_type n7(3, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n5 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n3, true, n6 });
          aw.push_internal({ n5, false, n7 });

          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, true, terminal_T });
          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true, terminal_T });
          aw.push_terminal({ n7, false, terminal_F });
          aw.push_terminal({ n7, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 3u));
          aw.push(level_info(3, 1u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n7
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n6
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, node::max_id - 1, node::pointer_type(3, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id - 1),
                                     node::pointer_type(2, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id - 1,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(2, node::max_id - 1))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(1, node::max_id - 1),
                                     node::pointer_type(1, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(4u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));

        // Over-approximation: since at level 3 the reductions of (4) and (6) is
        //                     yet not applied.
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(7u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(7u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(7u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(7u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(7u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("does forward the correct children [1]", [&]() {
        /*
        //               1                1        ---- x0
        //              / \              / \
        //             2   3             | 3       ---- x1
        //            / \ / \      =>    |/ \
        //           4   5   6           5   6     ---- x2
        //          / \ / \ / \         / \ / \
        //          F T F T T F         F T T F
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(2, 2);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n5 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n3, true, n6 });

          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });
          aw.push_terminal({ n6, false, terminal_T });
          aw.push_terminal({ n6, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 3u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // 5
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // 6
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2, node::max_id - 1, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True()); // 3
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(2, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // 1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(1, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));

        // Over-approximation: globally counted arc from (1) to (3)
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("does forward the correct children [2]", [&]() {
        /*
        //               1                1        ---- x0
        //              / \              / \
        //             2   3             | 3       ---- x1
        //            / \ / \      =>    |/ \
        //           4   5   6           5   6     ---- x2
        //          / \ / \ / \         / \ / \
        //          T F T F F T         T F F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(2, 2);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n5 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n3, true, n6 });

          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n4, true, terminal_F });
          aw.push_terminal({ n5, false, terminal_T });
          aw.push_terminal({ n5, true, terminal_F });
          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 3u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // 6
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // 5
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2, node::max_id - 1, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True()); // 3
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id - 1),
                                     node::pointer_type(2, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // 1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id - 1),
                                     node::pointer_type(1, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));

        // Over-approxmation: globally counted arc from (2) to (3)
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });
    });

    describe("Reduction Rule 1: BDD", [&]() {
      it("applies to terminal arcs", [&]() {
        /*
        //            1                  1     ---- x0
        //           / \                / \
        //           | 2                | 2    ---- x1
        //           |/ \      =>       |/ \
        //           3  4               3  |   ---- x2
        //          / \//               |\ /
        //          F  T                F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(2, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer early
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n1, false, n3 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n2, true, n4 });

          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n4, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
        }

        in->max_1level_cut = 3;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(2, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(1, node::max_id))));
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
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(4u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("applies to node arcs", [&]() {
        /*
        //            1                  1        ---- x0
        //           / \                / \
        //           | 2                | 2       ---- x1
        //           |/ \      =>       |/ \
        //           3  4               3   \     ---- x2
        //          / \ \\             / \  |
        //          F T  5             F T  5     ---- x3
        //              / \                / \
        //              F T                F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(2, 1);
        const arc::uid_type n5(3, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n1, false, n3 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n4, false, n5 });
          aw.push_internal({ n4, true, n5 });

          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 1u));
        }

        in->max_1level_cut = 3;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(3, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(1, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

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

        // Over-approxmation: globally counted arc from (2) to (5)
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("works with two reductions on the same level", [&]() {
        /*
        //            1                  1        ---- x0
        //           / \                / \
        //          2   3              |   |      ---- x1
        //          ||  ||     =>      |   |
        //          4   5              4   5      ---- x2
        //         / \ / \            / \ / \
        //         F T T F            F T T F
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n3, true, n5 });

          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_T });
          aw.push_terminal({ n5, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 2u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2, node::max_id - 1, terminal_T, terminal_F)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(2, node::max_id - 1))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));

        // Over-approxmation: globally counted arcs (1) to (4) and (1) to (5)
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));

        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));

        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("can be applied together with reduction rule 2 [1]", [&]() {
        /*
        //            1                  1     ---- x0
        //           / \                / \
        //           2 T                | T    ---- x1
        //          / \        =>       |
        //          3 4                 4      ---- x2
        //          |X|                / \
        //          F T                F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(2, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n2, true, n4 });

          aw.push_terminal({ n1, true, terminal_T });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, node::max_id, node::pointer_type(2, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));

        // Over-approximation: globally counted arc (1) to (4)
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));

        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("can be applied together with reduction rule 2 [2]", [&]() {
        /*
        //           1                   1       ---- 0
        //          / \                 / \
        //         2   3               2  |      ---- 1
        //        / \ / \     =>      / \ /
        //       /   4   5           /   4       ---- 2
        //      /   / \ / \         /   / \
        //      6   F T F T         6   F T      ---- 3
        //     / \                 / \
        //     F T                 F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(3, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true, n5 });
          aw.push_internal({ n2, false, n6 });

          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });
          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 1u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n6
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id),
                                     node::pointer_type(2, node::max_id))));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(1, node::max_id),
                                     node::pointer_type(2, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));

        // Over-approximation: globally counted arc from (1) to (4)
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));

        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));

        // Over-approximation: at the lowest level we don't know yet, that
        //                     nodes (4) and (5) will be merged.
        //
        //                     Currently, returns 5, i.e. maximum DiCut of '.size() + 1'.
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("can reduce the root [1]", [&]() {
        /*
        //           1                         ---- x0
        //          / \
        //          | 2                        ---- x1
        //          |/|         =>
        //          3 4                 4      ---- x2
        //          |X|                / \
        //          F T                F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(2, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n1, false, n3 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n2, true, n4 });

          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
        }

        in->max_1level_cut = 3;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

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

      it("can reduce the root [2]", [&]() {
        /*
        //         _1_                         ---- x0
        //        /   \
        //        2_ _3              _2_       ---- x1
        //        |_X_|             /   \
        //        4   5             4   5      ---- x2
        //       / \ / \     =>    / \ / \
        //       6  7   8          6  7   8    ---- x3
        //      / \/ \ / \        / \/ \ / \
        //      F  9 T F T        F  9 T F T   ---- x4
        //        / \               / \
        //        F T               F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(3, 0);
        const arc::uid_type n7(3, 1);
        const arc::uid_type n8(3, 2);
        const arc::uid_type n9(4, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n2, true, n5 });
          aw.push_internal({ n3, true, n5 });
          aw.push_internal({ n4, false, n6 });
          aw.push_internal({ n4, true, n7 });
          aw.push_internal({ n5, false, n7 });
          aw.push_internal({ n5, true, n8 });
          aw.push_internal({ n6, true, n9 });
          aw.push_internal({ n7, false, n9 });

          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n7, true, terminal_T });
          aw.push_terminal({ n8, false, terminal_F });
          aw.push_terminal({ n8, true, terminal_T });
          aw.push_terminal({ n9, false, terminal_F });
          aw.push_terminal({ n9, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 3u));
          aw.push(level_info(4, 1u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n9
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n8
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n7
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, node::max_id - 1, node::pointer_type(4, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n6
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, node::max_id - 2, terminal_F, node::pointer_type(4, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id - 1),
                                     node::pointer_type(3, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n4
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2,
                                     node::max_id - 1,
                                     node::pointer_type(3, node::max_id - 2),
                                     node::pointer_type(3, node::max_id - 1))));

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id - 1),
                                     node::pointer_type(2, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));
        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 3u)));
        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));
        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(3u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(4u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(8u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("accounts for multiple ingoing arcs to reduction rule 1 node", [&]() {
        /*
        //           1                   1       ---- x0
        //          / \                 / \
        //         2   \                2 T      ---- x1
        //        / \  |               / \
        //      _3_  \ |       =>    _3_ T       ---- x2
        //     /   \  \|            /   \
        //     4   5   6            4   5        ---- x3
        //    / \ / \ / \          / \ / \
        //    T F F T T T          T F F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(3, 0);
        const arc::uid_type n5(3, 1);
        const arc::uid_type n6(3, 2);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true, n5 });
          aw.push_internal({ n1, true, n6 });
          aw.push_internal({ n2, true, n6 });

          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n4, true, terminal_F });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });
          aw.push_terminal({ n6, false, terminal_T });
          aw.push_terminal({ n6, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 3u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(3, node::max_id - 1, terminal_T, terminal_F)));

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id - 1),
                                     node::pointer_type(3, node::max_id))));

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(2, node::max_id), terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, node::max_id, node::pointer_type(1, node::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));

        // Over-approximation: globally counted arc (1) to T and (2) to T
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(6u));

        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));

        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(6u));

        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));
      });

      it("can apply reduction rule 1 to a single node", [&]() {
        /*
        //           1                 F       ---- x0
        //          / \      =>
        //          F F
        */

        const arc::uid_type n1(0, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n1, true, terminal_F });

          aw.push(level_info(0, 1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // F
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

      it("can propagate reduction rule 1 up to a terminal", [&]() {
        /*
        //           1                  T
        //          / \
        //          | 2         =>
        //          |/ \
        //          T  T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });

          aw.push_terminal({ n1, false, terminal_T });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
        }

        in->max_1level_cut = 1;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

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

      it("can return non-reducible single-node variable with max_id", [&]() {
        /*
        //           1                 1       ---- x0
        //          / \      =>       / \
        //          F T               F T
        */

        const arc::uid_type n1(0, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n1, true, terminal_T });

          aw.push(level_info(0u, 1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0u, 1u)));
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

      it("can account for a chain of don't cares contributing to the maximum cut [1]", [&]() {
        /*
        //       _1_                 1             ---- x0
        //      /   \               / \
        //      2   3              /  3            ---- x1
        //      \\ / \             | / \
        //        4  5             |/  5           ---- x2
        //        \\/ \            || / \
        //         6  7      =>    || | 7          ---- x3
        //         \\/ \           || |/ \
        //          8  9           \\ || 9         ---- x4
        //          \\/ \           \\|// \
        //          10  11           10   11       ---- x5
        //         /  \/  \         /  \ /  \
        //         T  F   T         T  F F  T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(3, 0);
        const arc::uid_type n7(3, 1);
        const arc::uid_type n8(4, 0);
        const arc::uid_type n9(4, 1);
        const arc::uid_type n10(5, 0);
        const arc::uid_type n11(5, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true, n5 });
          aw.push_internal({ n4, false, n6 });
          aw.push_internal({ n4, true, n6 });
          aw.push_internal({ n5, false, n6 });
          aw.push_internal({ n5, true, n7 });
          aw.push_internal({ n6, false, n8 });
          aw.push_internal({ n6, true, n8 });
          aw.push_internal({ n7, false, n8 });
          aw.push_internal({ n7, true, n9 });
          aw.push_internal({ n8, false, n10 });
          aw.push_internal({ n8, true, n10 });
          aw.push_internal({ n9, false, n10 });
          aw.push_internal({ n9, true, n11 });

          aw.push_terminal({ n10, false, terminal_T });
          aw.push_terminal({ n10, true, terminal_F });
          aw.push_terminal({ n11, false, terminal_F });
          aw.push_terminal({ n11, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 2u));
          aw.push(level_info(4, 2u));
          aw.push(level_info(5, 2u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // n11
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n10
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(5, node::max_id - 1, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n9
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(4,
                                     node::max_id,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n7
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(3,
                                     node::max_id,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(4, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n5
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2,
                                     node::max_id,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(3, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n3
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(2, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(1, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(8u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(8u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("can account for a chain of don't cares contributing to the maximum cut [2]", [&]() {
        /*
        //            __1__                      __1__           ---- x0
        //           /     \                    /     \
        //           2_   _3                    2_   _3          ---- x1
        //          /  \ /  \                  /  \ /  \
        //          4_  5  _6                  4  | |  6         ---- x2
        //          | \| |/ |                 / \ | | / \
        //          7_  8  _9                 7 | | | | 9        ---- x3
        //          | \| |/ |                /\ | | | // \
        //          10  11 _12              10_\\ | ///__12      ---- x4
        //          | \| |/ |              /   \\\|////    \
        //          *   13  14*            *      13      14*    ---- x5
        //             /  \/  \                  /  \    /  \
        //             T  F   T                  T  F    F  T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(2, 2);
        const arc::uid_type n7(3, 0);
        const arc::uid_type n8(3, 1);
        const arc::uid_type n9(3, 2);
        const arc::uid_type n10(4, 0);
        const arc::uid_type n11(4, 1);
        const arc::uid_type n12(4, 2);
        const arc::uid_type n13(5, 0);
        const arc::uid_type n14(5, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n5 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n3, true, n6 });
          aw.push_internal({ n4, false, n7 });
          aw.push_internal({ n4, true, n8 });
          aw.push_internal({ n5, false, n8 });
          aw.push_internal({ n5, true, n8 });
          aw.push_internal({ n6, false, n8 });
          aw.push_internal({ n6, true, n9 });
          aw.push_internal({ n7, false, n10 });
          aw.push_internal({ n7, true, n11 });
          aw.push_internal({ n8, false, n11 });
          aw.push_internal({ n8, true, n11 });
          aw.push_internal({ n9, false, n11 });
          aw.push_internal({ n9, true, n12 });
          aw.push_internal({ n10, true, n13 });
          aw.push_internal({ n11, false, n13 });
          aw.push_internal({ n11, true, n13 });
          aw.push_internal({ n12, false, n13 });
          aw.push_internal({ n10, false, n14 });
          aw.push_internal({ n12, true, n14 });

          aw.push_terminal({ n13, false, terminal_T });
          aw.push_terminal({ n13, true, terminal_F });
          aw.push_terminal({ n14, false, terminal_F });
          aw.push_terminal({ n14, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 3u));
          aw.push(level_info(3, 3u));
          aw.push(level_info(4, 3u));
          aw.push(level_info(5, 2u));
        }

        in->max_1level_cut = 8;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // n14
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n13
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(5, node::max_id - 1, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n12
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(4,
                                     node::max_id,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n10
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(4,
                                     node::max_id - 1,
                                     node::pointer_type(5, node::max_id),
                                     node::pointer_type(5, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n7
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(3,
                                     node::max_id,
                                     node::pointer_type(4, node::max_id - 1),
                                     node::pointer_type(5, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n9
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(3,
                                     node::max_id - 1,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(4, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n4
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id),
                                     node::pointer_type(5, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n6
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2,
                                     node::max_id - 1,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(3, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n2
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(5, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n3
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id - 1,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(2, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(1, node::max_id),
                                     node::pointer_type(1, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(10u));
        AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(15u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(10u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(15u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(10u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(15u));
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(10u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(15u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(10u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(15u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(10u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(15u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(10u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(15u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(10u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(15u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("applies tainted arcs eagerly to the maximum cut", [&]() {
        /*
        //          __1__                __1__         ---- x0
        //         /     \              /     \
        //        _2_   _3_            _2_   _3_       ---- x1
        //       /   \ /   \          /   \ /   \
        //       4    5    6    ==>   4    5    6      ---- x2
        //      / \  / \  / \        / \  / \  / \
        //      F T  F 7  T F        F T  F |  T F     ---- x3
        //             ||                   |
        //             8                    8          ---- x4
        //            / \                  / \
        //            F T                  F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(2, 2);
        const arc::uid_type n7(3, 0);
        const arc::uid_type n8(4, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n5 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n3, true, n6 });
          aw.push_internal({ n5, true, n7 });
          aw.push_internal({ n7, false, n8 });
          aw.push_internal({ n7, true, n8 });

          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n6, false, terminal_T });
          aw.push_terminal({ n6, true, terminal_F });
          aw.push_terminal({ n8, false, terminal_F });
          aw.push_terminal({ n8, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 3u));
          aw.push(level_info(3, 1u));
          aw.push(level_info(4, 1u));

          in->max_1level_cut = 4;
        }

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // n8
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n4
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n6
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2, node::max_id - 1, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n5
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, node::max_id - 2, terminal_F, node::pointer_type(4, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n3
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id - 2),
                                     node::pointer_type(2, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n2
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id - 1,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(2, node::max_id - 2))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n2
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(1, node::max_id - 1),
                                     node::pointer_type(1, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 3u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(3u));

        // NOTE: the Internal cut is only 4, since the tainted edge is accounted for below x1.
        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(4u));

        // Yet, of course the cut is still an over-approximation when the arc is
        // tainting the levels below x2.
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(7u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(8u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(7u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(7u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(7u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(9u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("merges nodes, despite of reduction rule 1 flag on child", [&]() {
        /*
        //      _1_                       ---- x0
        //     /   \
        //     2   3     =>        2      ---- x1
        //    / \ / \             / \
        //    F 4 F T             F T     ---- x2
        //     / \
        //     T T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, true, n4 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n4, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 1u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        bdd out(__bdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(bdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // n2
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
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });
    });

    describe("Reduction Rule 1: ZDD", [&]() {
      it("applies to terminal arcs", [&]() {
        /*
        //           1                  1     ---- x0
        //          / \                / \
        //          | 2         =>     T T    ---- x1
        //          |/ \
        //          T  F
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });

          aw.push_terminal({ n1, false, terminal_T });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
        }

        in->max_1level_cut = 1;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id, terminal_T, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("applies to node arcs", [&]() {
        /*
        //            1                  1        ---- x0
        //           / \                / \
        //           | 2                | 2       ---- x1
        //           |/ \      =>       |/ \
        //           3   4              3   \     ---- x2
        //          / \ / \            / \  |
        //          F T 5 F            F T  5     ---- x3
        //             / \                 / \
        //             F T                 F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(2, 0);
        const arc::uid_type n4(2, 1);
        const arc::uid_type n5(3, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n1, false, n3 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n4, false, n5 });

          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, true, terminal_F });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 1u));
        }

        in->max_1level_cut = 3;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n5
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(3, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(1, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));

        // Over-approximation: globally counted arc (2) to (5)
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));

        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));

        // Over-approximation: at (5) we do not know the merge of (3) and (4).
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("can be applied together with reduction rule 2", [&]() {
        /*
        //              1                  1     ---- x0
        //             / \                ||
        //            2   3                3     ---- x1
        //           / \ / \              / \
        //           F T F 4              F T
        //                / \
        //                T F
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n3, true, n4 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_T });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n4, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 1u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n3
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());

        // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(1, node::max_id),
                                     node::pointer_type(1, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));

        // Over-approximation: globally counted arc (1) to (2) and (1) to (3)
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));

        // Over-approximation: after (4) is removed we don't know (2) and (3)
        //                     will be merged.
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("applies to a single node", [&]() {
        /*
        //           1                 T       ---- x0
        //          / \      =>
        //          T F
        */

        const arc::uid_type n1(0, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_terminal({ n1, false, terminal_T });
          aw.push_terminal({ n1, true, terminal_F });

          aw.push(level_info(0, 1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // F
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

      it("can reduce the root", [&]() {
        /*
        //            1                        ---- x0
        //           / \
        //           2 F        =>      2      ---- x1
        //          / \                / \
        //          F T                F T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, n2 });

          aw.push_terminal({ n1, true, terminal_F });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
        }

        in->max_1level_cut = 1;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());

        // n2
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

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can propagate reduction rule 1 up to a terminal", [&]() {
        /*
        //           1                  F
        //          / \
        //          | 2         =>
        //          |/ \
        //          F  F
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));
        }

        in->max_1level_cut = 1;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(out->is_canonical(), Is().True());

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

      it("can return non-reducible single-node variable with max_id [1]", [&]() {
        /*
        //           1                 1       ---- x42
        //          / \      =>       / \
        //          F T               F T
        */

        const arc::uid_type n1(42, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n1, true, terminal_T });

          aw.push(level_info(42, 1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(42, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(42u, 1u)));

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

      it("can return non-reducible single-node variable with max_id [2]", [&]() {
        /*
        //           1                 1       ---- x12
        //          / \      =>       / \
        //          T T               T T
        */

        const arc::uid_type n1(12, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_terminal({ n1, false, terminal_T });
          aw.push_terminal({ n1, true, terminal_T });

          aw.push(level_info(12, 1u));
        }

        in->max_1level_cut = 0;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(12, node::max_id, terminal_T, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(12u, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("can account for a chain of don't cares contributing to the maximum cut [1]", [&]() {
        /*
        //              _1_                 1             ---- x0
        //             /   \               / \
        //             2   3              2  |            ---- x1
        //            / \ / \            / \ |
        //            4  5  F            4 | |            ---- x2
        //           / \/ \             / \| |
        //           6  7 F             6 || |            ---- x3
        //          / \/ \             / \|| |
        //          8  9 F             8 ||| /            ---- x4
        //         / \/ \             / \||//
        //        10 11 F             10 11               ---- x5
        //       / | | \             / | | \
        //       F T T T             F T T T
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(3, 0);
        const arc::uid_type n7(3, 1);
        const arc::uid_type n8(4, 0);
        const arc::uid_type n9(4, 1);
        const arc::uid_type n10(5, 0);
        const arc::uid_type n11(5, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n5 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n4, false, n6 });
          aw.push_internal({ n4, true, n7 });
          aw.push_internal({ n5, false, n7 });
          aw.push_internal({ n6, false, n8 });
          aw.push_internal({ n6, true, n9 });
          aw.push_internal({ n7, false, n9 });
          aw.push_internal({ n8, false, n10 });
          aw.push_internal({ n8, true, n11 });
          aw.push_internal({ n9, false, n11 });

          aw.push_terminal({ n3, true, terminal_F });
          aw.push_terminal({ n5, true, terminal_F });
          aw.push_terminal({ n7, true, terminal_F });
          aw.push_terminal({ n9, true, terminal_F });
          aw.push_terminal({ n10, false, terminal_F });
          aw.push_terminal({ n10, true, terminal_T });
          aw.push_terminal({ n11, false, terminal_T });
          aw.push_terminal({ n11, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 2u));
          aw.push(level_info(4, 2u));
          aw.push(level_info(5, 2u));
        }

        in->max_1level_cut = 4;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // n11
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_T, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n10
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(5, node::max_id - 1, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n9
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(4,
                                     node::max_id,
                                     node::pointer_type(5, node::max_id - 1),
                                     node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n7
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(3,
                                     node::max_id,
                                     node::pointer_type(4, node::max_id),
                                     node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n5
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(2,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id),
                                     node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n3
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True()); // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(1, node::max_id),
                                     node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(8u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(8u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(6u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(8u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("Merges nodes, despite of reduction rule 1 flag on child", [&]() {
        /*
        //        _1_                1      ---- x0
        //       /   \               ||
        //       2   3     =>        2      ---- x1
        //      / \ / \             / \
        //      F 4 F T             F T     ---- x2
        //       / \
        //       T F
        */

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, true, n4 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n4, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 1u));
        }

        in->max_1level_cut = 2;

        // Reduce it
        zdd out(__zdd(in, exec_policy()));

        AssertThat(out->sorted, Is().True());
        AssertThat(out->indexable, Is().True());
        AssertThat(zdd_iscanonical(out), Is().True());

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True()); // n2
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True()); // n1
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(1, node::max_id),
                                     node::pointer_type(1, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));

        // Over-approximation: arc (2) to T is counted globally
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));

        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));

        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));

        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });
    });
  });
});
