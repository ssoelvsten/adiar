#include "../../../test.h"

#include <adiar/internal/data_types/request.h>

go_bandit([]() {
  describe("adiar/internal/data_types/request.h", []() {
    // TODO: test comparators directly rather than only through the algorithms

    describe("request<cardinality = 1>", []() {
      it("statically provides its cardinality of 1.", []() {
        const auto cardinality = request<1>::cardinality;
        AssertThat(cardinality, Is().EqualTo(1u));
      });

      it("knows the target must be sorted", []() {
        const auto sorted_target = request<1>::sorted_target;
        AssertThat(sorted_target, Is().True());

        const auto target__is_sorted = request<1>::target_t::is_sorted;
        AssertThat(target__is_sorted, Is().True());
      });

      it("has node carry size of 0 (8 bytes)", []() {
        const auto node_carry_size = request<1>::node_carry_size;
        AssertThat(node_carry_size, Is().EqualTo(0u));

        const request<1> rec({ request<1>::pointer_type(0, 0) }, {});
        AssertThat(sizeof(rec), Is().EqualTo(1u * 8u));
      });

      describe(".level()", []() {
        it("returns the level of the target [1]", []() {
          const request<1> req({ request<1>::pointer_type(0u, 0u) }, {});
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("returns the level of the target [2]", []() {
          const request<1> req({ request<1>::pointer_type(0u, 1u) }, {});
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("returns the level of the target [3]", []() {
          const request<1> req({ request<1>::pointer_type(1u, 0u) }, {});
          AssertThat(req.level(), Is().EqualTo(1u));
        });

        it("returns the level of the target [4]", []() {
          const request<1> req({ request<1>::pointer_type(42u, 21u) }, {});
          AssertThat(req.level(), Is().EqualTo(42u));
        });
      });

      describe(".nodes_carried()", []() {
        it("does not carry any nodes [1]", []() {
          const request<1> req({ request<1>::pointer_type(0u, 0u) }, {});
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("does not carry any nodes [2]", []() {
          const request<1> req({ request<1>::pointer_type(42u, 21u) }, {});
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });
      });

      describe(".empty_carry()", []() {
        it("is true", []() {
          const request<1> req(request<1>::pointer_type(0u, 0u), {});
          AssertThat(req.empty_carry(), Is().True());
        });
      });

      describe(".targets()", []() {
        it("is 0 for nil target", []() {
          const request<1> req(request<1>::pointer_type::nil(), {});
          AssertThat(req.targets(), Is().EqualTo(0));
        });

        it("is 1 for (0,0) target", []() {
          const request<1> req(request<1>::pointer_type(0, 0), {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for (MAX,MAX) target", []() {
          const request<1> req(request<1>::pointer_type(request<1>::pointer_type::max_label,
                                                        request<1>::pointer_type::max_id),
                               {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for F target", []() {
          const request<1> req(request<1>::pointer_type(false), {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for T target", []() {
          const request<1> req(request<1>::pointer_type(true), {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });
      });

      describe("to_string()", []() {
        it("prints with a nil target", []() {
          const request<1> req(request<1>::pointer_type::nil(), {});
          AssertThat(req.to_string(), Is().EqualTo("{(nil) | []}"));
        });

        it("prints for F target", []() {
          const request<1> req(request<1>::pointer_type(false), {});
          AssertThat(req.to_string(), Is().EqualTo("{(0) | []}"));
        });

        it("prints for T target", []() {
          const request<1> req(request<1>::pointer_type(true), {});
          AssertThat(req.to_string(), Is().EqualTo("{(1) | []}"));
        });

        it("prints for (4,2) target", []() {
          const request<1> req(request<1>::pointer_type(4, 2), {});
          AssertThat(req.to_string(), Is().EqualTo("{((4;2)) | []}"));
        });
      });
    });

    describe("request<cardinality = 2>", []() {
      it("statically provides its cardinality of 2.", []() {
        const auto cardinality = request<2>::cardinality;
        AssertThat(cardinality, Is().EqualTo(2u));
      });

      it("has the target not sorted by default", []() {
        const auto sorted_target = request<2>::sorted_target;
        AssertThat(sorted_target, Is().False());

        const auto target__is_sorted = request<2>::target_t::is_sorted;
        AssertThat(target__is_sorted, Is().False());
      });

      it("has the target sorted if set to do so", []() {
        const auto sorted_target = request<2, 0, true>::sorted_target;
        AssertThat(sorted_target, Is().True());

        const auto target__is_sorted = request<2, 0, true>::target_t::is_sorted;
        AssertThat(target__is_sorted, Is().True());
      });

      it("has default node carry size of 0 (16 bytes)", []() {
        const auto node_carry_size = request<2>::node_carry_size;
        AssertThat(node_carry_size, Is().EqualTo(0u));

        const request<2, 0> rec({ request<2>::pointer_type(0, 0), request<2>::pointer_type(0, 0) },
                                {});
        AssertThat(sizeof(rec), Is().EqualTo(2u * 8u + 0u * 2u * 8u));
      });

      it("can have a node carry size of 1 (32 bytes)", []() {
        const auto node_carry_size = request<2, 1>::node_carry_size;
        AssertThat(node_carry_size, Is().EqualTo(1u));

        const request<2, 1> rec({ request<2>::pointer_type(0, 0), request<2>::pointer_type(0, 0) },
                                { { { request<2>::pointer_type::nil() } } });
        AssertThat(sizeof(rec), Is().EqualTo(2u * 8u + 1u * 2u * 8u));
      });

      describe(".level()", []() {
        it("returns the level of the first uid in target [1]", []() {
          const request<2> req(
            { request<2>::pointer_type(0u, 1u), request<2>::pointer_type(1u, 0u) }, {});
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("returns the level of the first uid in target [2]", []() {
          const request<2> req(
            { request<2>::pointer_type(1u, 0u), request<2>::pointer_type(0u, 1u) }, {});
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("returns the level of the first uid in target [3]", []() {
          const request<2> req(
            { request<2>::pointer_type(21u, 11u), request<2>::pointer_type(42u, 8u) }, {});
          AssertThat(req.level(), Is().EqualTo(21u));
        });

        it("returns the level of the first uid in target [4]", []() {
          const request<2> req(
            { request<2>::pointer_type(42u, 11u), request<2>::pointer_type(21u, 42u) }, {});
          AssertThat(req.level(), Is().EqualTo(21u));
        });
      });

      describe(".nodes_carried()", []() {
        it("has no nodes when node_carry_size is 0", []() {
          const request<2, 0> req(
            { request<2>::pointer_type(1u, 1u), request<2>::pointer_type(1u, 0u) }, {});
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("has no nodes when node_carry_size is 1 with manually added nil()", []() {
          const request<2, 1> req(
            { request<2>::pointer_type(1u, 1u), request<2>::pointer_type(1u, 0u) },
            { { request<2>::pointer_type::nil() } });
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("has no nodes when node_carry_size is 1 with NO_CHILDREN()", []() {
          const request<2, 1> req(
            { request<2>::pointer_type(1u, 1u), request<2>::pointer_type(1u, 0u) },
            { { request<2>::NO_CHILDREN() } });
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("has one nodes when node_carry_size is 1 with non-nil content", []() {
          const request<2, 1> req(
            { request<2>::pointer_type(1u, 1u), request<2>::pointer_type(1u, 0u) },
            { { { request<2>::pointer_type(2u, 1u), request<2>::pointer_type(2u, 0u) } } });
          AssertThat(req.nodes_carried(), Is().EqualTo(1u));
        });
      });

      describe(".empty_carry()", []() {
        it("is true when node_carry_size is 0", []() {
          const request<2, 0> req(
            { request<2>::pointer_type(1u, 1u), request<2>::pointer_type(1u, 0u) }, {});
          AssertThat(req.empty_carry(), Is().True());
        });

        it("is true when node_carry_size is 1 with manually added nil()", []() {
          const request<2, 1> req(
            { request<2>::pointer_type(1u, 1u), request<2>::pointer_type(1u, 0u) },
            { { request<2>::pointer_type::nil() } });
          AssertThat(req.empty_carry(), Is().True());
        });

        it("is true when node_carry_size is 1 with NO_CHILDREN()", []() {
          const request<2, 1> req(
            { request<2>::pointer_type(1u, 1u), request<2>::pointer_type(1u, 0u) },
            { { request<2>::NO_CHILDREN() } });
          AssertThat(req.empty_carry(), Is().True());
        });

        it("is false when node_carry_size is 1 with non-nil content", []() {
          const request<2, 1> req(
            { request<2>::pointer_type(1u, 1u), request<2>::pointer_type(1u, 0u) },
            { { { request<2>::pointer_type(2u, 1u), request<2>::pointer_type(2u, 0u) } } });
          AssertThat(req.empty_carry(), Is().False());
        });
      });

      describe(".targets()", []() {
        it("is 0 for {nil, nil} target [unsorted]", []() {
          const request<2> req({ request<2>::pointer_type::nil(), request<2>::pointer_type::nil() },
                               {});
          AssertThat(req.targets(), Is().EqualTo(0));
        });

        it("is 0 for {nil, nil} target [sorted]", []() {
          const request<2, 0, 1> req(
            { request<2>::pointer_type::nil(), request<2>::pointer_type::nil() }, {});
          AssertThat(req.targets(), Is().EqualTo(0));
        });

        it("is 1 for {(0,0), nil} target [unsorted]", []() {
          const request<2> req({ request<2>::pointer_type(0, 0), request<2>::pointer_type::nil() },
                               {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for {nil, (0,0)} target [unsorted]", []() {
          const request<2> req({ request<2>::pointer_type(0, 0), request<2>::pointer_type::nil() },
                               {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for {(0,0), nil} target [sorted]", []() {
          const request<2, 0, 1> req(
            { request<2>::pointer_type(0, 0), request<2>::pointer_type::nil() }, {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for {F, nil} target [unsorted]", []() {
          const request<2> req({ request<2>::pointer_type(false), request<2>::pointer_type::nil() },
                               {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for {T, nil} target [sorted]", []() {
          const request<2, 0, 1> req(
            { request<2>::pointer_type(true), request<2>::pointer_type::nil() }, {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 2 for {(0,0), T} target [unsorted]", []() {
          const request<2> req({ request<2>::pointer_type(0, 0), request<2>::pointer_type(true) },
                               {});
          AssertThat(req.targets(), Is().EqualTo(2));
        });

        it("is 2 for {F, (1,0)} target [unsorted]", []() {
          const request<2> req({ request<2>::pointer_type(false), request<2>::pointer_type(1, 0) },
                               {});
          AssertThat(req.targets(), Is().EqualTo(2));
        });

        it("is 2 for {(0,0), T} target [sorted]", []() {
          const request<2, 0, 1> req(
            { request<2>::pointer_type(0, 0), request<2>::pointer_type(true) }, {});
          AssertThat(req.targets(), Is().EqualTo(2));
        });
      });

      describe("to_string()", []() {
        it("prints for F and T target", []() {
          const request<2> req({ request<2>::pointer_type(false), request<2>::pointer_type(true) },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{(0, 1) | []}"));
        });

        it("prints for T and F target", []() {
          const request<2> req({ request<2>::pointer_type(true), request<2>::pointer_type(false) },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{(1, 0) | []}"));
        });

        it("prints with a nil target", []() {
          const request<2> req({ request<2>::pointer_type(4, 2), request<2>::pointer_type::nil() },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{((4;2), nil) | []}"));
        });

        it("prints for (0,0) and (4,2) targets", []() {
          const request<2> req({ request<2>::pointer_type(0, 0), request<2>::pointer_type(4, 2) },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{((0;0), (4;2)) | []}"));
        });

        it("prints for (0,0) and (4,2) targets and non-empty node carry", []() {
          const request<2, 1> req(
                                  { request<2>::pointer_type(1, 1), request<2>::pointer_type(1, 0) },
                                  { { { request<2>::pointer_type(2, 1), request<2>::pointer_type(2, 0) } } });
          AssertThat(req.to_string(), Is().EqualTo("{((1;1), (1;0)) | [((2;1), (2;0))]}"));
        });

        it("prints for (0,0) and (4,2) targets and non-empty node carry", []() {
          const request<2, 1> req(
                                  { request<2>::pointer_type(0, 0), request<2>::pointer_type(4, 2) },
                                  { { { request<2>::pointer_type(5, 1), request<2>::pointer_type(true) } } });
          AssertThat(req.to_string(), Is().EqualTo("{((0;0), (4;2)) | [((5;1), 1)]}"));
        });
      });
    });

    describe("request<cardinality = 3>", []() {
      it("statically provides its cardinality of 3.", []() {
        const auto cardinality = request<3>::cardinality;
        AssertThat(cardinality, Is().EqualTo(3u));
      });

      it("has the target not sorted by default", []() {
        const auto sorted_target = request<3>::sorted_target;
        AssertThat(sorted_target, Is().False());

        const auto target__is_sorted = request<3>::target_t::is_sorted;
        AssertThat(target__is_sorted, Is().False());
      });

      it("has the target sorted if set to do so", []() {
        const auto sorted_target = request<3, 0, true>::sorted_target;
        AssertThat(sorted_target, Is().True());

        const auto target__is_sorted = request<3, 0, true>::target_t::is_sorted;
        AssertThat(target__is_sorted, Is().True());
      });

      it("has default node carry size of 0 (16 bytes)", []() {
        const auto node_carry_size = request<3>::node_carry_size;
        AssertThat(node_carry_size, Is().EqualTo(0u));

        const request<3, 0> rec({ request<3>::pointer_type(0, 0),
                                  request<3>::pointer_type(0, 0),
                                  request<3>::pointer_type(0, 0) },
                                {});
        AssertThat(sizeof(rec), Is().EqualTo(3u * 8u + 0u * 2u * 8u));
      });

      it("can have a node carry size of 1 (32 bytes)", []() {
        const auto node_carry_size = request<3, 1>::node_carry_size;
        AssertThat(node_carry_size, Is().EqualTo(1u));

        const request<3, 1> rec({ request<3>::pointer_type(0, 0),
                                  request<3>::pointer_type(0, 0),
                                  request<3>::pointer_type(0, 0) },
                                { { { request<3>::pointer_type::nil() } } });
        AssertThat(sizeof(rec), Is().EqualTo(3u * 8u + 1u * 2u * 8u));
      });

      it("can have a node carry size of 2 (48 bytes)", []() {
        const auto node_carry_size = request<3, 2>::node_carry_size;
        AssertThat(node_carry_size, Is().EqualTo(2u));

        const request<3, 2> rec(
          { request<3>::pointer_type(0, 0),
            request<3>::pointer_type(0, 0),
            request<3>::pointer_type(0, 0) },
          { { { request<3>::pointer_type::nil() }, { request<3>::pointer_type::nil() } } });
        AssertThat(sizeof(rec), Is().EqualTo(3u * 8u + 2u * 2u * 8u));
      });

      describe(".level()", []() {
        it("returns the level of the first uid in target [1]", []() {
          const request<3> req({ request<3>::pointer_type(0u, 2u),
                                 request<3>::pointer_type(1u, 1u),
                                 request<3>::pointer_type(2u, 0u) },
                               {});
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("returns the level of the first uid in target [2]", []() {
          const request<3> req({ request<3>::pointer_type(1u, 1u),
                                 request<3>::pointer_type(0u, 2u),
                                 request<3>::pointer_type(2u, 0u) },
                               {});
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("returns the level of the first uid in target [3]", []() {
          const request<3> req({ request<3>::pointer_type(1u, 1u),
                                 request<3>::pointer_type(2u, 0u),
                                 request<3>::pointer_type(0u, 2u) },
                               {});
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("returns the level of the first uid in target [4]", []() {
          const request<3> req({ request<3>::pointer_type(32u, 0u),
                                 request<3>::pointer_type(21u, 11u),
                                 request<3>::pointer_type(42u, 8u) },
                               {});
          AssertThat(req.level(), Is().EqualTo(21u));
        });

        it("returns the level of the first uid in target [4]", []() {
          const request<3> req({ request<3>::pointer_type(32u, 0u),
                                 request<3>::pointer_type(42u, 11u),
                                 request<3>::pointer_type(21u, 42u) },
                               {});
          AssertThat(req.level(), Is().EqualTo(21u));
        });
      });

      describe(".nodes_carried()", []() {
        it("has no nodes when node_carry_size is 0", []() {
          const request<3, 0> req({ request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  {});
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("has no nodes when node_carry_size is 1 with manually added nil()", []() {
          const request<3, 1> req({ request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  { { { request<3>::pointer_type::nil() } } });
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("has no nodes when node_carry_size is 2 with manually added nil()", []() {
          const request<3, 2> req(
            { request<3>::pointer_type(1u, 1u),
              request<3>::pointer_type(1u, 0u),
              request<3>::pointer_type(1u, 2u) },
            { { { request<3>::pointer_type::nil() }, { request<3>::pointer_type::nil() } } });
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("has no nodes when node_carry_size is 1 with NO_CHILDREN()", []() {
          const request<3, 1> req({ request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  { { request<3>::NO_CHILDREN() } });
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("has no nodes when node_carry_size is 2 with NO_CHILDREN()", []() {
          const request<3, 2> req({ request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  { { request<3>::NO_CHILDREN(), request<3>::NO_CHILDREN() } });
          AssertThat(req.nodes_carried(), Is().EqualTo(0u));
        });

        it("has one nodes when node_carry_size is 1 with non-nil content", []() {
          const request<3, 1> req(
            { request<3>::pointer_type(1u, 1u),
              request<3>::pointer_type(1u, 0u),
              request<3>::pointer_type(1u, 2u) },
            { { { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) } } });
          AssertThat(req.nodes_carried(), Is().EqualTo(1u));
        });

        it("has one nodes when node_carry_size is 2 with non-nil and nil content", []() {
          const request<3, 2> req(
            { request<3>::pointer_type(1u, 1u),
              request<3>::pointer_type(1u, 0u),
              request<3>::pointer_type(1u, 2u) },
            { { { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) },
                request<3>::NO_CHILDREN() } });
          AssertThat(req.nodes_carried(), Is().EqualTo(1u));
        });

        it("has two nodes when node_carry_size is 2 with non-nil content", []() {
          const request<3, 2> req(
            { request<3>::pointer_type(1u, 1u),
              request<3>::pointer_type(1u, 0u),
              request<3>::pointer_type(1u, 2u) },
            { { { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) },
                { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) } } });
          AssertThat(req.nodes_carried(), Is().EqualTo(2u));
        });
      });

      describe(".empty_carry()", []() {
        it("is true when node_carry_size is 0", []() {
          const request<3, 0> req({ request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  {});
          AssertThat(req.empty_carry(), Is().True());
        });

        it("is true when node_carry_size is 1 with NO_CHILDREN()", []() {
          const request<3, 1> req({ request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  { request<3>::NO_CHILDREN() });
          AssertThat(req.empty_carry(), Is().True());
        });

        it("is true nodes when node_carry_size is 2 with NO_CHILDREN()", []() {
          const request<3, 2> req({ request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  { { request<3>::NO_CHILDREN(), request<3>::NO_CHILDREN() } });
          AssertThat(req.empty_carry(), Is().True());
        });

        it("is false nodes when node_carry_size is 1 with non-nil content", []() {
          const request<3, 1> req(
            { request<3>::pointer_type(1u, 1u),
              request<3>::pointer_type(1u, 0u),
              request<3>::pointer_type(1u, 2u) },
            { { { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) } } });
          AssertThat(req.empty_carry(), Is().False());
        });

        it("is false nodes when node_carry_size is 2 with non-nil and nil content", []() {
          const request<3, 2> req(
            { request<3>::pointer_type(1u, 1u),
              request<3>::pointer_type(1u, 0u),
              request<3>::pointer_type(1u, 2u) },
            { { { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) },
                request<3>::NO_CHILDREN() } });
          AssertThat(req.empty_carry(), Is().False());
        });

        it("is false nodes when node_carry_size is 2 with non-nil content", []() {
          const request<3, 2> req(
            { request<3>::pointer_type(1u, 1u),
              request<3>::pointer_type(1u, 0u),
              request<3>::pointer_type(1u, 2u) },
            { { { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) },
                { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) } } });
          AssertThat(req.empty_carry(), Is().False());
        });
      });

      describe(".targets()", []() {
        it("is 0 for {nil, nil, nil} target [unsorted]", []() {
          const request<3> req({ request<3>::pointer_type::nil(),
                                 request<3>::pointer_type::nil(),
                                 request<3>::pointer_type::nil() },
                               {});
          AssertThat(req.targets(), Is().EqualTo(0));
        });

        it("is 0 for {nil, nil, nil} target [sorted]", []() {
          const request<3, 0, 1> req({ request<3>::pointer_type::nil(),
                                       request<3>::pointer_type::nil(),
                                       request<3>::pointer_type::nil() },
                                     {});
          AssertThat(req.targets(), Is().EqualTo(0));
        });

        it("is 1 for {nil, T, nil} target [unsorted]", []() {
          const request<3> req({ request<3>::pointer_type::nil(),
                                 request<3>::pointer_type(true),
                                 request<3>::pointer_type::nil() },
                               {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for {(1,0), nil, nil} target [unsorted]", []() {
          const request<3> req({ request<3>::pointer_type(1, 0),
                                 request<3>::pointer_type::nil(),
                                 request<3>::pointer_type::nil() },
                               {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for {nil, nil, (0,0)} target [unsorted]", []() {
          const request<3> req({ request<3>::pointer_type::nil(),
                                 request<3>::pointer_type::nil(),
                                 request<3>::pointer_type(0, 0) },
                               {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 1 for {T, nil, nil} target [sorted]", []() {
          const request<3, 0, 1> req({ request<3>::pointer_type(true),
                                       request<3>::pointer_type::nil(),
                                       request<3>::pointer_type::nil() },
                                     {});
          AssertThat(req.targets(), Is().EqualTo(1));
        });

        it("is 2 for {T, nil, (0,0)} target [unsorted]", []() {
          const request<3> req({ request<3>::pointer_type(true),
                                 request<3>::pointer_type::nil(),
                                 request<3>::pointer_type(0, 0) },
                               {});
          AssertThat(req.targets(), Is().EqualTo(2));
        });

        it("is 2 for {nil, (42,0), (0,0)} target [unsorted]", []() {
          const request<3> req({ request<3>::pointer_type::nil(),
                                 request<3>::pointer_type(42, 0),
                                 request<3>::pointer_type(0, 0) },
                               {});
          AssertThat(req.targets(), Is().EqualTo(2));
        });

        it("is 2 for {(42,0), (0,0), nil} target [unsorted]", []() {
          const request<3> req({ request<3>::pointer_type(42, 0),
                                 request<3>::pointer_type(0, 0),
                                 request<3>::pointer_type::nil() },
                               {});
          AssertThat(req.targets(), Is().EqualTo(2));
        });

        it("is 2 for {(2,8), F, nil} target [sorted]", []() {
          const request<3, 0, 1> req({ request<3>::pointer_type(2, 8),
                                       request<3>::pointer_type(false),
                                       request<3>::pointer_type::nil() },
                                     {});
          AssertThat(req.targets(), Is().EqualTo(2));
        });

        it("is 3 for {(2,8), F, (3,0)} target [unsorted]", []() {
          const request<3> req({ request<3>::pointer_type(2, 8),
                                 request<3>::pointer_type(false),
                                 request<3>::pointer_type(3, 0) },
                               {});
          AssertThat(req.targets(), Is().EqualTo(3));
        });

        it("is 3 for {(2,8), (3,0), T} target [sorted]", []() {
          const request<3, 0, 1> req({ request<3>::pointer_type(2, 8),
                                       request<3>::pointer_type(3, 0),
                                       request<3>::pointer_type(true) },
                                     {});
          AssertThat(req.targets(), Is().EqualTo(3));
        });
      });

      describe("to_string()", []() {
        it("prints for T, F, F target", []() {
          const request<3> req({ request<3>::pointer_type(true),
                                 request<3>::pointer_type(false),
                                 request<3>::pointer_type(false) },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{(1, 0, 0) | []}"));
        });

        it("prints for F, T, and F target", []() {
          const request<3> req({ request<3>::pointer_type(false),
                                 request<3>::pointer_type(true),
                                 request<3>::pointer_type(false) },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{(0, 1, 0) | []}"));
        });

        it("prints for F, F, and T target", []() {
          const request<3> req({ request<3>::pointer_type(false),
                                 request<3>::pointer_type(false),
                                 request<3>::pointer_type(true) },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{(0, 0, 1) | []}"));
        });

        it("printsfor (0,0), (4,2), and (2,1) targets", []() {
          const request<3> req({ request<3>::pointer_type(0, 0),
                                 request<3>::pointer_type(4, 2),
                                 request<3>::pointer_type(2, 1) },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{((0;0), (4;2), (2;1)) | []}"));
        });

        it("prints with a nil target", []() {
          const request<3> req({ request<3>::pointer_type(4, 2),
                                 request<3>::pointer_type(2, 1),
                                 request<3>::pointer_type::nil() },
                               {});
          AssertThat(req.to_string(), Is().EqualTo("{((4;2), (2;1), nil) | []}"));
        });

        it("prints for node carry of one pair of children", []() {
          const request<3, 1> req(
                                  { request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  { { { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) } } });
          AssertThat(req.to_string(), Is().EqualTo("{((1;1), (1;0), (1;2)) | [((2;1), (2;0))]}"));
        });

        it("prints for node carry of two pairs of children", []() {
          const request<3, 2> req(
                                  { request<3>::pointer_type(1u, 1u),
                                    request<3>::pointer_type(1u, 0u),
                                    request<3>::pointer_type(1u, 2u) },
                                  { { { request<3>::pointer_type(2u, 1u), request<3>::pointer_type(2u, 0u) },
                                      { request<3>::pointer_type(2u, 0u), request<3>::pointer_type(false) } } });
          AssertThat(req.to_string(), Is().EqualTo("{((1;1), (1;0), (1;2)) | [((2;1), (2;0)), ((2;0), 0)]}"));
        });
      });
    });

    describe("request_data<with_parent>", []() {
      using rt1 = request_data<1, with_parent>;
      using rt2 = request_data<2, with_parent>;

      describe("to_string()", []() {
        it("prints for (4,2) target and nil parent", []() {
          const rt1 req({ rt1::pointer_type(4,2), {}, { rt1::pointer_type::nil() } });
          AssertThat(req.to_string(), Is().EqualTo("{((4;2)) | [] | {nil}}"));
        });

        it("prints for (4,2) and (2,4) targets and level 7", []() {
          const rt2 req({ {rt2::pointer_type(4,2), rt2::pointer_type(2,4)}, {}, { rt2::pointer_type(1,0) } });
          AssertThat(req.to_string(), Is().EqualTo("{((4;2), (2;4)) | [] | {(1;0)}}"));
        });
      });
    });

    describe("request_data<with_level>", []() {
      using rt1 = request_data<1, with_level>;
      using rt2 = request_data<2, with_level>;

      describe(".level()", []() {
        it("picks target level for {(0,_)} ~ 5", []() {
          const rt1 req({ rt1::pointer_type(0, 0), {}, { 5 } });
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("picks target level for {(2,_)} ~ 3", []() {
          const rt1 req({ rt1::pointer_type(2, 8), {}, { 3 } });
          AssertThat(req.level(), Is().EqualTo(2u));
        });

        it("picks data level for {(2,_)} ~ 0", []() {
          const rt1 req({ rt1::pointer_type(2, 8), {}, { 0 } });
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("picks data level for {(2,_)} ~ 1", []() {
          const rt1 req({ rt1::pointer_type(2, 8), {}, { 1 } });
          AssertThat(req.level(), Is().EqualTo(1u));
        });

        it("picks target level for {(max,_)} ~ None", []() {
          const rt1 req(
            { rt1::pointer_type(rt1::pointer_type::max_label, rt1::pointer_type::max_id),
              {},
              { with_level::no_level } });
          AssertThat(req.level(), Is().EqualTo(rt1::pointer_type::max_label));
        });

        it("picks target level for {(0,_), T} ~ 2", []() {
          const rt2 req({ { rt2::pointer_type(0, 0), rt2::pointer_type(true) }, {}, { 2 } });
          AssertThat(req.level(), Is().EqualTo(0u));
        });

        it("picks target level for {(4,_), (2,_)} ~ 3", []() {
          const rt2 req({ { rt2::pointer_type(4, 42), rt2::pointer_type(2, 8) }, {}, { 3 } });
          AssertThat(req.level(), Is().EqualTo(2u));
        });

        it("picks data level for {(4,_), (3,_)} ~ 2", []() {
          const rt2 req({ { rt2::pointer_type(4, 2), rt2::pointer_type(3, 0) }, {}, { 2 } });
          AssertThat(req.level(), Is().EqualTo(2u));
        });

        it("picks data level for {(3,_), (3,_)} ~ 1", []() {
          const rt2 req({ { rt2::pointer_type(3, 2), rt2::pointer_type(3, 0) }, {}, { 1 } });
          AssertThat(req.level(), Is().EqualTo(1u));
        });

        it("picks target level for {(0,_), T} ~ None", []() {
          const rt2 req(
            { { rt2::pointer_type(0, 0), rt2::pointer_type(true) }, {}, { with_level::no_level } });
          AssertThat(req.level(), Is().EqualTo(0u));
        });
      });

      describe("to_string()", []() {
        it("prints for (4,2) target and level 3", []() {
          const rt1 req({ rt1::pointer_type(4,2), {}, { 3 } });
          AssertThat(req.to_string(), Is().EqualTo("{((4;2)) | [] | {3}}"));
        });

        it("prints for (4,2) and (2,4) targets and level 7", []() {
          const rt2 req({ {rt1::pointer_type(4,2), rt1::pointer_type(2,4)}, {}, { 7 } });
          AssertThat(req.to_string(), Is().EqualTo("{((4;2), (2;4)) | [] | {7}}"));
        });

        it("prints for F target and level 42", []() {
          const rt1 req({ rt1::pointer_type(false), {}, { 21 } });
          AssertThat(req.to_string(), Is().EqualTo("{(0) | [] | {21}}"));
        });

        it("prints for T target and level 42", []() {
          const rt1 req({ rt1::pointer_type(true), {}, { 42 } });
          AssertThat(req.to_string(), Is().EqualTo("{(1) | [] | {42}}"));
        });
      });
    });

    describe("request_data<with_parent_and_level>", []() {
      using rt1 = request_data<1, with_parent_and_level>;
      using rt2 = request_data<2, with_parent_and_level>;

      describe("to_string()", []() {
        it("prints for (4,2) target and nil parent", []() {
          const rt1 req({ rt1::pointer_type(4,2), {}, { rt1::pointer_type::nil(), 3 } });
          AssertThat(req.to_string(), Is().EqualTo("{((4;2)) | [] | {nil, 3}}"));
        });

        it("prints for (1,0) and (1,1) targets and (0,0) parent and level 2", []() {
          const rt2 req({ {rt2::pointer_type(1,0), rt2::pointer_type(1,1)}, {}, { rt2::pointer_type(0,0), 2 } });
          AssertThat(req.to_string(), Is().EqualTo("{((1;0), (1;1)) | [] | {(0;0), 2}}"));
        });
      });
    });
  });
});
