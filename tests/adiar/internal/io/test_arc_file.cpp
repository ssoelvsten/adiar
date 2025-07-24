#include "../../../test.h"
#include <filesystem>

#include <adiar/internal/io/narc_ifstream.h>
#include <adiar/internal/io/narc_raccess.h>

go_bandit([]() {
  describe("adiar/internal/io/arc_file.h , arc_ifstream.h , arc_ofstream.h", []() {
    describe("arc_ofstream", []() {
      it("can push level information", []() {
        levelized_file<arc> af;

        arc_ofstream aw(af);
        aw << (level_info{ 0, 1 }) << (level_info{ 1, 2 });
        aw.close();

        AssertThat(af.size(), Is().EqualTo(0u));
        AssertThat(af.levels(), Is().EqualTo(2u));

        // AssertThat(af.semi_transposed, Is().True());

        AssertThat(af.max_1level_cut, Is().EqualTo(cut::max));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(af.number_of_terminals[true], Is().EqualTo(0u));
      });

      it("can push internal arcs", []() {
        levelized_file<arc> af;

        arc_ofstream aw(af);
        aw.push_internal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(1, 0)));
        aw.push_internal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(1, 0)));
        aw.push_internal(arc(arc::pointer_type(1, 0), false, arc::pointer_type(2, 0)));
        aw.close();

        AssertThat(af.size(), Is().EqualTo(3u));
        AssertThat(af.size(0u), Is().EqualTo(3u));
        AssertThat(af.size(1u), Is().EqualTo(0u));
        AssertThat(af.size(2u), Is().EqualTo(0u));

        AssertThat(af.levels(), Is().EqualTo(0u));

        // AssertThat(af.semi_transposed, Is().True());

        AssertThat(af.max_1level_cut, Is().EqualTo(cut::max));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(af.number_of_terminals[true], Is().EqualTo(0u));
      });

      it("can push terminal arcs [in-order]", []() {
        levelized_file<arc> af;

        arc_ofstream aw(af);
        aw.push_terminal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(true)));
        aw.push_terminal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(true)));
        aw.close();

        AssertThat(af.size(), Is().EqualTo(2u));
        AssertThat(af.size(0u), Is().EqualTo(0u));
        AssertThat(af.size(1u), Is().EqualTo(2u));
        AssertThat(af.size(2u), Is().EqualTo(0u));

        AssertThat(af.levels(), Is().EqualTo(0u));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(af.number_of_terminals[true], Is().EqualTo(2u));

        // AssertThat(af.semi_transposed, Is().True());

        levelized_ifstream<arc> lfs(af);
        AssertThat(lfs.can_pull<0>(), Is().False());

        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),
                   Is().EqualTo(arc(arc::pointer_type(0, 0), false, arc::pointer_type(true))));
        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),
                   Is().EqualTo(arc(arc::pointer_type(0, 0), true, arc::pointer_type(true))));
        AssertThat(lfs.can_pull<1>(), Is().False());

        AssertThat(lfs.can_pull<2>(), Is().False());
      });

      it("can push terminal arcs [out-of-order]", []() {
        levelized_file<arc> af;

        arc_ofstream aw(af);
        aw.push_terminal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(true)));
        aw.push_terminal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(false)));
        aw.close();

        AssertThat(af.size(), Is().EqualTo(2u));
        AssertThat(af.size(0u), Is().EqualTo(0u));
        AssertThat(af.size(1u), Is().EqualTo(1u));
        AssertThat(af.size(2u), Is().EqualTo(1u));

        AssertThat(af.levels(), Is().EqualTo(0u));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(af.number_of_terminals[true], Is().EqualTo(1u));

        levelized_ifstream<arc> lfs(af);
        AssertThat(lfs.can_pull<0>(), Is().False());

        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),
                   Is().EqualTo(arc(arc::pointer_type(0, 0), true, arc::pointer_type(true))));
        AssertThat(lfs.can_pull<1>(), Is().False());

        AssertThat(lfs.can_pull<2>(), Is().True());
        AssertThat(lfs.pull<2>(),
                   Is().EqualTo(arc(arc::pointer_type(0, 0), false, arc::pointer_type(false))));
        AssertThat(lfs.can_pull<2>(), Is().False());
      });

      it("can push mixed arcs", []() {
        levelized_file<arc> af;

        arc_ofstream aw(af);

        aw << arc(arc::pointer_type(0, 0), false, arc::pointer_type(1, 0))
           << arc(arc::pointer_type(1, 0), true, arc::pointer_type(true))  // <-- in-order
           << arc(arc::pointer_type(1, 1), false, arc::pointer_type(true)) // <-- in-order
           << arc(arc::pointer_type(0, 0), true, arc::pointer_type(1, 1))
           << arc(arc::pointer_type(1, 0), false, arc::pointer_type(2, 0))
           << arc(arc::pointer_type(2, 0), false, arc::pointer_type(false)) // <-- in-order
           << arc(arc::pointer_type(2, 0), true, arc::pointer_type(true))   // <-- in-order
           << arc(arc::pointer_type(1, 1), true, arc::pointer_type(true));  // <-- out-of-order

        aw.close();

        AssertThat(af.size(), Is().EqualTo(8u));
        AssertThat(af.size(0u), Is().EqualTo(3u));
        AssertThat(af.size(1u), Is().EqualTo(4u));
        AssertThat(af.size(2u), Is().EqualTo(1u));

        AssertThat(af.levels(), Is().EqualTo(0u));

        AssertThat(af.number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(af.number_of_terminals[true], Is().EqualTo(4u));

        levelized_ifstream<arc> lfs(af);
        AssertThat(lfs.can_pull<0>(), Is().True());
        AssertThat(lfs.pull<0>(),
                   Is().EqualTo(arc(arc::pointer_type(0, 0), false, arc::pointer_type(1, 0))));
        AssertThat(lfs.can_pull<0>(), Is().True());
        AssertThat(lfs.pull<0>(),
                   Is().EqualTo(arc(arc::pointer_type(0, 0), true, arc::pointer_type(1, 1))));
        AssertThat(lfs.can_pull<0>(), Is().True());
        AssertThat(lfs.pull<0>(),
                   Is().EqualTo(arc(arc::pointer_type(1, 0), false, arc::pointer_type(2, 0))));
        AssertThat(lfs.can_pull<0>(), Is().False());

        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),
                   Is().EqualTo(arc(arc::pointer_type(1, 0), true, arc::pointer_type(true))));
        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),
                   Is().EqualTo(arc(arc::pointer_type(1, 1), false, arc::pointer_type(true))));
        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),
                   Is().EqualTo(arc(arc::pointer_type(2, 0), false, arc::pointer_type(false))));
        AssertThat(lfs.can_pull<1>(), Is().True());
        AssertThat(lfs.pull<1>(),
                   Is().EqualTo(arc(arc::pointer_type(2, 0), true, arc::pointer_type(true))));
        AssertThat(lfs.can_pull<1>(), Is().False());

        AssertThat(lfs.can_pull<2>(), Is().True());
        AssertThat(lfs.pull<2>(),
                   Is().EqualTo(arc(arc::pointer_type(1, 1), true, arc::pointer_type(true))));
        AssertThat(lfs.can_pull<2>(), Is().False());
      });
    });

    describe("arc_ofstream + arc_ifstream", []() {
      levelized_file<arc> af;
      /*
                     _0_     ---- x0
                    /   \
                    1   2    ---- x1
                   / \ / \
                   3 T | T
                  / \  |
                  F T  T
       */
      {
        arc_ofstream aw(af);
        aw << arc(arc::pointer_type(0, 0), false, arc::pointer_type(1, 0));
        aw << arc(arc::pointer_type(1, 0), true, arc::pointer_type(true));  // <-- in-order
        aw << arc(arc::pointer_type(1, 1), false, arc::pointer_type(true)); // <-- in-order
        aw << arc(arc::pointer_type(0, 0), true, arc::pointer_type(1, 1));
        aw << arc(arc::pointer_type(1, 0), false, arc::pointer_type(2, 0));
        aw << arc(arc::pointer_type(2, 0), false, arc::pointer_type(false)); // <-- in-order
        aw << arc(arc::pointer_type(2, 0), true, arc::pointer_type(true));   // <-- in-order
        aw << arc(arc::pointer_type(1, 1), true, arc::pointer_type(true));   // <-- out-of-order
      }

      it("merges terminal arcs on 'pull' [default: backwards]", [&af]() {
        arc_ifstream<> as(af);

        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(2, 0), true, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(2, 0), false, arc::pointer_type(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 1), true, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 1), false, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 0), true, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().False());
      });

      it("merges terminal arcs on 'pull' [non-default: forwards]", [&af]() {
        arc_ifstream<true> as(af);

        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 0), true, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 1), false, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 1), true, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(2, 0), false, arc::pointer_type(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(2, 0), true, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().False());
      });

      /*
      it("merges terminal arcs on 'pull' [negated]", [&af]() {
        arc_ifstream<> as(af, true);

        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(2, 0), true, arc::pointer_type(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(2, 0), false, arc::pointer_type(true))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 1), true, arc::pointer_type(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 1), false, arc::pointer_type(false))));
        AssertThat(as.can_pull_terminal(), Is().True());
        AssertThat(as.pull_terminal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 0), true, arc::pointer_type(false))));
        AssertThat(as.can_pull_terminal(), Is().False());
      });
      */

      it("reads internal arcs as given [non-default: forwards]", [&af]() {
        arc_ifstream<> as(af);

        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),
                   Is().EqualTo(arc(arc::pointer_type(1, 0), false, arc::pointer_type(2, 0))));
        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),
                   Is().EqualTo(arc(arc::pointer_type(0, 0), true, arc::pointer_type(1, 1))));
        AssertThat(as.can_pull_internal(), Is().True());
        AssertThat(as.pull_internal(),
                   Is().EqualTo(arc(arc::pointer_type(0, 0), false, arc::pointer_type(1, 0))));
        AssertThat(as.can_pull_internal(), Is().False());
      });

      it("provides number of unread terminals [negate=false]", [&af]() {
        arc_ifstream<> as(af);

        AssertThat(as.unread_terminals(), Is().EqualTo(5u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(1u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(4u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 3 ---> T

        AssertThat(as.unread_terminals(), Is().EqualTo(4u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(1u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(3u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 3 - -> F

        AssertThat(as.unread_terminals(), Is().EqualTo(3u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(3u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 2 ---> T

        AssertThat(as.unread_terminals(), Is().EqualTo(2u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(2u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 2 - -> T

        AssertThat(as.unread_terminals(), Is().EqualTo(1u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(1u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 1 - -> T

        AssertThat(as.unread_terminals(), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(0u));
      });

      /*
      it("provides number of unread terminals [negate=true]", [&af]() {
        arc_ifstream<> as(af, true);

        AssertThat(as.unread_terminals(), Is().EqualTo(5u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(4u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(1u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 3 ---> F

        AssertThat(as.unread_terminals(), Is().EqualTo(4u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(3u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(1u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 3 - -> T

        AssertThat(as.unread_terminals(), Is().EqualTo(3u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(3u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(0u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 2 ---> F

        AssertThat(as.unread_terminals(), Is().EqualTo(2u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(2u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(0u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 2 - -> F

        AssertThat(as.unread_terminals(), Is().EqualTo(1u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(1u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(0u));

        AssertThat(as.can_pull_terminal(), Is().True());
        as.pull_terminal(); // 1 - -> F

        AssertThat(as.unread_terminals(), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(false), Is().EqualTo(0u));
        AssertThat(as.unread_terminals(true), Is().EqualTo(0u));
      });
      */
    });

    describe("arc_ofstream + narc_ifstream", []() {
      it("marks file as de-transposed on attach", []() {
        levelized_file<arc> af;

        {
          arc_ofstream aw(af);
          aw.push_terminal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(false)));
          aw.push_terminal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(true)));

          aw.push(level_info(0, 1));
          aw.close();
        }

        AssertThat(af.semi_transposed, Is().True());
        narc_ifstream ns(af);
        AssertThat(af.semi_transposed, Is().False());
      });

      levelized_file<arc> x0_ordered;
      /*
      //       1         ---- x0
      //      / \
      //      F T
      */
      {
        arc_ofstream aw(x0_ordered);
        aw.push_terminal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(false)));
        aw.push_terminal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(true)));

        aw.push(level_info(0, 1));
        aw.close();
      }

      levelized_file<arc> x0_unordered;
      /*
      //       1         ---- x0
      //      / \
      //      F T        <-- arcs swapped
      */
      {
        arc_ofstream aw(x0_unordered);
        aw.push_terminal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(true)));
        aw.push_terminal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(false)));

        aw.push(level_info(0, 1));
        aw.close();
      }

      it("can pull from 'x0' BDD [in-order]", [&]() {
        narc_ifstream ns(x0_ordered);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can pull single-node BDD [out-of-order]", [&]() {
        narc_ifstream ns(x0_unordered);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can reattach to same file", [&]() {
        narc_ifstream ns(x0_ordered);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can peek single-node BDD [in-order]", [&]() {
        narc_ifstream ns(x0_ordered);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(0, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
      });

      it("can peek single-node BDD [out-of-order]", [&]() {
        narc_ifstream ns(x0_unordered);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(0, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
      });

      /*
      it("can read single-node BDD [negated]", [&]() {
        narc_ifstream ns(x0_ordered, true);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(true), node::pointer_type(false))));

        AssertThat(ns.can_pull(), Is().False());
      });
      */

      it("can pull after peek of single-node BDD", [&]() {
        narc_ifstream ns(x0_ordered);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(0, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());

        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      levelized_file<arc> large_untransposed;
      /*
      //             1        ---- x0
      //            / \
      //            2 |       ---- x1
      //           / \|
      //           F _3_      ---- x2
      //            /   \
      //            4   5     ---- x3
      //           / \ / \
      //           F T T F
      */

      {
        arc_ofstream aw(large_untransposed);

        aw.push(level_info(0, 1));

        aw.push_internal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(1, 0)));
        aw.push_internal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(2, 0)));

        aw.push(level_info(1, 1));

        aw.push_terminal(arc(arc::pointer_type(1, 0), false, arc::pointer_type(false)));
        aw.push_internal(arc(arc::pointer_type(1, 0), true, arc::pointer_type(2, 0)));

        aw.push(level_info(2, 1));

        aw.push_internal(arc(arc::pointer_type(2, 0), false, arc::pointer_type(3, 0)));
        aw.push_internal(arc(arc::pointer_type(2, 0), true, arc::pointer_type(3, 1)));

        aw.push(level_info(3, 2));

        aw.push_terminal(arc(arc::pointer_type(3, 0), false, arc::pointer_type(false)));
        aw.push_terminal(arc(arc::pointer_type(3, 0), true, arc::pointer_type(true)));

        aw.push_terminal(arc(arc::pointer_type(3, 1), false, arc::pointer_type(true)));
        aw.push_terminal(arc(arc::pointer_type(3, 1), true, arc::pointer_type(false)));

        aw.close();

        large_untransposed.semi_transposed = false;
      }

      it("can pull larger BDD [internals already sorted]", [&]() {
        AssertThat(large_untransposed.semi_transposed, Is().False());

        narc_ifstream ns(large_untransposed);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(1, 0), node::pointer_type(2, 0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1, 0, node::pointer_type(false), node::pointer_type(2, 0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2, 0, node::pointer_type(3, 0), node::pointer_type(3, 1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(3, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(3, 1, node::pointer_type(true), node::pointer_type(false))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can peek and pull larger BDD", [&]() {
        AssertThat(large_untransposed.semi_transposed, Is().False());

        narc_ifstream ns(large_untransposed);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(0, 0, node::pointer_type(1, 0), node::pointer_type(2, 0))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(1, 0), node::pointer_type(2, 0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(1, 0, node::pointer_type(false), node::pointer_type(2, 0))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1, 0, node::pointer_type(false), node::pointer_type(2, 0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(2, 0, node::pointer_type(3, 0), node::pointer_type(3, 1))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2, 0, node::pointer_type(3, 0), node::pointer_type(3, 1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(3, 0, node::pointer_type(false), node::pointer_type(true))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(3, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(3, 1, node::pointer_type(true), node::pointer_type(false))));
        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(3, 1, node::pointer_type(true), node::pointer_type(false))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("can peek the same node twice in larger BDD", [&]() {
        AssertThat(large_untransposed.semi_transposed, Is().False());

        narc_ifstream ns(large_untransposed);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(1, 0), node::pointer_type(2, 0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.peek(),
                   Is().EqualTo(node(1, 0, node::pointer_type(false), node::pointer_type(2, 0))));
        AssertThat(ns.peek(),
                   Is().EqualTo(node(1, 0, node::pointer_type(false), node::pointer_type(2, 0))));

        AssertThat(ns.pull(),
                   Is().EqualTo(node(1, 0, node::pointer_type(false), node::pointer_type(2, 0))));

        AssertThat(ns.can_pull(), Is().True());
      });

      it("can pull larger BDD [internals need sorting]", []() {
        levelized_file<arc> af;
        /*
        //       1     ---- x0
        //      / \
        //      | 2    ---- x1
        //      |/ \
        //      3  T   ---- x2
        //     / \
        //     F T
        */

        {
          arc_ofstream aw(af);

          aw.push(level_info(0, 1));

          aw.push_internal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(1, 0)));
          aw.push_terminal(arc(arc::pointer_type(1, 0), true, arc::pointer_type(true)));

          aw.push(level_info(1, 1));

          aw.push_internal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(2, 0)));
          aw.push_internal(arc(arc::pointer_type(1, 0), false, arc::pointer_type(2, 0)));
          aw.push_terminal(arc(arc::pointer_type(2, 0), false, arc::pointer_type(false)));
          aw.push_terminal(arc(arc::pointer_type(2, 0), true, arc::pointer_type(true)));

          aw.push(level_info(2, 1));

          aw.close();
        }

        AssertThat(af.semi_transposed, Is().True());

        narc_ifstream ns(af);

        AssertThat(af.semi_transposed, Is().False());

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(2, 0), node::pointer_type(1, 0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1, 0, node::pointer_type(2, 0), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().False());
      });

      levelized_file<arc> large_untransposed2;
      /*
      //         1       ---- x0
      //        / \
      //       /  2      ---- x1
      //       | / \
      //       3 F 4     ---- x2
      //      / \ / \
      //      F  5  T    ---- x3
      //        / \
      //        F T
      */

      {
        arc_ofstream aw(large_untransposed2);

        aw.push(level_info(0, 1));

        aw.push_internal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(2, 0)));
        aw.push_internal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(1, 0)));

        aw.push(level_info(1, 1));

        aw.push_terminal(arc(arc::pointer_type(1, 0), false, arc::pointer_type(false)));
        aw.push_internal(arc(arc::pointer_type(1, 0), true, arc::pointer_type(2, 1)));

        aw.push(level_info(2, 2));

        aw.push_terminal(arc(arc::pointer_type(2, 0), false, arc::pointer_type(false)));
        aw.push_internal(arc(arc::pointer_type(2, 0), true, arc::pointer_type(3, 0)));

        aw.push_internal(arc(arc::pointer_type(2, 1), false, arc::pointer_type(3, 0)));
        aw.push_terminal(arc(arc::pointer_type(2, 1), true, arc::pointer_type(true)));

        aw.push(level_info(3, 1));

        aw.push_terminal(arc(arc::pointer_type(3, 0), false, arc::pointer_type(false)));
        aw.push_terminal(arc(arc::pointer_type(3, 0), true, arc::pointer_type(true)));

        aw.close();

        large_untransposed2.semi_transposed = false;
      }

      it("can pull larger BDD [reverse]", [&]() {
        AssertThat(large_untransposed2.semi_transposed, Is().False());

        narc_ifstream<true> ns(large_untransposed2);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(3, 0, node::pointer_type(false), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2, 1, node::pointer_type(3, 0), node::pointer_type(true))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2, 0, node::pointer_type(false), node::pointer_type(3, 0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1, 0, node::pointer_type(false), node::pointer_type(2, 1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(2, 0), node::pointer_type(1, 0))));

        AssertThat(ns.can_pull(), Is().False());
      });

      /*
      it("can pull larger BDD [negated + reverse]", [&]() {
        AssertThat(large_untransposed2.semi_transposed, Is().False());

        narc_ifstream<true> ns(large_untransposed2, true);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(3, 0, node::pointer_type(true), node::pointer_type(false))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2, 1, node::pointer_type(3, 0), node::pointer_type(false))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(2, 0, node::pointer_type(true), node::pointer_type(3, 0))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(1, 0, node::pointer_type(true), node::pointer_type(2, 1))));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(),
                   Is().EqualTo(node(0, 0, node::pointer_type(2, 0), node::pointer_type(1, 0))));

        AssertThat(ns.can_pull(), Is().False());
      });
      */
    });

    describe("arc_ofstream + narc_raccess", []() {
      levelized_file<arc> afA;
      /*
      //            1          ---- afA
      //           / \
      //           F T
      */
      {
        arc_ofstream aw(afA);
        aw.push_terminal(arc(arc::pointer_type(1, 0), false, arc::pointer_type(false)));
        aw.push_terminal(arc(arc::pointer_type(1, 0), true, arc::pointer_type(true)));

        aw.push(level_info(1, 1));
        aw.close();
      }

      const node A_n1(1, 0, node::pointer_type(false), node::pointer_type(true));

      levelized_file<arc> afB;
      /*
      //              _1_        ---- x0
      //             /   \
      //            _2_   \      ---- afA
      //           /   \   \
      //           3    4  5     ---- x2
      //          / \  / \/ \
      //          T 6  F 7  T    ---- x4
      //           / \  / \
      //           T F  F T
      //
      // The BDD will be constructed semi-transposed, i.e. the edge from 1 ---> 5 is "late"
      */
      {
        arc_ofstream aw(afB);

        aw.push_internal(arc(arc::pointer_type(0, 0), false, arc::pointer_type(1, 0)));
        aw.push_internal(arc(arc::pointer_type(1, 0), false, arc::pointer_type(2, 0)));
        aw.push_internal(arc(arc::pointer_type(1, 0), true, arc::pointer_type(2, 1)));
        aw.push_internal(arc(arc::pointer_type(0, 0), true, arc::pointer_type(2, 2)));
        aw.push_internal(arc(arc::pointer_type(2, 0), true, arc::pointer_type(4, 0)));
        aw.push_internal(arc(arc::pointer_type(2, 1), true, arc::pointer_type(4, 1)));
        aw.push_internal(arc(arc::pointer_type(2, 2), false, arc::pointer_type(4, 1)));

        aw.push_terminal(arc(arc::pointer_type(2, 0), false, arc::pointer_type(true)));
        aw.push_terminal(arc(arc::pointer_type(2, 1), false, arc::pointer_type(false)));
        aw.push_terminal(arc(arc::pointer_type(2, 2), true, arc::pointer_type(true)));
        aw.push_terminal(arc(arc::pointer_type(4, 0), false, arc::pointer_type(true)));
        aw.push_terminal(arc(arc::pointer_type(4, 0), true, arc::pointer_type(false)));
        aw.push_terminal(arc(arc::pointer_type(4, 1), false, arc::pointer_type(false)));
        aw.push_terminal(arc(arc::pointer_type(4, 1), true, arc::pointer_type(true)));

        aw.push(level_info(0, 1));
        aw.push(level_info(1, 1));
        aw.push(level_info(2, 3));
        aw.push(level_info(4, 2));

        aw.close();
      }

      const node B_n7(4, 1, node::pointer_type(false), node::pointer_type(true));
      const node B_n6(4, 0, node::pointer_type(true), node::pointer_type(false));
      const node B_n5(2, 2, B_n7.uid(), node::pointer_type(true));
      const node B_n4(2, 1, node::pointer_type(false), B_n7.uid());
      const node B_n3(2, 0, node::pointer_type(true), B_n6.uid());
      const node B_n2(1, 0, B_n3.uid(), B_n4.uid());
      const node B_n1(0, 0, B_n2.uid(), B_n5.uid());

      describe(".setup_next_level(...) + .[has_]current_level() + ", [&]() {
        it("has empty levels before root [A]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afA);
          narc_raccess nara(in);

          AssertThat(nara.has_current_level(), Is().False());

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(1));

          nara.setup_next_level(0u);

          AssertThat(nara.has_current_level(), Is().True());
          AssertThat(nara.current_level(), Is().EqualTo(0));

          AssertThat(nara.empty_level(), Is().True());

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(1));
        });

        it("has empty levels after having skipped content [A]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afA);
          narc_raccess nara(in);

          AssertThat(nara.has_current_level(), Is().False());

          AssertThat(nara.empty_level(), Is().True());

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(1));

          nara.setup_next_level(2u);

          AssertThat(nara.has_current_level(), Is().True());
          AssertThat(nara.current_level(), Is().EqualTo(2));

          AssertThat(nara.empty_level(), Is().True());

          AssertThat(nara.has_next_level(), Is().False());
        });

        it("sets up the first default 'next' level to be the root [A]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afA);
          narc_raccess nara(in);

          AssertThat(nara.has_current_level(), Is().False());

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(1));

          nara.setup_next_level();
          AssertThat(nara.has_current_level(), Is().True());
          AssertThat(nara.current_level(), Is().EqualTo(1));

          AssertThat(nara.empty_level(), Is().False());
          AssertThat(nara.current_width(), Is().EqualTo(1u));
        });

        it("sets up consecutive default 'next' level to be the non-empty levels [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          AssertThat(nara.has_current_level(), Is().False());

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(0));

          nara.setup_next_level();

          AssertThat(nara.has_current_level(), Is().True());
          AssertThat(nara.current_level(), Is().EqualTo(0));

          AssertThat(nara.empty_level(), Is().False());
          AssertThat(nara.current_width(), Is().EqualTo(1u));

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(1));

          nara.setup_next_level();

          AssertThat(nara.has_current_level(), Is().True());
          AssertThat(nara.current_level(), Is().EqualTo(1));

          AssertThat(nara.empty_level(), Is().False());
          AssertThat(nara.current_width(), Is().EqualTo(1u));

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(2));

          nara.setup_next_level();

          AssertThat(nara.has_current_level(), Is().True());
          AssertThat(nara.current_level(), Is().EqualTo(2));

          AssertThat(nara.empty_level(), Is().False());
          AssertThat(nara.current_width(), Is().EqualTo(3u));

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(4));

          nara.setup_next_level();

          AssertThat(nara.has_current_level(), Is().True());
          AssertThat(nara.current_level(), Is().EqualTo(4));

          AssertThat(nara.empty_level(), Is().False());
          AssertThat(nara.current_width(), Is().EqualTo(2u));

          AssertThat(nara.has_next_level(), Is().False());
        });

        it("can go to empty level in-between non-empty ones [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nra(in);
          nra.setup_next_level(3u);

          AssertThat(nra.has_current_level(), Is().True());
          AssertThat(nra.current_level(), Is().EqualTo(3));

          AssertThat(nra.empty_level(), Is().True());

          AssertThat(nra.has_next_level(), Is().True());
          AssertThat(nra.next_level(), Is().EqualTo(4));
        });

        it("can go from empty level to the next non-empty one [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nra(in);
          nra.setup_next_level(3u);

          AssertThat(nra.has_next_level(), Is().True());
          AssertThat(nra.next_level(), Is().EqualTo(4));

          nra.setup_next_level();

          AssertThat(nra.has_current_level(), Is().True());
          AssertThat(nra.current_level(), Is().EqualTo(4));

          AssertThat(nra.empty_level(), Is().False());
        });
      });

      describe(".at(...)", [&]() {
        it("provides random access to root level [idx] [A]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afA);
          narc_raccess nara(in);

          nara.setup_next_level();
          AssertThat(nara.at(0u), Is().EqualTo(A_n1));
        });

        it("provides random access to root level [uid] [A]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afA);
          narc_raccess nara(in);

          nara.setup_next_level();
          AssertThat(nara.at(A_n1.uid()), Is().EqualTo(A_n1));
        });

        it("provides random access to root [idx] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level();
          AssertThat(nara.at(0u), Is().EqualTo(B_n1));
        });

        it("provides random access to root [uid] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level();
          AssertThat(nara.at(B_n1.uid()), Is().EqualTo(B_n1));
        });

        it("provides random access to non-root single-node level [idx] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(1u);
          AssertThat(nara.at(0u), Is().EqualTo(B_n2));
        });

        it("provides random access to non-root single-node level [uid] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(1u);
          AssertThat(nara.at(B_n2.uid()), Is().EqualTo(B_n2));
        });

        it("provides in-order access to multi-node level in order [idx] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(0u), Is().EqualTo(B_n3));
          AssertThat(nara.at(1u), Is().EqualTo(B_n4));
          AssertThat(nara.at(2u), Is().EqualTo(B_n5));
        });

        it("provides in-order access to multi-node level [uid] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(B_n3.uid()), Is().EqualTo(B_n3));
          AssertThat(nara.at(B_n4.uid()), Is().EqualTo(B_n4));
          AssertThat(nara.at(B_n5.uid()), Is().EqualTo(B_n5));
        });

        it("allows skipping over nodes on multi-node level [idx] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(0u), Is().EqualTo(B_n3));
          AssertThat(nara.at(2u), Is().EqualTo(B_n5));
        });

        it("provides in-order random access to multi-node level [uid] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(B_n4.uid()), Is().EqualTo(B_n4));
        });

        it("provides out-of-order random access to multi-node level [idx] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(2u), Is().EqualTo(B_n5));
          AssertThat(nara.at(0u), Is().EqualTo(B_n3));
          AssertThat(nara.at(1u), Is().EqualTo(B_n4));
        });

        it("provides out-of-order random access to multi-node level [uid] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(B_n5.uid()), Is().EqualTo(B_n5));
          AssertThat(nara.at(B_n3.uid()), Is().EqualTo(B_n3));
          AssertThat(nara.at(B_n4.uid()), Is().EqualTo(B_n4));
        });

        it("provides recurring out-of-order random access to multi-node level [idx] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(0u), Is().EqualTo(B_n3));
          AssertThat(nara.at(2u), Is().EqualTo(B_n5));
          AssertThat(nara.at(1u), Is().EqualTo(B_n4));
          AssertThat(nara.at(0u), Is().EqualTo(B_n3));
          AssertThat(nara.at(2u), Is().EqualTo(B_n5));
          AssertThat(nara.at(0u), Is().EqualTo(B_n3));
          AssertThat(nara.at(1u), Is().EqualTo(B_n4));
        });

        it("provides recurring out-of-order random access to multi-node level [uid] [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(B_n3.uid()), Is().EqualTo(B_n3));
          AssertThat(nara.at(B_n5.uid()), Is().EqualTo(B_n5));
          AssertThat(nara.at(B_n4.uid()), Is().EqualTo(B_n4));
          AssertThat(nara.at(B_n3.uid()), Is().EqualTo(B_n3));
          AssertThat(nara.at(B_n5.uid()), Is().EqualTo(B_n5));
          AssertThat(nara.at(B_n3.uid()), Is().EqualTo(B_n3));
          AssertThat(nara.at(B_n4.uid()), Is().EqualTo(B_n4));
        });

        it("provides access after having gone by default from an empty level [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(3u);

          AssertThat(nara.has_next_level(), Is().True());
          AssertThat(nara.next_level(), Is().EqualTo(4));

          nara.setup_next_level();

          AssertThat(nara.has_current_level(), Is().True());
          AssertThat(nara.current_level(), Is().EqualTo(4));

          AssertThat(nara.empty_level(), Is().False());

          AssertThat(nara.at(B_n7.uid()), Is().EqualTo(B_n7));
          AssertThat(nara.at(B_n6.uid()), Is().EqualTo(B_n6));
          AssertThat(nara.at(B_n6.uid()), Is().EqualTo(B_n6));
          AssertThat(nara.at(B_n7.uid()), Is().EqualTo(B_n7));
        });
      });

      describe(".root()", [&]() {
        it("provides root before accessing anything [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          AssertThat(nara.root(), Is().EqualTo(arc::pointer_type(0, 0)));
        });

        it("provides root after accessing node below it [B]", [&]() {
          levelized_file<arc> in = levelized_file<arc>::copy(afB);
          narc_raccess nara(in);

          nara.setup_next_level(2u);
          AssertThat(nara.at(B_n4.uid()), Is().EqualTo(B_n4));

          AssertThat(nara.root(), Is().EqualTo(B_n1.uid()));
        });
      });
    });
  });
});
