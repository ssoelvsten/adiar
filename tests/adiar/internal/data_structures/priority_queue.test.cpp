#include "../../../test.h"

#include <adiar/internal/data_structures/priority_queue.h>

go_bandit([]() {
  describe("adiar/internal/priority_queue.h", []() {
    describe("priority_queue<memory_mode::Internal, int, std::less<>>", []() {
      priority_queue<memory_mode::Internal, int, std::less<>> pq(1024, 16);

      it("is initially empty", [&pq]() {
        AssertThat(pq.empty(), Is().True());
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.has_top(), Is().False());
      });

      it("can push elements", [&pq]() {
        pq.push(3);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(1u));
        AssertThat(pq.has_top(), Is().True());

        pq.push(1);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(2u));
        AssertThat(pq.has_top(), Is().True());

        pq.push(4);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(3u));
        AssertThat(pq.has_top(), Is().True());
      });

      it("can peek smallest element", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(1));
      });

      it("Can pop smallest element", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(1));
        pq.pop();
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
      });

      it("can push a new element (> .top())", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
        pq.push(4);
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
      });

      it("can push a new element (< .top)()", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
        pq.push(2);
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
      });

      it("can pop remaining elements", [&pq]() {
        AssertThat(pq.size(), Is().EqualTo(4u));

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(4));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(4));
        pq.pop();

        AssertThat(pq.has_top(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(0u));
      });
    });

    describe("priority_queue<memory_mode::External, int, std::less<>>", []() {
      priority_queue<memory_mode::External, int, std::less<>> pq(8 * 1024 * 1024, 16);

      it("Is initially empty", [&pq]() {
        AssertThat(pq.empty(), Is().True());
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.has_top(), Is().False());
      });

      it("can push elements", [&pq]() {
        pq.push(3);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(1u));
        AssertThat(pq.has_top(), Is().True());

        pq.push(1);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(2u));
        AssertThat(pq.has_top(), Is().True());

        pq.push(4);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(3u));
        AssertThat(pq.has_top(), Is().True());
      });

      it("can peek smallest element", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(1));
      });

      it("Can pop smallest element", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(1));
        pq.pop();
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
      });

      it("can push a new element (> .top())", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
        pq.push(4);
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
      });

      it("can push a new element (< .top)()", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
        pq.push(2);
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
      });

      it("can pop remaining elements", [&pq]() {
        AssertThat(pq.size(), Is().EqualTo(4u));

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(4));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(4));
        pq.pop();

        AssertThat(pq.has_top(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(0u));
      });
    });

    describe("priority_queue<memory_mode::Internal, int, std::greater<>>", []() {
      priority_queue<memory_mode::Internal, int, std::greater<>> pq(1024, 16);

      it("is initially empty", [&pq]() {
        AssertThat(pq.empty(), Is().True());
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.has_top(), Is().False());
      });

      it("can push elements", [&pq]() {
        pq.push(2);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(1u));
        AssertThat(pq.has_top(), Is().True());

        pq.push(4);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(2u));
        AssertThat(pq.has_top(), Is().True());

        pq.push(1);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(3u));
        AssertThat(pq.has_top(), Is().True());
      });

      it("can peek smallest element", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(4));
      });

      it("Can pop smallest element", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(4));
        pq.pop();
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
      });

      it("can push a new element (> .top())", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
        pq.push(1);
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
      });

      it("can push a new element (< .top)()", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
        pq.push(3);
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
      });

      it("can pop remaining elements", [&pq]() {
        AssertThat(pq.size(), Is().EqualTo(4u));

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(1));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(1));
        pq.pop();

        AssertThat(pq.has_top(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(0u));
      });
    });

    describe("priority_queue<memory_mode::External, int, std::greater<>>", []() {
      priority_queue<memory_mode::External, int, std::greater<>> pq(8 * 1024 * 1024, 16);

      it("is initially empty", [&pq]() {
        AssertThat(pq.empty(), Is().True());
        AssertThat(pq.size(), Is().EqualTo(0u));
        AssertThat(pq.has_top(), Is().False());
      });

      it("can push elements", [&pq]() {
        pq.push(2);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(1u));
        AssertThat(pq.has_top(), Is().True());

        pq.push(4);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(2u));
        AssertThat(pq.has_top(), Is().True());

        pq.push(1);
        AssertThat(pq.empty(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(3u));
        AssertThat(pq.has_top(), Is().True());
      });

      it("can peek smallest element", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(4));
      });

      it("Can pop smallest element", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(4));
        pq.pop();
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
      });

      it("can push a new element (> .top())", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
        pq.push(1);
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
      });

      it("can push a new element (< .top)()", [&pq]() {
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
        pq.push(3);
        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
      });

      it("can pop remaining elements", [&pq]() {
        AssertThat(pq.size(), Is().EqualTo(4u));

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(3));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(2));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(1));
        pq.pop();

        AssertThat(pq.has_top(), Is().True());
        AssertThat(pq.top(), Is().EqualTo(1));
        pq.pop();

        AssertThat(pq.has_top(), Is().False());
        AssertThat(pq.size(), Is().EqualTo(0u));
      });
    });
  });
});
