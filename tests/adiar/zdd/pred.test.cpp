#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/pred.cpp", [&]() {
    const zdd::pointer_type terminal_T = zdd::pointer_type(true);
    const zdd::pointer_type terminal_F = zdd::pointer_type(false);

    // Ø
    shared_levelized_file<zdd::node_type> zdd_empty_nf;
    {
      node_ofstream nw(zdd_empty_nf);
      nw << node(false);
    }

    auto zdd_empty_nf_copy = shared_levelized_file<zdd::node_type>::copy(zdd_empty_nf);

    // { Ø }
    shared_levelized_file<zdd::node_type> zdd_null_nf;
    {
      node_ofstream nw(zdd_null_nf);
      nw << node(true);
    }

    auto zdd_null_nf_copy = shared_levelized_file<zdd::node_type>::copy(zdd_null_nf);

    // { {1} }
    shared_levelized_file<zdd::node_type> zdd_A_nf;
    {
      node_ofstream nw(zdd_A_nf);
      nw << node(1, zdd::max_id, terminal_F, terminal_T);
    }

    auto zdd_A_nf_copy = shared_levelized_file<zdd::node_type>::copy(zdd_A_nf);

    // { {1}, {1,2} }
    shared_levelized_file<zdd::node_type> zdd_B_nf;
    {
      node_ofstream nw(zdd_B_nf);
      nw << node(2, zdd::max_id, terminal_T, terminal_T)
         << node(1, zdd::max_id, terminal_F, zdd::pointer_type(2, zdd::max_id));
    }

    // { Ø, {1}, {2}, {1,2} }
    shared_levelized_file<zdd::node_type> zdd_C_nf;
    {
      node_ofstream nw(zdd_C_nf);
      nw << node(2, zdd::max_id, terminal_T, terminal_T)
         << node(1,
                 zdd::max_id,
                 zdd::pointer_type(2, zdd::max_id),
                 zdd::pointer_type(2, zdd::max_id));
    }

    auto zdd_C_nf_copy = shared_levelized_file<zdd::node_type>::copy(zdd_C_nf);

    // { {1}, {1,2}, {0,1,3} }
    shared_levelized_file<zdd::node_type> zdd_D_nf;
    {
      node_ofstream nw(zdd_D_nf);
      nw << node(3, zdd::max_id, terminal_F, terminal_T)
         << node(2, zdd::max_id, terminal_T, terminal_T)
         << node(1, zdd::max_id, terminal_F, zdd::pointer_type(3, zdd::max_id))
         << node(1, zdd::max_id - 1, terminal_F, zdd::pointer_type(2, zdd::max_id))
         << node(0,
                 zdd::max_id,
                 zdd::pointer_type(1, zdd::max_id - 1),
                 zdd::pointer_type(1, zdd::max_id));
    }

    auto zdd_D_nf_copy = shared_levelized_file<zdd::node_type>::copy(zdd_D_nf);

    // { Ø, {0,3} }
    shared_levelized_file<zdd::node_type> zdd_E_nf;
    {
      node_ofstream nw(zdd_E_nf);
      nw << node(3, zdd::max_id, terminal_F, terminal_T)
         << node(0, zdd::max_id, terminal_T, zdd::pointer_type(3, zdd::max_id));
    }

    auto zdd_E_nf_copy = shared_levelized_file<zdd::node_type>::copy(zdd_E_nf);

    // { {2} }
    shared_levelized_file<zdd::node_type> zdd_F_nf;
    {
      node_ofstream nw(zdd_F_nf);
      nw << node(2, zdd::max_id, terminal_F, terminal_T);
    }

    // { {0,2,4} }
    shared_levelized_file<zdd::node_type> zdd_G_nf;
    {
      node_ofstream nw(zdd_G_nf);
      nw << node(4, zdd::max_id, terminal_F, terminal_T)
         << node(2, zdd::max_id, terminal_F, zdd::pointer_type(4, zdd::max_id))
         << node(0, zdd::max_id, terminal_F, zdd::pointer_type(2, zdd::max_id));
    }

    // -------------------------------------------------------------------------
    // NOTE: For the overloads from <adiar/internal/dd_func.h>, please look in
    //       'test/adiar/internal/test_dd_func.cpp'.
    //
    // NOTE: For `zdd_equal` and `zdd_unequal` look at
    //       'test/adiar/internal/algorithms/test_pred.cpp'
    //
    // TODO: Move ZDD variants over here?
    // -------------------------------------------------------------------------

    describe("zdd_ispoint", [&]() {
      it("rejects Ø", [&]() { AssertThat(zdd_ispoint(zdd_empty_nf), Is().False()); });

      it("accepts { Ø }", [&]() { AssertThat(zdd_ispoint(zdd_null_nf), Is().True()); });

      it("accepts { {1} }", [&]() { AssertThat(zdd_ispoint(zdd_A_nf), Is().True()); });

      it("accepts { {2} }", [&]() { AssertThat(zdd_ispoint(zdd_E_nf), Is().False()); });

      it("rejects { {1}, {1,2} }", [&]() { AssertThat(zdd_ispoint(zdd_B_nf), Is().False()); });

      it("rejects { Ø, {1}, {2}, {1,2} }",
         [&]() { AssertThat(zdd_ispoint(zdd_C_nf), Is().False()); });

      it("rejects { {1}, {1,2}, {0,1,3} }",
         [&]() { AssertThat(zdd_ispoint(zdd_D_nf), Is().False()); });

      it("rejects { Ø, {0,3} }", [&]() { AssertThat(zdd_ispoint(zdd_D_nf), Is().False()); });

      it("accepts { {0,2,4} }", [&]() { AssertThat(zdd_ispoint(zdd_G_nf), Is().True()); });
    });

    describe("zdd_subseteq", [&]() {
      it("accepts same file", [&]() {
        AssertThat(zdd_subseteq(zdd_A_nf, zdd_A_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_B_nf, zdd_B_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_C_nf, zdd_C_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_D_nf, zdd_D_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_E_nf, zdd_E_nf), Is().True());
      });

      it("accepts on equal ZDDs", [&]() {
        AssertThat(zdd_subseteq(zdd_A_nf, zdd_A_nf_copy), Is().True());
        AssertThat(zdd_subseteq(zdd_C_nf, zdd_C_nf_copy), Is().True());
        AssertThat(zdd_subseteq(zdd_D_nf, zdd_D_nf_copy), Is().True());
        AssertThat(zdd_subseteq(zdd_E_nf, zdd_E_nf_copy), Is().True());
      });

      it("can compare Ø and { Ø }", [&]() {
        AssertThat(zdd_subseteq(zdd_empty_nf, zdd_null_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_null_nf, zdd_empty_nf), Is().False());
      });

      it("can compare { {1} } and { {2} }", [&]() {
        AssertThat(zdd_subseteq(zdd_A_nf, zdd_F_nf), Is().False());
        AssertThat(zdd_subseteq(zdd_F_nf, zdd_A_nf), Is().False());
      });

      it("can compare { {1} } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_subseteq(zdd_A_nf, zdd_B_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_B_nf, zdd_A_nf), Is().False());
      });

      it("can compare { Ø } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_subseteq(zdd_null_nf, zdd_B_nf), Is().False());
        AssertThat(zdd_subseteq(zdd_B_nf, zdd_null_nf), Is().False());
      });

      it("can compare { Ø, {0,3} } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_subseteq(zdd_E_nf, zdd_B_nf), Is().False());
        AssertThat(zdd_subseteq(zdd_B_nf, zdd_E_nf), Is().False());
      });

      it("can identify subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
        // Positive cases
        AssertThat(zdd_subseteq(zdd_empty_nf, zdd_C_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_null_nf, zdd_C_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_A_nf, zdd_C_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_B_nf, zdd_C_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_C_nf_copy, zdd_C_nf), Is().True());

        // Negative cases
        AssertThat(zdd_subseteq(zdd_D_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_subseteq(zdd_E_nf, zdd_C_nf), Is().False());
      });

      it("can identify subsets of { {1}, {1,2}, {0,1,3} } and ", [&]() {
        // Positive cases
        AssertThat(zdd_subseteq(zdd_empty_nf, zdd_D_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_A_nf, zdd_D_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_B_nf, zdd_D_nf), Is().True());
        AssertThat(zdd_subseteq(zdd_D_nf_copy, zdd_D_nf), Is().True());

        // Negative cases
        AssertThat(zdd_subseteq(zdd_null_nf, zdd_D_nf), Is().False());
        AssertThat(zdd_subseteq(zdd_C_nf, zdd_D_nf), Is().False());
        AssertThat(zdd_subseteq(zdd_E_nf, zdd_D_nf), Is().False());
      });
    });

    describe("zdd_subset", [&]() {
      it("rejects same file", [&]() {
        AssertThat(zdd_subset(zdd_A_nf, zdd_A_nf), Is().False());
        AssertThat(zdd_subset(zdd_B_nf, zdd_B_nf), Is().False());
        AssertThat(zdd_subset(zdd_C_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_subset(zdd_D_nf, zdd_D_nf), Is().False());
        AssertThat(zdd_subset(zdd_E_nf, zdd_E_nf), Is().False());
      });

      it("rejects on equal ZDDs", [&]() {
        AssertThat(zdd_subset(zdd_A_nf, zdd_A_nf_copy), Is().False());
        AssertThat(zdd_subset(zdd_C_nf, zdd_C_nf_copy), Is().False());
        AssertThat(zdd_subset(zdd_D_nf, zdd_D_nf_copy), Is().False());
        AssertThat(zdd_subset(zdd_E_nf, zdd_E_nf_copy), Is().False());
      });

      it("can compare Ø and { Ø }", [&]() {
        AssertThat(zdd_subset(zdd_empty_nf, zdd_null_nf), Is().True());
        AssertThat(zdd_subset(zdd_null_nf, zdd_empty_nf), Is().False());
      });

      it("can compare { {1} } and { {2} }", [&]() {
        AssertThat(zdd_subset(zdd_A_nf, zdd_F_nf), Is().False());
        AssertThat(zdd_subset(zdd_F_nf, zdd_A_nf), Is().False());
      });

      it("can compare { {1} } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_subset(zdd_A_nf, zdd_B_nf), Is().True());
        AssertThat(zdd_subset(zdd_B_nf, zdd_A_nf), Is().False());
      });

      it("can compare { Ø } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_subset(zdd_null_nf, zdd_B_nf), Is().False());
        AssertThat(zdd_subset(zdd_B_nf, zdd_null_nf), Is().False());
      });

      it("can compare { Ø, {0,3} } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_subset(zdd_E_nf, zdd_B_nf), Is().False());
        AssertThat(zdd_subset(zdd_B_nf, zdd_E_nf), Is().False());
      });

      it("can identify subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
        // Positive cases
        AssertThat(zdd_subset(zdd_empty_nf, zdd_C_nf), Is().True());
        AssertThat(zdd_subset(zdd_null_nf, zdd_C_nf), Is().True());
        AssertThat(zdd_subset(zdd_A_nf, zdd_C_nf), Is().True());
        AssertThat(zdd_subset(zdd_B_nf, zdd_C_nf), Is().True());

        // Equality case
        AssertThat(zdd_subset(zdd_C_nf_copy, zdd_C_nf), Is().False());

        // Negative cases
        AssertThat(zdd_subset(zdd_D_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_subset(zdd_E_nf, zdd_C_nf), Is().False());
      });

      it("can identify subsets of { {1}, {1,2}, {0,1,3} } and ", [&]() {
        // Positive cases
        AssertThat(zdd_subset(zdd_empty_nf, zdd_D_nf), Is().True());
        AssertThat(zdd_subset(zdd_A_nf, zdd_D_nf), Is().True());
        AssertThat(zdd_subset(zdd_B_nf, zdd_D_nf), Is().True());

        // Equality case
        AssertThat(zdd_subset(zdd_D_nf_copy, zdd_D_nf), Is().False());

        // Negative cases
        AssertThat(zdd_subset(zdd_null_nf, zdd_D_nf), Is().False());
        AssertThat(zdd_subset(zdd_C_nf, zdd_D_nf), Is().False());
        AssertThat(zdd_subset(zdd_E_nf, zdd_D_nf), Is().False());
      });
    });

    describe("zdd_disjoint", [&]() {
      it("rejects same file", [&]() {
        AssertThat(zdd_disjoint(zdd_A_nf, zdd_A_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_B_nf, zdd_B_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_C_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_D_nf, zdd_D_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_E_nf, zdd_E_nf), Is().False());
      });

      it("accepts same file for Ø",
         [&]() { AssertThat(zdd_disjoint(zdd_empty_nf, zdd_empty_nf), Is().True()); });

      it("rejects same file for { Ø }",
         [&]() { AssertThat(zdd_disjoint(zdd_null_nf, zdd_null_nf), Is().False()); });

      it("rejects on equal ZDDs", [&]() {
        AssertThat(zdd_disjoint(zdd_A_nf, zdd_A_nf_copy), Is().False());
        AssertThat(zdd_disjoint(zdd_C_nf, zdd_C_nf_copy), Is().False());
        AssertThat(zdd_disjoint(zdd_D_nf, zdd_D_nf_copy), Is().False());
        AssertThat(zdd_disjoint(zdd_E_nf, zdd_E_nf_copy), Is().False());
      });

      it("can compare Ø and { Ø }", [&]() {
        AssertThat(zdd_disjoint(zdd_empty_nf, zdd_null_nf), Is().True());
        AssertThat(zdd_disjoint(zdd_null_nf, zdd_empty_nf), Is().True());
      });

      it("accepts Ø and Ø",
         [&]() { AssertThat(zdd_disjoint(zdd_empty_nf, zdd_empty_nf_copy), Is().True()); });

      it("rejects { Ø } and { Ø }", [&]() {
        AssertThat(zdd_disjoint(zdd_empty_nf, zdd_null_nf), Is().True());
        AssertThat(zdd_disjoint(zdd_null_nf, zdd_empty_nf), Is().True());
      });

      it("accepts Ø and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_disjoint(zdd_empty_nf, zdd_B_nf), Is().True());
        AssertThat(zdd_disjoint(zdd_B_nf, zdd_empty_nf), Is().True());
      });

      it("accepts { {1} } and { {2} }", [&]() {
        AssertThat(zdd_disjoint(zdd_A_nf, zdd_F_nf), Is().True());
        AssertThat(zdd_disjoint(zdd_F_nf, zdd_A_nf), Is().True());
      });

      it("rejects { {1} } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_disjoint(zdd_A_nf, zdd_B_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_B_nf, zdd_A_nf), Is().False());
      });

      it("accepts { Ø } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_disjoint(zdd_null_nf, zdd_B_nf), Is().True());
        AssertThat(zdd_disjoint(zdd_B_nf, zdd_null_nf), Is().True());
      });

      it("accepts { Ø, {0,3} } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_disjoint(zdd_E_nf, zdd_B_nf), Is().True());
        AssertThat(zdd_disjoint(zdd_B_nf, zdd_E_nf), Is().True());
      });

      it("rejects subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
        AssertThat(zdd_disjoint(zdd_D_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_C_nf, zdd_D_nf), Is().False());
      });

      it("rejects subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
        AssertThat(zdd_disjoint(zdd_null_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_A_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_B_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_C_nf_copy, zdd_C_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_E_nf, zdd_C_nf), Is().False());
        AssertThat(zdd_disjoint(zdd_F_nf, zdd_C_nf), Is().False());
      });

      it("accepts non-subsets of { Ø, {1}, {2}, {1,2} }",
         [&]() { AssertThat(zdd_disjoint(zdd_empty_nf, zdd_C_nf), Is().True()); });

      it("accepts { {2} } and { {1}, {1,2} }", [&]() {
        AssertThat(zdd_disjoint(zdd_F_nf, zdd_B_nf), Is().True());
        AssertThat(zdd_disjoint(zdd_B_nf, zdd_F_nf), Is().True());
      });
    });
  });
});
