#include "../../../test.h"
#include <fstream>

#include <adiar/internal/algorithms/dot.h>

go_bandit([]() {
  describe("adiar/dot.h", [&]() {
    it("can output .dot for a terminal-only BDD", [&]() {
      bdd terminal_T = bdd_terminal(true);

      bdd_printdot(terminal_T, "dot_test_terminal_T.dot");
      int exit_value = system("dot -O -Tpng dot_test_terminal_T.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot for a BDD with internal nodes (excl. their ids)", [&]() {
      shared_levelized_file<bdd::node_type> reduced_bdd;

      { // Garbage collect writer early
        node_ofstream rw(reduced_bdd);

        rw << node(42, 1, ptr_uint64(false), ptr_uint64(true))
           << node(42, 0, ptr_uint64(true), ptr_uint64(false))
           << node(1, 2, ptr_uint64(42, 0), ptr_uint64(42, 1))
           << node(0, 1, ptr_uint64(1, 2), ptr_uint64(false));
      }

      bdd_printdot(reduced_bdd, "dot_test_bdd__without_ids.dot" /*, false*/);
      int exit_value = system("dot -O -Tpng dot_test_bdd__without_ids.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot for a BDD with internal nodes (incl. their ids)", [&]() {
      shared_levelized_file<bdd::node_type> reduced_bdd;

      { // Garbage collect writer early
        node_ofstream rw(reduced_bdd);

        rw << node(1, 1, ptr_uint64(false), ptr_uint64(true))
           << node(1, 0, ptr_uint64(true), ptr_uint64(false))
           << node(0, 0, ptr_uint64(1, 0), ptr_uint64(1, 1));
      }

      bdd_printdot(reduced_bdd, "dot_test_bdd__with_ids.dot", true);
      int exit_value = system("dot -O -Tpng dot_test_bdd__with_ids.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot of a BDD into an output stream", [&]() {
      bdd f = bdd_terminal(false);

      std::ofstream out;
      out.open("dot_test_bdd_ostream.dot");

      bdd_printdot(f, out);

      out.close();

      int exit_value = system("dot -O -Tpng dot_test_bdd_ostream.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    ////////////////////////////////////////////////////////////////////////////
    it("can output .dot for a terminal-only ZDD", [&]() {
      zdd terminal_F = zdd_empty();

      zdd_printdot(terminal_F, "dot_test_terminal_F.dot");
      int exit_value = system("dot -O -Tpng dot_test_terminal_F.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot for a ZDD with internal nodes", [&]() {
      shared_levelized_file<zdd::node_type> reduced_zdd;

      { // Garbage collect writer early
        node_ofstream rw(reduced_zdd);

        rw << node(42, 0, ptr_uint64(false), ptr_uint64(true))
           << node(1, 2, ptr_uint64(42, 0), ptr_uint64(42, 0))
           << node(0, 1, ptr_uint64(1, 2), ptr_uint64(true));
      }

      zdd_printdot(reduced_zdd, "dot_test_zdd.dot");
      int exit_value = system("dot -O -Tpng dot_test_zdd.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    it("can output .dot of a ZDD into an output stream", [&]() {
      zdd A = zdd_null();

      std::ofstream out;
      out.open("dot_test_zdd_ostream.dot");

      zdd_printdot(A, out);

      out.close();

      int exit_value = system("dot -O -Tpng dot_test_zdd_ostream.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });

    ////////////////////////////////////////////////////////////////////////////
    it("can output an unreduced diagram [internal API]", [&]() {
      shared_levelized_file<arc> unreduced_bdd;

      { // Garbage collect writer early
        arc_ofstream uw(unreduced_bdd);

        uw.push_terminal({ flag(ptr_uint64(2, 0)), ptr_uint64(true) });
        uw.push_terminal({ ptr_uint64(2, 0), ptr_uint64(true) });
        uw.push_terminal({ ptr_uint64(1, 1), ptr_uint64(true) });

        uw.push_internal({ ptr_uint64(0, 0), ptr_uint64(1, 0) });
        uw.push_internal({ ptr_uint64(0, 0), ptr_uint64(1, 1) });
        uw.push_internal({ ptr_uint64(1, 0), ptr_uint64(2, 0) });
        uw.push_internal({ flag(ptr_uint64(1, 0)), ptr_uint64(2, 0) });
        uw.push_internal({ flag(ptr_uint64(1, 1)), ptr_uint64(2, 0) });
      }

      print_dot(unreduced_bdd, "dot_test_unreduced.dot");
      int exit_value = system("dot -O -Tpng dot_test_unreduced.dot");
      AssertThat(exit_value, Is().EqualTo(0));
    });
  });
});
