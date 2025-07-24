#include "../../../test.h"

#include <adiar/internal/data_types/convert.h>

go_bandit([]() {
  describe("adiar/internal/data_types/convert.h", []() {
    // TODO: more than one unit test per function, please...

    describe("low_arc_of(node)", []() {
      it("should extract low arc from node", [&]() {
        const node n(7, 42, ptr_uint64(8, 21), ptr_uint64(9, 8));
        const arc a = low_arc_of(n);

        AssertThat(a.source().is_flagged(), Is().False());
        AssertThat(a.source().label(), Is().EqualTo(7u));
        AssertThat(a.source().id(), Is().EqualTo(42u));

        AssertThat(a.out_idx(), Is().EqualTo(0u));
        AssertThat(a.source().out_idx(), Is().EqualTo(0u));

        AssertThat(a.target().is_flagged(), Is().False());
        AssertThat(a.target().label(), Is().EqualTo(8u));
        AssertThat(a.target().id(), Is().EqualTo(21u));
      });
    });

    describe("high_arc_of(node)", []() {
      it("should extract high arc from node", [&]() {
        const node n(6, 13, ptr_uint64(8, 21), ptr_uint64(9, 8));
        const arc a = high_arc_of(n);

        AssertThat(a.source().is_flagged(), Is().False());
        AssertThat(a.source().label(), Is().EqualTo(6u));
        AssertThat(a.source().id(), Is().EqualTo(13u));

        AssertThat(a.out_idx(), Is().EqualTo(1u));
        AssertThat(a.source().out_idx(), Is().EqualTo(1u));

        AssertThat(a.target().is_flagged(), Is().False());
        AssertThat(a.target().label(), Is().EqualTo(9u));
        AssertThat(a.target().id(), Is().EqualTo(8u));
      });
    });

    describe("node_of(arc, arc)", []() {
      it("should combine low and high arcs to nodes into single node", [&]() {
        const arc l_arc(uid_uint64(7, 42), false, ptr_uint64(9, 8));
        const arc h_arc(uid_uint64(7, 42), true, ptr_uint64(8, 21));

        const node n = node_of(l_arc, h_arc);

        AssertThat(n.label(), Is().EqualTo(7u));
        AssertThat(n.id(), Is().EqualTo(42u));

        AssertThat(n.low(), Is().EqualTo(l_arc.target()));
        AssertThat(n.high(), Is().EqualTo(h_arc.target()));
      });

      it("should combine low and high arcs to terminals into single node", [&]() {
        const arc l_arc(uid_uint64(7, 42), false, ptr_uint64(false));
        const arc h_arc(uid_uint64(7, 42), true, ptr_uint64(true));

        const node n = node_of(l_arc, h_arc);

        AssertThat(n.label(), Is().EqualTo(7u));
        AssertThat(n.id(), Is().EqualTo(42u));

        AssertThat(n.low(), Is().EqualTo(l_arc.target()));
        AssertThat(n.high(), Is().EqualTo(h_arc.target()));
      });
    });

    describe("node_of(label, arc, arc)", []() {
      it("should combine low and high arcs to nodes into single node", [&]() {
        const arc l_arc(uid_uint64(17, 42), false, ptr_uint64(9, 8));
        const arc h_arc(uid_uint64(17, 42), true, ptr_uint64(8, 21));

        const node n = node_of(7, l_arc, h_arc);

        AssertThat(n.label(), Is().EqualTo(7u));
        AssertThat(n.id(), Is().EqualTo(42u));

        AssertThat(n.low(), Is().EqualTo(l_arc.target()));
        AssertThat(n.high(), Is().EqualTo(h_arc.target()));
      });

      it("should combine low and high arcs to terminals into single node", [&]() {
        const arc l_arc(uid_uint64(17, 42), false, ptr_uint64(false));
        const arc h_arc(uid_uint64(17, 42), true, ptr_uint64(true));

        const node n = node_of(8, l_arc, h_arc);

        AssertThat(n.label(), Is().EqualTo(8u));
        AssertThat(n.id(), Is().EqualTo(42u));

        AssertThat(n.low(), Is().EqualTo(l_arc.target()));
        AssertThat(n.high(), Is().EqualTo(h_arc.target()));
      });
    });
  });
});
