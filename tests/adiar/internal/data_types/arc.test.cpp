#include "../../../test.h"

go_bandit([]() {
  describe("adiar/internal/data_types/arc.h", []() {
    describe("arc", []() {
      const ptr_uint64 terminal_F = ptr_uint64(false);
      const ptr_uint64 terminal_T = ptr_uint64(true);

      it("should be a POD", []() { AssertThat(std::is_pod<arc>::value, Is().True()); });

      it("should take up 16 bytes of memory", [&]() {
        const arc arc = { ptr_uint64(42, 2), terminal_F };
        AssertThat(sizeof(arc), Is().EqualTo(2u * 8u));
      });

      describe("arc(pointer_type &s, pointer_type &t)", [&] {
        it("creates arc (42,2) - -> F", [&]() {
          const arc a = { ptr_uint64(42, 2, 0), terminal_F };

          AssertThat(a.source().is_node(), Is().True());
          AssertThat(a.source().label(), Is().EqualTo(42u));
          AssertThat(a.source().id(), Is().EqualTo(2u));

          AssertThat(a.out_idx(), Is().EqualTo(0u));

          AssertThat(a.target().is_terminal(), Is().True());
          AssertThat(a.target().value(), Is().EqualTo(false));
        });

        it("creates arc '(2,0) ---> T'", [&]() {
          const arc a = { ptr_uint64(2, 0, 1), terminal_T };

          AssertThat(a.source().is_node(), Is().True());
          AssertThat(a.source().label(), Is().EqualTo(2u));
          AssertThat(a.source().id(), Is().EqualTo(0u));

          AssertThat(a.out_idx(), Is().EqualTo(1u));

          AssertThat(a.target().is_terminal(), Is().True());
          AssertThat(a.target().value(), Is().EqualTo(true));
        });

        it("creates arc '(2,0) ---> (3,1)'", [&]() {
          const arc a = { ptr_uint64(2, 0, 1), ptr_uint64(3, 1) };

          AssertThat(a.source().is_node(), Is().True());
          AssertThat(a.source().label(), Is().EqualTo(2u));
          AssertThat(a.source().id(), Is().EqualTo(0u));

          AssertThat(a.out_idx(), Is().EqualTo(1u));

          AssertThat(a.target().is_node(), Is().True());
          AssertThat(a.target().label(), Is().EqualTo(3u));
          AssertThat(a.target().id(), Is().EqualTo(1u));
        });
      });

      describe("arc(pointer_type &s, pointer_type::out_idx o, pointer_type &t)", [&] {
        it("creates arc '(42,2) - -> F'", [&]() {
          const arc a = { uid_uint64(42, 2), 0, terminal_F };

          AssertThat(a.source().is_node(), Is().True());
          AssertThat(a.source().label(), Is().EqualTo(42u));
          AssertThat(a.source().id(), Is().EqualTo(2u));

          AssertThat(a.out_idx(), Is().EqualTo(0u));

          AssertThat(a.target().is_terminal(), Is().True());
          AssertThat(a.target().value(), Is().EqualTo(false));
        });

        it("creates arc '(2,0) ---> T'", [&]() {
          const arc a = { uid_uint64(2, 0), 1, terminal_T };

          AssertThat(a.source().is_node(), Is().True());
          AssertThat(a.source().label(), Is().EqualTo(2u));
          AssertThat(a.source().id(), Is().EqualTo(0u));

          AssertThat(a.out_idx(), Is().EqualTo(1u));

          AssertThat(a.target().is_terminal(), Is().True());
          AssertThat(a.target().value(), Is().EqualTo(true));
        });

        it("creates arc '(2,0) ---> (3,1)'", [&]() {
          const arc a = { uid_uint64(2, 0), 1, ptr_uint64(3, 1) };

          AssertThat(a.source().is_node(), Is().True());
          AssertThat(a.source().label(), Is().EqualTo(2u));
          AssertThat(a.source().id(), Is().EqualTo(0u));

          AssertThat(a.out_idx(), Is().EqualTo(1u));

          AssertThat(a.target().is_node(), Is().True());
          AssertThat(a.target().label(), Is().EqualTo(3u));
          AssertThat(a.target().id(), Is().EqualTo(1u));
        });
      });

      describe(".out_idx()", [&] {
        it("should have the out-index stored in source [{ ptr, ptr }]", [&]() {
          const arc arc_low = { uid_uint64(0, 0).as_ptr(false), ptr_uint64(3, 2) };
          AssertThat(arc_low.out_idx(), Is().False());
        });

        it("should have the out-index stored in source [{ uid, idx, ptr }]", [&]() {
          const arc arc_low = { uid_uint64(0, 0), false, ptr_uint64(3, 2) };
          AssertThat(arc_low.out_idx(), Is().False());
        });

        it("should have the out-index stored in source [{ ptr, ptr }]", [&]() {
          const arc arc_high = { uid_uint64(3, 2).as_ptr(true), ptr_uint64(4, 3) };
          AssertThat(arc_high.out_idx(), Is().True());
        });

        it("should have the out-index stored in source [{ uid, idx, ptr }]", [&]() {
          const arc arc_high = { uid_uint64(3, 2), true, ptr_uint64(4, 3) };
          AssertThat(arc_high.out_idx(), Is().True());
        });
      });

      describe("equality (== / !=)", [&] {
        it("should be equal by their content [idx = 0]", [&]() {
          const ptr_uint64 s = ptr_uint64(4, 2);
          const ptr_uint64 t = ptr_uint64(42, 3);

          const arc arc_1 = { s, false, t };
          const arc arc_2 = { s, false, t };

          AssertThat(arc_1 == arc_2, Is().True());
          AssertThat(arc_1 != arc_2, Is().False());
        });

        it("should be equal by their content [idx = 1]", [&]() {
          const ptr_uint64 s = ptr_uint64(0, 0);
          const ptr_uint64 t = ptr_uint64(1, 0);

          const arc arc_1 = { s, true, t };
          const arc arc_2 = { s, true, t };

          AssertThat(arc_1 == arc_2, Is().True());
          AssertThat(arc_1 != arc_2, Is().False());
        });

        it("should unequal by their source", [&]() {
          const uid_uint64 u1 = ptr_uint64(4, 2);
          const uid_uint64 u2 = ptr_uint64(4, 3);
          const uid_uint64 u3 = ptr_uint64(3, 2);

          const arc arc_1 = { u1, u2 };
          const arc arc_2 = { u3, u2 };

          AssertThat(arc_1 == arc_2, Is().False());
          AssertThat(arc_1 != arc_2, Is().True());
        });

        it("should unequal based on out index by their content", [&]() {
          const uid_uint64 u1 = ptr_uint64(4, 2);
          const uid_uint64 u2 = ptr_uint64(4, 3);

          const arc arc_1 = { u1.as_ptr(false), u2 };
          const arc arc_2 = { u1.as_ptr(true), u2 };

          AssertThat(arc_1 == arc_2, Is().False());
          AssertThat(arc_1 != arc_2, Is().True());
        });

        it("should unequal by their target", [&]() {
          const uid_uint64 u1 = ptr_uint64(4, 2);
          const uid_uint64 u2 = ptr_uint64(4, 3);
          const uid_uint64 u3 = ptr_uint64(3, 2);

          const arc arc_1 = { u1, u2 };
          const arc arc_2 = { u1, u3 };

          AssertThat(arc_1 == arc_2, Is().False());
          AssertThat(arc_1 != arc_2, Is().True());
        });
      });

      describe("not ( ! )", [&] {
        it("should leave node_ptr target unchanged", [&]() {
          const arc a = { ptr_uint64(1, 0), ptr_uint64(2, 0) };
          AssertThat(!a, Is().EqualTo(a));
        });

        it("should negate 'false' terminal target", [&]() {
          const arc a = { ptr_uint64(1, 0), terminal_F };
          AssertThat(!a, Is().EqualTo(arc{ ptr_uint64(1, 0), terminal_T }));
        });

        it("should negate 'true' terminal target", [&]() {
          const arc a = { ptr_uint64(1, 0), terminal_T };
          AssertThat(!a, Is().EqualTo(arc{ ptr_uint64(1, 0), terminal_F }));
        });
      });
    });
  });
});
