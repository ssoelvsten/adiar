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
    });
  });
});
