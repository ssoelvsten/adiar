#include "../../../test.h"

#include <adiar/internal/data_structures/vector.h>

go_bandit([]() {
  describe("adiar/internal/vector.h", []() {
    describe("vector<memory_mode::Internal, int>", []() {
      vector<memory_mode::Internal, int> v(16);

      it("is initially empty", [&v]() {
        AssertThat(v.empty(), Is().True());
        AssertThat(v.size(), Is().EqualTo(0u));
      });

      it("can push elements", [&v]() {
        v.push_back(3);
        v.push_back(1);
        v.push_back(2);

        AssertThat(v.empty(), Is().False());
        AssertThat(v.size(), Is().EqualTo(3u));
      });

      it("can retrieve the front element", [&v]() { AssertThat(v.front(), Is().EqualTo(3)); });

      it("can retrieve the back element", [&v]() { AssertThat(v.back(), Is().EqualTo(2)); });

      it("can retrieve elements via '.at(i)'", [&v]() {
        AssertThat(v.at(0), Is().EqualTo(3));
        AssertThat(v.at(1), Is().EqualTo(1));
        AssertThat(v.at(2), Is().EqualTo(2));
      });

      it("can pop element", [&v]() {
        v.pop_back();

        AssertThat(v.empty(), Is().False());
        AssertThat(v.size(), Is().EqualTo(2u));
      });

      it("can retrieve different element from the back",
         [&v]() { AssertThat(v.back(), Is().EqualTo(1)); });

      it("can new elements", [&v]() {
        v.push_back(4);
        v.push_back(4);

        AssertThat(v.empty(), Is().False());
        AssertThat(v.size(), Is().EqualTo(4u));
      });

      it("can retrieve different element from the back",
         [&v]() { AssertThat(v.back(), Is().EqualTo(4)); });

      it("can retrieve elements via '[i]'", [&v]() {
        AssertThat(v[0], Is().EqualTo(3));
        AssertThat(v[1], Is().EqualTo(1));
        AssertThat(v[2], Is().EqualTo(4));
        AssertThat(v[3], Is().EqualTo(4));
      });

      it("can iterate forwards through the vector", [&v]() {
        int i                 = 0;
        const int expected[4] = { 3, 1, 4, 4 };

        for (auto iter = v.begin(); iter != v.end(); ++iter) {
          AssertThat(*iter, Is().EqualTo(expected[i]));
          i += 1;
        }
      });

      it("can iterate backwards through the vector", [&v]() {
        int i                 = 3;
        const int expected[4] = { 3, 1, 4, 4 };

        for (auto iter = v.rbegin(); iter != v.rend(); ++iter) {
          AssertThat(*iter, Is().EqualTo(expected[i]));
          i -= 1;
        }
      });
    });

    describe("vector<memory_mode::External, int>", []() {
      // TODO: Add tests when implemented.
    });
  });
});
