#include "../../../test.h"

#include <adiar/internal/data_structures/level_merger.h>

go_bandit([]() {
  describe("adiar/internal/level_merger.h", []() {
    const ptr_uint64 terminal_F = ptr_uint64(false);
    const ptr_uint64 terminal_T = ptr_uint64(true);

    shared_levelized_file<dd::node_type> nf_x0;
    /*
    //          1      ---- x0
    //         / \
    //         F T
    */
    {
      node_ofstream nw(nf_x0);
      nw << node(0, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<dd::node_type> nf_x1;
    /*
    //          1      ---- x1
    //         / \
    //         F T
    */
    {
      node_ofstream nw(nf_x1);
      nw << node(1, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<dd::node_type> nf_x0_or_x2;
    /*
    //           1     ---- x0
    //          / \
    //          | T
    //          |
    //          2      ---- x2
    //         / \
    //         F T
    */
    {
      node_ofstream nw(nf_x0_or_x2);
      nw << node(2, node::max_id, terminal_F, terminal_T)
         << node(0, node::max_id, node::pointer_type(2, node::max_id), terminal_T);
    }

    describe("level_merger(dd ...)", [&]() {
      it("can pull from a single diagram [x0]", [&]() {
        level_merger<std::less<>, 1> merger({ dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from a single diagram [x1]", [&]() {
        level_merger<std::less<>, 1> merger({ dd(nf_x1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from a single diagram [x0 | x2]", [&]() {
        level_merger<std::less<>, 1> merger({ dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x0, x1] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x0), dd(nf_x1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x1, x0] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x1), dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x1, x0] (std::greater)", [&]() {
        level_merger<std::greater<>, 2> merger({ dd(nf_x1), dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x0, x0 | x2] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x0), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x0 | x2, x0] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x0_or_x2), dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x0 | x2, x1] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x0_or_x2), dd(nf_x1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x1, x0 | x2] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x1), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can shift levels of a single diagram [x0 +1]", [&]() {
        level_merger<std::less<>, 1> merger({ dd(nf_x0, false, +1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can shift levels of a single diagram [x1 +2]", [&]() {
        level_merger<std::less<>, 1> merger({ dd(nf_x1, false, +2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can shift levels of a single diagram [x1 -1]", [&]() {
        level_merger<std::less<>, 1> merger({ dd(nf_x1, false, -1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge shifted levels of two diagrams [x0 +1, x0 | x2] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x0, false, +1), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge shifted levels of two diagrams [x1 +1, x0 | x2] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x1, false, +1), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge shifted levels of two diagrams [x1 -1, x0 | x2] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x1, false, -1), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge shifted levels of two diagrams [x1 +2, x0 | x2 +1] (std::less)", [&]() {
        level_merger<std::less<>, 2> merger({ dd(nf_x1, false, +2), dd(nf_x0_or_x2, false, +1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });
    });

    describe("level_merger(gen ...)", []() {
      it("can pull from a single generator []", []() {
        const std::vector<int> xs = {};

        level_merger<std::less<>, 1> merger({ make_generator(xs.begin(), xs.end()) });

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from a single generator [0]", []() {
        const std::vector<int> xs = { 0 };

        level_merger<std::less<>, 1> merger({ make_generator(xs.begin(), xs.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from a single generator [1]", []() {
        const std::vector<int> xs = { 1 };

        level_merger<std::less<>, 1> merger({ make_generator(xs.begin(), xs.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from a single generator [0,2,3]", []() {
        const std::vector<int> xs = { 0, 2, 3 };

        level_merger<std::less<>, 1> merger({ make_generator(xs.begin(), xs.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge from a two generators [0,2,3]+[2] (std::less)", []() {
        const std::vector<int> xs = { 0, 2, 3 };
        const std::vector<int> ys = { 2 };

        level_merger<std::less<>, 2> merger(
          { make_generator(xs.begin(), xs.end()), make_generator(ys.begin(), ys.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge from a two generators [0,2,3]+[1,2] (std::less)", []() {
        const std::vector<int> xs = { 0, 2, 3 };
        const std::vector<int> ys = { 1, 2 };

        level_merger<std::less<>, 2> merger(
          { make_generator(xs.begin(), xs.end()), make_generator(ys.begin(), ys.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge from a two generators [0,2]+[] (std::less)", []() {
        const std::vector<int> xs = { 0, 2 };
        const std::vector<int> ys = {};

        level_merger<std::less<>, 2> merger(
          { make_generator(xs.begin(), xs.end()), make_generator(ys.begin(), ys.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge from a two generators [2]+[0,2] (std::less)", []() {
        const std::vector<int> xs = { 2 };
        const std::vector<int> ys = { 0, 2 };

        level_merger<std::less<>, 2> merger(
          { make_generator(xs.begin(), xs.end()), make_generator(ys.begin(), ys.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge from a two generators []+[0,2] (std::less)", []() {
        const std::vector<int> xs = {};
        const std::vector<int> ys = { 0, 2 };

        level_merger<std::less<>, 2> merger(
          { make_generator(xs.begin(), xs.end()), make_generator(ys.begin(), ys.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge from a two generators [4,2,0]+[3,2,0] (std::greater)", []() {
        const std::vector<int> xs = { 4, 2, 0 };
        const std::vector<int> ys = { 3, 2, 0 };

        level_merger<std::greater<>, 2> merger(
          { make_generator(xs.begin(), xs.end()), make_generator(ys.begin(), ys.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(4u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().False());
      });
    });

    describe("level_merger(...)", [&]() {
      it("can merge a diagram [x0] and a generator [0,2] (std::less)", [&]() {
        const std::vector<int> xs = { 0, 2 };

        level_merger<std::less<>, 2> merger({ dd(nf_x0), make_generator(xs.begin(), xs.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge a generator [0,2] and a diagram [x0] (std::less)", [&]() {
        const std::vector<int> xs = { 0, 2 };

        level_merger<std::less<>, 2> merger({ make_generator(xs.begin(), xs.end()), dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge a diagram [x1] and a generator [0,2] (std::less)", [&]() {
        const std::vector<int> xs = { 0, 2 };

        level_merger<std::less<>, 2> merger({ dd(nf_x1), make_generator(xs.begin(), xs.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge a generator [0,2] and a diagram [x1] (std::less)", [&]() {
        const std::vector<int> xs = { 0, 2 };

        level_merger<std::less<>, 2> merger({ make_generator(xs.begin(), xs.end()), dd(nf_x1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge a diagram [x0|x1] and generator [0,2] (std::less)", [&]() {
        const std::vector<int> xs = { 0, 2 };

        level_merger<std::less<>, 2> merger(
          { dd(nf_x0_or_x2), make_generator(xs.begin(), xs.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge a generator [0,2] and a diagram [x0|x1] (std::less)", [&]() {
        const std::vector<int> xs = { 0, 2 };

        level_merger<std::less<>, 2> merger(
          { make_generator(xs.begin(), xs.end()), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge a diagram [x0|x1] and generator [1,2] (std::less)", [&]() {
        const std::vector<int> xs = { 1, 2 };

        level_merger<std::less<>, 2> merger(
          { dd(nf_x0_or_x2), make_generator(xs.begin(), xs.end()) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge a generator [1,2] and a diagram [x0|x1] (std::less)", [&]() {
        const std::vector<int> xs = { 1, 2 };

        level_merger<std::less<>, 2> merger(
          { make_generator(xs.begin(), xs.end()), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });
    });
  });
});
