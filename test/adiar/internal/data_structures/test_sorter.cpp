#include "../../../test.h"

#include <adiar/internal/data_structures/sorter.h>

go_bandit([]() {
  describe("adiar/internal/sorter.h", []() {
    describe("sorter<memory_mode::Internal, int, std::less<>>", []() {
      sorter<memory_mode::Internal, int, std::less<>> s(1024, 16);

      it("is initially empty", [&s]() {
        AssertThat(s.empty(), Is().True());
        AssertThat(s.size(), Is().EqualTo(0u));
      });

      it("can push elements", [&s]() {
        AssertThat(s.can_push(), Is().True());
        s.push(2);
        s.push(4);
        s.push(3);
        s.push(1);
        s.push(4);
        s.push(6);
      });

      it("can sort elements", [&s]() {
        AssertThat(s.can_push(), Is().True());
        s.sort();
        AssertThat(s.can_push(), Is().False());
        AssertThat(s.can_pull(), Is().True());
      });

      it("can .top() first element", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(1));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(1));
      });

      it("can .pull() first element", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.pull(), Is().EqualTo(1));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(2));
      });

      it("can .top() and .pull() remaining elements", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(2));
        AssertThat(s.pull(), Is().EqualTo(2));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(3));
        AssertThat(s.pull(), Is().EqualTo(3));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(4));
        AssertThat(s.pull(), Is().EqualTo(4));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(4));
        AssertThat(s.pull(), Is().EqualTo(4));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(6));
        AssertThat(s.pull(), Is().EqualTo(6));
      });

      it("can .push() after .reset()", [&s]() {
        AssertThat(s.can_push(), Is().False());
        s.reset();
        AssertThat(s.can_push(), Is().True());
        s.push(2);
        s.push(1);
        s.sort();
        AssertThat(s.can_push(), Is().False());

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.pull(), Is().EqualTo(1));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.pull(), Is().EqualTo(2));

        AssertThat(s.can_pull(), Is().True());
      });
    });

    describe("sorter<memory_mode::External, int, std::less<>>", []() {
      sorter<memory_mode::External, int, std::less<>> s(8 * 1024 * 1024, 16);

      it("is initially empty", [&s]() {
        AssertThat(s.empty(), Is().True());
        AssertThat(s.size(), Is().EqualTo(0u));
      });

      it("can push elements", [&s]() {
        // AssertThat(s.can_push(), Is().True());
        s.push(2);
        s.push(4);
        s.push(3);
        s.push(1);
        s.push(4);
        s.push(6);
      });

      it("can sort elements", [&s]() {
        // AssertThat(s.can_push(), Is().True());
        s.sort();
        // AssertThat(s.can_push(), Is().False());
        AssertThat(s.can_pull(), Is().True());
      });

      it("can .top() first element", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(1));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(1));
      });

      it("can .pull() first element", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.pull(), Is().EqualTo(1));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(2));
      });

      it("can .top() and .pull() remaining elements", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(2));
        AssertThat(s.pull(), Is().EqualTo(2));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(3));
        AssertThat(s.pull(), Is().EqualTo(3));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(4));
        AssertThat(s.pull(), Is().EqualTo(4));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(4));
        AssertThat(s.pull(), Is().EqualTo(4));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(6));
        AssertThat(s.pull(), Is().EqualTo(6));
      });
    });

    describe("sorter<memory_mode::Internal, int, std::greater<>>", []() {
      sorter<memory_mode::Internal, int, std::greater<>> s(1024, 16);

      it("is initially empty", [&s]() {
        AssertThat(s.empty(), Is().True());
        AssertThat(s.size(), Is().EqualTo(0u));
      });

      it("can push elements", [&s]() {
        AssertThat(s.can_push(), Is().True());
        s.push(5);
        s.push(3);
        s.push(4);
        s.push(6);
        s.push(3);
        s.push(1);
      });

      it("can sort elements", [&s]() {
        AssertThat(s.can_push(), Is().True());
        s.sort();
        AssertThat(s.can_push(), Is().False());
        AssertThat(s.can_pull(), Is().True());
      });

      it("can .top() first element", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(6));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(6));
      });

      it("can .pull() first element", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.pull(), Is().EqualTo(6));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(5));
      });

      it("can .top() and .pull() remaining elements", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(5));
        AssertThat(s.pull(), Is().EqualTo(5));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(4));
        AssertThat(s.pull(), Is().EqualTo(4));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(3));
        AssertThat(s.pull(), Is().EqualTo(3));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(3));
        AssertThat(s.pull(), Is().EqualTo(3));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(1));
        AssertThat(s.pull(), Is().EqualTo(1));
      });

      it("can .push() after .reset()", [&s]() {
        AssertThat(s.can_push(), Is().False());
        s.reset();
        AssertThat(s.can_push(), Is().True());
        s.push(1);
        s.push(2);
        s.sort();
        AssertThat(s.can_push(), Is().False());

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.pull(), Is().EqualTo(2));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.pull(), Is().EqualTo(1));

        AssertThat(s.can_pull(), Is().True());
      });
    });

    describe("sorter<memory_mode::External, int, std::greater<>>", []() {
      sorter<memory_mode::Internal, int, std::greater<>> s(1024, 16);

      it("is initially empty", [&s]() {
        AssertThat(s.empty(), Is().True());
        AssertThat(s.size(), Is().EqualTo(0u));
      });

      it("can push elements", [&s]() {
        // AssertThat(s.can_push(), Is().True());
        s.push(5);
        s.push(3);
        s.push(4);
        s.push(6);
        s.push(3);
        s.push(1);
      });

      it("can sort elements", [&s]() {
        // AssertThat(s.can_push(), Is().True());
        s.sort();
        // AssertThat(s.can_push(), Is().False());
        AssertThat(s.can_pull(), Is().True());
      });

      it("can .top() first element", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(6));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(6));
      });

      it("can .pull() first element", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.pull(), Is().EqualTo(6));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(5));
      });

      it("can .top() and .pull() remaining elements", [&s]() {
        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(5));
        AssertThat(s.pull(), Is().EqualTo(5));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(4));
        AssertThat(s.pull(), Is().EqualTo(4));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(3));
        AssertThat(s.pull(), Is().EqualTo(3));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(3));
        AssertThat(s.pull(), Is().EqualTo(3));

        AssertThat(s.can_pull(), Is().True());
        AssertThat(s.top(), Is().EqualTo(1));
        AssertThat(s.pull(), Is().EqualTo(1));
      });
    });
  });
});
