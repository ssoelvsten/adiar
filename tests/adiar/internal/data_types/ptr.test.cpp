#include "../../../test.h"

go_bandit([]() {
  describe("adiar/internal/data_types/ptr.h", []() {
    describe("ptr_uint64", []() {
      it("should take up same amount of space as a 'uint64_t'",
         [&]() { AssertThat(sizeof(ptr_uint64), Is().EqualTo(sizeof(uint64_t))); });

      describe("nil", [&]() {
        describe(".is_flagged() and flag(...)/unflag(...)", []() {
          it("is not flagged by default",
             [&]() { AssertThat(ptr_uint64::nil().is_flagged(), Is().False()); });

          it("can be flagged",
             [&]() { AssertThat(flag(ptr_uint64::nil()).is_flagged(), Is().True()); });

          it("can be unflagged",
             [&]() { AssertThat(unflag(flag(ptr_uint64::nil())).is_flagged(), Is().False()); });
        });

        it("level for Nil is larger than Terminals and Labels", [&]() {
          AssertThat(ptr_uint64::nil_level, Is().GreaterThan(ptr_uint64::terminal_level));
          AssertThat(ptr_uint64::nil_level, Is().GreaterThan(ptr_uint64::max_label));
        });

        it(".level() is 'nil_level'", [&]() {
          const ptr_uint64 p = ptr_uint64::nil();

          AssertThat(p.level(), Is().EqualTo(ptr_uint64::nil_level));
          AssertThat(flag(p).level(), Is().EqualTo(ptr_uint64::nil_level));
        });

        it(".is_nil() accepts Nil", [&]() {
          const ptr_uint64 p = ptr_uint64::nil();

          AssertThat(p.is_nil(), Is().True());
          AssertThat(flag(p).is_nil(), Is().True());
        });

        it(".is_terminal() rejects Nil", [&]() {
          const ptr_uint64 p = ptr_uint64::nil();

          AssertThat(p.is_terminal(), Is().False());
          AssertThat(flag(p).is_terminal(), Is().False());
        });

        it(".is_false() rejects Nil", [&]() {
          const ptr_uint64 p = ptr_uint64::nil();

          AssertThat(p.is_false(), Is().False());
          AssertThat(flag(p).is_false(), Is().False());
        });

        it(".is_true() rejects Nil", [&]() {
          const ptr_uint64 p = ptr_uint64::nil();

          AssertThat(p.is_true(), Is().False());
          AssertThat(flag(p).is_true(), Is().False());
        });

        it(".is_node() rejects Nil", [&]() {
          const ptr_uint64 p = ptr_uint64::nil();

          AssertThat(p.is_node(), Is().False());
          AssertThat(flag(p).is_node(), Is().False());
        });

        it("has essential(Nil) return Nil with the flag set to 'false'", [&]() {
          const ptr_uint64 p = ptr_uint64::nil();

          AssertThat(essential(p), Is().EqualTo(p));
          AssertThat(essential(flag(p)), Is().EqualTo(p));
        });
      });

      describe("terminal", [&]() {
        it("level for Terminals is larger than Labels", [&]() {
          AssertThat(ptr_uint64::terminal_level, Is().GreaterThan(ptr_uint64::max_label));
        });

        describe(".is_flagged() and flag(...), unflag(...)", []() {
          it("'false' terminal is unflagged by default", [&]() {
            const ptr_uint64 p = ptr_uint64(false);
            AssertThat(p.is_flagged(), Is().False());
          });

          it("'true' terminal is unflagged by default", [&]() {
            const ptr_uint64 p = ptr_uint64(true);
            AssertThat(p.is_flagged(), Is().False());
          });

          it("can set the flag on 'false' terminal", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(false));
            AssertThat(flag(p).is_flagged(), Is().True());
          });

          it("can set the flag on 'true' terminal", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(true));
            AssertThat(flag(p).is_flagged(), Is().True());
          });

          it("can unset the flag on 'false' terminal", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(false));
            AssertThat(unflag(flag(p)).is_flagged(), Is().False());
          });

          it("can unset the flag on 'true' terminal", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(true));
            AssertThat(unflag(flag(p)).is_flagged(), Is().False());
          });
        });

        describe(".is_nil()", []() {
          it("rejects 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(p.is_nil(), Is().False());
            AssertThat(flag(p).is_nil(), Is().False());
          });

          it("rejects 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(p.is_nil(), Is().False());
            AssertThat(flag(p).is_nil(), Is().False());
          });
        });

        describe(".level()", []() {
          it("is 'terminal_level' for 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(p.level(), Is().EqualTo(ptr_uint64::terminal_level));
            AssertThat(flag(p).level(), Is().EqualTo(ptr_uint64::terminal_level));
          });

          it("is 'terminal_level' for 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(p.level(), Is().EqualTo(ptr_uint64::terminal_level));
            AssertThat(flag(p).level(), Is().EqualTo(ptr_uint64::terminal_level));
          });
        });

        describe(".is_terminal()", []() {
          it("accepts 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(p.is_terminal(), Is().True());
            AssertThat(flag(p).is_terminal(), Is().True());
          });

          it("accepts 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(p.is_terminal(), Is().True());
            AssertThat(flag(p).is_terminal(), Is().True());
          });
        });

        describe(".value()", []() {
          it("retrieves value from 'false' terminal", [&]() {
            ptr_uint64 p = ptr_uint64(false);

            AssertThat(p.value(), Is().False());
            AssertThat(flag(p).value(), Is().False());
          });

          it("retrieves value from 'true' terminal", [&]() {
            ptr_uint64 p = ptr_uint64(true);

            AssertThat(p.value(), Is().True());
            AssertThat(flag(p).value(), Is().True());
          });
        });

        describe(".is_false()", []() {
          it("accepts 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(p.is_false(), Is().True());
            AssertThat(flag(p).is_false(), Is().True());
          });

          it("rejects 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(p.is_false(), Is().False());
            AssertThat(flag(p).is_false(), Is().False());
          });
        });

        describe(".is_true()", []() {
          it("rejects 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(p.is_true(), Is().False());
            AssertThat(flag(p).is_true(), Is().False());
          });

          it("accepts 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(p.is_true(), Is().True());
            AssertThat(flag(p).is_true(), Is().True());
          });
        });

        describe(".is_node()", []() {
          it("rejects 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(p.is_node(), Is().False());
            AssertThat(flag(p).is_node(), Is().False());
          });

          it("rejects 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(p.is_node(), Is().False());
            AssertThat(flag(p).is_node(), Is().False());
          });
        });

        describe("not ( ! )", []() {
          it("returns 'nil' as-is", [&]() {
            const ptr_uint64 p = ptr_uint64::nil();

            AssertThat(!p, Is().EqualTo(p));
            AssertThat(!flag(p), Is().EqualTo(flag(p)));
          });

          it("returns minimal Node ptr as-is", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(!p, Is().EqualTo(p));
            AssertThat(!flag(p), Is().EqualTo(flag(p)));
          });

          it("returns maximal Node ptr as-is", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(!p, Is().EqualTo(p));
            AssertThat(!flag(p), Is().EqualTo(flag(p)));
          });

          it("negates 'false' into 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(!p, Is().EqualTo(ptr_uint64(true)));
          });

          it("negates 'true' into 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(!p, Is().EqualTo(ptr_uint64(false)));
          });

          it("preserves flag when negating 'false' terminal", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(false));

            AssertThat(!p, Is().EqualTo(flag(ptr_uint64(true))));
          });

          it("preserves flag when negating 'true' terminal", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(true));

            AssertThat(!p, Is().EqualTo(flag(ptr_uint64(false))));
          });
        });

        describe("bit-not ( ~ )", []() {
          it("negates unflagged 'false' into flagged 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(~p, Is().EqualTo(flag(ptr_uint64(true))));
          });

          it("negates unflagged 'true' into flagged 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(~p, Is().EqualTo(flag(ptr_uint64(false))));
          });

          it("negates flagged 'false' terminal into unflagged 'true' terminal", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(false));

            AssertThat(~p, Is().EqualTo(ptr_uint64(true)));
          });

          it("negates flagged 'true' terminal into unflagged 'false' terminal", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(true));

            AssertThat(~p, Is().EqualTo(ptr_uint64(false)));
          });
        });

        describe("xor ( ^ )", []() {
          it("computes 'false' for 'false' ^ 'false' terminals", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);

            AssertThat(p_false ^ p_false, Is().EqualTo(ptr_uint64(false)));
          });

          it("computes 'true' for 'false' ^ 'true' terminals", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);
            const ptr_uint64 p_true  = ptr_uint64(true);

            AssertThat(p_false ^ p_true, Is().EqualTo(ptr_uint64(true)));
            AssertThat(p_true ^ p_false, Is().EqualTo(ptr_uint64(true)));
          });

          it("computes 'false' for 'true' ^ 'true' terminals", [&]() {
            const ptr_uint64 p_true = ptr_uint64(true);

            AssertThat(p_true ^ p_true, Is().EqualTo(ptr_uint64(false)));
          });

          it("also combines flags with XOR", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);
            const ptr_uint64 p_true  = ptr_uint64(true);

            AssertThat(p_false ^ p_true, Is().EqualTo(p_true));
            AssertThat(flag(p_false) ^ p_true, Is().EqualTo(flag(p_true)));
            AssertThat(p_true ^ flag(p_true), Is().EqualTo(flag(p_false)));
            AssertThat(flag(p_false) ^ flag(p_true), Is().EqualTo(p_true));
          });
        });

        describe("and ( & )", []() {
          it("computes 'false' for 'false' ^ 'false' terminals", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);

            AssertThat(p_false & p_false, Is().EqualTo(ptr_uint64(false)));
          });

          it("computes 'false' for 'false' ^ 'true' terminals", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);
            const ptr_uint64 p_true  = ptr_uint64(true);

            AssertThat(p_false & p_true, Is().EqualTo(ptr_uint64(false)));
            AssertThat(p_true & p_false, Is().EqualTo(ptr_uint64(false)));
          });

          it("computes 'true' for 'true' ^ 'true' terminals", [&]() {
            const ptr_uint64 p_true = ptr_uint64(true);

            AssertThat(p_true & p_true, Is().EqualTo(ptr_uint64(true)));
          });

          it("also combines flags with AND", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);
            const ptr_uint64 p_true  = ptr_uint64(true);

            AssertThat(p_false & p_true, Is().EqualTo(p_false));
            AssertThat(flag(p_false) & p_true, Is().EqualTo(p_false));
            AssertThat(p_true & flag(p_true), Is().EqualTo(p_true));
            AssertThat(flag(p_false) & flag(p_true), Is().EqualTo(flag(p_false)));
          });
        });

        describe("or ( | )", []() {
          it("computes 'false' for 'false' ^ 'false' terminals", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);

            AssertThat(p_false | p_false, Is().EqualTo(ptr_uint64(false)));
          });

          it("computes 'true' for 'false' ^ 'true' terminals", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);
            const ptr_uint64 p_true  = ptr_uint64(true);

            AssertThat(p_false | p_true, Is().EqualTo(ptr_uint64(true)));
            AssertThat(p_true | p_false, Is().EqualTo(ptr_uint64(true)));
          });

          it("computes 'true' for 'true' ^ 'true' terminals", [&]() {
            const ptr_uint64 p_true = ptr_uint64(true);

            AssertThat(p_true | p_true, Is().EqualTo(ptr_uint64(true)));
          });

          it("also combines flags with OR", [&]() {
            const ptr_uint64 p_false = ptr_uint64(false);
            const ptr_uint64 p_true  = ptr_uint64(true);

            AssertThat(p_false | p_true, Is().EqualTo(p_true));
            AssertThat(flag(p_false) | p_true, Is().EqualTo(flag(p_true)));
            AssertThat(p_false | flag(p_false), Is().EqualTo(flag(p_false)));
            AssertThat(flag(p_false) | flag(p_true), Is().EqualTo(flag(p_true)));
          });
        });

        describe("essential(...)", []() {
          it("strips off flag from 'false' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(essential(p), Is().EqualTo(p));
            AssertThat(essential(flag(p)), Is().EqualTo(p));
          });

          it("strips off flag from 'true' terminal", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(essential(p), Is().EqualTo(p));
            AssertThat(essential(flag(p)), Is().EqualTo(p));
          });
        });

        describe("cnot(...)", []() {
          it("leaves 'false' terminal as-is if flag is not set", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(cnot(p, false), Is().EqualTo(p));
            AssertThat(cnot(flag(p), false), Is().EqualTo(flag(p)));
          });

          it("leaves 'true' terminal as-is if flag is not set", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(cnot(p, false), Is().EqualTo(p));
            AssertThat(cnot(flag(p), false), Is().EqualTo(flag(p)));
          });

          it("negates 'false' terminal if flag is set", [&]() {
            const ptr_uint64 p = ptr_uint64(false);

            AssertThat(cnot(p, true), Is().EqualTo(ptr_uint64(true)));
            AssertThat(cnot(flag(p), true), Is().EqualTo(flag(ptr_uint64(true))));
          });

          it("negates 'true' terminal if flag is set", [&]() {
            const ptr_uint64 p = ptr_uint64(true);

            AssertThat(cnot(p, true), Is().EqualTo(ptr_uint64(false)));
            AssertThat(cnot(flag(p), true), Is().EqualTo(flag(ptr_uint64(false))));
          });
        });
      });

      describe("internal nodes", [&]() {
        describe(".is_flagged() and flag(...), unflag(...)", [&]() {
          it("is unflagged by default for minimal Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(p.is_flagged(), Is().False());
          });

          it("is unflagged by default for maximal Node Ptr", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.is_flagged(), Is().False());
          });

          it("accepts flagged minimal Node Ptr", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(0, 0, 0));

            AssertThat(p.is_flagged(), Is().True());
          });

          it("accepts some flagged Node Ptr", [&]() {
            const ptr_uint64 p = flag(ptr_uint64(42, 2, true));
            AssertThat(p.is_flagged(), Is().True());
          });

          it("accepts flagged maximal Node Ptr", [&]() {
            const ptr_uint64 p =
              flag(ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx));

            AssertThat(p.is_flagged(), Is().True());
          });

          it("rejects unflagged minimal Node Ptr", [&]() {
            const ptr_uint64 p = unflag(flag(ptr_uint64(0, 0, 0)));

            AssertThat(p.is_flagged(), Is().False());
          });

          it("rejects some unflagged Node Ptr", [&]() {
            const ptr_uint64 p = unflag(flag(ptr_uint64(17, 3, false)));
            AssertThat(p.is_flagged(), Is().False());
          });

          it("rejects unflagged maximal Node Ptr", [&]() {
            const ptr_uint64 p = unflag(
              flag(ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx)));

            AssertThat(p.is_flagged(), Is().False());
          });
        });

        describe(".is_nil()", []() {
          it("rejects minimal Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(p.is_nil(), Is().False());
            AssertThat(flag(p).is_nil(), Is().False());
          });

          it("rejects some Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(42, 7);

            AssertThat(p.is_nil(), Is().False());
            AssertThat(flag(p).is_nil(), Is().False());
          });

          it("rejects maximal Node Ptr", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.is_nil(), Is().False());
            AssertThat(flag(p).is_nil(), Is().False());
          });
        });

        describe(".is_terminal()", []() {
          it("rejects minimal Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(p.is_terminal(), Is().False());
            AssertThat(flag(p).is_terminal(), Is().False());
          });

          it("rejects maximal Node Ptr", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.is_terminal(), Is().False());
            AssertThat(flag(p).is_terminal(), Is().False());
          });
        });

        describe(".is_false()", []() {
          it("rejects minimal Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(p.is_false(), Is().False());
            AssertThat(flag(p).is_false(), Is().False());
          });

          it("rejects maximal Node Ptr", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.is_false(), Is().False());
            AssertThat(flag(p).is_false(), Is().False());
          });
        });

        describe(".is_true()", []() {
          it("rejects minimal Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(p.is_true(), Is().False());
            AssertThat(flag(p).is_true(), Is().False());
          });

          it("rejects maximal Node Ptr", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.is_true(), Is().False());
            AssertThat(flag(p).is_true(), Is().False());
          });
        });

        describe(".is_node()", [&]() {
          it("accepts minimal Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(p.is_node(), Is().True());
            AssertThat(flag(p).is_node(), Is().True());
          });

          it("accepts some Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(42, 18, false);

            AssertThat(p.is_node(), Is().True());
            AssertThat(flag(p).is_node(), Is().True());
          });

          it("accepts maximal Node Ptr", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.is_node(), Is().True());
            AssertThat(flag(p).is_node(), Is().True());
          });
        });

        describe(".label(), .level()", [&]() {
          it("should store and retrieve label for (0, max_id, false) Ptr", [&]() {
            // This checks overflow of ids/out_idx into labels
            const ptr_uint64 p = ptr_uint64(0, ptr_uint64::max_id, true);

            AssertThat(p.label(), Is().EqualTo(0u));
            AssertThat(flag(p).label(), Is().EqualTo(0u));

            AssertThat(p.label(), Is().EqualTo(p.level()));
          });

          it("should store and retrieve label for (12, max_id, false) Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(12, ptr_uint64::max_id, false);

            AssertThat(p.label(), Is().EqualTo(12u));
            AssertThat(flag(p).label(), Is().EqualTo(12u));

            AssertThat(p.label(), Is().EqualTo(p.level()));
          });

          it("should store and retrieve for (42, 2) Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(42, 2);

            AssertThat(p.label(), Is().EqualTo(42u));
            AssertThat(flag(p).label(), Is().EqualTo(42u));

            AssertThat(p.label(), Is().EqualTo(p.level()));
          });

          it("should store and retrieve for (21, 0, true) Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(21, 0, true);

            AssertThat(p.label(), Is().EqualTo(21u));
            AssertThat(flag(p).label(), Is().EqualTo(21u));

            AssertThat(p.label(), Is().EqualTo(p.level()));
          });

          it("should store and retrieve for (max_label, max_id) Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id);

            AssertThat(p.label(), Is().EqualTo(ptr_uint64::max_label));
            AssertThat(flag(p).label(), Is().EqualTo(ptr_uint64::max_label));

            AssertThat(p.label(), Is().EqualTo(p.level()));
          });

          it("should store and retrieve for (max_label, max_id, max_out_idx) Ptr", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.label(), Is().EqualTo(ptr_uint64::max_label));
            AssertThat(flag(p).label(), Is().EqualTo(ptr_uint64::max_label));

            AssertThat(p.label(), Is().EqualTo(p.level()));
          });
        });

        describe(".out_idx()", [&]() {
          it("has maximum out-index be 1",
             [&]() { AssertThat(ptr_uint64::max_out_idx, Is().EqualTo(1u)); });

          it("has default value of 0 in (0,0)", [&]() {
            const ptr_uint64 p = ptr_uint64(2, 42);

            AssertThat(p.out_idx(), Is().EqualTo(0u));
            AssertThat(flag(p).out_idx(), Is().EqualTo(0u));
          });

          it("has default value of 0 in (2,42)", [&]() {
            const ptr_uint64 p = ptr_uint64(2, 42);

            AssertThat(p.out_idx(), Is().EqualTo(0u));
            AssertThat(flag(p).out_idx(), Is().EqualTo(0u));
          });

          it("has default value of 0 in (0,max_id)", [&]() {
            // This checks the id does not flow into the out_idx
            const ptr_uint64 p = ptr_uint64(0, ptr_uint64::max_id);

            AssertThat(p.out_idx(), Is().EqualTo(0u));
            AssertThat(flag(p).out_idx(), Is().EqualTo(0u));
          });

          it("retrieves 0 from (2,42,false)", [&]() {
            const ptr_uint64 p = ptr_uint64(2, 42, false);

            AssertThat(p.out_idx(), Is().EqualTo(0u));
            AssertThat(flag(p).out_idx(), Is().EqualTo(0u));
          });

          it("retrieves 0 from (max_label,max_id,false)", [&]() {
            const ptr_uint64 p = ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, false);

            AssertThat(p.out_idx(), Is().EqualTo(0u));
            AssertThat(flag(p).out_idx(), Is().EqualTo(0u));
          });

          it("retrieves 1 from (0,0,true)", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, true);

            AssertThat(p.out_idx(), Is().EqualTo(1u));
            AssertThat(flag(p).out_idx(), Is().EqualTo(1u));
          });

          it("retrieves 1 from (2,42,true)", [&]() {
            const ptr_uint64 p = ptr_uint64(2, 42, true);

            AssertThat(p.out_idx(), Is().EqualTo(1u));
            AssertThat(flag(p).out_idx(), Is().EqualTo(1u));
          });

          it("retrieves 1 from (max_label,max_id,true)", [&]() {
            const ptr_uint64 p = ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, true);

            AssertThat(p.out_idx(), Is().EqualTo(1u));
            AssertThat(flag(p).out_idx(), Is().EqualTo(1u));
          });
        });

        describe(".id()", [&]() {
          it("retrieve 0 from (0,0)", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0);

            AssertThat(p.id(), Is().EqualTo(0u));
            AssertThat(flag(p).id(), Is().EqualTo(0u));
          });

          it("retrieve 0 from (0,0,0)", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(p.id(), Is().EqualTo(0u));
            AssertThat(flag(p).id(), Is().EqualTo(0u));
          });

          it("retrieves 0 from (0,0,max_out_idx)", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, ptr_uint64::max_out_idx);

            AssertThat(p.id(), Is().EqualTo(0u));
            AssertThat(flag(p).id(), Is().EqualTo(0u));
          });

          it("retrieve max_id from (0,max_id)", [&]() {
            const ptr_uint64 p = ptr_uint64(0, ptr_uint64::max_id);

            AssertThat(p.id(), Is().EqualTo(ptr_uint64::max_id));
            AssertThat(flag(p).id(), Is().EqualTo(ptr_uint64::max_id));
          });

          it("retrieves max_id from (0,max_id,max_out_idx)", [&]() {
            const ptr_uint64 p = ptr_uint64(0, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.id(), Is().EqualTo(ptr_uint64::max_id));
            AssertThat(flag(p).id(), Is().EqualTo(ptr_uint64::max_id));
          });

          it("retrieves max_id from (max_label,max_id,max_out_idx)", [&]() {
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(p.id(), Is().EqualTo(ptr_uint64::max_id));
            AssertThat(flag(p).id(), Is().EqualTo(ptr_uint64::max_id));
          });

          it("retrieves 0 from (max_label,0,max_out_idx)", [&]() {
            const ptr_uint64 p = ptr_uint64(ptr_uint64::max_label, 0, ptr_uint64::max_out_idx);

            AssertThat(p.id(), Is().EqualTo(0u));
            AssertThat(flag(p).id(), Is().EqualTo(0u));
          });

          it("retrieves 42 from (2,42)", [&]() {
            const ptr_uint64 p = ptr_uint64(2, 42);

            AssertThat(p.id(), Is().EqualTo(42u));
            AssertThat(flag(p).id(), Is().EqualTo(42u));
          });

          it("retrieve 21 from (2,21)", [&]() {
            const ptr_uint64 p = ptr_uint64(2, 21);

            AssertThat(p.id(), Is().EqualTo(21u));
            AssertThat(flag(p).id(), Is().EqualTo(21u));
          });
        });

        describe("essential(...)", []() {
          it("strips off flag from minimal Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(essential(p), Is().EqualTo(p));
            AssertThat(essential(flag(p)), Is().EqualTo(p));
          });

          it("keeps out_idx as 0 for minimal Node Ptr", [&]() {
            const ptr_uint64 p = ptr_uint64(0, 0, 0);

            AssertThat(essential(p).out_idx(), Is().EqualTo(0u));
          });

          it("strips off flag from maximal Node Ptr", [&]() {
            // Use '.is_flagged()' to test it independently of '.out_idx()'
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(essential(p).is_flagged(), Is().False());
            AssertThat(essential(flag(p)).is_flagged(), Is().False());
          });

          it("strips off out_idx from maximal Node Ptr", [&]() {
            // Use '.out_idx()' to test it independently of '.out_idx()'
            const ptr_uint64 p =
              ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

            AssertThat(essential(p).out_idx(), Is().EqualTo(0u));
          });
        });

        describe("replace(...)", [&]() {
          it("shifts x0 -> x1", [&]() {
            const ptr_uint64 in(0, 0);
            const ptr_uint64 out = replace(in, 1);
            AssertThat(out, Is().EqualTo(ptr_uint64(1, 0)));
          });

          it("doubles x3 -> x6", [&]() {
            const ptr_uint64 in(3, 0);
            const ptr_uint64 out = replace(in, 6);
            AssertThat(out, Is().EqualTo(ptr_uint64(6, 0)));
          });

          it("squares x3 -> x9", [&]() {
            const ptr_uint64 in(3, 0);
            const ptr_uint64 out = replace(in, 9);
            AssertThat(out, Is().EqualTo(ptr_uint64(9, 0)));
          });

          it("preserves 'id' when replacing variable", [&]() {
            const ptr_uint64 in(0, 42);
            const ptr_uint64 out = replace(in, 2);
            AssertThat(out, Is().EqualTo(ptr_uint64(2, 42)));
          });

          it("preserves 'out_idx' when replacing variable", [&]() {
            const ptr_uint64 in(42, 0, true);
            const ptr_uint64 out = replace(in, 21);
            AssertThat(out, Is().EqualTo(ptr_uint64(21, 0, true)));
          });

          it("preserves 'flag' when replacing variable", [&]() {
            const ptr_uint64 in  = flag(ptr_uint64(21, 0));
            const ptr_uint64 out = replace(in, 42);
            AssertThat(out, Is().EqualTo(flag(ptr_uint64(42, 0))));
          });
        });

        describe("essential_replace(...)", [&]() {
          it("does nothing to 'nil'", [&]() {
            const ptr_uint64 in  = ptr_uint64::nil();
            const ptr_uint64 out = essential_replace(in, 0);
            AssertThat(out, Is().EqualTo(ptr_uint64::nil()));
          });

          it("does nothing to 'F'", [&]() {
            const ptr_uint64 in(false);
            const ptr_uint64 out = essential_replace(in, 1);
            AssertThat(out, Is().EqualTo(ptr_uint64(false)));
          });

          it("does nothing to 'T'", [&]() {
            const ptr_uint64 in(true);
            const ptr_uint64 out = essential_replace(in, 2);
            AssertThat(out, Is().EqualTo(ptr_uint64(true)));
          });

          it("can shift x2 -> x3", [&]() {
            const ptr_uint64 in(2, 0);
            const ptr_uint64 out = essential_replace(in, 3);
            AssertThat(out, Is().EqualTo(ptr_uint64(3, 0)));
          });

          it("can double x2 -> x4", [&]() {
            const ptr_uint64 in(2, 0);
            const ptr_uint64 out = essential_replace(in, 4);
            AssertThat(out, Is().EqualTo(ptr_uint64(4, 0)));
          });

          it("preserves 'id' when replacing variable", [&]() {
            const ptr_uint64 in(0, 42);
            const ptr_uint64 out = essential_replace(in, 2);
            AssertThat(out, Is().EqualTo(ptr_uint64(2, 42)));
          });

          it("removes 'out_idx' and 'flag' flag when replacing variable", [&]() {
            const ptr_uint64 in  = flag(ptr_uint64(21, 8, true));
            const ptr_uint64 out = essential_replace(in, 42);
            AssertThat(out, Is().EqualTo(ptr_uint64(42, 8)));
          });
        });

        describe("shift_replace(...)", [&]() {
          it("leaves pointer as-is [x0 + 0]", [&]() {
            const ptr_uint64 in(0, 0);
            const ptr_uint64 out = shift_replace(in, +0);
            AssertThat(out, Is().EqualTo(ptr_uint64(0, 0)));
          });

          it("leaves pointer as-is [x42 + 0]", [&]() {
            const ptr_uint64 in(42, 0);
            const ptr_uint64 out = shift_replace(in, +0);
            AssertThat(out, Is().EqualTo(ptr_uint64(42, 0)));
          });

          it("leaves pointer as-is [false + 42]", [&]() {
            const ptr_uint64 in(false);
            const ptr_uint64 out = shift_replace(in, +42);
            AssertThat(out, Is().EqualTo(ptr_uint64(false)));
          });

          it("leaves pointer as-is [true - 42]", [&]() {
            const ptr_uint64 in(true);
            const ptr_uint64 out = shift_replace(in, -42);
            AssertThat(out, Is().EqualTo(ptr_uint64(true)));
          });

          it("leaves pointer as-is [nil - 3]", [&]() {
            const ptr_uint64 in  = ptr_uint64::nil();
            const ptr_uint64 out = shift_replace(in, -3);
            AssertThat(out, Is().EqualTo(ptr_uint64::nil()));
          });

          it("shifts [x0 + 1]", [&]() {
            const ptr_uint64 in(0, 0);
            const ptr_uint64 out = shift_replace(in, +1);
            AssertThat(out, Is().EqualTo(ptr_uint64(1, 0)));
          });

          it("shifts [x1 - 1]", [&]() {
            const ptr_uint64 in(1, 0);
            const ptr_uint64 out = shift_replace(in, -1);
            AssertThat(out, Is().EqualTo(ptr_uint64(0, 0)));
          });

          it("shifts [x2 + 2]", [&]() {
            const ptr_uint64 in(2, 0);
            const ptr_uint64 out = shift_replace(in, 4);
            AssertThat(out, Is().EqualTo(ptr_uint64(6, 0)));
          });

          it("shifts [x0 + max]", [&]() {
            const ptr_uint64 in(0, 0);
            const ptr_uint64 out = shift_replace(in, ptr_uint64::max_label);
            AssertThat(out, Is().EqualTo(ptr_uint64(ptr_uint64::max_label, 0)));
          });

          it("shifts [xmax - max]", [&]() {
            const ptr_uint64 in(ptr_uint64::max_label, 0);
            const ptr_uint64 out = shift_replace(in, -static_cast<int>(ptr_uint64::max_label));
            AssertThat(out, Is().EqualTo(ptr_uint64(0, 0)));
          });

          it("preserves 'id' when replacing variable [x2 + 2]", [&]() {
            const ptr_uint64 in(2, 21);
            const ptr_uint64 out = shift_replace(in, 2);
            AssertThat(out, Is().EqualTo(ptr_uint64(4, 21)));
          });

          it("preserves 'id' when replacing variable [xmax - max]", [&]() {
            const ptr_uint64 in(ptr_uint64::max_label, ptr_uint64::max_id);
            const ptr_uint64 out = shift_replace(in, -static_cast<int>(ptr_uint64::max_label));
            AssertThat(out, Is().EqualTo(ptr_uint64(0, ptr_uint64::max_id)));
          });

          it("preserves 'out_idx' when replacing variable", [&]() {
            const ptr_uint64 in(42, 0, true);
            const ptr_uint64 out = shift_replace(in, -21);
            AssertThat(out, Is().EqualTo(ptr_uint64(21, 0, true)));
          });

          it("preserves 'flag' when replacing variable", [&]() {
            const ptr_uint64 in  = flag(ptr_uint64(21, 0));
            const ptr_uint64 out = shift_replace(in, +21);
            AssertThat(out, Is().EqualTo(flag(ptr_uint64(42, 0))));
          });
        });
      });

      describe("ordering ( < )", [&]() {
        it("should sort by label, then by id", [&]() {
          const ptr_uint64 node_1_2 = ptr_uint64(1, 2);
          const ptr_uint64 node_2_1 = ptr_uint64(2, 1);
          const ptr_uint64 node_2_2 = ptr_uint64(2, 2);

          AssertThat(node_1_2 < node_2_1, Is().True());
          AssertThat(node_2_1 < node_2_2, Is().True());
        });

        it("should sort by label and id independent of the flag", [&]() {
          const ptr_uint64 node_1_2 = ptr_uint64(1, 2);
          const ptr_uint64 node_2_1 = ptr_uint64(2, 1);

          AssertThat(node_1_2 < flag(node_2_1), Is().True());
          AssertThat(flag(node_1_2) < node_2_1, Is().True());
          AssertThat(flag(node_1_2) < flag(node_2_1), Is().True());
        });

        it("should sort by id, then by out-index", [&]() {
          const ptr_uint64 node_1_T = ptr_uint64(0, 1, true);
          const ptr_uint64 node_2_F = ptr_uint64(0, 2, false);
          const ptr_uint64 node_2_T = ptr_uint64(0, 2, true);

          AssertThat(node_1_T < node_2_F, Is().True());
          AssertThat(node_2_F < node_2_T, Is().True());
        });

        it("should sort by id and out-index independent of the flag", [&]() {
          const ptr_uint64 node_1_T = ptr_uint64(0, 1, true);
          const ptr_uint64 node_2_F = ptr_uint64(0, 2, false);

          AssertThat(node_1_T < flag(node_2_F), Is().True());
          AssertThat(flag(node_1_T) < node_2_F, Is().True());
          AssertThat(flag(node_1_T) < flag(node_2_F), Is().True());
        });

        it("should sort by label, then id, and finally out-index", [&]() {
          const ptr_uint64 node_a = ptr_uint64(0, 3, true);
          const ptr_uint64 node_b = ptr_uint64(1, 2, false);

          AssertThat(node_a < node_b, Is().True());

          const ptr_uint64 node_c = ptr_uint64(2, 0, false);
          const ptr_uint64 node_d = ptr_uint64(2, 0, true);

          AssertThat(node_c < node_d, Is().True());
        });

        it("should sort by label id, and out-index independent of the flag", [&]() {
          const ptr_uint64 node_a = ptr_uint64(0, 3, true);
          const ptr_uint64 node_b = ptr_uint64(1, 2, false);

          AssertThat(node_a < flag(node_b), Is().True());
          AssertThat(flag(node_a) < node_b, Is().True());
          AssertThat(flag(node_a) < flag(node_b), Is().True());

          const ptr_uint64 node_c = ptr_uint64(2, 0, false);
          const ptr_uint64 node_d = ptr_uint64(2, 0, true);

          AssertThat(flag(node_c) < node_d, Is().True());
          AssertThat(node_c < flag(node_d), Is().True());
          AssertThat(flag(node_c) < flag(node_d), Is().True());
        });

        it("should sort 'false' terminal after (largest) internal node", [&]() {
          const ptr_uint64 p_terminal = ptr_uint64(false);

          // Create a node pointer with the highest possible raw value
          const ptr_uint64 p_node =
            flag(ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx));

          AssertThat(p_node < p_terminal, Is().True());
          AssertThat(flag(p_node) < p_terminal, Is().True());
          AssertThat(p_node < flag(p_terminal), Is().True());
          AssertThat(flag(p_node) < flag(p_terminal), Is().True());
        });

        it("should sort 'true' terminal after (largest) internal node", [&]() {
          const ptr_uint64 p_terminal = ptr_uint64(true);

          // Create a node pointer with the highest possible raw value
          const ptr_uint64 p_node =
            ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id, ptr_uint64::max_out_idx);

          AssertThat(p_node < p_terminal, Is().True());
          AssertThat(flag(p_node) < p_terminal, Is().True());
          AssertThat(p_node < flag(p_terminal), Is().True());
          AssertThat(flag(p_node) < flag(p_terminal), Is().True());
        });
      });
    });
  });
});
