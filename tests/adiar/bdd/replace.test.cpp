#include "adiar/internal/algorithms/replace.h"
#include "../../test.h"
#include "adiar/bdd.h"
#include "adiar/bdd/bdd_policy.h"
#include "adiar/exec_policy.h"
#include "adiar/internal/algorithms/dot.h"
#include "adiar/internal/algorithms/reduce.h"
#include "adiar/internal/data_types/level_info.h"
#include "adiar/internal/io/arc_ofstream.h"
#include "adiar/internal/io/shared_file_ptr.h"
#include "adiar/internal/util.h"
#include "adiar/types.h"
#include "bandit/assertion_frameworks/snowhouse/assert.h"
#include "bandit/assertion_frameworks/snowhouse/fluent/fluent.h"
#include <cassert>
#include <string>

void fixed_printdot(__bdd bdd, std::string fn) {
  shared_levelized_file<arc> arcs;
  {
    arc_ofstream uw(arcs);
    arc_test_ifstream out_arcs(bdd);
    while (out_arcs.can_pull_internal()) {
      uw.push_internal(out_arcs.pull_internal());
    }
    while (out_arcs.can_pull_terminal()) {
      uw.push_terminal(out_arcs.pull_terminal());
    }
  }
  print_dot(arcs, fn);
}

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

    const bdd::pointer_type terminal_T = bdd::pointer_type(true);
    const bdd::pointer_type terminal_F = bdd::pointer_type(false);

    shared_levelized_file<bdd::node_type> bdd_x0_nf;
    /*
    //          1        ---- x0
    //         / \
    //         F T
    */
    { // Garbage collect writers early
      node_ofstream nw(bdd_x0_nf);
      nw << node(0, bdd::max_id, terminal_F, terminal_T);
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
      nw << node(1, bdd::max_id, terminal_F, terminal_T);
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
      nw << node(2, bdd::max_id, terminal_F, terminal_T);
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
      const node n3 = node(4, bdd::max_id, terminal_F, terminal_T);
      const node n2 = node(2, bdd::max_id, n3.uid(), terminal_T);
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_1_nf);
      nw << n3 << n2 << n1;
    }
    const bdd bdd_1(bdd_1_nf);

    shared_levelized_file<bdd::node_type> bdd_1_ext_nf;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x2
    //       |/ \
    //       3  |      ---- x4
    //      / \ |
    //     4   5       ---- x5
    //    / \ / \
    //   F   T   F 
    */
    { // Garbage collect early and free write-lock
      const node n5 = node(5, bdd::max_id, terminal_T, terminal_F);
      const node n4 = node(5, bdd::max_id-1, terminal_F, terminal_T);
      const node n3 = node(4, bdd::max_id, n4, n5);
      const node n2 = node(2, bdd::max_id, n3.uid(), n5);
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_1_ext_nf);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_1_ext(bdd_1_ext_nf);


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
      const node n3 = node(1, bdd::max_id, terminal_T, terminal_F);
      const node n2 = node(1, bdd::max_id - 2, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

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
      const node n4 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n3 = node(1, bdd::max_id, terminal_F, n4.uid());
      const node n2 = node(1, bdd::max_id - 1, n4.uid(), terminal_F);
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_3_nf);
      nw << n4 << n3 << n2 << n1;
    }
    const bdd bdd_3(bdd_3_nf);


    shared_levelized_file<bdd::node_type> bdd_4_nf;
    /*
    //
    //        1_       ---- x0
    //        | \
    //        2 |      ---- x1
    //       / \|
    //      F   T
    */

    { // Garbage collect early and free write-lock
      const node n2 = node(1, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id,n2.uid(), terminal_T );

      node_ofstream nw(bdd_4_nf);
      nw << n2 << n1;
    }
    const bdd bdd_4(bdd_4_nf);



    shared_levelized_file<bdd::node_type> bdd_5_nf;
    /*
    //
    //        __1__         ---- x0
    //       /     \
    //      2      3        ---- x1
    //     / \    / \
    //    F   4  F   5      ---- x2
    //       / \    / \
    //      F   T  T   F
    //
    */

    { // Garbage collect early and free write-lock
      const node n5 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n4 = node(3, bdd::max_id-1, terminal_F, terminal_T);
      const node n3 = node(1, bdd::max_id, terminal_F, n5.uid());
      const node n2 = node(1, bdd::max_id-1, terminal_F, n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_5_nf);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_5(bdd_5_nf);

    // Big tree from the apply tests
    shared_levelized_file<bdd::node_type> bdd_6_nf;
    /*
    //            1         ---- x0
    //           / \
    //           2 3        ---- x1
    //         _/ X \_
    //        | _/ \_ |
    //         X     X
    //        / \   / \
    //       4  5  6  7     ---- x2
    //      / \/ \/ \/ \
    //      F T  8  T  F    ---- x3
    //          / \
    //          F T
    */

    { // Garbage collect early and free write-lock
      const node n8 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n7 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n6 = node(2, bdd::max_id - 1, n8.uid(), terminal_T);
      const node n5 = node(2, bdd::max_id - 2, terminal_T, n8.uid());
      const node n4 = node(2, bdd::max_id - 3, terminal_F, terminal_T);
      const node n3 = node(1, bdd::max_id, n4.uid(), n6.uid());
      const node n2 = node(1, bdd::max_id - 1, n5.uid(), n7.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_6_nf);
      nw << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_6(bdd_6_nf);

    shared_levelized_file<bdd::node_type> bdd_7_nf;
    /*
    //          1         ---- x0
    //        _/ \
    //       2    |       ---- x1
    //      / \   |
    //     F  |   3       ---- x2
    //         \ / \_
    //          4    5    ---- x3
    //         / \  / \
    //        F  T T   F
    */

    { // Garbage collect early and free write-lock
      const node n5 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n4 = node(3, bdd::max_id - 1, terminal_F, terminal_T);
      const node n3 = node(2, bdd::max_id, n4.uid(), n5.uid());
      const node n2 = node(1, bdd::max_id, terminal_F, n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_7_nf);
      nw <<  n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_7(bdd_7_nf);

    shared_levelized_file<bdd::node_type> bdd_8_nf;
    //purpose - minimal(ish) example highligting need to change nested sweeping terminal case
    // sweeping on lvl 1 here -> request with 2 leaf children, should not be surpressed 
    /*
    //          1         ---- x0
    //         / \  
    //        /   2       ---- x1
    //       /    |\
    //      3     | \     ---- x2
    //     / \    |  \
    //    F   T   F   T
    */

    { // Garbage collect early and free write-lock
      const node n3 = node(2, bdd::max_id, terminal_F, terminal_T);
      const node n2 = node(1, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_8_nf);
      nw << n3 << n2 << n1;
    }
    const bdd bdd_8(bdd_8_nf);

    shared_levelized_file<bdd::node_type> bdd_9_nf;
    //purpose - many levels to facilitate swaps not side by side
    // also exp worst case if bad order
    /*
    //  (x0 /\ x1) \/ (x2 /\ x3) \/ (x4 /\ x5) \/ (x6 /\ x7)
    */

    { // Garbage collect early and free write-lock
      const node n7 = node(7, bdd::max_id, terminal_F, terminal_T);
      const node n6 = node(6, bdd::max_id, terminal_F, n7.uid());
      const node n5 = node(5, bdd::max_id, n6.uid(), terminal_T);
      const node n4 = node(4, bdd::max_id, n6.uid(), n5.uid());
      const node n3 = node(3, bdd::max_id, n4.uid(), terminal_T);
      const node n2 = node(2, bdd::max_id, n4.uid(), n3.uid());
      const node n1 = node(1, bdd::max_id, n2.uid(), terminal_T);
      const node n0 = node(0, bdd::max_id, n2.uid(), n1.uid());

      node_ofstream nw(bdd_9_nf);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1 << n0;
    }
    const bdd bdd_9(bdd_9_nf);

    shared_levelized_file<bdd::node_type> bdd_10_nf;
    //purpose - has edge crossing 2 layers
    /*
    //       _1      --x0   
    //      / |             
    //     /  2      --x1   
    //    /  / \            
    //   /  3   4    --x2   
    //  /  /|   | \          
    //  | F  \  F  T         
    //   \____5      --x3   
    //       / \            
    //      F   T           
    */

    { // Garbage collect early and free write-lock
      const node n5 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n4 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n3 = node(2, bdd::max_id-1, terminal_F, n5.uid());
      const node n2 = node(1, bdd::max_id, n3.uid(), n4.uid());
      const node n1 = node(0, bdd::max_id, n5.uid(), n2.uid());

      node_ofstream nw(bdd_10_nf);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_10(bdd_10_nf);


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
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("swaps levels 0 and 4 [bdd_1]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          AssertThat(replace__infer_type<bdd_policy>(bdd_1,m), Is().EqualTo(replace_type::Non_Monotone));
          const bdd out = bdd_replace(bdd_1, m);
          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id, terminal_F,terminal_T)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, terminal_F,bdd::pointer_type(4,bdd::max_id))));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0,bdd::max_id, bdd::pointer_type(2,bdd::max_id), terminal_T)));
          AssertThat(out_nodes.can_pull(), Is().False());
        });

        // TODO: Add more complex inputs that test for all relevant behaviours of applying the
        //       Nested Sweeping framework to move levels.
        describe("Jump Down cases", [&]() {
          //TODO: check that level info file is created correctly

          it("Jump down with node and leaf children" , [&]() {
            /*
            //        1        ---- x2
            //       / \
            //       | 2       ---- x0...3?
            //       |/ \
            //       3  T      ---- x4
            //      / \
            //      F T
            */
            const mapping_type m = [](const int x) { if (x == 0) return 3;
                                                     return x; };
            __bdd res = bdd_replace(bdd_1, m);

            arc_test_ifstream out_arcs(res);
            
            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), true, bdd::uid_type(3,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), false, bdd::uid_type(4,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), false, bdd::uid_type(4,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().False());

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(4,0), false, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(4,0), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().False());
          });

          it("Jump down with subtree children" , [&]() {
            /*
            //        1        ---- x2
            //       / \
            //       | 2       ---- x0...3?
            //       |/ \
            //       3  |      ---- x4
            //      / \ |
            //     4   5       ---- x5
            //    / \ / \
            //   F   T   F 
            */
            const mapping_type m = [](const int x) { if (x == 0) return 3;
                                                     return x; };
            __bdd res = bdd_replace(bdd_1_ext, m);

            arc_test_ifstream out_arcs(res);

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), true, bdd::uid_type(3,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), false, bdd::uid_type(4,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), false, bdd::uid_type(4,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(4,0), false, bdd::pointer_type(5,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), true, bdd::pointer_type(5,1)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(4,0), true, bdd::pointer_type(5,1)}));


            AssertThat(out_arcs.can_pull_internal(), Is().False());

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(5,0), false, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(5,0), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(5,1), false, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(5,1), true, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().False());
          });

          it("Jump down for 2 nodes in same mapping" , [&]() {
            /*
            //        1        ---- x2
            //       / \
            //       | 2       ---- x0...3?
            //       |/ \_
            //      3     4    ---- x5
            //     / \   / \
            //    5   6 T   F  ---- x4...6?
            //   / \ / \
            //  F   T   F 
            */
            const mapping_type m = [](const int x) {if (x == 0) return 3;
                                                    if (x == 4) return 6;
                                                    return x; };
            __bdd res = bdd_replace(bdd_1_ext, m);

            arc_test_ifstream out_arcs(res);

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), true, bdd::uid_type(3,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), false, bdd::uid_type(5,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), false, bdd::uid_type(5,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), true, bdd::uid_type(5,1)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(5,0), false, bdd::pointer_type(6,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(5,0), true, bdd::pointer_type(6,1)}));

            AssertThat(out_arcs.can_pull_internal(), Is().False());

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(5,1), false, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(5,1), true, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(6,0), false, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(6,0), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(6,1), false, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(6,1), true, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().False());
          });

          it("jumps the root down to the bottom layer" , [&]() {
            /*
            //        _1_         ---- x2
            //       /   \
            //      2     3       ---- x4
            //     / \   / \
            //    |  T  4  T      ---- x0...5?
            //    |    / \
            //    F   F   T
            */
            const mapping_type m = [](const int x) { if (x == 0) return 5;
                                                     else return x; };
            __bdd res = bdd_replace(bdd_1, m);

            arc_test_ifstream out_arcs(res);

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), false, bdd::pointer_type(4,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), true, bdd::pointer_type(4,1)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(4,1), false, bdd::pointer_type(5,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().False());

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(4,0), false, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(4,0), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(4,1), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(5,0), false, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(5,0), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().False());
          });

          it("jumps that move through a double layer" , [&]() {
            /*
            //
            //          __1__        ---- x1
            //         /     \
            //        2      3       ---- x2
            //       / \    / \
            //      |  F   |  F
            //      4      5         ---- x0...3?
            //     / \    / \
            //    T  F   F   T
            //
            */
            const mapping_type m = [](const int x) { if (x == 0) return 3;
                                                     else return x; };
            __bdd res = bdd_replace(bdd_3, m);

            arc_test_ifstream out_arcs(res);

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(1,0), false, bdd::pointer_type(2,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(1,0), true, bdd::pointer_type(2,1)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,1), false, bdd::pointer_type(3,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), false, bdd::pointer_type(3,1)}));

            AssertThat(out_arcs.can_pull_internal(), Is().False());

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), true, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(2,1), true, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), false, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,1), false, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,1), true, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().False());
          });

          it("Jump down and has two children" , [&]() {
            /*
            //
            //        __1__            ---- x1
            //       /     \
            //      F    __3__         ---- x0...2?
            //          /     \
            //         4       5       ---- x3
            //        / \     / \
            //       F   T   T   F
            //
            */
            const mapping_type m = [](const int x) { if (x == 0) return 2;
                                                     else return x; };
            __bdd res = bdd_replace(bdd_5, m);

            arc_test_ifstream out_arcs(res);

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(1,0), true, bdd::pointer_type(2,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), false, bdd::pointer_type(3,0)}));

            AssertThat(out_arcs.can_pull_internal(), Is().True());
            AssertThat(out_arcs.pull_internal(),
                       Is().EqualTo(arc{bdd::uid_type(2,0), true, bdd::pointer_type(3,1)}));

            AssertThat(out_arcs.can_pull_internal(), Is().False());

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(1,0), false, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), false, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,0), true, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,1), false, terminal_T}));

            AssertThat(out_arcs.can_pull_terminal(), Is().True());
            AssertThat(out_arcs.pull_terminal(),
                       Is().EqualTo(arc{bdd::uid_type(3,1), true, terminal_F}));

            AssertThat(out_arcs.can_pull_terminal(), Is().False());
          });

          it("jumps down and relables levels [bdd_2]", [&]() {
            const mapping_type m = [](const int x) { return 4 - x; };
            //technically also "swaps" levels but not detected as such since the levels that are mapped to are currently empty
            const bdd out = bdd_replace(bdd_2, m);

            node_test_ifstream out_nodes(out);
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id, terminal_F,terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id-1, terminal_T,terminal_F)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(3,bdd::max_id,
              node::pointer_type(4, bdd::max_id),node::pointer_type(4, bdd::max_id-1))));
            AssertThat(out_nodes.can_pull(), Is().False());
          });
        });

        describe( "Adjacent swap cases", [&]() {
          it("swaps levels in BDD_4" , [&]() {
            /*
            //
            //        1_       ---- x0?
            //        | \
            //        2 |      ---- x1?
            //       / \|
            //      F   T
            */
            const mapping_type m = [](const int x) { if (x == 0) return 1;
                                                     if (x == 1) return 0;
                                                     return x; };
            AssertThat(replace__infer_type<bdd_policy>(bdd_4, m), Is().EqualTo(replace_type::Swap_Adjacent));

            bdd res = bdd_replace(bdd_4, m);
            node_test_ifstream out_nodes(res);
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(0,bdd::max_id, bdd::pointer_type(1,bdd::max_id), terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().False());
          });
          
          it("swap top many children [bdd_6]" , [&]() {
            /*
            //            1         ---- x0...1?
            //           / \
            //           2 3        ---- x1...0?
            //         _/ X \_
            //        | _/ \_ |
            //        _X     X_
            //       /  \   /  \
            //      4   5  6    7     ---- x2
            //     / \ / \/ \  / \
            //    T  F T 8  T F  T    ---- x3
            //          / \
            //          F T
            */
            const mapping_type m = [](const int x) { if (x == 0) return 1;
                                                     if (x == 1) return 0;
                                                     else return x; };
            AssertThat(replace__infer_type<bdd_policy>(bdd_6, m), Is().EqualTo(replace_type::Swap_Adjacent));

            bdd res = bdd_replace(bdd_6, m);
            node_test_ifstream out_nodes(res);
            bdd_printdot(res, "big_sweep_example.dot");
            
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(3,bdd::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id-1, node::pointer_type(3,bdd::max_id), terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id-2, terminal_T, terminal_F)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id-3, terminal_T, node::pointer_type(3,bdd::max_id))));
            AssertThat(out_nodes.can_pull(), Is().True());

            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id, node::pointer_type(2,bdd::max_id-3), node::pointer_type(2,bdd::max_id))));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id-1, node::pointer_type(2,bdd::max_id-2), node::pointer_type(2,bdd::max_id-1))));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(0,bdd::max_id, node::pointer_type(1,bdd::max_id), node::pointer_type(1,bdd::max_id-1))));
          });
          it( "swaps bottom layers in bdd_3" , [&]() {
            //map swapping levels 1 and 2
            /*
            //       _1_        ---- x0       //        _1_
            //      /   \                     //       /   \
            //      2   3       ---- x1...2?  //      2     3
            //     / \ / \                    //     / \   / \
            //     | F F |                    //    4  |   5  \
            //      \   /                     //   / \ |  / \  |
            //       \ /                      //  1   0  0  1  0
            //        4         ---- x2...1?
            //       / \
            //       T F
            */
            const mapping_type m = [](const int x) { if (x == 1) return 2;
                                                           if (x == 2) return 1;
                                                           else return x; };

            AssertThat(replace__infer_type<bdd_policy>(bdd_3, m) == replace_type::Swap_Adjacent, Is().True());
            bdd out = bdd_replace(bdd_3, m, replace_type::Swap_Adjacent);

            node_test_ifstream out_nodes(out);
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, terminal_F,terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id-1, terminal_T,terminal_F)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id, node::pointer_type(2,bdd::max_id), terminal_F)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id-1, node::pointer_type(2,bdd::max_id-1), terminal_F)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(0,bdd::max_id, node::pointer_type(1,bdd::max_id-1), node::pointer_type(1,bdd::max_id))));
            AssertThat(out_nodes.can_pull(), Is().False());
          });

          it("handles two side-by-side non-overlapping swaps [bdd_6]", [&](){
            const replace_func<bdd_policy> m = [](const int x) { if (x == 0) {return 1;}
                                                               if (x == 1) {return 0;}
                                                               if (x == 2) {return 3;}
                                                               if (x == 3) {return 2;}
                                                               else return x; };

            AssertThat(replace__infer_type<bdd_policy>(bdd_6, m) == replace_type::Swap_Adjacent, Is().True());                                                  
            bdd out = bdd_replace(bdd_6,m);
            node_test_ifstream out_nodes(out);
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(3,node::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(3,node::max_id-1, terminal_T, terminal_F)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,node::max_id, node::pointer_type(3,node::max_id), terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,node::max_id-1, node::pointer_type(3,node::max_id-1), terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,node::max_id, node::pointer_type(2,node::max_id-1), node::pointer_type(3,node::max_id))));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,node::max_id-1, node::pointer_type(3,node::max_id-1), node::pointer_type(2,node::max_id))));
            AssertThat(out_nodes.can_pull(), Is().True());    
            AssertThat(out_nodes.pull(), Is().EqualTo(node(0,node::max_id, node::pointer_type(1,node::max_id), node::pointer_type(1,node::max_id-1))));
            AssertThat(out_nodes.can_pull(), Is().False());
          });

          it("handles two sperated non-overlapping swaps [bdd_9]", [&](){
            const replace_func<bdd_policy> m = [](const int x) { if (x == 1) {return 2;}
                                                               if (x == 2) {return 1;}
                                                               if (x == 5) {return 6;}
                                                               if (x == 6) {return 5;}
                                                               return x; };
            AssertThat(replace__infer_type<bdd_policy>(bdd_9, m) == replace_type::Swap_Adjacent, Is().True());
            bdd out = bdd_replace(bdd_9, m);

            node_test_ifstream out_nodes(out);
            //for readability: predefined node uids..
            node::pointer_type n7_uid(7,node::max_id);
            node::pointer_type n6m_uid(6,node::max_id);
            node::pointer_type n6m1_uid(6,node::max_id-1);
            node::pointer_type n5m_uid(5,node::max_id);
            node::pointer_type n5m1_uid(5,node::max_id-1);
            node::pointer_type n4_uid(4,node::max_id);
            node::pointer_type n3_uid(3,node::max_id);
            node::pointer_type n2m_uid(2,node::max_id);
            node::pointer_type n2m1_uid(2,node::max_id-1);
            node::pointer_type n1m_uid(1,node::max_id);
            node::pointer_type n1m1_uid(1,node::max_id-1);

            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(7,node::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(6,node::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(6,node::max_id-1, n7_uid, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(5,node::max_id, terminal_F, n7_uid)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(5,node::max_id-1,  n6m_uid, n6m1_uid)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(4,node::max_id,  n5m_uid, n5m1_uid)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(3,node::max_id,  n4_uid, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,node::max_id,  n4_uid, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2,node::max_id-1,  n3_uid, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,node::max_id,  n4_uid, n3_uid)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1,node::max_id-1,  n2m_uid , n2m1_uid)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(0,node::max_id,  n1m_uid, n1m1_uid)));
            AssertThat(out_nodes.can_pull(), Is().False());
          });

          it("handles adjacent-swap crossing arcs", [&](){
            /*
            //       _1      --x0   //            1
            //      / |             //           / \
            //     /  2      --x1   //          /   2
            //    /  / \            //         /   / \
            //   /  3   4    --x2   //        /   3   4
            //  /  /|   | \          //      /   / \  | \
            //  | F  \  F  T         //      |  F   T |  F
            //   \____5      --x3   //       5________/
            //       / \            //      / \
            //      F   T           //     F   T 
            */
          

          const replace_func<bdd_policy> m = [](const int x) { if (x == 1) {return 2;}
                                                                     if (x == 2) {return 1;}
                                                                    return x; };
          bdd out = bdd_replace(bdd_10,m);
          node_test_ifstream out_nodes(out);
          node::pointer_type n5_uid(3,node::max_id);
          node::pointer_type n4_uid(2,node::max_id-1);
          node::pointer_type n3_uid(2,node::max_id);
          node::pointer_type n2_uid(1,node::max_id);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3,node::max_id, terminal_F, terminal_T)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2,node::max_id, terminal_F, terminal_T)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2,node::max_id-1, n5_uid, terminal_F)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1,node::max_id, n3_uid, n4_uid)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0,node::max_id, n5_uid, n2_uid)));
          AssertThat(out_nodes.can_pull(), Is().False());
          });
        });

        describe("Jump-up cases", [&](){

          it("jumps bottom layer up [bdd_1_ext]", [&](){
            /*
            //        1        ---- x0   //           1         --- x0
            //       / \                 //          / \
            //       | 2       ---- x2   //         2   3       --- x1
            //       |/ \                //        / \ / \
            //       3  |      ---- x4   //       /   X   \
            //      / \ |                //      |   4 \   5    --- x2
            //     4   5       ---- x5   //       \ / \ \ / \
            //    / \ / \                //        6   T 7   F  --- x4
            //   F   T   F               //       / \   / \
            //                                   F   T T   F
            */
            const mapping_type m = [](const int x) { if (x == 5) return 1;
                                                           else return x; };
            bdd out = bdd_replace(bdd_1_ext, m);
            
            //expected res tree
            shared_levelized_file<bdd::node_type> er_file;
            {  
              const node n7 = node(4, bdd::max_id, terminal_T, terminal_F);
              const node n6 = node(4, bdd::max_id - 1, terminal_F, terminal_T);
              const node n5 = node(2, bdd::max_id, n7.uid(), terminal_F);
              const node n4 = node(2, bdd::max_id -1, n6.uid(), terminal_T);
              const node n3 = node(1, bdd::max_id, n4.uid(), n5.uid());
              const node n2 = node(1, bdd::max_id -1, n6.uid(), n7.uid());
              const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

              node_ofstream nw(er_file);
              nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
            }
            const bdd expected_res(er_file);
            AssertThat(out, Is().EqualTo(expected_res));
          });
        });
        describe( "Non-monotonic test cases", [&]() {
          //true non-monotone case
          it("reverses the order [bdd 7]", [&]() {
            /* org tree bdd_7               // res tree
            //          1         ---- x0   //          1        --- x3..0
            //        _/ \                  //         / \
            //       2    |       ---- x1   //        2   3_      --- x2..1
            //      / \   |                 //      / |  /  \
            //     F  |   3       ---- x2   //      | | 4    5    --- x1..2
            //         \ / \_               //      | \ | \  | \
            //          4    5    ---- x3   //      |   6  | F  7 --- x0..3
            //         / \  / \             //       \ / \ |   / \
            //        F  T T   F            //        F   T   T   F
            */

            const mapping_type m = [](const int x) { if (x == 0) return 3;
                                                           if (x == 1) return 2;
                                                           if (x == 2) return 1;
                                                           if (x == 3) return 0;
                                                           else return x; };
            bdd res = bdd_replace(bdd_7, m);
            AssertThat(replace__infer_type<bdd_policy>(bdd_7,m), Is().EqualTo(replace_type::Non_Monotone));
            shared_levelized_file<bdd::node_type> bdd_expected_nf;
            { // Garbage collect early and free write-lock
              const node n7 = node(3, bdd::max_id, terminal_T, terminal_F);
              const node n6 = node(3, bdd::max_id-1, terminal_F, terminal_T);
              const node n5 = node(2, bdd::max_id, terminal_F, n7.uid());
              const node n4 = node(2, bdd::max_id-1, n6.uid(), terminal_T);
              const node n3 = node(1, bdd::max_id, n4.uid(), n5.uid());
              const node n2 = node(1, bdd::max_id-1, terminal_F, n6.uid());
              const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

              node_ofstream nw(bdd_expected_nf);
              nw << n7<< n6 << n5 << n4 << n3 << n2 << n1;
            }
            const bdd bdd_expected(bdd_expected_nf);
            AssertThat(res, Is().EqualTo(bdd_expected));
          });
          
          
          it("tests multiple inner sweeps" , [&]() {
           /*
           //            1            ---- x3...0?
           //         __/ \
           //        |     2          ---- x1
           //        |   _/ \_
           //        3  4     5       ---- x2
           //       / \/ \   / \
           //      F  6   F T   7     ---- x0...3?
           //        / \       / \
           //       F  T      T   F
           */
            shared_levelized_file<bdd::node_type> bdd_expected_nf;
            { // Garbage collect early and free write-lock
              const node n7 = node(3, bdd::max_id, terminal_T, terminal_F);
              const node n6 = node(3, bdd::max_id - 1, terminal_F, terminal_T);
              const node n5 = node(2, bdd::max_id, terminal_T, n7.uid());
              const node n4 = node(2, bdd::max_id - 1, n6.uid(), terminal_F);
              const node n3 = node(2, bdd::max_id - 2, terminal_F, n6.uid());
              const node n2 = node(1, bdd::max_id, n4.uid(), n5.uid());
              const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

              node_ofstream nw(bdd_expected_nf);
              nw << n7 << n6 << n5 << n4 << n3 << n2 << n1;
            }
            const bdd bdd_expected(bdd_expected_nf);
            const mapping_type m = [](const int x) { if (x == 0) return 3;
                                                     if (x == 3) return 0;
                                                     else return x; };
            bdd res =  bdd_replace(bdd_7, m);
            AssertThat(res, Is().EqualTo(bdd_expected));
            //TODO: why is this an unsupported type
            //AssertThat(res, Is().EqualTo(bdd_7));
          });
        });

        it("Non-monotonic - Jump down with node and leaf children" , [&]() {
            /*
            //        1        ---- x2
            //       / \
            //       | 2       ---- x0...3?
            //       |/ \
            //       3  T      ---- x4
            //      / \
            //      F T
            */
            const mapping_type m = [](const int x) { if (x == 0) return 3;
            return x; };
            bdd expected_res = bdd_replace(bdd_1, m);
            bdd res = bdd_replace(bdd_1, m, replace_type::Non_Monotone);
            AssertThat(res, Is().EqualTo(expected_res));

        });

        it("Non-monotonic - Jump down with subtree children" , [&]() {
            /*
            //        1        ---- x2
            //       / \
            //       | 2       ---- x0...3?
            //       |/ \
            //       3  |      ---- x4
            //      / \ |
            //     4   5       ---- x5
            //    / \ / \
            //   F   T   F 
            */
            const mapping_type m = [](const int x) { if (x == 0) return 3;
            return x; };
            bdd expected_res = bdd_replace(bdd_1_ext, m);
            bdd res = bdd_replace(bdd_1_ext, m, replace_type::Non_Monotone);
            AssertThat(res, Is().EqualTo(expected_res));


        });

        it("Non-monotonic - Jump down for 2 nodes in same mapping" , [&]() {
            /*
            //        1        ---- x2
            //       / \
            //       | 2       ---- x0...3?
            //       |/ \_
            //      3     4    ---- x5
            //     / \   / \
            //    5   6 T   F  ---- x4...6?
            //   / \ / \
            //  F   T   F 
            */
            const mapping_type m = [](const int x) {if (x == 0) return 3;
                                                    if (x == 4) return 6;
                                                    return x; };
            bdd expected_res = bdd_replace(bdd_1_ext, m);
            bdd res = bdd_replace(bdd_1_ext, m, replace_type::Non_Monotone);
            AssertThat(res, Is().EqualTo(expected_res));

        });

        it("Non-monotonic - jumps the root down to the bottom layer" , [&]() {
            /*
            //        _1_         ---- x2
            //       /   \
            //      2     3       ---- x4
            //     / \   / \
            //    |  T  4  T      ---- x0...5?
            //    |    / \
            //    F   F   T
            */
            const mapping_type m = [](const int x) {if (x == 0) return 5;
                                                    else return x;};

            bdd expected_res = bdd_replace(bdd_1, m);
            bdd res = bdd_replace(bdd_1, m, replace_type::Non_Monotone);
            AssertThat(res, Is().EqualTo(expected_res));

        });

        it("Non-monotonic - jumps that move through a double layer" , [&]() {
            /*
            //
            //          __1__        ---- x1
            //         /     \
            //        2      3       ---- x2
            //       / \    / \
            //      |  F   |  F
            //      4      5         ---- x0...3?
            //     / \    / \
            //    T  F   F   T
            //
            */
            const mapping_type m = [](const int x) {if (x == 0) return 3;
                                                    else return x; };
            bdd expected_res = bdd_replace(bdd_3, m);
            bdd res = bdd_replace(bdd_3, m, replace_type::Non_Monotone);
            AssertThat(res, Is().EqualTo(expected_res));


        });

        it("Non-monotonic -Jump down and has two children" , [&]() {
            /*
            //
            //        __1__            ---- x1
            //       /     \
            //      F    __3__         ---- x0...2?
            //          /     \
            //         4       5       ---- x3
            //        / \     / \
            //       F   T   T   F
            //
            */
            const mapping_type m = [](const int x) {if (x == 0) return 2;
                                                    else return x; };
            bdd expected_res = bdd_replace(bdd_5, m);
            bdd res = bdd_replace(bdd_5, m, replace_type::Non_Monotone);
            AssertThat(res, Is().EqualTo(expected_res));

        });

        it("correctly handles terminal requests in nested sweeping [bdd_8]", [&](){
            /*
            */
            const mapping_type m = [](const int x) {if (x == 1) return 2;
                                                          if (x == 2) return 1; 
                                                          return x;};
            bdd out = bdd_replace(bdd_8, m, replace_type::Non_Monotone);
            node_test_ifstream out_nodes(out);
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id, node::pointer_type(1,node::max_id), node::pointer_type(2,node::max_id))));
            AssertThat(out_nodes.can_pull(), Is().False());
        });

        //TODO: more tests?!
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

 
      it(" renames single node in [x0] to x1 if 'replace_type' is 'Non_Monotone'", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out = bdd_replace(bdd_x0, m, replace_type::Non_Monotone);

        //should just call new node x1?
        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());
      });

      it("swaps top and bottom layer in bdd1 if 'replace_type' is 'Non_Monotone'", [&]() {
        // NOTE: This mapping proves it can swap levels
        const mapping_type m = [](const int x) { return 4 - x; }; //swaps top and bottom levels
        const bdd out = bdd_replace(bdd_1, m, replace_type::Non_Monotone);

        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id, terminal_F,terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, terminal_F,bdd::pointer_type(4,bdd::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0,bdd::max_id, bdd::pointer_type(2,bdd::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());
      });

      it("shifts all layers when 'replace_type' is 'Non_Monotone' [bdd_2]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'; the BDD should end up reduced.
        // runs no inner sweeps for this
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out = bdd_replace(bdd_2, m, replace_type::Non_Monotone);

        //TODO - look into why the two bottom nodes get swapped id's in this case
        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id-1, terminal_T, terminal_F)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id,
                                                                 bdd::pointer_type(2,node::max_id),
                                                                 bdd::pointer_type(2,node::max_id-1))));
        AssertThat(out_nodes.can_pull(), Is().False());
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
      const bdd::uid_type n1(0, 0);

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
      const bdd::uid_type n1(0, 0);
      const bdd::uid_type n2(2, 0);

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
      const bdd::uid_type n1(0, 0);
      const bdd::uid_type n2(2, 0);
      const bdd::uid_type n3(4, 0);

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
      const bdd::uid_type n1(0, 0);
      const bdd::uid_type n2(1, 0);
      const bdd::uid_type n3(1, 1);

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
      const bdd::uid_type n1(0, 0);
      const bdd::uid_type n2(1, 0);
      const bdd::uid_type n3(1, 1);
      const bdd::uid_type n4(2, 0);

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
      const bdd::uid_type n6(2, 2);
      const bdd::uid_type n5(2, 1);
      const bdd::uid_type n4(2, 0);
      const bdd::uid_type n3(1, 1);
      const bdd::uid_type n2(1, 0);
      const bdd::uid_type n1(0, 0);

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

        it("re-labels and reduces bdd when map is non monotone [__bdd_x0_unreduced]",
           [&]() {
            const mapping_type m = [](const int x) { return 4 - x; };
            bdd out = bdd_replace(__bdd(__bdd_x0_unreduced, exec_policy()), m);

            node_test_ifstream out_nodes(out);
            AssertThat(out_nodes.can_pull(), Is().True());
            AssertThat(out_nodes.pull(), Is().EqualTo(node(4,node::max_id, terminal_F, terminal_T)));
            AssertThat(out_nodes.can_pull(), Is().False());
           });
        
        it("swaps top and bottom levels [__bdd_1]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out = bdd_replace(__bdd(__bdd_1, exec_policy()), m);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id, terminal_F,terminal_T)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, terminal_F,bdd::pointer_type(4,bdd::max_id))));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0,bdd::max_id, bdd::pointer_type(2,bdd::max_id), terminal_T)));
          AssertThat(out_nodes.can_pull(), Is().False());
        });

        it("swaps levels [__bdd_2]", [&]() {
          const mapping_type m = [](const int x) { return 4 - x; };
          const bdd out = bdd_replace(__bdd(__bdd_2, exec_policy()), m);

          node_test_ifstream out_nodes(out);
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id, terminal_F,terminal_T)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id-1, terminal_T,terminal_F)));
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3,bdd::max_id,
             node::pointer_type(4, bdd::max_id),node::pointer_type(4, bdd::max_id-1))));
          AssertThat(out_nodes.can_pull(), Is().False());
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


      it("shifts x0 to x1 when 'replace_type' is 'Non_Monotone' [bdd_x0]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out = bdd_replace(exec_policy(), __bdd(bdd_x0_nf), m, replace_type::Non_Monotone);

        //should just call new node x1?
        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());
      });

      it("shifts x0 to x1 when 'replace_type' is 'Non_Monotone' [__bdd_x0]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out = bdd_replace(__bdd(__bdd_x0, exec_policy()), m, replace_type::Non_Monotone);

        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());
      });

      it("produces reduced node x1 when 'replace_type' is 'Non_Monotone' [__bdd_x0_unreduced]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'
        // NOTE when doing non-monotone sweep output is also reduced
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out = bdd_replace(__bdd(__bdd_x0_unreduced, exec_policy()), m, replace_type::Non_Monotone);

        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());
      });

      it("swaps top and bottom levels if 'replace_type' is 'Non_Monotone' [bdd_1]", [&]() {
        // NOTE: This mapping proves it can swap levels
        const mapping_type m = [](const int x) { return 4 - x; };
        const bdd out = bdd_replace(__bdd(bdd_1_nf), m, replace_type::Non_Monotone);
        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id, terminal_F,terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, terminal_F,bdd::pointer_type(4,bdd::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0,bdd::max_id, bdd::pointer_type(2,bdd::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

      });

      it("performs replacement when 'replace_type' is 'Non_Monotone' [__bdd_1]", [&]() {
        // NOTE: This mapping proves it can swap levels
        const mapping_type m = [](const int x) { return 4 - x; };
        const bdd out = bdd_replace(__bdd(__bdd_1, exec_policy()), m, replace_type::Non_Monotone);

        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4,bdd::max_id, terminal_F,terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, terminal_F,bdd::pointer_type(4,bdd::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(0,bdd::max_id, bdd::pointer_type(2,bdd::max_id), terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

      });

      it("shifts and reduces if 'replace_type' is 'Non_Monotone' [bdd_3]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'; the BDD should end up reduced.
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out = bdd_replace(__bdd(bdd_3), m, replace_type::Non_Monotone);
        //TODO once again - why do the ids of nodes get swapped
        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3,bdd::max_id, terminal_T,terminal_F)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, bdd::pointer_type(3,bdd::max_id), terminal_F)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id-1, terminal_F, bdd::pointer_type(3,bdd::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id,
           bdd::pointer_type(2,bdd::max_id), bdd::pointer_type(2,bdd::max_id-1))));
        AssertThat(out_nodes.can_pull(), Is().False());
      });

      it("shifts and reduces if 'replace_type' is 'Non_Monotone' [__bdd_3_unreduced]", [&]() {
        // NOTE: This function is in fact 'Affine'/'Shift'; the BDD should end up reduced.
        const mapping_type m = [](const int x) { return x + 1; };
        const bdd out = bdd_replace(__bdd(__bdd_3_unreduced, exec_policy()), m, replace_type::Non_Monotone);

        node_test_ifstream out_nodes(out);
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3,bdd::max_id, terminal_T,terminal_F)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id, bdd::pointer_type(3,bdd::max_id), terminal_F)));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2,bdd::max_id-1, terminal_F, bdd::pointer_type(3,bdd::max_id))));
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1,bdd::max_id,
           bdd::pointer_type(2,bdd::max_id), bdd::pointer_type(2,bdd::max_id-1))));
        AssertThat(out_nodes.can_pull(), Is().False());
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
