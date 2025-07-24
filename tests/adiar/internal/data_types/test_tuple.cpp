#include "../../../test.h"

#include <adiar/internal/data_types/tuple.h>

go_bandit([]() {
  describe("adiar/internal/data_types/tuple.h", []() {
    describe("tuple<int, 1>", []() {
      it("statically provides its cardinality of 1.", []() {
        const auto cardinality = tuple<int, 1>::cardinality;
        AssertThat(cardinality, Is().EqualTo(1u));
      });

      it("is trivially sorted even though not specified.", []() {
        const auto is_sorted = tuple<int, 1>::is_sorted;
        AssertThat(is_sorted, Is().EqualTo(true));
      });

      describe("unordered access: '[]' operator", []() {
        it("provides unordered access [42]", []() {
          const tuple<int, 1> t = { 42 };
          AssertThat(t[0], Is().EqualTo(42));
        });

        it("provides unordered access [0]", []() {
          const tuple<int, 1> t = { 0 };
          AssertThat(t[0], Is().EqualTo(0));
        });

        it("provides unordered access [1]", []() {
          const tuple<int, 1> t = { 1 };
          AssertThat(t[0], Is().EqualTo(1));
        });

        it("provides unordered access [-1]", []() {
          const tuple<int, 1> t = { -1 };
          AssertThat(t[0], Is().EqualTo(-1));
        });
      });

      describe("ordered access: 'first' function", []() {
        it("provides ordered access to first element [42]", []() {
          const tuple<int, 1> t = { 42 };
          AssertThat(t.first(), Is().EqualTo(42));
        });

        it("provides ordered access to first element [0]", []() {
          const tuple<int, 1> t = { 0 };
          AssertThat(t.first(), Is().EqualTo(0));
        });

        it("provides ordered access to first element [1]", []() {
          const tuple<int, 1> t = { 1 };
          AssertThat(t.first(), Is().EqualTo(1));
        });

        it("provides ordered access to first element [-1]", []() {
          const tuple<int, 1> t = { -1 };
          AssertThat(t.first(), Is().EqualTo(-1));
        });
      });

      describe("Coordinate-wise Equality '=='", []() {
        it("claims {-1} == {-1}", []() {
          const tuple<int, 2> t1 = { -1 };
          const tuple<int, 2> t2 = { -1 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims  {0} == {0}", []() {
          const tuple<int, 2> t1 = { 0 };
          const tuple<int, 2> t2 = { 0 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims  {1} == {1}", []() {
          const tuple<int, 2> t1 = { 1 };
          const tuple<int, 2> t2 = { 1 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims {-1} != {1}", []() {
          const tuple<int, 2> t1 = { -1 };
          const tuple<int, 2> t2 = { 1 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });
      });

      describe("Lexicographical Ordering '<'", []() {
        it("claims  {0}  < {1}", []() {
          const tuple<int, 1> t1 = { 0 };
          const tuple<int, 1> t2 = { 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {-1}  < {0}", []() {
          const tuple<int, 1> t1 = { -0 };
          const tuple<int, 1> t2 = { 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {21}  < {42}", []() {
          const tuple<int, 1> t1 = { 21 };
          const tuple<int, 1> t2 = { 42 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {-1}  < {1}", []() {
          const tuple<int, 1> t1 = { -1 };
          const tuple<int, 1> t2 = { 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims  {0} !< {0}", []() {
          const tuple<int, 1> t1 = { 0 };
          const tuple<int, 1> t2 = { 0 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims  {1} !< {1}", []() {
          const tuple<int, 1> t1 = { 1 };
          const tuple<int, 1> t2 = { 1 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });
      });

      describe("tuple_first_lt", []() {
        it("claims  {0}  < {1}", []() {
          const tuple<int, 1> t1 = { 0 };
          const tuple<int, 1> t2 = { 1 };

          const bool lt_lr = tuple_first_lt<tuple<int, 1>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 1>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {-1}  < {1}", []() {
          const tuple<int, 1> t1 = { -1 };
          const tuple<int, 1> t2 = { 1 };

          const bool lt_lr = tuple_first_lt<tuple<int, 1>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 1>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {21}  < {42}", []() {
          const tuple<int, 1> t1 = { 21 };
          const tuple<int, 1> t2 = { 42 };

          const bool lt_lr = tuple_first_lt<tuple<int, 1>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 1>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {42} !< {42}", []() {
          const tuple<int, 1> t1 = { 42 };
          const tuple<int, 1> t2 = { 42 };

          const bool lt_lr = tuple_first_lt<tuple<int, 1>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_first_lt<tuple<int, 1>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims  {0} !< {0}", []() {
          const tuple<int, 1> t1 = { 0 };
          const tuple<int, 1> t2 = { 0 };

          const bool lt_lr = tuple_first_lt<tuple<int, 1>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_first_lt<tuple<int, 1>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });
    });

    describe("tuple<int, 2>", []() {
      it("statically provides its cardinality of 2.", []() {
        const auto cardinality = tuple<int, 2>::cardinality;
        AssertThat(cardinality, Is().EqualTo(2u));
      });

      it("is not trivially sorted.", []() {
        const auto is_sorted = tuple<int, 2>::is_sorted;
        AssertThat(is_sorted, Is().EqualTo(false));
      });

      describe("unordered access: '[]' operator", []() {
        it("provides unordered access [42,22]", []() {
          const tuple<int, 2> t = { 42, 22 };

          AssertThat(t[0], Is().EqualTo(42));
          AssertThat(t[1], Is().EqualTo(22));
        });

        it("provides unordered access [0, -1]", []() {
          const tuple<int, 2> t = { 0, -1 };

          AssertThat(t[0], Is().EqualTo(0));
          AssertThat(t[1], Is().EqualTo(-1));
        });
      });

      describe("ordered access: 'first' and 'second' function", []() {
        it("provides ordered access to first element", []() {
          const tuple<int, 2> t1 = { 21, 42 };
          AssertThat(t1.first(), Is().EqualTo(21));

          const tuple<int, 2> t2 = { 42, 21 };
          AssertThat(t2.first(), Is().EqualTo(21));
        });

        it("provides ordered access to first element [negative value]", []() {
          const tuple<int, 2> t = { -2, -1 };
          AssertThat(t.first(), Is().EqualTo(-2));
        });

        it("provides ordered access to first element [0 vs. 1]", []() {
          const tuple<int, 2> t1 = { 1, 0 };
          AssertThat(t1.first(), Is().EqualTo(0));

          const tuple<int, 2> t2 = { 0, 1 };
          AssertThat(t2.first(), Is().EqualTo(0));
        });

        it("provides ordered access to first element [tie]", []() {
          const tuple<int, 2> t = { 21, 21 };
          AssertThat(t.first(), Is().EqualTo(21));
        });

        it("provides ordered access to second element", []() {
          const tuple<int, 2> t1 = { 21, 42 };
          AssertThat(t1.second(), Is().EqualTo(42));

          const tuple<int, 2> t2 = { 42, 21 };
          AssertThat(t2.second(), Is().EqualTo(42));
        });

        it("provides ordered access to second element [negative values]", []() {
          const tuple<int, 2> t = { -2, -1 };
          AssertThat(t.second(), Is().EqualTo(-1));
        });

        it("provides ordered access to second element [0 vs. 1]", []() {
          const tuple<int, 2> t1 = { 1, 0 };
          AssertThat(t1.second(), Is().EqualTo(1));

          const tuple<int, 2> t2 = { 0, 1 };
          AssertThat(t2.second(), Is().EqualTo(1));
        });

        it("provides ordered access to second element [tie]", []() {
          const tuple<int, 2> t = { 21, 21 };
          AssertThat(t.second(), Is().EqualTo(21));
        });
      });

      it("can construct a tuple with copies of a single element [-1]", []() {
        const tuple<int, 2> t = { -1 };

        AssertThat(t[0], Is().EqualTo(-1));
        AssertThat(t[1], Is().EqualTo(-1));
      });

      it("can construct a tuple with copies of a single element [0]", []() {
        const tuple<int, 2> t = { 0 };

        AssertThat(t[0], Is().EqualTo(0));
        AssertThat(t[1], Is().EqualTo(0));
      });

      it("can construct a tuple with copies of a single element [1]", []() {
        const tuple<int, 2> t = { 1 };

        AssertThat(t[0], Is().EqualTo(1));
        AssertThat(t[1], Is().EqualTo(1));
      });

      it("can construct a tuple with copies of a single element [42]", []() {
        const tuple<int, 2> t = { 42 };

        AssertThat(t[0], Is().EqualTo(42));
        AssertThat(t[1], Is().EqualTo(42));
      });

      it("can be constructed from a std::array", []() {
        const std::array<int, 2> arr = { 8, 2 };
        const tuple<int, 2> t(arr);

        AssertThat(t[0], Is().EqualTo(8));
        AssertThat(t[1], Is().EqualTo(2));
      });

      describe("Coordinate-wise Equality '=='", []() {
        it("claims   {0, 0} == {0, 0}", []() {
          const tuple<int, 2> t1 = { 0, 0 };
          const tuple<int, 2> t2 = { 0, 0 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims   {1, 0} == {1, 0}", []() {
          const tuple<int, 2> t1 = { 1, 0 };
          const tuple<int, 2> t2 = { 1, 0 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims   {1, 0} != {0, 1}", []() {
          const tuple<int, 2> t1 = { 1, 0 };
          const tuple<int, 2> t2 = { 0, 1 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });

        it("claims {42, 21} != {42, 20}", []() {
          const tuple<int, 2> t1 = { 42, 21 };
          const tuple<int, 2> t2 = { 42, 20 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });
      });

      describe("Lexicographical Ordering '<'", []() {
        it("claims   {0, 2}  < {1, 2}", []() {
          const tuple<int, 2> t1 = { 0, 2 };
          const tuple<int, 2> t2 = { 1, 2 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims   {1, 2}  < {2, 0}", []() {
          const tuple<int, 2> t1 = { 0, 2 };
          const tuple<int, 2> t2 = { 1, 0 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims   {0, 0}  < {0, 1}", []() {
          const tuple<int, 2> t1 = { 0, 0 };
          const tuple<int, 2> t2 = { 0, 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims   {0, 0}  < {0, 1}", []() {
          const tuple<int, 2> t1 = { 0, 0 };
          const tuple<int, 2> t2 = { 0, 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims   {0, 0} !< {0, 0}", []() {
          const tuple<int, 2> t1 = { 0, 0 };
          const tuple<int, 2> t2 = { 0, 0 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims   {1, 0} !< {1, 0}", []() {
          const tuple<int, 2> t1 = { 1, 0 };
          const tuple<int, 2> t2 = { 1, 0 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {21, 42} !< {21, 42}", []() {
          const tuple<int, 2> t1 = { 21, 42 };
          const tuple<int, 2> t2 = { 21, 42 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });
      });

      describe("tuple_first_lt", []() {
        it("claims   {0, 2}  < {1, 2}", []() {
          const tuple<int, 2> t1 = { 0, 2 };
          const tuple<int, 2> t2 = { 1, 2 };

          const bool lt_lr = tuple_first_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {2, 0}  < {1, 2}", []() {
          const tuple<int, 2> t1 = { 2, 0 };
          const tuple<int, 2> t2 = { 1, 2 };

          const bool lt_lr = tuple_first_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {0, 1}  < {2, 0}", []() {
          const tuple<int, 2> t1 = { 0, 1 };
          const tuple<int, 2> t2 = { 2, 0 };

          const bool lt_lr = tuple_first_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {0, 0} !< {0, 0}", []() {
          const tuple<int, 2> t1 = { 0, 0 };
          const tuple<int, 2> t2 = { 0, 0 };

          const bool lt_lr = tuple_first_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_first_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0} !< {1, 0}", []() {
          const tuple<int, 2> t1 = { 1, 0 };
          const tuple<int, 2> t2 = { 1, 0 };

          const bool lt_lr = tuple_first_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_first_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {42, 21} !< {42, 21}", []() {
          const tuple<int, 2> t1 = { 42, 21 };
          const tuple<int, 2> t2 = { 42, 21 };

          const bool lt_lr = tuple_first_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_first_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });

      describe("tuple_second_lt", []() {
        it("claims   {1, 0}  < {2, 0}", []() {
          const tuple<int, 2> t1 = { 1, 0 };
          const tuple<int, 2> t2 = { 2, 0 };

          const bool lt_lr = tuple_second_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {2, 1}  < {3, 0}", []() {
          const tuple<int, 2> t1 = { 2, 1 };
          const tuple<int, 2> t2 = { 3, 0 };

          const bool lt_lr = tuple_second_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {2, 3}  < {3, 1}", []() {
          const tuple<int, 2> t1 = { 2, 3 };
          const tuple<int, 2> t2 = { 3, 1 };

          const bool lt_lr = tuple_second_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims  {2, -1}  < {2, 0}", []() {
          const tuple<int, 2> t1 = { 2, -1 };
          const tuple<int, 2> t2 = { 2, 0 };

          const bool lt_lr = tuple_second_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {0, 0} !< {0, 0}", []() {
          const tuple<int, 2> t1 = { 0, 0 };
          const tuple<int, 2> t2 = { 0, 0 };

          const bool lt_lr = tuple_second_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_second_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0} !< {1, 0}", []() {
          const tuple<int, 2> t1 = { 1, 0 };
          const tuple<int, 2> t2 = { 1, 0 };

          const bool lt_lr = tuple_second_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_second_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {42, 21} !< {42, 21}", []() {
          const tuple<int, 2> t1 = { 42, 21 };
          const tuple<int, 2> t2 = { 42, 21 };

          const bool lt_lr = tuple_second_lt<tuple<int, 2>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_second_lt<tuple<int, 2>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });
    });

    describe("tuple<int, 2, sorted>", []() {
      // TODO
    });

    describe("tuple<int, 3>", []() {
      it("statically provides its cardinality of 3.", []() {
        const auto cardinality = tuple<int, 3>::cardinality;
        AssertThat(cardinality, Is().EqualTo(3u));
      });

      it("is not trivially sorted.", []() {
        const auto is_sorted = tuple<int, 3>::is_sorted;
        AssertThat(is_sorted, Is().EqualTo(false));
      });

      describe("unordered access: '[]' operator", []() {
        it("provides unordered access [42, 22, -21]", []() {
          const tuple<int, 3> t = { 42, 22, -21 };

          AssertThat(t[0], Is().EqualTo(42));
          AssertThat(t[1], Is().EqualTo(22));
          AssertThat(t[2], Is().EqualTo(-21));
        });

        it("provides unordered access [0, -1, 1]", []() {
          const tuple<int, 3> t = { 0, -1, 1 };

          AssertThat(t[0], Is().EqualTo(0));
          AssertThat(t[1], Is().EqualTo(-1));
          AssertThat(t[2], Is().EqualTo(1));
        });
      });

      describe("ordered access: 'first' 'second', and 'third' functions", []() {
        it("provides ordered access to first element", []() {
          const tuple<int, 3> t1 = { 21, 42, 22 };
          AssertThat(t1.first(), Is().EqualTo(21));

          const tuple<int, 3> t2 = { 42, 21, 22 };
          AssertThat(t2.first(), Is().EqualTo(21));

          const tuple<int, 3> t3 = { 42, 22, 21 };
          AssertThat(t3.first(), Is().EqualTo(21));
        });

        it("provides ordered access to first element [negative value]", []() {
          const tuple<int, 3> t = { -2, -1, -3 };
          AssertThat(t.first(), Is().EqualTo(-3));
        });

        it("provides ordered access to first element [0 vs. 1]", []() {
          const tuple<int, 3> t1 = { 1, 0, 1 };
          AssertThat(t1.first(), Is().EqualTo(0));

          const tuple<int, 3> t2 = { 0, 1, 1 };
          AssertThat(t2.first(), Is().EqualTo(0));

          const tuple<int, 3> t3 = { 1, 1, 0 };
          AssertThat(t3.first(), Is().EqualTo(0));
        });

        it("provides ordered access to first element [tie]", []() {
          const tuple<int, 3> t1 = { 42, 21, 21 };
          AssertThat(t1.first(), Is().EqualTo(21));

          const tuple<int, 3> t2 = { 22, 41, 22 };
          AssertThat(t2.first(), Is().EqualTo(22));

          const tuple<int, 3> t3 = { 20, 20, 40 };
          AssertThat(t3.first(), Is().EqualTo(20));

          const tuple<int, 3> t4 = { 16, 16, 16 };
          AssertThat(t4.first(), Is().EqualTo(16));
        });

        it("provides ordered access to second element", []() {
          const tuple<int, 3> t1 = { 21, 42, 22 };
          AssertThat(t1.second(), Is().EqualTo(22));

          const tuple<int, 3> t2 = { 42, 21, 22 };
          AssertThat(t2.second(), Is().EqualTo(22));

          const tuple<int, 3> t3 = { 42, 22, 21 };
          AssertThat(t3.second(), Is().EqualTo(22));
        });

        it("provides ordered access to second element [negative values]", []() {
          const tuple<int, 3> t1 = { -2, -1, -3 };
          AssertThat(t1.second(), Is().EqualTo(-2));

          const tuple<int, 3> t2 = { -1, -2, -3 };
          AssertThat(t2.second(), Is().EqualTo(-2));

          const tuple<int, 3> t3 = { -3, -1, -2 };
          AssertThat(t3.second(), Is().EqualTo(-2));
        });

        it("provides ordered access to second element [0, -1 vs. 1]", []() {
          const tuple<int, 3> t1 = { 0, -1, 1 };
          AssertThat(t1.second(), Is().EqualTo(0));

          const tuple<int, 3> t2 = { -1, 0, 1 };
          AssertThat(t2.second(), Is().EqualTo(0));

          const tuple<int, 3> t3 = { 1, -1, 0 };
          AssertThat(t3.second(), Is().EqualTo(0));
        });

        it("provides ordered access to second element [tie]", []() {
          const tuple<int, 3> t1 = { 0, 1, 1 };
          AssertThat(t1.second(), Is().EqualTo(1));

          const tuple<int, 3> t2 = { 1, 0, 1 };
          AssertThat(t2.second(), Is().EqualTo(1));

          const tuple<int, 3> t3 = { 1, 1, 0 };
          AssertThat(t3.second(), Is().EqualTo(1));

          const tuple<int, 3> t4 = { 1, 1, 1 };
          AssertThat(t4.second(), Is().EqualTo(1));
        });

        it("provides ordered access to third element", []() {
          const tuple<int, 3> t1 = { 21, 42, 22 };
          AssertThat(t1.third(), Is().EqualTo(42));

          const tuple<int, 3> t2 = { 42, 21, 22 };
          AssertThat(t2.third(), Is().EqualTo(42));

          const tuple<int, 3> t3 = { 21, 22, 42 };
          AssertThat(t3.third(), Is().EqualTo(42));
        });

        it("provides ordered access to third element [negative values]", []() {
          const tuple<int, 3> t1 = { -2, -1, -3 };
          AssertThat(t1.third(), Is().EqualTo(-1));

          const tuple<int, 3> t2 = { -1, -2, -3 };
          AssertThat(t2.third(), Is().EqualTo(-1));

          const tuple<int, 3> t3 = { -3, -2, -1 };
          AssertThat(t3.third(), Is().EqualTo(-1));
        });

        it("provides ordered access to third element [0, -1 vs. 1]", []() {
          const tuple<int, 3> t1 = { 0, -1, 1 };
          AssertThat(t1.third(), Is().EqualTo(1));

          const tuple<int, 3> t2 = { -1, 1, 0 };
          AssertThat(t2.third(), Is().EqualTo(1));

          const tuple<int, 3> t3 = { 1, -1, 0 };
          AssertThat(t3.third(), Is().EqualTo(1));
        });

        it("provides ordered access to third element [tie for 'second()']", []() {
          const tuple<int, 3> t1 = { 0, 0, 1 };
          AssertThat(t1.third(), Is().EqualTo(1));

          const tuple<int, 3> t2 = { 1, 0, 0 };
          AssertThat(t2.third(), Is().EqualTo(1));

          const tuple<int, 3> t3 = { 0, 0, 1 };
          AssertThat(t3.third(), Is().EqualTo(1));
        });

        it("provides ordered access to third element [tie for 'third()']", []() {
          const tuple<int, 3> t1 = { 0, 1, 1 };
          AssertThat(t1.third(), Is().EqualTo(1));

          const tuple<int, 3> t2 = { 1, 1, 0 };
          AssertThat(t2.third(), Is().EqualTo(1));

          const tuple<int, 3> t3 = { 0, 1, 1 };
          AssertThat(t3.third(), Is().EqualTo(1));

          const tuple<int, 3> t4 = { 1, 1, 1 };
          AssertThat(t4.third(), Is().EqualTo(1));
        });
      });

      it("can construct a tuple with copies of a single element [-1]", []() {
        const tuple<int, 3> t = { -1 };

        AssertThat(t[0], Is().EqualTo(-1));
        AssertThat(t[1], Is().EqualTo(-1));
        AssertThat(t[2], Is().EqualTo(-1));
      });

      it("can construct a tuple with copies of a single element [0]", []() {
        const tuple<int, 3> t = { 0 };

        AssertThat(t[0], Is().EqualTo(0));
        AssertThat(t[1], Is().EqualTo(0));
        AssertThat(t[2], Is().EqualTo(0));
      });

      it("can construct a tuple with copies of a single element [1]", []() {
        const tuple<int, 3> t = { 1 };

        AssertThat(t[0], Is().EqualTo(1));
        AssertThat(t[1], Is().EqualTo(1));
        AssertThat(t[2], Is().EqualTo(1));
      });

      it("can construct a tuple with copies of a single element [42]", []() {
        const tuple<int, 3> t = { 42 };

        AssertThat(t[0], Is().EqualTo(42));
        AssertThat(t[1], Is().EqualTo(42));
        AssertThat(t[2], Is().EqualTo(42));
      });

      it("can be constructed from a std::array", []() {
        const std::array<int, 3> arr = { 8, 2, 5 };
        const tuple<int, 3> t(arr);

        AssertThat(t[0], Is().EqualTo(8));
        AssertThat(t[1], Is().EqualTo(2));
        AssertThat(t[2], Is().EqualTo(5));
      });

      describe("Coordinate-wise Equality '=='", []() {
        it("claims    {0, 0, 0} == {0, 0, 0}", []() {
          const tuple<int, 3> t1 = { 0, 0, 0 };
          const tuple<int, 3> t2 = { 0, 0, 0 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims    {1, 0, 1} == {1, 0, 1}", []() {
          const tuple<int, 3> t1 = { 1, 0, 1 };
          const tuple<int, 3> t2 = { 1, 0, 1 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims    {1, 1, 2} != {0, 1, 2}", []() {
          const tuple<int, 3> t1 = { 1, 1, 2 };
          const tuple<int, 3> t2 = { 0, 1, 2 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });

        it("claims    {1, 0, 1} != {1, 1, 1}", []() {
          const tuple<int, 3> t1 = { 1, 0, 1 };
          const tuple<int, 3> t2 = { 1, 1, 1 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });

        it("claims    {2, 1, 0} != {2, 1, 1}", []() {
          const tuple<int, 3> t1 = { 2, 1, 0 };
          const tuple<int, 3> t2 = { 2, 1, 1 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });

        it("claims {42, 21, 20} != {42, 20, 20}", []() {
          const tuple<int, 3> t1 = { 42, 21, 20 };
          const tuple<int, 3> t2 = { 42, 20, 20 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });
      });

      describe("Lexicographical Ordering '<'", []() {
        it("claims    {0, 2, 1}  < {1, 2, 1}", []() {
          const tuple<int, 3> t1 = { 0, 2, 1 };
          const tuple<int, 3> t2 = { 1, 2, 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims    {1, 2, 1}  < {2, 0, 0}", []() {
          const tuple<int, 3> t1 = { 0, 2, 1 };
          const tuple<int, 3> t2 = { 1, 0, 0 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims    {0, 0, 0}  < {0, 1, 0}", []() {
          const tuple<int, 3> t1 = { 0, 0, 0 };
          const tuple<int, 3> t2 = { 0, 1, 0 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims    {0, 0, 0}  < {0, 0, 1}", []() {
          const tuple<int, 3> t1 = { 0, 0, 0 };
          const tuple<int, 3> t2 = { 0, 0, 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims    {0, 1, 0}  < {0, 1, 1}", []() {
          const tuple<int, 3> t1 = { 0, 1, 0 };
          const tuple<int, 3> t2 = { 0, 1, 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims    {0, 0, 0} !< {0, 0, 0}", []() {
          const tuple<int, 3> t1 = { 0, 0, 0 };
          const tuple<int, 3> t2 = { 0, 0, 0 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims    {1, 0, 1} !< {1, 0, 1}", []() {
          const tuple<int, 3> t1 = { 1, 0, 1 };
          const tuple<int, 3> t2 = { 1, 0, 1 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {21, 42, 16} !< {21, 42, 16}", []() {
          const tuple<int, 3> t1 = { 21, 42, 16 };
          const tuple<int, 3> t2 = { 21, 42, 16 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });
      });

      describe("tuple_first_lt", []() {
        it("claims    {0, 2, 2}  < {1, 2, 2}", []() {
          const tuple<int, 3> t1 = { 0, 2, 2 };
          const tuple<int, 3> t2 = { 1, 2, 0 };

          const bool lt_lr = tuple_first_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims    {2, 0, 1}  < {1, 2, 1}", []() {
          const tuple<int, 3> t1 = { 2, 0, 1 };
          const tuple<int, 3> t2 = { 1, 2, 1 };

          const bool lt_lr = tuple_first_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims    {0, 1, 0}  < {2, 0, 0}", []() {
          const tuple<int, 3> t1 = { 0, 1, 0 };
          const tuple<int, 3> t2 = { 2, 0, 0 };

          const bool lt_lr = tuple_first_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims    {2, 0, 0}  < {2, 0, 1}", []() {
          const tuple<int, 3> t1 = { 2, 0, 0 };
          const tuple<int, 3> t2 = { 2, 0, 1 };

          const bool lt_lr = tuple_first_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims    {0, 0, 0} !< {0, 0, 0}", []() {
          const tuple<int, 3> t1 = { 0, 0, 0 };
          const tuple<int, 3> t2 = { 0, 0, 0 };

          const bool lt_lr = tuple_first_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_first_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims    {1, 0, 1} !< {1, 0, 1}", []() {
          const tuple<int, 3> t1 = { 1, 0, 1 };
          const tuple<int, 3> t2 = { 1, 0, 1 };

          const bool lt_lr = tuple_first_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_first_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });

      describe("tuple_second_lt", []() {
        it("claims   {1, 0, 4}  < {2, 0, 4}", []() {
          const tuple<int, 3> t1 = { 1, 0, 4 };
          const tuple<int, 3> t2 = { 2, 0, 4 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0, 0}  < {2, 0, 1}", []() {
          const tuple<int, 3> t1 = { 1, 0, 0 };
          const tuple<int, 3> t2 = { 2, 0, 1 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0, 0}  < {2, 0, 0}", []() {
          const tuple<int, 3> t1 = { 1, 0, 0 };
          const tuple<int, 3> t2 = { 2, 0, 0 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {2, 1, 5}  < {3, 0, 4}", []() {
          const tuple<int, 3> t1 = { 2, 1, 5 };
          const tuple<int, 3> t2 = { 3, 0, 4 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {2, 3, 4}  < {3, 1, 2}", []() {
          const tuple<int, 3> t1 = { 2, 3, 4 };
          const tuple<int, 3> t2 = { 3, 1, 3 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims  {2, -1, 2}  < {2, 0, 2}", []() {
          const tuple<int, 3> t1 = { 2, -1, 2 };
          const tuple<int, 3> t2 = { 2, 0, 2 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {2, 1, 3}  < {2, 1, 4}", []() {
          const tuple<int, 3> t1 = { 2, 1, 3 };
          const tuple<int, 3> t2 = { 2, 1, 4 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {0, 0, 0} !< {0, 0, 0}", []() {
          const tuple<int, 3> t1 = { 0, 0, 0 };
          const tuple<int, 3> t2 = { 0, 0, 0 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0, 1} !< {1, 0, 1}", []() {
          const tuple<int, 3> t1 = { 1, 0, 1 };
          const tuple<int, 3> t2 = { 1, 0, 1 };

          const bool lt_lr = tuple_second_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_second_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });

      describe("tuple_third_lt", []() {
        it("claims   {2, 0, 1}  < {4, 1, 0}", []() {
          const tuple<int, 3> t1 = { 2, 0, 0 };
          const tuple<int, 3> t2 = { 4, 1, 0 };

          const bool lt_lr = tuple_third_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 2, 1}  < {1, 3, 1}", []() {
          const tuple<int, 3> t1 = { 1, 2, 1 };
          const tuple<int, 3> t2 = { 1, 3, 1 };

          const bool lt_lr = tuple_third_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0, 4}  < {1, 0, 5}", []() {
          const tuple<int, 3> t1 = { 1, 0, 4 };
          const tuple<int, 3> t2 = { 1, 0, 5 };

          const bool lt_lr = tuple_third_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0, 4}  < {2, 0, 4}", []() {
          const tuple<int, 3> t1 = { 1, 0, 4 };
          const tuple<int, 3> t2 = { 2, 0, 4 };

          const bool lt_lr = tuple_third_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0, 3}  < {1, 1, 3}", []() {
          const tuple<int, 3> t1 = { 1, 0, 4 };
          const tuple<int, 3> t2 = { 1, 1, 4 };

          const bool lt_lr = tuple_third_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 4, 2}  < {1, 4, 3}", []() {
          const tuple<int, 3> t1 = { 1, 4, 2 };
          const tuple<int, 3> t2 = { 1, 4, 3 };

          const bool lt_lr = tuple_third_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {0, 0, 0} !< {0, 0, 0}", []() {
          const tuple<int, 3> t1 = { 0, 0, 0 };
          const tuple<int, 3> t2 = { 0, 0, 0 };

          const bool lt_lr = tuple_third_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_third_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims   {1, 0, 1} !< {1, 0, 1}", []() {
          const tuple<int, 3> t1 = { 1, 0, 1 };
          const tuple<int, 3> t2 = { 1, 0, 1 };

          const bool lt_lr = tuple_third_lt<tuple<int, 3>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_third_lt<tuple<int, 3>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });
    });

    describe("tuple<int, 3, sorted>", []() {
      // TODO
    });

    describe("tuple<int, 4, unsorted>", []() {
      it("statically provides its cardinality of 4.", []() {
        const auto cardinality = tuple<int, 4>::cardinality;
        AssertThat(cardinality, Is().EqualTo(4u));
      });

      it("is not trivially sorted.", []() {
        const auto is_sorted = tuple<int, 4>::is_sorted;
        AssertThat(is_sorted, Is().EqualTo(false));
      });

      describe("unordered access: '[]' operator", []() {
        it("provides unordered access [42, 22, 7, -21]", []() {
          const tuple<int, 4> t = { 42, 22, 7, -21 };

          AssertThat(t[0], Is().EqualTo(42));
          AssertThat(t[1], Is().EqualTo(22));
          AssertThat(t[2], Is().EqualTo(7));
          AssertThat(t[3], Is().EqualTo(-21));
        });

        it("provides unordered access [0, -1, 1, -2]", []() {
          const tuple<int, 4> t = { 0, -1, 1, -2 };

          AssertThat(t[0], Is().EqualTo(0));
          AssertThat(t[1], Is().EqualTo(-1));
          AssertThat(t[2], Is().EqualTo(1));
          AssertThat(t[3], Is().EqualTo(-2));
        });
      });

      describe("ordered access: 'first' 'second', 'third', and 'fourth' functions", []() {
        it("provides ordered access to first element", []() {
          const tuple<int, 4> t1 = { 21, 42, 22, 32 };
          AssertThat(t1.first(), Is().EqualTo(21));

          const tuple<int, 4> t2 = { 42, 21, 22, 32 };
          AssertThat(t2.first(), Is().EqualTo(21));

          const tuple<int, 4> t3 = { 42, 22, 21, 32 };
          AssertThat(t3.first(), Is().EqualTo(21));

          const tuple<int, 4> t4 = { 42, 22, 32, 21 };
          AssertThat(t4.first(), Is().EqualTo(21));
        });

        it("provides ordered access to first element [negative value]", []() {
          const tuple<int, 4> t = { -2, -1, -4, -3 };
          AssertThat(t.first(), Is().EqualTo(-4));
        });

        it("provides ordered access to first element [0 vs. 1]", []() {
          const tuple<int, 4> t1 = { 0, 1, 1, 1 };
          AssertThat(t1.first(), Is().EqualTo(0));

          const tuple<int, 4> t2 = { 1, 0, 1, 1 };
          AssertThat(t2.first(), Is().EqualTo(0));

          const tuple<int, 4> t3 = { 1, 1, 0, 1 };
          AssertThat(t3.first(), Is().EqualTo(0));

          const tuple<int, 4> t4 = { 1, 1, 1, 0 };
          AssertThat(t4.first(), Is().EqualTo(0));
        });

        it("provides ordered access to first element [tie]", []() {
          const tuple<int, 4> t1 = { 42, 21, 21, 32 };
          AssertThat(t1.first(), Is().EqualTo(21));

          const tuple<int, 4> t2 = { 22, 41, 22, 32 };
          AssertThat(t2.first(), Is().EqualTo(22));

          const tuple<int, 4> t3 = { 20, 32, 40, 20 };
          AssertThat(t3.first(), Is().EqualTo(20));

          const tuple<int, 4> t4 = { 16, 16, 16, 16 };
          AssertThat(t4.first(), Is().EqualTo(16));
        });

        it("provides ordered access to second element", []() {
          const tuple<int, 4> t1 = { 22, 42, 21, 32 };
          AssertThat(t1.second(), Is().EqualTo(22));

          const tuple<int, 4> t2 = { 42, 22, 21, 32 };
          AssertThat(t2.second(), Is().EqualTo(22));

          const tuple<int, 4> t3 = { 42, 21, 22, 32 };
          AssertThat(t3.second(), Is().EqualTo(22));

          const tuple<int, 4> t4 = { 42, 21, 32, 22 };
          AssertThat(t4.second(), Is().EqualTo(22));
        });

        it("provides ordered access to second element [negative value]", []() {
          const tuple<int, 4> t = { -2, -1, -4, -3 };
          AssertThat(t.second(), Is().EqualTo(-3));
        });

        it("provides ordered access to second element [tie]", []() {
          const tuple<int, 4> t1 = { 42, 21, 32, 32 };
          AssertThat(t1.second(), Is().EqualTo(32));

          const tuple<int, 4> t2 = { 22, 41, 22, 32 };
          AssertThat(t2.second(), Is().EqualTo(22));

          const tuple<int, 4> t3 = { 20, 32, 20, 20 };
          AssertThat(t3.second(), Is().EqualTo(20));

          const tuple<int, 4> t4 = { 16, 16, 16, 16 };
          AssertThat(t4.second(), Is().EqualTo(16));
        });

        it("provides ordered access to second element [tie on third/fourth]", []() {
          const tuple<int, 4> t1 = { 42, 21, 22, 42 };
          AssertThat(t1.second(), Is().EqualTo(22));
        });

        it("provides ordered access to third element", []() {
          const tuple<int, 4> t1 = { 32, 21, 42, 22 };
          AssertThat(t1.third(), Is().EqualTo(32));

          const tuple<int, 4> t2 = { 21, 32, 42, 22 };
          AssertThat(t2.third(), Is().EqualTo(32));

          const tuple<int, 4> t3 = { 21, 42, 32, 22 };
          AssertThat(t3.third(), Is().EqualTo(32));

          const tuple<int, 4> t4 = { 21, 42, 22, 32 };
          AssertThat(t4.third(), Is().EqualTo(32));
        });

        it("provides ordered access to third element [negative value]", []() {
          const tuple<int, 4> t = { -2, -1, -4, -3 };
          AssertThat(t.third(), Is().EqualTo(-2));
        });

        it("provides ordered access to third element [tie]", []() {
          const tuple<int, 4> t1 = { 42, 21, 32, 32 };
          AssertThat(t1.third(), Is().EqualTo(32));

          const tuple<int, 4> t2 = { 22, 22, 41, 22 };
          AssertThat(t2.third(), Is().EqualTo(22));

          const tuple<int, 4> t4 = { 16, 16, 16, 16 };
          AssertThat(t4.third(), Is().EqualTo(16));
        });

        it("provides ordered access to third element [tie on first/second]", []() {
          const tuple<int, 4> t2 = { 22, 41, 22, 32 };
          AssertThat(t2.third(), Is().EqualTo(32));
        });

        it("provides ordered access to fourth element", []() {
          const tuple<int, 4> t1 = { 42, 21, 22, 32 };
          AssertThat(t1.fourth(), Is().EqualTo(42));

          const tuple<int, 4> t2 = { 21, 42, 22, 32 };
          AssertThat(t2.fourth(), Is().EqualTo(42));

          const tuple<int, 4> t3 = { 21, 22, 42, 32 };
          AssertThat(t3.fourth(), Is().EqualTo(42));

          const tuple<int, 4> t4 = { 21, 22, 32, 42 };
          AssertThat(t4.fourth(), Is().EqualTo(42));
        });

        it("provides ordered access to fourth element [negative value]", []() {
          const tuple<int, 4> t = { -2, -1, -4, -3 };
          AssertThat(t.fourth(), Is().EqualTo(-1));
        });

        it("provides ordered access to fourth element [0 vs. 1]", []() {
          const tuple<int, 4> t1 = { 1, 0, 0, 0 };
          AssertThat(t1.fourth(), Is().EqualTo(1));

          const tuple<int, 4> t2 = { 0, 1, 0, 0 };
          AssertThat(t2.fourth(), Is().EqualTo(1));

          const tuple<int, 4> t3 = { 0, 0, 1, 0 };
          AssertThat(t3.fourth(), Is().EqualTo(1));

          const tuple<int, 4> t4 = { 0, 0, 0, 1 };
          AssertThat(t4.fourth(), Is().EqualTo(1));
        });

        it("provides ordered access to fourth element [tie]", []() {
          const tuple<int, 4> t1 = { 42, 42, 21, 32 };
          AssertThat(t1.fourth(), Is().EqualTo(42));

          const tuple<int, 4> t2 = { 42, 21, 42, 32 };
          AssertThat(t2.fourth(), Is().EqualTo(42));

          const tuple<int, 4> t3 = { 42, 21, 32, 42 };
          AssertThat(t3.fourth(), Is().EqualTo(42));

          const tuple<int, 4> t4 = { 21, 42, 32, 42 };
          AssertThat(t4.fourth(), Is().EqualTo(42));

          const tuple<int, 4> t5 = { 21, 32, 42, 42 };
          AssertThat(t5.fourth(), Is().EqualTo(42));
        });
      });

      it("can construct a tuple with copies of a single element [-1]", []() {
        const tuple<int, 4> t = { -1 };

        AssertThat(t[0], Is().EqualTo(-1));
        AssertThat(t[1], Is().EqualTo(-1));
        AssertThat(t[2], Is().EqualTo(-1));
        AssertThat(t[3], Is().EqualTo(-1));
      });

      it("can construct a tuple with copies of a single element [0]", []() {
        const tuple<int, 4> t = { 0 };

        AssertThat(t[0], Is().EqualTo(0));
        AssertThat(t[1], Is().EqualTo(0));
        AssertThat(t[2], Is().EqualTo(0));
        AssertThat(t[3], Is().EqualTo(0));
      });

      it("can construct a tuple with copies of a single element [1]", []() {
        const tuple<int, 4> t = { 1 };

        AssertThat(t[0], Is().EqualTo(1));
        AssertThat(t[1], Is().EqualTo(1));
        AssertThat(t[2], Is().EqualTo(1));
        AssertThat(t[3], Is().EqualTo(1));
      });

      it("can construct a tuple with copies of a single element [42]", []() {
        const tuple<int, 4> t = { 42 };

        AssertThat(t[0], Is().EqualTo(42));
        AssertThat(t[1], Is().EqualTo(42));
        AssertThat(t[2], Is().EqualTo(42));
        AssertThat(t[3], Is().EqualTo(42));
      });

      it("can be constructed from a std::array", []() {
        const std::array<int, 4> arr = { 8, 2, 5, 3 };
        const tuple<int, 4> t(arr);

        AssertThat(t[0], Is().EqualTo(8));
        AssertThat(t[1], Is().EqualTo(2));
        AssertThat(t[2], Is().EqualTo(5));
        AssertThat(t[3], Is().EqualTo(3));
      });

      describe("Coordinate-wise Equality '=='", []() {
        it("claims {0, 0, 0, 0} == {0, 0, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 0, 0 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims {1, 2, 3, 4} == {1, 2, 3, 4}", []() {
          const tuple<int, 4> t1 = { 1, 2, 3, 4 };
          const tuple<int, 4> t2 = { 1, 2, 3, 4 };

          AssertThat(t1, Is().EqualTo(t2));
          AssertThat(t2, Is().EqualTo(t1));
        });

        it("claims {0, 0, 0, 0} != {1, 0, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 1, 0, 0, 0 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });

        it("claims {0, 0, 0, 0} != {0, 1, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 1, 0, 0 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });

        it("claims {0, 0, 0, 0} != {0, 0, 1, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 1, 0 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });

        it("claims {0, 0, 0, 0} != {0, 0, 0, 1}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 0, 1 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });

        it("claims {4, 8, 3, 7} != {4, 8, 4, 7}", []() {
          const tuple<int, 4> t1 = { 4, 8, 3, 7 };
          const tuple<int, 4> t2 = { 4, 8, 4, 7 };

          AssertThat(t1, Is().Not().EqualTo(t2));
          AssertThat(t2, Is().Not().EqualTo(t1));
        });
      });

      describe("Lexicographical Ordering '<'", []() {
        it("claims {0, 2, 1, 3}  < {1, 2, 1, 3}", []() {
          const tuple<int, 4> t1 = { 0, 2, 1, 3 };
          const tuple<int, 4> t2 = { 1, 2, 1, 3 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {0, 2, 4, 8}  < {1, 0, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 2, 4, 8 };
          const tuple<int, 4> t2 = { 1, 0, 0, 0 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {1, 2, 4, 8}  < {1, 3, 4, 8}", []() {
          const tuple<int, 4> t1 = { 1, 2, 4, 8 };
          const tuple<int, 4> t2 = { 1, 3, 4, 8 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {1, 3, 4, 8}  < {1, 4, 3, 2}", []() {
          const tuple<int, 4> t1 = { 1, 3, 4, 8 };
          const tuple<int, 4> t2 = { 1, 4, 3, 2 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {1, 3, 4, 8}  < {1, 4, 3, 2}", []() {
          const tuple<int, 4> t1 = { 1, 3, 4, 8 };
          const tuple<int, 4> t2 = { 1, 4, 3, 2 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {1, 3, 3, 8}  < {1, 3, 4, 8}", []() {
          const tuple<int, 4> t1 = { 1, 3, 3, 8 };
          const tuple<int, 4> t2 = { 1, 3, 4, 8 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {1, 2, 3, 2}  < {1, 2, 4, 8}", []() {
          const tuple<int, 4> t1 = { 1, 2, 3, 2 };
          const tuple<int, 4> t2 = { 1, 2, 4, 8 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {0, 0, 0, 0}  < {0, 0, 0, 1}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 0, 1 };

          AssertThat(t1, Is().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {0, 0, 0, 0} !< {0, 0, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 0, 0 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });

        it("claims {1, 2, 3, 4} !< {1, 2, 3, 4}", []() {
          const tuple<int, 4> t1 = { 1, 2, 3, 4 };
          const tuple<int, 4> t2 = { 1, 2, 3, 4 };

          AssertThat(t1, Is().Not().LessThan(t2));
          AssertThat(t2, Is().Not().LessThan(t1));
          AssertThat(t1, Is().Not().GreaterThan(t2));
        });
      });

      describe("tuple_first_lt", []() {
        it("claims {4, 3, 2, 0}  < {1, 2, 3, 4}", []() {
          const tuple<int, 4> t1 = { 4, 3, 2, 0 };
          const tuple<int, 4> t2 = { 1, 2, 3, 4 };

          const bool lt_lr = tuple_first_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {4, 3, 0, 2}  < {2, 1, 3, 4}", []() {
          const tuple<int, 4> t1 = { 4, 3, 0, 2 };
          const tuple<int, 4> t2 = { 2, 1, 3, 4 };

          const bool lt_lr = tuple_first_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {4, 0, 3, 2}  < {2, 3, 1, 4}", []() {
          const tuple<int, 4> t1 = { 4, 0, 3, 2 };
          const tuple<int, 4> t2 = { 2, 3, 1, 4 };

          const bool lt_lr = tuple_first_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {4, 2, 3, 0}  < {4, 3, 2, 1}", []() {
          const tuple<int, 4> t1 = { 4, 2, 3, 0 };
          const tuple<int, 4> t2 = { 4, 3, 2, 1 };

          const bool lt_lr = tuple_first_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 2, 3, 4}  < {2, 0, 3, 4}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 4 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_first_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 0, 0, 1}  < {0, 0, 1, 0}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 4 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_first_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_first_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 0, 0, 0} !< {0, 0, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 0, 0 };

          const bool lt_lr = tuple_first_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_first_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });

      describe("tuple_second_lt", []() {
        it("claims {4, 3, 1, 0}  < {0, 2, 3, 4}", []() {
          const tuple<int, 4> t1 = { 4, 3, 1, 0 };
          const tuple<int, 4> t2 = { 0, 2, 3, 4 };

          const bool lt_lr = tuple_second_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {4, 3, 0, 1}  < {2, 0, 3, 4}", []() {
          const tuple<int, 4> t1 = { 4, 3, 0, 1 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_second_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {4, 0, 1, 3}  < {2, 3, 0, 4}", []() {
          const tuple<int, 4> t1 = { 4, 0, 1, 3 };
          const tuple<int, 4> t2 = { 2, 3, 0, 4 };

          const bool lt_lr = tuple_second_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {4, 3, 1, 0}  < {4, 3, 0, 2}", []() {
          const tuple<int, 4> t1 = { 4, 3, 1, 0 };
          const tuple<int, 4> t2 = { 4, 3, 0, 2 };

          const bool lt_lr = tuple_second_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 2, 3, 4}  < {2, 0, 3, 4}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 4 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_second_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 0, 0, 1}  < {0, 0, 1, 0}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 4 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_second_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_second_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 0, 0, 0} !< {0, 0, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 0, 0 };

          const bool lt_lr = tuple_second_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_second_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });

      describe("tuple_third_lt", []() {
        it("claims {4, 2, 1, 0}  < {0, 1, 3, 4}", []() {
          const tuple<int, 4> t1 = { 4, 2, 1, 0 };
          const tuple<int, 4> t2 = { 0, 1, 3, 4 };

          const bool lt_lr = tuple_third_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {4, 1, 2, 0}  < {4, 0, 3, 0}", []() {
          const tuple<int, 4> t1 = { 4, 1, 2, 0 };
          const tuple<int, 4> t2 = { 4, 0, 3, 0 };

          const bool lt_lr = tuple_third_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {4, 0, 2, 0}  < {4, 0, 0, 3}", []() {
          const tuple<int, 4> t1 = { 4, 0, 2, 0 };
          const tuple<int, 4> t2 = { 4, 0, 0, 3 };

          const bool lt_lr = tuple_third_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 2, 3, 4}  < {2, 0, 3, 4}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 4 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_third_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 0, 0, 1}  < {0, 0, 1, 0}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 4 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_third_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_third_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 0, 0, 0} !< {0, 0, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 0, 0 };

          const bool lt_lr = tuple_third_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_third_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });

      describe("tuple_fourth_lt", []() {
        it("claims {3, 2, 1, 0}  < {0, 1, 2, 4}", []() {
          const tuple<int, 4> t1 = { 3, 2, 1, 0 };
          const tuple<int, 4> t2 = { 0, 1, 2, 4 };

          const bool lt_lr = tuple_fourth_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_fourth_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 2, 1, 3}  < {0, 1, 2, 4}", []() {
          const tuple<int, 4> t1 = { 0, 2, 1, 3 };
          const tuple<int, 4> t2 = { 0, 1, 2, 4 };

          const bool lt_lr = tuple_fourth_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_fourth_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 2, 3, 1}  < {0, 1, 1, 4}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 1 };
          const tuple<int, 4> t2 = { 0, 1, 1, 4 };

          const bool lt_lr = tuple_fourth_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_fourth_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 2, 3, 4}  < {2, 0, 3, 4}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 4 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_fourth_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_fourth_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 0, 0, 1}  < {0, 0, 1, 0}", []() {
          const tuple<int, 4> t1 = { 0, 2, 3, 4 };
          const tuple<int, 4> t2 = { 2, 0, 3, 4 };

          const bool lt_lr = tuple_fourth_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().True());

          const bool lt_rl = tuple_fourth_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });

        it("claims {0, 0, 0, 0} !< {0, 0, 0, 0}", []() {
          const tuple<int, 4> t1 = { 0, 0, 0, 0 };
          const tuple<int, 4> t2 = { 0, 0, 0, 0 };

          const bool lt_lr = tuple_fourth_lt<tuple<int, 4>>()(t1, t2);
          AssertThat(lt_lr, Is().False());

          const bool lt_rl = tuple_fourth_lt<tuple<int, 4>>()(t2, t1);
          AssertThat(lt_rl, Is().False());
        });
      });
    });

    describe("tuple<int, 4, sorted>", []() {
      // TODO
    });
  });
});
