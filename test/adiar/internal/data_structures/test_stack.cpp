#include "../../../test.h"

#include <adiar/internal/data_structures/stack.h>

go_bandit([]() {
  describe("adiar/internal/sorter.h", []() {
    describe("sorter<memory_mode::Internal, int>", []() {
      stack<memory_mode::Internal, int> s(16);

      it("is initially empty", [&s]() {
        AssertThat(s.empty(), Is().True());
        AssertThat(s.size(), Is().EqualTo(0u));
      });

      it("can push elements", [&s]() {
        s.push(3);
        s.push(1);
        s.push(2);

        AssertThat(s.empty(), Is().False());
        AssertThat(s.size(), Is().EqualTo(3u));
      });

      it("can top latest element", [&s]() {
        AssertThat(s.top(), Is().EqualTo(2));
        AssertThat(s.size(), Is().EqualTo(3u));
      });

      it("can pull latest element", [&s]() {
        AssertThat(s.pull(), Is().EqualTo(2));
        AssertThat(s.size(), Is().EqualTo(2u));
      });

      it("can push a new element", [&s]() {
        s.push(4);
        AssertThat(s.top(), Is().EqualTo(4));
        AssertThat(s.size(), Is().EqualTo(3u));
      });

      it("can pop an element", [&s]() {
        s.pop();
        AssertThat(s.top(), Is().EqualTo(1));
        AssertThat(s.size(), Is().EqualTo(2u));
      });

      it("can pull remaining elements", [&s]() {
        AssertThat(s.pull(), Is().EqualTo(1));
        AssertThat(s.size(), Is().EqualTo(1u));
        AssertThat(s.pull(), Is().EqualTo(3));
        AssertThat(s.size(), Is().EqualTo(0u));
        AssertThat(s.empty(), Is().True());
      });
    });

    describe("sorter<memory_mode::External, int>", []() {
      // TODO: Add tests when implemented.
    });
  });
});
