#include "../../../test.h"

go_bandit([]() {
  describe("adiar/internal/data_types/uid.h", []() {
    it("has same size as the underlying 'ptr_uint64'",
       []() { AssertThat(sizeof(uid_uint64), Is().EqualTo(sizeof(ptr_uint64))); });

    describe("reexposure of underlying pointer (without auxiliary data)", []() {
      it("wraps compile-time constants", []() {
        // Nil constants
        AssertThat(uid_uint64::nil_level, Is().EqualTo(uid_uint64::nil_level));

        // Terminal constants
        AssertThat(uid_uint64::terminal_level, Is().EqualTo(uid_uint64::terminal_level));

        // Internal Node constants
        AssertThat(uid_uint64::max_label, Is().EqualTo(uid_uint64::max_label));
        AssertThat(uid_uint64::max_id, Is().EqualTo(uid_uint64::max_id));
      });

      describe(".level()", []() {
        it("is 'terminal_level' for 'false' terminal", []() {
          const uid_uint64 u(false);

          AssertThat(u.level(), Is().EqualTo(uid_uint64::terminal_level));
        });

        it("is 'terminal_level' for 'true' terminal", []() {
          const uid_uint64 u(true);

          AssertThat(u.level(), Is().EqualTo(uid_uint64::terminal_level));
        });

        it("is 'max_label' for (max_label,42)", []() {
          const uid_uint64 u(uid_uint64::max_label, 42);

          AssertThat(u.level(), Is().EqualTo(uid_uint64::max_label));
        });

        it("is 0 for (0,0)", []() {
          const uid_uint64 u(0, 0);

          AssertThat(u.level(), Is().EqualTo(0u));
        });
      });

      describe(".is_terminal()", []() {
        it("accept 'false' terminal", []() {
          const uid_uint64 u(false);

          AssertThat(u.is_terminal(), Is().True());
        });

        it("accept 'true' terminal", []() {
          const uid_uint64 u(true);

          AssertThat(u.is_terminal(), Is().True());
        });

        it("rejects minimal Node Uid", []() {
          const uid_uint64 u = uid_uint64(ptr_uint64(0, 0, 0));

          AssertThat(u.is_terminal(), Is().False());
        });

        it("rejects maximal Node Uid", []() {
          const uid_uint64 u =
            uid_uint64(ptr_uint64(uid_uint64::max_label, uid_uint64::max_id, uid_uint64::max_out_idx));

          AssertThat(u.is_terminal(), Is().False());
        });
      });

      describe(".value()", []() {
        it("returns 'false' terminal value", []() {
          const uid_uint64 u(false);

          AssertThat(u.value(), Is().False());
        });

        it("returns 'true' terminal value", []() {
          const uid_uint64 u(true);

          AssertThat(u.value(), Is().True());
        });
      });

      describe(".is_false()", []() {
        it("accepts 'false' terminal", []() {
          const uid_uint64 u(false);

          AssertThat(u.is_false(), Is().True());
        });

        it("rejects 'true' terminal", []() {
          const uid_uint64 u(true);

          AssertThat(u.is_false(), Is().False());
        });

        it("rejects minimal Node Uid", []() {
          const uid_uint64 u = uid_uint64(ptr_uint64(0, 0, 0));

          AssertThat(u.is_false(), Is().False());
        });

        it("rejects maximal Node Uid", []() {
          const uid_uint64 u =
            uid_uint64(ptr_uint64(uid_uint64::max_label, uid_uint64::max_id, uid_uint64::max_out_idx));

          AssertThat(u.is_false(), Is().False());
        });
      });

      describe(".is_true()", []() {
        it("rejects 'false' terminal", []() {
          const uid_uint64 u(false);

          AssertThat(u.is_true(), Is().False());
        });

        it("accepts 'true' terminal", []() {
          const uid_uint64 u(true);

          AssertThat(u.is_true(), Is().True());
        });

        it("rejects minimal Node Uid", []() {
          const uid_uint64 u = uid_uint64(ptr_uint64(0, 0, 0));

          AssertThat(u.is_true(), Is().False());
        });

        it("rejects maximal Node Uid", []() {
          const uid_uint64 u =
            uid_uint64(ptr_uint64(uid_uint64::max_label, uid_uint64::max_id, uid_uint64::max_out_idx));

          AssertThat(u.is_true(), Is().False());
        });
      });

      describe(".is_node()", []() {
        it("rejects 'false' terminal", []() {
          const uid_uint64 u(false);

          AssertThat(u.is_node(), Is().False());
        });

        it("rejects 'true' terminal", []() {
          const uid_uint64 u(true);

          AssertThat(u.is_node(), Is().False());
        });

        it("accepts minimal Node Uid", []() {
          const uid_uint64 u = uid_uint64(ptr_uint64(0, 0, 0));

          AssertThat(u.is_node(), Is().True());
        });

        it("accepts maximal Node Uid", []() {
          const uid_uint64 u =
            uid_uint64(ptr_uint64(uid_uint64::max_label, uid_uint64::max_id, uid_uint64::max_out_idx));

          AssertThat(u.is_node(), Is().True());
        });
      });
    });

    describe("removal of auxiliary data", []() {
      describe(".is_flagged(), flag(...)", []() {
        it("should strip away flag when copying from 'false' pointer", []() {
          const ptr_uint64 p(false);
          const uid_uint64 u(flag(p));

          AssertThat(p.is_flagged(), Is().False());
          AssertThat(u, Is().EqualTo(p));
        });

        it("should strip away flag when copying from 'true' pointer", []() {
          const ptr_uint64 p(true);
          const uid_uint64 u(flag(p));

          AssertThat(p.is_flagged(), Is().False());
          AssertThat(u, Is().EqualTo(p));
        });

        it("should strip away flag when copying from 'internal node' pointer", []() {
          const ptr_uint64 p(53, 4);
          const uid_uint64 u(flag(p));

          AssertThat(p.is_flagged(), Is().False());
          AssertThat(u, Is().EqualTo(p));
        });
      });

      describe(".out_idx()", []() {
        it("should strip away out-index from 'internal node' pointer [1]", []() {
          const ptr_uint64 p1(53, 4, false);
          const uid_uint64 u1(p1);

          const ptr_uint64 p2(53, 4, true);
          const uid_uint64 u2(p2);

          AssertThat(u1, Is().EqualTo(u2));
        });

        it("should strip away out-index 'internal node' pointer [2]", []() {
          const ptr_uint64 p1(42, 0, false);
          const uid_uint64 u1(p1);

          const ptr_uint64 p2(42, 0, true);
          const uid_uint64 u2(p2);

          AssertThat(u1, Is().EqualTo(u2));
        });
      });

      describe(".as_ptr(...)", []() {
        it("by default provides itself with out-index 0", []() {
          const uid_uint64 u(42, 0);
          AssertThat(u.as_ptr(), Is().EqualTo(ptr_uint64(42, 0, false)));
        });

        it("can provide a pointer with out-index 0", []() {
          const uid_uint64 u(42, 0);
          AssertThat(u.as_ptr(false), Is().EqualTo(ptr_uint64(42, 0, false)));
        });

        it("can provide a pointer with out-index 1", []() {
          const uid_uint64 u(42, 0);
          AssertThat(u.as_ptr(true), Is().EqualTo(ptr_uint64(42, 0, true)));
        });
      });
    });
  });
});
