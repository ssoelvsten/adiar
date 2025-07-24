#include "../../../test.h"

#include <adiar/internal/data_structures/level_merger.h>

go_bandit([]() {
  describe("adiar/internal/level_merger.h", []() {
    describe("level_merger<shared_levelized_file<int>, ...>", []() {
      it("can pull from one level_info stream", []() {
        shared_levelized_file<int> f;
        { // Garbage collect the writer
          levelized_ofstream<int> fw(f);

          fw.push(level_info(4, 1u));
          fw.push(level_info(3, 2u));
          fw.push(level_info(2, 2u));
          fw.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 1u> merger;

        merger.hook({ f });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(4u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can peek from one level_info streams", []() {
        shared_levelized_file<int> f;

        { // Garbage collect the writer
          levelized_ofstream<int> fw(f);

          fw.push(level_info(4, 1u));
          fw.push(level_info(3, 2u));
          fw.push(level_info(2, 1u));
          fw.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 1> merger;

        merger.hook({ f });

        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.peek(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.peek(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.pull(), Is().EqualTo(4u));
      });

      it("can pull from merge of two level_info streams, where one is empty [1]", []() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writer
          levelized_ofstream<int> fw1(f1);

          fw1.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams, where one is empty [2]", []() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writer
          levelized_ofstream<int> fw1(f1);

          fw1.push(level_info(1, 1u));
          fw1.push(level_info(2, 1u));
        }

        level_merger<shared_levelized_file<int>, std::greater<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams [1]", []() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_ofstream<int> fw1(f1);

          fw1.push(level_info(4, 1u));
          fw1.push(level_info(2, 2u));
          fw1.push(level_info(1, 1u));

          levelized_ofstream<int> fw2(f2);

          fw2.push(level_info(4, 1u));
          fw2.push(level_info(3, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(4u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams [2] (std::less)", []() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_ofstream<int> fw1(f1);

          fw1.push(level_info(2, 1u));

          levelized_ofstream<int> fw2(f2);

          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from merge of two level_info streams [2] (std::greater)", []() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_ofstream<int> fw1(f1);

          fw1.push(level_info(2, 1u));

          levelized_ofstream<int> fw2(f2);

          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::greater<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can peek merge of two level_info stream", []() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_ofstream<int> fw1(f1);

          fw1.push(level_info(4, 2u));
          fw1.push(level_info(2, 1u));

          levelized_ofstream<int> fw2(f2);

          fw2.push(level_info(4, 3u));
          fw2.push(level_info(3, 2u));
          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.peek(), Is().EqualTo(1u));
        AssertThat(merger.pull(), Is().EqualTo(1u));
        AssertThat(merger.peek(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(2u));
        AssertThat(merger.peek(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(3u));
        AssertThat(merger.peek(), Is().EqualTo(4u));
        AssertThat(merger.pull(), Is().EqualTo(4u));
      });

      it("can merge levels in reverse", []() {
        shared_levelized_file<int> f1;
        shared_levelized_file<int> f2;

        { // Garbage collect the writers
          levelized_ofstream<int> fw1(f1);

          fw1.push(level_info(4, 2u));
          fw1.push(level_info(2, 1u));

          levelized_ofstream<int> fw2(f2);

          fw2.push(level_info(4, 3u));
          fw2.push(level_info(3, 2u));
          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::greater<>, 2, true> merger;

        merger.hook({ f1, f2 });

        AssertThat(merger.peek(), Is().EqualTo(4u));
        AssertThat(merger.pull(), Is().EqualTo(4u));
        AssertThat(merger.peek(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(3u));
        AssertThat(merger.peek(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(2u));
        AssertThat(merger.peek(), Is().EqualTo(1u));
        AssertThat(merger.pull(), Is().EqualTo(1u));
      });

      it("can pull, even after the original files have been deleted", []() {
        shared_levelized_file<int> f1 = shared_levelized_file<int>();
        shared_levelized_file<int> f2 = shared_levelized_file<int>();

        { // Garbage collect the writers
          levelized_ofstream<int> fw1(f1);

          fw1.push(level_info(4, 2u));
          fw1.push(level_info(2, 1u));

          levelized_ofstream<int> fw2(f2);

          fw2.push(level_info(4, 1u));
          fw2.push(level_info(3, 2u));
          fw2.push(level_info(1, 1u));
        }

        level_merger<shared_levelized_file<int>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

        f1.reset();
        f2.reset();

        AssertThat(merger.pull(), Is().EqualTo(1u));
        AssertThat(merger.pull(), Is().EqualTo(2u));
        AssertThat(merger.pull(), Is().EqualTo(3u));
        AssertThat(merger.pull(), Is().EqualTo(4u));
      });
    });

    describe("level_merger<shared_file<label_type>, ...>", []() {
      it("can use a single label_file", []() {
        shared_file<ptr_uint64::label_type> f;

        { // Garbage collect the writers
          ofstream<ptr_uint64::label_type> w(f);
          w << 0 << 2 << 3;
        }

        level_merger<shared_file<ptr_uint64::label_type>, std::less<>, 1> merger;

        merger.hook({ f });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge two label_files", []() {
        shared_file<ptr_uint64::label_type> f1;
        shared_file<ptr_uint64::label_type> f2;

        { // Garbage collect the writers
          ofstream<ptr_uint64::label_type> w1(f1);
          w1 << 0 << 2 << 3;

          ofstream<ptr_uint64::label_type> w2(f2);
          w2 << 0 << 1 << 3;
        }

        level_merger<shared_file<ptr_uint64::label_type>, std::less<>, 2> merger;

        merger.hook({ f1, f2 });

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
    });

    describe("level_merger<dd, ...>", []() {
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

      it("can pull from a single diagram [x0]", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 1> merger;
        merger.hook({ dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from a single diagram [x1]", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 1> merger;
        merger.hook({ dd(nf_x1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can pull from a single diagram [x0 | x2]", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 1> merger;
        merger.hook({ dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x0, x1] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x0), dd(nf_x1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x1, x0] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x1), dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x1, x0] (std::greater)", [&]() {
        level_merger<shared_levelized_file<node>, std::greater<>, 2> merger;
        merger.hook({ dd(nf_x1), dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x0, x0 | x2] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x0), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x0 | x2, x0] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x0_or_x2), dd(nf_x0) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x0 | x2, x1] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x0_or_x2), dd(nf_x1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge levels of two diagrams [x1, x0 | x2] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x1), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can shift levels of a single diagram [x0 +1]", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 1> merger;
        merger.hook({ dd(nf_x0, false, +1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can shift levels of a single diagram [x1 +2]", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 1> merger;
        merger.hook({ dd(nf_x1, false, +2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can shift levels of a single diagram [x1 -1]", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 1> merger;
        merger.hook({ dd(nf_x1, false, -1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge shifted levels of two diagrams [x0 +1, x0 | x2] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x0, false, +1), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge shifted levels of two diagrams [x1 +1, x0 | x2] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x1, false, +1), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge shifted levels of two diagrams [x1 -1, x0 | x2] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x1, false, -1), dd(nf_x0_or_x2) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(0u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(2u));

        AssertThat(merger.can_pull(), Is().False());
      });

      it("can merge shifted levels of two diagrams [x1 +2, x0 | x2 +1] (std::less)", [&]() {
        level_merger<shared_levelized_file<node>, std::less<>, 2> merger;
        merger.hook({ dd(nf_x1, false, +2), dd(nf_x0_or_x2, false, +1) });

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(1u));

        AssertThat(merger.can_pull(), Is().True());
        AssertThat(merger.pull(), Is().EqualTo(3u));

        AssertThat(merger.can_pull(), Is().False());
      });
    });
  });
});
