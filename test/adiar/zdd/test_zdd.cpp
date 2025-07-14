#include "../../test.h"

go_bandit([]() {
  describe("adiar/zdd/zdd.h", [&]() {
    shared_levelized_file<zdd::node_type> x0_nf;
    {
      node_ofstream nw_0(x0_nf);
      nw_0 << node(0, node::max_id, ptr_uint64(false), ptr_uint64(true));
    }

    zdd x0(x0_nf);

    shared_levelized_file<zdd::node_type> x1_nf;
    {
      node_ofstream nw_1(x1_nf);
      nw_1 << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true));
    }

    zdd x1(x1_nf);

    shared_levelized_file<zdd::node_type> x0_or_x1_nf;
    {
      node_ofstream nw_01(x0_or_x1_nf);

      nw_01 << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true));

      nw_01 << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), ptr_uint64(true));
    }

    zdd x0_or_x1(x0_or_x1_nf);

    shared_levelized_file<zdd::node_type> terminal_T_nf;
    {
      node_ofstream nw_T(terminal_T_nf);
      nw_T << node(true);
    }

    zdd terminal_T(terminal_T_nf);

    shared_levelized_file<zdd::node_type> terminal_F_nf;
    {
      node_ofstream nw_F(terminal_F_nf);
      nw_F << node(false);
    }

    zdd terminal_F(terminal_F_nf);

    __zdd::shared_arc_file_type x0_or_x1_af;

    {
      arc_ofstream aw(x0_or_x1_af);
      aw.push_internal(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) });

      aw.push_terminal(arc{ ptr_uint64(0, 0), true, ptr_uint64(true) });
      aw.push_terminal(arc{ ptr_uint64(1, 0), false, ptr_uint64(false) });
      aw.push_terminal(arc{ ptr_uint64(1, 0), true, ptr_uint64(true) });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 1u));
    }

    x0_or_x1_af->max_1level_cut = 1;

    describe("__zdd class", [&]() {
      it("copy-constructs values from zdd", [&]() {
        __zdd t1 = x0_or_x1;
        AssertThat(t1.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t1._negate, Is().False());
      });

      it("copy-constructs values from __zdd", [&]() {
        __zdd t1 = x0_or_x1;
        __zdd t2 = t1;
        AssertThat(t2.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t2.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t2._negate, Is().False());
      });

      it("copy-constructs values from shared_levelized_file<zdd::node_type>", [&]() {
        __zdd t1 = x0_or_x1;
        AssertThat(t1.has<shared_levelized_file<zdd::node_type>>(), Is().True());
        AssertThat(t1.get<shared_levelized_file<zdd::node_type>>(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t1._negate, Is().False());
      });

      it("copy-constructs values from __zdd::shared_arc_file_type", [&]() {
        __zdd t1 = __zdd(x0_or_x1_af, exec_policy::access::Random_Access);
        AssertThat(t1.has<__zdd::shared_arc_file_type>(), Is().True());
        AssertThat(t1.get<__zdd::shared_arc_file_type>(), Is().EqualTo(x0_or_x1_af));
        AssertThat(t1._negate, Is().False());
        AssertThat(t1._policy, Is().EqualTo(exec_policy(exec_policy::access::Random_Access)));
      });
    });

    describe("zdd", [&]() {
      it("default-constructs as an F terminal", [&]() {
        const zdd t;
        AssertThat(t, Is().EqualTo(terminal_F));
      });

      it("conversion-constructs boolean 'false' value as an F terminal", [&]() {
        const zdd t(false);
        AssertThat(t, Is().EqualTo(terminal_F));
      });

      it("conversion-constructs boolean 'true' value as a T terminal", [&]() {
        const zdd t(true);
        AssertThat(t, Is().EqualTo(terminal_T));
      });

      it("conversion-constructs zdd::shared_arc_file_type and auxiliary values", [&]() {
        const zdd t(x0_or_x1_nf, false, +0);
        AssertThat(t.file_ptr(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t.is_negated(), Is().False());
        AssertThat(t.shift(), Is().EqualTo(+0));
      });

      it("conversion-constructs zdd::shared_arc_file_type and auxiliary values", [&]() {
        const zdd t(zdd(terminal_T_nf, true, +1));
        AssertThat(t.file_ptr(), Is().EqualTo(terminal_T_nf));
        AssertThat(t.is_negated(), Is().True());
        AssertThat(t.shift(), Is().EqualTo(+1));
      });

      it("copy-constructs from another ZDD", [&]() {
        const zdd t(zdd(terminal_F_nf, false, -1));
        AssertThat(t.file_ptr(), Is().EqualTo(terminal_F_nf));
        AssertThat(t.is_negated(), Is().False());
        AssertThat(t.shift(), Is().EqualTo(-1));
      });

      it("copy-constructs from another ZDD", [&]() {
        const zdd t(zdd(terminal_T_nf, true, +3));
        AssertThat(t.file_ptr(), Is().EqualTo(terminal_T_nf));
        AssertThat(t.is_negated(), Is().True());
        AssertThat(t.shift(), Is().EqualTo(+3));
      });

      it("reduces during conversion-constructor from __zdd with __zdd::shared_arc_file_type",
         [&]() {
           const zdd out(__zdd(x0_or_x1_af, exec_policy()));
           AssertThat(out, Is().EqualTo(x0_or_x1));
         });

      it("copy-assigns from another ZDD", [&]() {
        zdd t(terminal_T_nf, true, +3);
        t = zdd(x0_or_x1_nf, false, +0);
        AssertThat(t.file_ptr(), Is().EqualTo(x0_or_x1_nf));
        AssertThat(t.is_negated(), Is().False());
        AssertThat(t.shift(), Is().EqualTo(+0));
      });

      it("copy-assigns from another ZDD", [&]() {
        zdd t(x0_or_x1_nf, true, +0);
        t = zdd(terminal_T_nf, false, +1);
        AssertThat(t.file_ptr(), Is().EqualTo(terminal_T_nf));
        AssertThat(t.is_negated(), Is().False());
        AssertThat(t.shift(), Is().EqualTo(+1));
      });

      it("reduces during move-assignment from __zdd::shared_arc_file_type", [&]() {
        __zdd in(x0_or_x1_af, exec_policy());
        const zdd out = std::move(in);
        AssertThat(out, Is().EqualTo(x0_or_x1));
      });
    });

    describe("operators", [&]() {
      describe("==, !=", [&]() {
        it("rejects Ø == {Ø}", [&]() { AssertThat(terminal_F, Is().Not().EqualTo(terminal_T)); });

        it("accepts {{0}} == {{0}} (different files)", [&]() {
          shared_levelized_file<zdd::node_type> other_nf;
          {
            node_ofstream nw(other_nf);
            nw << node(0, node::max_id, ptr_uint64(false), ptr_uint64(true));
          }
          zdd other(other_nf);

          AssertThat(x0, Is().EqualTo(other));
        });

        it("rejects {{0}} == {{1}}", [&]() { AssertThat(x0, Is().Not().EqualTo(x1)); });

        it("rejects {{0}, {1}} == {{0}}", [&]() { AssertThat(x0_or_x1, Is().Not().EqualTo(x0)); });
      });

      describe("~, &, |", [&]() {
        it("computes ~{{0},{1}} == {Ø,{0,1}} with {0,1} domain", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          shared_levelized_file<zdd::node_type> expected;
          {
            node_ofstream nw(expected);
            nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
               << node(0, node::max_id, ptr_uint64(true), ptr_uint64(1, ptr_uint64::max_id));
          }
          AssertThat(~x0_or_x1 == zdd(expected), Is().True());
        });

        it("computes {{0}} | {{1}} == {{0}, {1}}",
           [&]() { AssertThat((x0 | x1) == x0_or_x1, Is().True()); });

        it("accumulates with '|=(zdd&)' operator", [&]() {
          zdd A = terminal_F;
          A |= x0;
          AssertThat(A, Is().EqualTo(x0));

          A |= x1;
          AssertThat(A, Is().EqualTo(x0_or_x1));
        });

        it("computes with __zdd&& operators [|]", [&]() {
          const zdd A = (x0_or_x1 | (x0 | x1)) | x0;
          AssertThat(A, Is().EqualTo(x0_or_x1));
        });

        it("computes {{0}} & {{0},{1}} == {{0}}",
           [&]() { AssertThat((x0 & x0_or_x1) == x0, Is().True()); });

        it("accumulates with '&=(zdd&)' operator", [&]() {
          zdd A = x0_or_x1;
          A &= x0;
          AssertThat(A, Is().EqualTo(x0));

          A &= x1;
          AssertThat(A, Is().EqualTo(terminal_F));
        });

        it("computes with __zdd&& operators [&]", [&]() {
          const zdd A = (x0 & (x0_or_x1 & x0)) & x1;
          AssertThat(A, Is().EqualTo(terminal_F));
        });

        it("accumulates with '|=(__zdd&&)' and '&=(__zdd&&)'", [&]() {
          zdd A = x0;

          A |= x0_or_x1 & x1;
          AssertThat(A, Is().EqualTo(x0_or_x1));

          A &= x0_or_x1 & x1;
          AssertThat(A, Is().EqualTo(x1));
        });

        it("computes with __zdd&& operators [|,&]", [&]() {
          const zdd A = ((x0_or_x1 | x0) | ((x0 & x1) | x1)) & x0;
          AssertThat(A, Is().EqualTo(x0));
        });

        it("computes with __zdd&& operators [~]", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          const zdd A = ~(x0 | x1);
          AssertThat(~x0_or_x1 == A, Is().True());
        });
      });

      describe("-, +, *", [&]() {
        it("computes -{{0},{1}} == {Ø,{0,1}} [zdd&] with {0,1} domain", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          shared_levelized_file<zdd::node_type> expected;
          {
            node_ofstream nw(expected);
            nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
               << node(0, node::max_id, ptr_uint64(true), ptr_uint64(1, ptr_uint64::max_id));
          }
          AssertThat(-x0_or_x1 == zdd(expected), Is().True());
        });

        it("computes -{{0},{1}} == {Ø,{0,1}} [__zdd&&] with {0,1} domain", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          shared_levelized_file<zdd::node_type> expected;
          {
            node_ofstream nw(expected);
            nw << node(1, node::max_id, ptr_uint64(false), ptr_uint64(true))
               << node(0, node::max_id, ptr_uint64(true), ptr_uint64(1, ptr_uint64::max_id));
          }
          AssertThat(-(x0 | x1) == zdd(expected), Is().True());
        });

        it("computes {{0},{1}} - {{0}} == {{1}}",
           [&]() { AssertThat((x0_or_x1 - x0) == x1, Is().True()); });

        it("accumulates with '-=(zdd&)'", [&]() {
          zdd A = x0_or_x1;

          A -= x0;
          AssertThat(A == x1, Is().True());

          A -= x1;
          AssertThat(A == terminal_F, Is().True());
        });

        it("accumulates with '-=(__zdd&&)'", [&]() {
          zdd A = x0_or_x1;

          A -= x0 | x1;
          AssertThat(A == terminal_F, Is().True());
        });

        it("computes +{{0}} == {{0}} [zdd&]", [&]() { AssertThat(+x0 == x0, Is().True()); });

        it("computes +{{0}} == {{0}} [__zdd&&]",
           [&]() { AssertThat(+(x0 & x0_or_x1) == x0, Is().True()); });

        it("computes {{0}} + {{1}} == {{0}, {1}}",
           [&]() { AssertThat((x0 + x1) == x0_or_x1, Is().True()); });

        it("accumulates with '+=(zdd&)' operator", [&]() {
          zdd A = terminal_F;
          A += x0;
          AssertThat(A, Is().EqualTo(x0));

          A += x1;
          AssertThat(A, Is().EqualTo(x0_or_x1));
        });

        it("computes with __zdd&& operators [+]", [&]() {
          const zdd A = (x0_or_x1 + (x0 + x1)) + x0;
          AssertThat(A, Is().EqualTo(x0_or_x1));
        });

        it("computes {{0}} * {{0},{1}} == {{0}}",
           [&]() { AssertThat((x0 * x0_or_x1) == x0, Is().True()); });

        it("accumulates with '*=(zdd&)' operator", [&]() {
          zdd A = x0_or_x1;
          A *= x0;
          AssertThat(A, Is().EqualTo(x0));

          A *= x1;
          AssertThat(A, Is().EqualTo(terminal_F));
        });

        it("computes with __zdd&& operators [*]", [&]() {
          const zdd A = (x0 * (x0_or_x1 & x0)) & x1;
          AssertThat(A, Is().EqualTo(terminal_F));
        });

        it("accumulates with '+=(__zdd&&)' and '*=(__zdd&&)'", [&]() {
          zdd A = x0;

          A += x0_or_x1 & x1;
          AssertThat(A, Is().EqualTo(x0_or_x1));

          A *= x0_or_x1 & x1;
          AssertThat(A, Is().EqualTo(x1));
        });

        it("computes with __zdd&& operators [+,*]", [&]() {
          const zdd A = ((x0_or_x1 + x0) + ((x0 * x1) + x1)) * x0;
          AssertThat(A, Is().EqualTo(x0));
        });

        it("computes with __zdd&& operators [-]", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          const zdd A = -(x0 | x1);
          AssertThat(-x0_or_x1 == A, Is().True());
        });

        it("computes with __zdd&& operators [|,-,]", [&]() {
          const std::vector<int> dom = { 0, 1 };
          domain_set(dom.begin(), dom.end());

          const zdd out      = -(-(x0 | x1) - terminal_T);
          const zdd expected = x0 | x1 | terminal_T;
          AssertThat(expected, Is().EqualTo(out));
        });

        it("computes with __zdd&& operators [-,+,*]", [&]() {
          const zdd A = ((x0_or_x1 - x0) + ((x0 + x1) * (x0_or_x1 + x1))) - (x0_or_x1 - x0);
          AssertThat(x0, Is().EqualTo(A));
        });
      });

      describe("==, !=", [&]() {
        it("checks two derivations of same __zdd&& [==]",
           [&]() { AssertThat((x0 | x1) == ((x0_or_x1 - x1) | x1), Is().True()); });

        it("checks two derivations of same __zdd&& [!=]",
           [&]() { AssertThat((x0 | x1) != ((x0_or_x1 - x1) | x1), Is().False()); });

        it("checks two derivations of different __zdd&& [==]",
           [&]() { AssertThat((x0 | x1) == (x0_or_x1 - x1), Is().False()); });

        it("checks two derivations of different __zdd&& [!=]",
           [&]() { AssertThat((x0 | x1) != (x0_or_x1 - x1), Is().True()); });
      });

      describe("<, <=, >, >=", [&]() {
        it("checks {{0}} < {{0}, {1}}", [&]() {
          AssertThat(x0 < x0_or_x1, Is().True());
          AssertThat(x0 < (x0 | x1), Is().True());
        });

        it("checks {{0}} < {{1}}", [&]() { AssertThat(x0 < x1, Is().False()); });

        it("checks {{0}, {1}} < {{0}, {1}}", [&]() {
          AssertThat((x0 | x1) < x0_or_x1, Is().False());
          AssertThat(x0_or_x1 < (x0 | x1), Is().False());
          AssertThat((x0 | x1) < (x0 | x1), Is().False());
        });

        it("checks {{0}} <= {{0}, {1}}", [&]() {
          AssertThat(x0 <= x0_or_x1, Is().True());
          AssertThat(x0 <= (x0 | x1), Is().True());
        });

        it("checks {{0}, {1}} <= {{0}, {1}}", [&]() {
          AssertThat((x0 | x1) <= x0_or_x1, Is().True());
          AssertThat((x0 | x1) <= (x0 | x1), Is().True());
        });

        it("checks {{0}} <= {{1}}", [&]() { AssertThat(x0 <= x1, Is().False()); });

        it("checks {{0}, {1}} > {{0}}", [&]() { AssertThat(x0_or_x1 > x0, Is().True()); });

        it("checks {{0}} > {{1}}", [&]() { AssertThat(x0 > x1, Is().False()); });

        it("checks {{0}, {1}} > {{0}, {1}}", [&]() {
          AssertThat((x0 | x1) > x0_or_x1, Is().False());
          AssertThat(x0_or_x1 > (x0 | x1), Is().False());
          AssertThat((x0 | x1) > (x0 | x1), Is().False());
        });

        it("checks {{0}, {1}} >= {{0}}", [&]() { AssertThat(x0_or_x1 >= x0, Is().True()); });

        it("checks {{0}, {1}} >= {{0}, {1}}", [&]() {
          AssertThat((x0 | x1) >= x0_or_x1, Is().True());
          AssertThat(x0_or_x1 >= (x0 | x1), Is().True());
          AssertThat((x0 | x1) >= (x0 | x1), Is().True());
        });

        it("checks {{0}} >= {{1}}", [&]() {
          AssertThat(x0 >= x1, Is().False());
          AssertThat(x0 >= (x0_or_x1 & x1), Is().False());
          AssertThat((x0_or_x1 & x0) >= x1, Is().False());
          AssertThat((x0_or_x1 & x0) >= (x0_or_x1 & x1), Is().False());
        });
      });
    });
  });
});
