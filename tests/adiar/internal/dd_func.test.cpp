#include "../../test.h"
#include <iterator>

#include <adiar/internal/dd_func.h>

go_bandit([]() {
  describe("adiar/internal/dd_func.h", []() {
    const ptr_uint64 false_ptr = ptr_uint64(false);
    const ptr_uint64 true_ptr  = ptr_uint64(true);

    shared_levelized_file<node> terminal_F;
    {
      node_ofstream writer(terminal_F);
      writer << node(false);
    }

    shared_levelized_file<node> terminal_T;
    {
      node_ofstream writer(terminal_T);
      writer << node(true);
    }

    shared_levelized_file<node> x0;
    {
      node_ofstream writer(x0);
      writer << node(0, node::max_id, false_ptr, true_ptr);
    }

    shared_levelized_file<node> x42;
    {
      node_ofstream writer(x42);
      writer << node(42, node::max_id, false_ptr, true_ptr);
    }

    shared_levelized_file<bdd::node_type> bdd_file;
    {
      node n3 = node(4, node::max_id, true_ptr, false_ptr);
      node n2 = node(3, node::max_id, n3.uid(), false_ptr);
      node n1 = node(1, node::max_id, n3.uid(), n2.uid());
      node_ofstream writer(bdd_file);
      writer << n3 << n2 << n1;
    }

    shared_levelized_file<zdd::node_type> zdd_file;
    {
      node n4 = node(2, node::max_id, true_ptr, true_ptr);
      node n3 = node(2, node::max_id - 1, false_ptr, true_ptr);
      node n2 = node(1, node::max_id, n4.uid(), n4.uid());
      node n1 = node(0, node::max_id, n3.uid(), n4.uid());
      node_ofstream writer(zdd_file);
      writer << n4 << n3 << n2 << n1;
    }

    describe("dd_support(f, cb)", [&]() {
      it("returns [1,3,4] for a BDD with multiple nodes", [&]() {
        std::vector<int> out;

        bdd_support(bdd_file, [&out](auto x) { out.push_back(x); });

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(3u));
        AssertThat(out.at(0), Is().EqualTo(1));
        AssertThat(out.at(1), Is().EqualTo(3));
        AssertThat(out.at(2), Is().EqualTo(4));
      });

      it("returns [0,1,2] for a ZDD with multiple nodes", [&]() {
        std::vector<int> out;

        zdd_support(zdd_file, [&out](auto x) { out.push_back(x); });

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(3u));
        AssertThat(out.at(0), Is().EqualTo(0));
        AssertThat(out.at(1), Is().EqualTo(1));
        AssertThat(out.at(2), Is().EqualTo(2));
      });
    });

    describe("dd_support(f, begin, end)", [&]() {
      it("does not write anything for BDD false terminal", [&]() {
        std::vector<int> out;

        auto iter = bdd_support(terminal_F, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(0u));

        // Check state of 'iter'
        iter = 42;
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(42));
      });

      it("does not write anything for ZDD true terminal", [&]() {
        std::vector<int> out;

        auto iter = zdd_support(terminal_T, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(0u));

        // Check state of 'iter'
        iter = 8;
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(8));
      });

      it("returns [0] for a ZDD with one node (label 0)", [&]() {
        std::vector<int> out;

        auto iter = zdd_support(x0, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(0));

        // Check state of 'iter'
        iter = 8;
        AssertThat(out.size(), Is().EqualTo(2u));
        AssertThat(out.at(1), Is().EqualTo(8));
      });

      it("returns [42] for a ZDD with one node (label 42)", [&]() {
        std::vector<int> out;

        auto iter = zdd_support(x42, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(1u));
        AssertThat(out.at(0), Is().EqualTo(42));

        // Check state of 'iter'
        iter = 8;
        AssertThat(out.size(), Is().EqualTo(2u));
        AssertThat(out.at(1), Is().EqualTo(8));
      });

      it("returns [1,3,4] for a BDD with multiple nodes", [&]() {
        std::vector<int> out;

        auto iter = bdd_support(bdd_file, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(3u));
        AssertThat(out.at(0), Is().EqualTo(1));
        AssertThat(out.at(1), Is().EqualTo(3));
        AssertThat(out.at(2), Is().EqualTo(4));

        // Check state of 'iter'
        iter = 8;
        AssertThat(out.size(), Is().EqualTo(4u));
        AssertThat(out.at(3), Is().EqualTo(8));
      });

      it("returns [0,1,2] for a ZDD with multiple nodes", [&]() {
        std::vector<int> out;

        auto iter = zdd_support(zdd_file, std::back_inserter(out));

        // Check state of 'out'
        AssertThat(out.size(), Is().EqualTo(3u));
        AssertThat(out.at(0), Is().EqualTo(0));
        AssertThat(out.at(1), Is().EqualTo(1));
        AssertThat(out.at(2), Is().EqualTo(2));

        // Check state of 'iter'
        iter = 8;
        AssertThat(out.size(), Is().EqualTo(4u));
        AssertThat(out.at(3), Is().EqualTo(8));
      });
    });

    describe("canonicity, i.e. sorted & indexable", [&]() {
      // TODO
    });

    describe("terminal predicates [dd_...]", [&]() {
      describe("dd::is_terminal()", [&]() {
        // TODO
      });

      describe("dd_isterminal(...)", [&]() {
        it("rejects x0 as a BDD terminal file", [&]() {
          AssertThat(bdd_isconst(x0), Is().False());
          AssertThat(bdd_isterminal(x0), Is().False());
        });

        it("rejects x0 as a ZDD terminal file",
           [&]() { AssertThat(zdd_isterminal(x0), Is().False()); });

        it("rejects larger BDD as being a terminal", [&]() {
          AssertThat(bdd_isconst(bdd_file), Is().False());
          AssertThat(bdd_isterminal(bdd_file), Is().False());
        });

        it("rejects larger ZDD as being a terminal",
           [&]() { AssertThat(zdd_isterminal(zdd_file), Is().False()); });

        it("accepts a BDD true terminal", [&]() {
          AssertThat(bdd_isconst(terminal_T), Is().True());
          AssertThat(bdd_isterminal(terminal_T), Is().True());
        });

        it("accepts a ZDD true terminal",
           [&]() { AssertThat(zdd_isterminal(terminal_T), Is().True()); });

        it("accepts a BDD false terminal", [&]() {
          AssertThat(bdd_isconst(terminal_F), Is().True());
          AssertThat(bdd_isterminal(terminal_F), Is().True());
        });

        it("accepts a ZDD false terminal",
           [&]() { AssertThat(zdd_isterminal(terminal_F), Is().True()); });
      });

      describe("dd::value()", [&]() {
        // TODO
      });

      describe("dd_valueof(...)", [&]() {
        it("extracts from a true BDD terminal",
           [&]() { AssertThat(dd_valueof(bdd(terminal_T)), Is().True()); });

        it("extracts from a negation of a true BDD terminal",
           [&]() { AssertThat(dd_valueof(~bdd(terminal_T)), Is().False()); });

        it("extracts from a true ZDD terminal",
           [&]() { AssertThat(dd_valueof(zdd(terminal_T)), Is().True()); });

        it("extracts from a false BDD terminal",
           [&]() { AssertThat(dd_valueof(bdd(terminal_F)), Is().False()); });

        it("extracts from a negation of a false BDD terminal",
           [&]() { AssertThat(dd_valueof(~bdd(terminal_F)), Is().True()); });

        it("extracts from a false ZDD terminal",
           [&]() { AssertThat(dd_valueof(zdd(terminal_F)), Is().False()); });
      });

      describe("dd::is_false()", [&]() {
        // TODO
      });

      describe("bdd_isfalse / zdd_isfalse / zdd_isempty", [&]() {
        it("rejects x0 [BDD]", [&]() { AssertThat(bdd_isfalse(x0), Is().False()); });

        it("rejects x0 [ZDD]", [&]() {
          AssertThat(zdd_isfalse(x0), Is().False());
          AssertThat(zdd_isempty(x0), Is().False());
        });

        it("rejects x42 [BDD]", [&]() { AssertThat(bdd_isfalse(x42), Is().False()); });

        it("rejects x42 [ZDD]", [&]() {
          AssertThat(zdd_isfalse(x42), Is().False());
          AssertThat(zdd_isempty(x42), Is().False());
        });

        it("rejects larger decision diagram [BDD]",
           [&]() { AssertThat(bdd_isfalse(bdd_file), Is().False()); });

        it("rejects larger decision diagram [ZDD]", [&]() {
          AssertThat(zdd_isfalse(zdd_file), Is().False());
          AssertThat(zdd_isempty(zdd_file), Is().False());
        });

        it("rejects true terminal [BDD]",
           [&]() { AssertThat(bdd_isfalse(terminal_T), Is().False()); });

        it("rejects true terminal [ZDD]", [&]() {
          AssertThat(zdd_isfalse(terminal_T), Is().False());
          AssertThat(zdd_isempty(terminal_T), Is().False());
        });

        it("accepts false terminal [BDD]",
           [&]() { AssertThat(bdd_isfalse(terminal_F), Is().True()); });

        it("accepts false terminal [ZDD]", [&]() {
          AssertThat(zdd_isfalse(terminal_F), Is().True());
          AssertThat(zdd_isempty(terminal_F), Is().True());
        });
      });

      describe("dd::is_true()", [&]() {
        // TODO
      });

      describe("bdd_istrue / zdd_istrue / zdd_isnull", [&]() {
        it("rejects x0 [BDD]", [&]() { AssertThat(bdd_istrue(x0), Is().False()); });

        it("rejects x0 [ZDD]", [&]() {
          AssertThat(zdd_istrue(x0), Is().False());
          AssertThat(zdd_isnull(x0), Is().False());
        });

        it("rejects x42 [BDD]", [&]() { AssertThat(bdd_istrue(x42), Is().False()); });

        it("rejects x42 [ZDD]", [&]() {
          AssertThat(zdd_istrue(x42), Is().False());
          AssertThat(zdd_isnull(x42), Is().False());
        });

        it("rejects larger decision diagram [BDD]",
           [&]() { AssertThat(bdd_istrue(bdd_file), Is().False()); });

        it("rejects larger decision diagram [ZDD]", [&]() {
          AssertThat(zdd_istrue(zdd_file), Is().False());
          AssertThat(zdd_isnull(zdd_file), Is().False());
        });

        it("rejects false terminal [BDD]",
           [&]() { AssertThat(bdd_istrue(terminal_F), Is().False()); });

        it("rejects false terminal [ZDD]", [&]() {
          AssertThat(zdd_istrue(terminal_F), Is().False());
          AssertThat(zdd_isnull(terminal_F), Is().False());
        });

        it("accepts a true terminal [BDD]",
           [&]() { AssertThat(bdd_istrue(terminal_T), Is().True()); });

        it("accepts a true terminal [ZDD]", [&]() {
          AssertThat(zdd_istrue(terminal_T), Is().True());
          AssertThat(zdd_isnull(terminal_T), Is().True());
        });
      });
    });

    describe("bdd_topvar / zdd_topvar", [&]() {
      // TODO: When 'minvar != topvar', move these tests out

      it("returns 0 for x0 BDD", [&]() {
        AssertThat(bdd_topvar(x0), Is().EqualTo(0u));
        AssertThat(bdd_minvar(x0), Is().EqualTo(0u));
      });

      it("returns 0 for x0 ZDD", [&]() {
        AssertThat(zdd_topvar(x0), Is().EqualTo(0u));
        AssertThat(zdd_minvar(x0), Is().EqualTo(0u));
      });

      it("returns 42 for x42 BDD", [&]() {
        AssertThat(bdd_topvar(x42), Is().EqualTo(42u));
        AssertThat(bdd_minvar(x42), Is().EqualTo(42u));
      });

      it("returns 42 for x42 ZDD", [&]() {
        AssertThat(zdd_topvar(x42), Is().EqualTo(42u));
        AssertThat(zdd_minvar(x42), Is().EqualTo(42u));
      });

      it("returns root variable for larger BDD", [&]() {
        AssertThat(bdd_topvar(bdd_file), Is().EqualTo(1u));
        AssertThat(bdd_minvar(bdd_file), Is().EqualTo(1u));
      });

      it("returns root variable for larger ZDD", [&]() {
        AssertThat(zdd_topvar(zdd_file), Is().EqualTo(0u));
        AssertThat(zdd_minvar(zdd_file), Is().EqualTo(0u));
      });

      it("throws an exception for F terminal [BDD]", [&]() {
        AssertThrows(invalid_argument, bdd_topvar(terminal_F));
        AssertThrows(invalid_argument, bdd_minvar(terminal_F));
      });

      it("throws an exception for T terminal [BDD]", [&]() {
        AssertThrows(invalid_argument, bdd_topvar(terminal_T));
        AssertThrows(invalid_argument, bdd_minvar(terminal_T));
      });

      it("throws an exception for F terminal [ZDD]", [&]() {
        AssertThrows(invalid_argument, zdd_topvar(terminal_F));
        AssertThrows(invalid_argument, zdd_minvar(terminal_F));
      });

      it("throws an exception for T terminal [ZDD]", [&]() {
        AssertThrows(invalid_argument, zdd_topvar(terminal_T));
        AssertThrows(invalid_argument, zdd_minvar(terminal_T));
      });

      it("returns shifted root variable for larger BDD [+1]", [&]() {
        AssertThat(bdd_topvar(bdd(bdd_file, false, +1)), Is().EqualTo(2u));
        AssertThat(bdd_minvar(bdd(bdd_file, false, +1)), Is().EqualTo(2u));
      });

      it("returns shifted root variable for larger BDD [+2]", [&]() {
        AssertThat(bdd_topvar(bdd(bdd_file, false, +2)), Is().EqualTo(3u));
        AssertThat(bdd_minvar(bdd(bdd_file, false, +2)), Is().EqualTo(3u));
      });

      it("returns shifted root variable for larger BDD [-1]", [&]() {
        AssertThat(bdd_topvar(bdd(bdd_file, false, -1)), Is().EqualTo(0u));
        AssertThat(bdd_minvar(bdd(bdd_file, false, -1)), Is().EqualTo(0u));
      });
    });

    describe("bdd_maxvar / zdd_maxvar", [&]() {
      it("returns 0 for x0 BDD", [&]() { AssertThat(bdd_maxvar(x0), Is().EqualTo(0u)); });

      it("returns 0 for x0 ZDD", [&]() { AssertThat(zdd_maxvar(x0), Is().EqualTo(0u)); });

      it("returns 42 for x42 BDD", [&]() { AssertThat(bdd_maxvar(x42), Is().EqualTo(42u)); });

      it("returns 42 for x42 ZDD", [&]() { AssertThat(zdd_maxvar(x42), Is().EqualTo(42u)); });

      it("returns root variable for larger BDD",
         [&]() { AssertThat(bdd_maxvar(bdd_file), Is().EqualTo(4u)); });

      it("returns root variable for larger ZDD",
         [&]() { AssertThat(zdd_maxvar(zdd_file), Is().EqualTo(2u)); });

      it("throws an exception for F terminal [BDD]",
         [&]() { AssertThrows(invalid_argument, bdd_maxvar(terminal_F)); });

      it("throws an exception for T terminal [BDD]",
         [&]() { AssertThrows(invalid_argument, bdd_maxvar(terminal_T)); });

      it("throws an exception for F terminal [ZDD]",
         [&]() { AssertThrows(invalid_argument, zdd_maxvar(terminal_F)); });

      it("throws an exception for T terminal [ZDD]",
         [&]() { AssertThrows(invalid_argument, zdd_maxvar(terminal_T)); });

      it("returns shifted root variable for larger BDD [+1]",
         [&]() { AssertThat(bdd_maxvar(bdd(bdd_file, false, +1)), Is().EqualTo(5u)); });

      it("returns shifted root variable for larger BDD [+2]",
         [&]() { AssertThat(bdd_maxvar(bdd(bdd_file, false, +2)), Is().EqualTo(6u)); });

      it("returns shifted root variable for larger BDD [-1]",
         [&]() { AssertThat(bdd_maxvar(bdd(bdd_file, false, -1)), Is().EqualTo(3u)); });
    });
  });
});
