#include "../../test.h"

go_bandit([]() {
  describe("src/adiar/bdd.h", []() {
    shared_levelized_file<bdd::node_type> x0_nf;

    {
      node_ofstream nw(x0_nf);
      nw << node(0, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    }

    bdd x0(x0_nf);

    shared_levelized_file<bdd::node_type> x1_nf;

    {
      node_ofstream nw(x1_nf);
      nw << node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true));
    }

    bdd x1(x1_nf);

    shared_levelized_file<bdd::node_type> x0_and_x1_nf;

    {
      node_ofstream nw(x0_and_x1_nf);
      nw << node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))
         << node(0, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(1, bdd::max_id));
    }

    bdd x0_and_x1(x0_and_x1_nf);
    bdd x0_nand_x1(x0_and_x1_nf, true);

    shared_levelized_file<bdd::node_type> terminal_T_nf;
    {
      node_ofstream nw(terminal_T_nf);
      nw << node(true);
    }

    bdd terminal_T(terminal_T_nf);

    shared_levelized_file<bdd::node_type> terminal_F_nf;
    {
      node_ofstream nw(terminal_F_nf);
      nw << node(false);
    }

    bdd terminal_F(terminal_F_nf);

    shared_levelized_file<arc> x0_af;

    {
      arc_ofstream aw(x0_af);
      aw.push_internal(arc{ bdd::uid_type(0, 0), true, bdd::pointer_type(1, 0) });

      aw.push_terminal(arc{ bdd::uid_type(0, 0), false, bdd::pointer_type(false) });
      aw.push_terminal(arc{ bdd::uid_type(1, 0), false, bdd::pointer_type(true) });
      aw.push_terminal(arc{ bdd::uid_type(1, 0), true, bdd::pointer_type(true) });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 1u));
    }

    x0_af->max_1level_cut = 1;

    describe("__bdd", [&]() {
      it("conversion-constructs values from bdd", [&]() {
        __bdd t = bdd(x0_and_x1);
        AssertThat(t.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t._negate, Is().False());
        AssertThat(t._shift, Is().EqualTo(0));
      });

      it("conversion-constructs values from negated bdd", [&]() {
        __bdd t = bdd(x0_nand_x1);
        AssertThat(t.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t._negate, Is().True());
        AssertThat(t._shift, Is().EqualTo(0));
      });

      it("conversion-constructs values from shifted bdd", [&]() {
        __bdd t = bdd(x0_and_x1_nf, false, +2);
        AssertThat(t.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t._negate, Is().False());
        AssertThat(t._shift, Is().EqualTo(+2));
      });

      it("conversion-constructs values from negated and shifted bdd", [&]() {
        __bdd t = bdd(x0_and_x1_nf, true, +2);
        AssertThat(t.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t._negate, Is().True());
        AssertThat(t._shift, Is().EqualTo(+2));
      });

      it("copy-constructs values from __bdd", [&]() {
        __bdd t1 = x0_and_x1;
        __bdd t2 = t1;
        AssertThat(t2.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t2.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t2._negate, Is().False());
        AssertThat(t1._shift, Is().EqualTo(0));
      });

      it("copy-constructs values from shifted __bdd", [&]() {
        __bdd t1 = bdd(x0_and_x1_nf, false, +3);
        __bdd t2 = t1;
        AssertThat(t2.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t2.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t2._negate, Is().False());
        AssertThat(t1._shift, Is().EqualTo(+3));
      });

      it("conversion-constructs values from shared_levelized_file<bdd::node_type>", [&]() {
        __bdd t = x0_and_x1;
        AssertThat(t.has<shared_levelized_file<bdd::node_type>>(), Is().True());
        AssertThat(t.get<__bdd::shared_node_file_type>(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t._negate, Is().False());
        AssertThat(t._shift, Is().EqualTo(0));
      });

      it("conversion-constructs values from shared_levelized_file<arc>", [&]() {
        __bdd t1(x0_af, exec_policy::memory::Internal);
        AssertThat(t1.has<shared_levelized_file<arc>>(), Is().True());
        AssertThat(t1.get<__bdd::shared_arc_file_type>(), Is().EqualTo(x0_af));
        AssertThat(t1._negate, Is().False());
        AssertThat(t1._shift, Is().EqualTo(0));
        AssertThat(t1._policy, Is().EqualTo(exec_policy(exec_policy::memory::Internal)));
      });
    });

    describe("bdd", [&]() {
      it("default constructs to an F terminal", [&]() {
        const bdd t;
        AssertThat(t, Is().EqualTo(terminal_F));
      });

      it("conversion-constructs boolean 'false' value as an F terminal", [&]() {
        const bdd t(false);
        AssertThat(t, Is().EqualTo(terminal_F));
      });

      it("conversion-constructs boolean 'true' value as a T terminal", [&]() {
        const bdd t(true);
        AssertThat(t, Is().EqualTo(terminal_T));
      });

      it("conversion-constructs from node file and auxiliary values", [&]() {
        const bdd t(x0_and_x1_nf, false, +0);
        AssertThat(t.file_ptr(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t.is_negated(), Is().False());
        AssertThat(t.shift(), Is().EqualTo(0));
      });

      it("conversion-constructs from node file and auxiliary values", [&]() {
        const bdd t(x0_and_x1_nf, true, +2);
        AssertThat(t.file_ptr(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t.is_negated(), Is().True());
        AssertThat(t.shift(), Is().EqualTo(+2));
      });

      it("copy-constructs from another BDD", [&]() {
        const bdd t(bdd(x0_and_x1_nf, false, +0));
        AssertThat(t.file_ptr(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t.is_negated(), Is().False());
        AssertThat(t.shift(), Is().EqualTo(0));
      });

      it("copy-constructs from another BDD", [&]() {
        const bdd t(bdd(x0_and_x1_nf, true, +2));
        AssertThat(t.file_ptr(), Is().EqualTo(x0_and_x1_nf));
        AssertThat(t.is_negated(), Is().True());
        AssertThat(t.shift(), Is().EqualTo(+2));
      });

      it("reduces during conversion-construction from __bdd with __bdd::shared_arc_file_type",
         [&]() {
           const bdd out(__bdd(x0_af, exec_policy()));
           AssertThat(out, Is().EqualTo(x0));
         });

      it("copy-assignment from another BDD", [&]() {
        bdd t(x0_and_x1_nf, true, +2);
        t = bdd(terminal_T_nf, false, +1);
        AssertThat(t.file_ptr(), Is().EqualTo(terminal_T_nf));
        AssertThat(t.is_negated(), Is().False());
        AssertThat(t.shift(), Is().EqualTo(+1));
      });

      it("copy-assignment from another BDD", [&]() {
        bdd t(terminal_T_nf, true, +2);
        t = bdd(terminal_F_nf, true, +8);
        AssertThat(t.file_ptr(), Is().EqualTo(terminal_F_nf));
        AssertThat(t.is_negated(), Is().True());
        AssertThat(t.shift(), Is().EqualTo(+8));
      });

      it("reduces during move-assignment from __bdd::shared_arc_file_type", [&]() {
        __bdd in(x0_af, exec_policy());
        const bdd out = std::move(in);
        AssertThat(out, Is().EqualTo(x0));
      });
    });

    describe("operator overloading", [&]() {
      describe("==, !=", [&]() {
        it("checks F != T", [&]() { AssertThat(terminal_F, Is().Not().EqualTo(terminal_T)); });

        it("checks F != ~F", [&]() { AssertThat(terminal_F, Is().Not().EqualTo(~terminal_F)); });

        it("checks F != (x0 & x1)",
           [&]() { AssertThat(terminal_F, Is().Not().EqualTo(x0_and_x1)); });

        it("checks (x0 & x1) != T",
           [&]() { AssertThat(x0_and_x1, Is().Not().EqualTo(terminal_T)); });

        it("checks '(x0 & x1) != ~(x0 & x1)'",
           [&]() { AssertThat(x0_and_x1, Is().Not().EqualTo(x0_nand_x1)); });

        it("checks (x0 & x1) matches itself",
           [&]() { AssertThat(x0_and_x1, Is().EqualTo(x0_and_x1)); });

        it("checks (x0 & x1) matches a copy", [&]() {
          shared_levelized_file<bdd::node_type> other_nf;
          {
            node_ofstream nw(other_nf);
            nw << node(1, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(true))
               << node(0, bdd::max_id, bdd::pointer_type(false), bdd::pointer_type(1, bdd::max_id));
          }
          const bdd other(other_nf);
          AssertThat(x0_and_x1, Is().EqualTo(other));
        });
      });

      describe("~, &, |, ^", [&]() {
        it("negates 'T' into 'F'", [&]() { AssertThat(terminal_F, Is().EqualTo(~terminal_T)); });

        it("negates '(x0 & x1)' into ~(x0 & x1)'",
           [&]() { AssertThat(~x0_and_x1, Is().EqualTo(x0_nand_x1)); });

        it("negates '~(x0 & x1)' into '(x0 & x1)'",
           [&]() { AssertThat(~x0_and_x1, Is().EqualTo(x0_nand_x1)); });

        it("computes 'x0 & x1'", [&]() {
          const bdd f = x0 & x1;
          AssertThat(f, Is().EqualTo(x0_and_x1));
        });

        it("computes '~(~x0 | ~x1)'", [&]() {
          const bdd f = ~(~x0 | ~x1);
          AssertThat(f, Is().EqualTo(x0_and_x1));
        });

        it("computes 'x0 ^ x1' to be '(~x0 & x1) | (x0 & ~x1)'", [&]() {
          const bdd f = x0 ^ x1;

          const bdd g1 = ~x0 & x1;
          const bdd g2 = x0 & ~x1;
          const bdd g  = g1 | g2;

          AssertThat(f, Is().EqualTo(g));
        });

        it("negates 'x0 & x1' __bdd&& into '~(x0 & x1)", [&]() {
          AssertThat(x0_nand_x1 == ~(x0 & x1), Is().True());
          AssertThat(x0_nand_x1 != ~(x0 & x1), Is().False());
        });

        it("resolves 'x0 ^ x1' to be '(~x0 & x1) | (x0 & ~x1)'", [&]() {
          const bdd f = x0 ^ x1;
          const bdd g = (~x0 & x1) | (x0 & ~x1);
          AssertThat(f, Is().EqualTo(g));
        });

        it("accumulates with '&=(bdd&)' operator", [&]() {
          bdd f = x0;
          AssertThat(f == x0, Is().True());

          f &= x1;
          AssertThat(f == (x0 & x1), Is().True());

          f &= terminal_F;
          AssertThat(f == terminal_F, Is().True());
        });

        it("accumulates with '&=(__bdd&&)' operator", [&]() {
          bdd f = terminal_T;
          f &= x0 & x1;
          AssertThat((x0 & x1) == f, Is().True());
        });

        it("accumulates with '|=(bdd&)' operator", [&]() {
          bdd f = x0;
          AssertThat(f == x0, Is().True());

          f |= x1;
          AssertThat(f == (x0 | x1), Is().True());

          f |= terminal_T;
          AssertThat(terminal_T == f, Is().True());
        });

        it("accumulates with '|=(__bdd&&)' operator", [&]() {
          bdd f = terminal_F;
          f |= x0 & x1;
          AssertThat(f == (x0 & x1), Is().True());
        });

        it("accumulates with '^=(bdd&)' operator", [&]() {
          bdd f = x0;
          AssertThat(f == x0, Is().True());

          f ^= x1;
          AssertThat(f == (x0 ^ x1), Is().True());

          f ^= x0;
          AssertThat(f == x1, Is().True());
        });

        it("accumulates with '^=(__bdd&&)' operator", [&]() {
          bdd f = terminal_T;
          f ^= x0 & x1;
          AssertThat(~(x0 & x1) == f, Is().True());
        });

        it("computes with __bdd&& operators [1]", [&]() {
          const bdd f = ((x0 & ~x1) | (~x0 & x1)) ^ ((x1 ^ x0) & (~x0 & x1));
          AssertThat(f == (x0 & ~x1), Is().True());
          AssertThat((x0 & ~x1) == f, Is().True());
        });

        it("computes with __bdd&& operators [2]", [&]() {
          const bdd f = ((x0 & ~x1) | (~x0 & x1)) ^ ((x1 ^ x0) & (~x0 & x1));
          AssertThat(f == (x0 & ~x1), Is().True());
          AssertThat((x0 & ~x1) == f, Is().True());
        });

        it("computes with __bdd&& and bdd& operators [1]", [&]() {
          // Notice, that the two expressions with terminal_T and terminal_F
          // shortcut with the operator for a bdd with the negation flag
          // set correctly.
          const bdd f = ((x0 & x1) | (~x0 & x1)) ^ ((terminal_T ^ x0) & (terminal_F | x1));
          AssertThat(f == (x0 & x1), Is().True());
          AssertThat((x0 & x1) == f, Is().True());
        });

        it("computes with __bdd&& and bdd& operators [2]", [&]() {
          // The right-hand-side will evaluate to a bdd, since terminal_T negates.
          const bdd f = ((~x0 | (x0 & x1)) ^ (terminal_T ^ (x1 ^ x0)));
          AssertThat(f == (~x0 & x1), Is().True());
          AssertThat((~x0 & x1) == f, Is().True());
        });
      });

      describe("==, !=", [&]() {
        it("check two derivations of same __bdd&& [==]",
           [&]() { AssertThat((x0 ^ x1) == ((x0 & ~x1) | (~x0 & x1)), Is().True()); });

        it("check two derivations of same __bdd&& [!=]",
           [&]() { AssertThat((x0 ^ x1) != ((x0 & ~x1) | (~x0 & x1)), Is().False()); });

        it("checks two derivations of different __bdd&& [==]",
           [&]() { AssertThat((x0 | x1) == ((x0 & x1) | (~x0 & x1)), Is().False()); });

        it("checks two derivations of different __bdd&& [!=]",
           [&]() { AssertThat((x0 | x1) != ((x0 & x1) | (~x0 & x1)), Is().True()); });
      });

      describe("-", [&]() {
        it("does nothing with '+(bdd&)' [x0]", [&]() { AssertThat(+x0 == x0, Is().True()); });

        it("does nothing with '+(__bdd&) [x0 & x1]",
           [&]() { AssertThat(+(x0 & x1) == x0_and_x1, Is().True()); });

        it("computes 'x0 + x1'", [&]() {
          const bdd f = x0 + x1;
          AssertThat(f == (x0 | x1), Is().True());
        });

        it("accumulates with '+=(bdd&)' operator", [&]() {
          bdd f = x0;
          f += x1;
          AssertThat(f == (x0 | x1), Is().True());

          f += terminal_T;
          AssertThat(f == terminal_T, Is().True());
        });

        it("accumulates with '+=(__bdd&&)' operator", [&]() {
          bdd f = terminal_F;
          f += x0 & x1;
          AssertThat(f == (x0 & x1), Is().True());
        });

        it("computes 'x0 + x1'", [&]() {
          const bdd f = x0 * x1;
          AssertThat(f == (x0 & x1), Is().True());
        });

        it("accumulates with '+=(bdd&)' operator", [&]() {
          bdd f = x0;
          f *= x1;
          AssertThat(f == (x0 & x1), Is().True());

          f *= terminal_F;
          AssertThat(f == terminal_F, Is().True());
        });

        it("accumulates with '+=(__bdd&&)' operator", [&]() {
          bdd f = terminal_T;
          f *= x0 & x1;
          AssertThat(f == (x0 & x1), Is().True());
        });

        it("negates with '-(bdd&)' [x0]", [&]() { AssertThat(-x0 == ~x0, Is().True()); });

        it("negates with '-(bdd&)' [x0 & x1]",
           [&]() { AssertThat(-x0_and_x1 == ~x0_and_x1, Is().True()); });

        it("negates with '-(__bdd&) [x0 & x1]",
           [&]() { AssertThat(-(x0 & x1) == ~x0_and_x1, Is().True()); });

        it("computes 'x0 - x1'", [&]() {
          const bdd f = x0 - x1;
          AssertThat(f == (x0 & ~x1), Is().True());
        });

        it("accumulates with '-=(bdd&)' operator", [&]() {
          bdd f = x0;
          f -= x1;
          AssertThat(f == (x0 & ~x1), Is().True());

          f -= x0;
          AssertThat(f == terminal_F, Is().True());
        });

        it("accumulates with '-=(__bdd&&)' operator", [&]() {
          bdd f = terminal_T;
          f -= x0 & x1;
          AssertThat(f == ~(x0 & x1), Is().True());
        });

        it("computes with '-(__bdd&&)' and '-(bdd&)' operators", [&]() {
          const bdd f = x0_and_x1 - x0 - x1;
          AssertThat(f == terminal_F, Is().True());
          AssertThat(terminal_F == f, Is().True());
        });
      });
    });
  });
});
