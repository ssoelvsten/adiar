#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/pred.cpp", []() {
    shared_levelized_file<bdd::node_type> bdd_T;
    /*
    //           T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_T);
      nw << node(true);
    }

    shared_levelized_file<bdd::node_type> bdd_F;
    /*
    //           F
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_F);
      nw << node(false);
    }

    const ptr_uint64 terminal_T(true);
    const ptr_uint64 terminal_F(false);

    shared_levelized_file<bdd::node_type> bdd_x0;
    /*
    //         1      ---- x0
    //        / \
    //        F T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_x0);
      nw << node(0, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_not_x0;
    /*
    //         1      ---- x0
    //        / \
    //        T F
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_not_x0);
      nw << node(0, node::max_id, terminal_T, terminal_F);
    }

    shared_levelized_file<bdd::node_type> bdd_x1;
    /*
    //         1      ---- x1
    //        / \
    //        F T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_x1);
      nw << node(1, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<bdd::node_type> bdd_0and2;
    /*
    //         1      ---- x0
    //        / \
    //        F 2     ---- x2
    //         / \
    //         F T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_0and2);

      const node n2(2, node::max_id, terminal_F, terminal_T);
      const node n1(0, node::max_id, terminal_F, n2.uid());

      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_0or2;
    /*
    //         1      ---- x0
    //        / \
    //        2 T     ---- x2
    //       / \
    //       F T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_0or2);

      const node n2(2, node::max_id, terminal_F, terminal_T);
      const node n1(0, node::max_id, n2.uid(), terminal_T);

      nw << n2 << n1;
    }

    shared_levelized_file<bdd::node_type> bdd_23_13;
    /*
    //         1      ---- x0
    //        / \
    //        2 |     ---- x1
    //       / \|
    //       F  3     ---- x2
    //         / \
    //         F T
    */

    { // Garbage collect writer to free write-lock
      node_ofstream nw(bdd_23_13);

      const node n3(2, node::max_id, terminal_F, terminal_T);
      const node n2(1, node::max_id, terminal_F, n3.uid());
      const node n1(0, node::max_id, n2.uid(), n3.uid());

      nw << n3 << n2 << n1;
    }

    // -------------------------------------------------------------------------
    // NOTE: For the overloads from <adiar/internal/dd_func.h>, please look in
    //       'test/adiar/internal/test_dd_func.cpp'.
    //
    // NOTE: For `bdd_equal` and `bdd_unequal` look at
    //       'test/adiar/internal/algorithms/test_pred.cpp'
    //
    // TODO: Move BDD variants over here?
    // -------------------------------------------------------------------------

    describe("bdd_isvar", [&]() {
      it("rejects F terminal", [&]() { AssertThat(bdd_isvar(bdd_F), Is().False()); });

      it("rejects T terminal", [&]() { AssertThat(bdd_isvar(bdd_T), Is().False()); });

      it("accepts  x0 [file content]", [&]() { AssertThat(bdd_isvar(bdd_x0), Is().True()); });

      it("accepts  x0 [complement flag]",
         [&]() { AssertThat(bdd_isvar(bdd(bdd_not_x0, true)), Is().True()); });

      it("accepts ~x0 [file content]", [&]() { AssertThat(bdd_isvar(bdd_not_x0), Is().True()); });

      it("accepts ~x0 [complement flag]",
         [&]() { AssertThat(bdd_isvar(bdd(bdd_x0, true)), Is().True()); });

      it("accepts  x1 [file content]", [&]() { AssertThat(bdd_isvar(bdd_x1), Is().True()); });

      it("accepts ~x1 [complement flag]",
         [&]() { AssertThat(bdd_isvar(bdd(bdd_x1, true)), Is().True()); });

      it("rejects x0 & x2", [&]() { AssertThat(bdd_isvar(bdd_0and2), Is().False()); });
    });

    describe("bdd_isithvar", [&]() {
      it("rejects F terminal", [&]() { AssertThat(bdd_isithvar(bdd_F), Is().False()); });

      it("rejects T terminal", [&]() { AssertThat(bdd_isithvar(bdd_T), Is().False()); });

      it("accepts  x0 [file content]", [&]() { AssertThat(bdd_isithvar(bdd_x0), Is().True()); });

      it("accepts  x0 [complement flag]",
         [&]() { AssertThat(bdd_isithvar(bdd(bdd_not_x0, true)), Is().True()); });

      it("rejects ~x0 [file content]",
         [&]() { AssertThat(bdd_isithvar(bdd_not_x0), Is().False()); });

      it("rejects ~x0 [complement flag]",
         [&]() { AssertThat(bdd_isithvar(bdd(bdd_x0, true)), Is().False()); });

      it("accepts  x1 [file content]", [&]() { AssertThat(bdd_isithvar(bdd_x1), Is().True()); });

      it("rejects ~x1 [complement flag]",
         [&]() { AssertThat(bdd_isithvar(bdd(bdd_x1, true)), Is().False()); });

      it("rejects x0 & x2", [&]() { AssertThat(bdd_isithvar(bdd_0and2), Is().False()); });
    });

    describe("bdd_isnithvar", [&]() {
      it("rejects F terminal", [&]() { AssertThat(bdd_isnithvar(bdd_F), Is().False()); });

      it("rejects T terminal", [&]() { AssertThat(bdd_isnithvar(bdd_T), Is().False()); });

      it("rejects  x0 [file content]", [&]() { AssertThat(bdd_isnithvar(bdd_x0), Is().False()); });

      it("rejects  x0 [complement flag]",
         [&]() { AssertThat(bdd_isnithvar(bdd(bdd_not_x0, true)), Is().False()); });

      it("accepts ~x0 [file content]",
         [&]() { AssertThat(bdd_isnithvar(bdd_not_x0), Is().True()); });

      it("accepts ~x0 [complement flag]",
         [&]() { AssertThat(bdd_isnithvar(bdd(bdd_x0, true)), Is().True()); });

      it("rejects  x1 [file content]", [&]() { AssertThat(bdd_isnithvar(bdd_x1), Is().False()); });

      it("accepts ~x1 [complement flag]",
         [&]() { AssertThat(bdd_isnithvar(bdd(bdd_x1, true)), Is().True()); });

      it("rejects x0 & x2", [&]() { AssertThat(bdd_isnithvar(bdd_0and2), Is().False()); });
    });

    describe("bdd_iscube", [&]() {
      it("rejects F terminal", [&]() { AssertThat(bdd_iscube(bdd_F), Is().False()); });

      it("accepts T terminal", [&]() { AssertThat(bdd_iscube(bdd_T), Is().True()); });

      it("accepts  x0 [file content]", [&]() { AssertThat(bdd_iscube(bdd_x0), Is().True()); });

      it("accepts  x0 [complement flag]",
         [&]() { AssertThat(bdd_iscube(bdd(bdd_not_x0, true)), Is().True()); });

      it("accepts ~x0 [file content]", [&]() { AssertThat(bdd_iscube(bdd_not_x0), Is().True()); });

      it("accepts ~x0 [complement flag]",
         [&]() { AssertThat(bdd_iscube(bdd(bdd_x0, true)), Is().True()); });

      it("accepts  x1 [file content]", [&]() { AssertThat(bdd_iscube(bdd_x1), Is().True()); });

      it("accepts ~x1 [complement flag]",
         [&]() { AssertThat(bdd_iscube(bdd(bdd_x1, true)), Is().True()); });

      it("accepts x0 & x2", [&]() { AssertThat(bdd_iscube(bdd_0and2), Is().True()); });

      it("rejects x0 | x2", [&]() { AssertThat(bdd_iscube(bdd_0or2), Is().False()); });

      it("accepts ~(x0 | x2) [complement flag]",
         [&]() { AssertThat(bdd_iscube(bdd(bdd_0or2, true)), Is().True()); });

      it("rejects (x0 | x1) & x2", [&]() { AssertThat(bdd_iscube(bdd_23_13), Is().False()); });
    });
  });
});
