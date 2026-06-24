#include "../test.h"

#include <adiar/bvec.h>

go_bandit([]() {
  describe("adiar/bvec.h", []() {
    describe("bvec_false()", []() {
      it("has variable bitlen by default", [&]() {
        const bvec x = bvec_false();

        AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
        AssertThat(x.size(), Is().EqualTo(0u));
      });

      it("has 16 bits when asked", [&]() {
        const bvec x = bvec_false(16);

        AssertThat(x.bitlen(), Is().EqualTo(16u));
        AssertThat(x.size(), Is().EqualTo(0u));
      });

      it("has all its bits set to false", [&]() {
        const bvec x = bvec_false(8);

        for (size_t i = 0; i < x.bitlen(); i++) { AssertThat(x.at(i), Is().EqualTo(bdd_false())); }
      });
    });

    describe("bvec_true()", []() {
      it("has no bits set when set so without a bitlen", [&]() {
        const bvec x = bvec_true(8);
        AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
        AssertThat(x.size(), Is().EqualTo(8u));

        // Check that all bits are one
        for (size_t i = 0; i < x.size(); i++) { AssertThat(x.at(i), Is().EqualTo(bdd_true())); }
      });

      it("has 32 bits set when set so without a bitlen", [&]() {
        const bvec x = bvec_true(32);
        AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
        AssertThat(x.size(), Is().EqualTo(32u));

        // Check that all bits are one
        for (size_t i = 0; i < x.size(); i++) { AssertThat(x.at(i), Is().EqualTo(bdd_true())); }
      });

      it("has 4 bits set when set so with a fixed bitlen of 8", [&]() {
        const bvec x = bvec_true(8, 4);
        AssertThat(x.bitlen(), Is().EqualTo(8u));
        AssertThat(x.size(), Is().EqualTo(4u));

        // Check that the bits are set as expected
        AssertThat(x.at(0), Is().EqualTo(bdd_true()));
        AssertThat(x.at(1), Is().EqualTo(bdd_true()));
        AssertThat(x.at(2), Is().EqualTo(bdd_true()));
        AssertThat(x.at(3), Is().EqualTo(bdd_true()));
        AssertThat(x.at(4), Is().EqualTo(bdd_false()));
        AssertThat(x.at(5), Is().EqualTo(bdd_false()));
        AssertThat(x.at(6), Is().EqualTo(bdd_false()));
        AssertThat(x.at(7), Is().EqualTo(bdd_false()));
      });

      it("truncates when more bits are set to one than the bit length", [&]() {
        const bvec x = bvec_true(8, 12);
        AssertThat(x.bitlen(), Is().EqualTo(8u));
        AssertThat(x.size(), Is().EqualTo(8u));

        // Check that the bits are set as expected
        AssertThat(x.at(0), Is().EqualTo(bdd_true()));
        AssertThat(x.at(1), Is().EqualTo(bdd_true()));
        AssertThat(x.at(2), Is().EqualTo(bdd_true()));
        AssertThat(x.at(3), Is().EqualTo(bdd_true()));
        AssertThat(x.at(4), Is().EqualTo(bdd_true()));
        AssertThat(x.at(5), Is().EqualTo(bdd_true()));
        AssertThat(x.at(6), Is().EqualTo(bdd_true()));
        AssertThat(x.at(7), Is().EqualTo(bdd_true()));
      });
    });

    describe("bvec_const", []() {
      it("0 has by default variadic bitlen", [&]() {
        const bvec x = bvec_const(0);
        AssertThat(x.size(), Is().EqualTo(0u));
        AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
      });

      it("0 can have a custom bitlen", [&]() {
        const bvec x = bvec_const(7, 0);
        AssertThat(x.size(), Is().EqualTo(0u));
        AssertThat(x.bitlen(), Is().EqualTo(7u));
      });

      it("0 is derived to be ()", [&]() {
        const bvec x = bvec_const(0);

        // Check that all bits are zero
        for (size_t i = 0; i < x.bitlen(); i++) { AssertThat(x.at(i), Is().EqualTo(bdd_false())); }
      });

      it("1 has by default variadic bitlen", [&]() {
        const bvec x = bvec_const(1);
        AssertThat(x.size(), Is().EqualTo(1u));
        AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
      });

      it("1 can have a custom bitlen", [&]() {
        const bvec x = bvec_const(16, 1);
        AssertThat(x.size(), Is().EqualTo(1u));
        AssertThat(x.bitlen(), Is().EqualTo(16u));
      });

      it("1 is derived to be (1)", [&]() {
        const bvec x = bvec_const(1);

        // Check that the first bit is set
        AssertThat(x.at(0), Is().EqualTo(bdd_true()));

        // Check that all bits are zero
        for (size_t i = 1; i < x.bitlen(); i++) { AssertThat(x.at(i), Is().EqualTo(bdd_false())); }
      });

      it("2 is derived to be (01)", [&]() {
        const bvec x = bvec_const(2);

        // Check that the first bit is set
        AssertThat(x.at(0), Is().EqualTo(bdd_false()));
        AssertThat(x.at(1), Is().EqualTo(bdd_true()));

        // Check that all bits are zero
        for (size_t i = 2; i < x.bitlen(); i++) { AssertThat(x.at(i), Is().EqualTo(bdd_false())); }
      });

      it("42 has default size as bitlen", [&]() {
        const bvec x = bvec_const(42);
        AssertThat(x.size(), Is().EqualTo(6u));
        AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
      });

      it("42 can have a custom bitlen", [&]() {
        const bvec x = bvec_const(24, 42);
        AssertThat(x.size(), Is().EqualTo(6u));
        AssertThat(x.bitlen(), Is().EqualTo(24u));
      });

      it("42 is derived to be (101010)", [&]() {
        const bvec x = bvec_const(42);

        // Check that encoding is 101010
        AssertThat(x.at(0), Is().EqualTo(bdd_false()));
        AssertThat(x.at(1), Is().EqualTo(bdd_true()));
        AssertThat(x.at(2), Is().EqualTo(bdd_false()));
        AssertThat(x.at(3), Is().EqualTo(bdd_true()));
        AssertThat(x.at(4), Is().EqualTo(bdd_false()));
        AssertThat(x.at(5), Is().EqualTo(bdd_true()));
      });

      it("(int) -1 is derived to be (111...1) with 32 bits", [&]() {
        const bvec x = bvec_const(32, -1);

        AssertThat(x.bitlen(), Is().EqualTo(32u));
        for (size_t i = 0; i < x.bitlen(); i++) { AssertThat(x.at(i), Is().EqualTo(bdd_true())); }
      });

      it("-2^32 is derived to be (000..01) with 32 bits", [&]() {
        const bvec x = bvec_const(32, INT32_MIN);

        AssertThat(x.bitlen(), Is().EqualTo(32u));
        for (size_t i = 0; i < x.bitlen() - 1; i++) {
          AssertThat(x.at(i), Is().EqualTo(bdd_false()));
        }

        AssertThat(x.at(x.bitlen() - 1), Is().EqualTo(bdd_true()));
      });
    });

    describe("bvec::bvec(...)", []() {
      describe("bvec::bvec(value)", []() {
        it("derives 0 to be () with 0 bits required", [&]() {
          const bvec x(0);

          AssertThat(x.size(), Is().EqualTo(0u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
        });

        it("derives 1 to be (1) with 1 bit required", [&]() {
          const bvec x(1);

          AssertThat(x.size(), Is().EqualTo(1u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));

          // Check that encoding is 1
          AssertThat(x.at(0), Is().EqualTo(bdd_true()));
        });

        it("derives 2 to be (01) with 2 bits required", [&]() {
          const bvec x(2);

          AssertThat(x.size(), Is().EqualTo(2u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));

          // Check that encoding is 01
          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
        });

        it("derives 3 to be (01) with 2 bits required", [&]() {
          const bvec x(3);

          AssertThat(x.size(), Is().EqualTo(2u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));

          // Check that encoding is 11
          AssertThat(x.at(0), Is().EqualTo(bdd_true()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
        });

        it("derives 42 to be (010101) with 6 bits required", [&]() {
          const bvec x(42);

          AssertThat(x.size(), Is().EqualTo(6u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));

          // Check that encoding is 101010
          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_true()));
        });
      });

      describe("bvec::bvec(bitlen, value)", []() {
        it("derives 0 to be () with 7 number of bits", [&]() {
          const bvec x(7, 0);

          AssertThat(x.size(), Is().EqualTo(0u));
          AssertThat(x.bitlen(), Is().EqualTo(7u));

          // Check that all bits are false
          for (size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(x.at(i), Is().EqualTo(bdd_false()));
          }
        });

        it("derives 1 to be (1) with 9 number of bits", [&]() {
          const bvec x(9, 1);

          AssertThat(x.size(), Is().EqualTo(1u));
          AssertThat(x.bitlen(), Is().EqualTo(9u));

          // Check that encoding is 1
          AssertThat(x.at(0), Is().EqualTo(bdd_true()));

          // Check that the rest of the bits are zero
          for (size_t i = 1; i < x.bitlen(); i++) {
            AssertThat(x.at(i), Is().EqualTo(bdd_false()));
          }
        });

        it("derives 42 to be (010101) with 11 number of bits", [&]() {
          const bvec x(11, 42);

          AssertThat(x.size(), Is().EqualTo(6u));
          AssertThat(x.bitlen(), Is().EqualTo(11u));

          // Check that encoding is 101010
          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_true()));

          // Check that the rest of the bits are zero
          for (size_t i = 6; i < x.bitlen(); i++) {
            AssertThat(x.at(i), Is().EqualTo(bdd_false()));
          }
        });

        it("truncates 42 to be (0101) with 4 number of bits", [&]() {
          const bvec x(4, 42);

          AssertThat(x.size(), Is().EqualTo(4u));
          AssertThat(x.bitlen(), Is().EqualTo(4u));

          // Check that encoding is 101010
          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
        });
      });

      describe("bvec::bvec(bits)", []() {
        it("copies over bits (001)", [&]() {
          const bvec x({ bdd_false(), bdd_false(), bdd_true() });

          AssertThat(x.size(), Is().EqualTo(3u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));

          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_false()));
          AssertThat(x.at(2), Is().EqualTo(bdd_true()));
        });

        it("copies over bits (1011)", [&]() {
          const bvec x({ bdd_true(), bdd_false(), bdd_true(), bdd_true() });

          AssertThat(x.size(), Is().EqualTo(4u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));

          AssertThat(x.at(0), Is().EqualTo(bdd_true()));
          AssertThat(x.at(1), Is().EqualTo(bdd_false()));
          AssertThat(x.at(2), Is().EqualTo(bdd_true()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
        });

        it("ignores false bits at the end of (1010)", [&]() {
          const bvec x({ bdd_true(), bdd_false(), bdd_true(), bdd_false() });

          AssertThat(x.size(), Is().EqualTo(3u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));

          AssertThat(x.at(0), Is().EqualTo(bdd_true()));
          AssertThat(x.at(1), Is().EqualTo(bdd_false()));
          AssertThat(x.at(2), Is().EqualTo(bdd_true()));
          AssertThat(x.at(3), Is().EqualTo(bdd_false()));
        });
      });

      describe("bvec::bvec(bitlen, bits)", []() {
        it("copies over bits (1001) with a bit length of 8", [&]() {
          const bvec x(8, { bdd_true(), bdd_false(), bdd_false(), bdd_true() });

          AssertThat(x.size(), Is().EqualTo(4u));
          AssertThat(x.bitlen(), Is().EqualTo(8u));

          AssertThat(x.at(0), Is().EqualTo(bdd_true()));
          AssertThat(x.at(1), Is().EqualTo(bdd_false()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_false()));
          AssertThat(x.at(6), Is().EqualTo(bdd_false()));
          AssertThat(x.at(7), Is().EqualTo(bdd_false()));
        });

        it("ignores false bits at the end of (1000)", [&]() {
          const bvec x(8, { bdd_true(), bdd_false(), bdd_false(), bdd_false() });

          AssertThat(x.size(), Is().EqualTo(1u));
          AssertThat(x.bitlen(), Is().EqualTo(8u));

          AssertThat(x.at(0), Is().EqualTo(bdd_true()));
          AssertThat(x.at(1), Is().EqualTo(bdd_false()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_false()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_false()));
          AssertThat(x.at(6), Is().EqualTo(bdd_false()));
          AssertThat(x.at(7), Is().EqualTo(bdd_false()));
        });

        it("truncates non-false bits at the end of (1011) down to 2 bits", [&]() {
          const bvec x(2, { bdd_true(), bdd_false(), bdd_false(), bdd_false() });

          AssertThat(x.size(), Is().EqualTo(1u));
          AssertThat(x.bitlen(), Is().EqualTo(2u));

          AssertThat(x.at(0), Is().EqualTo(bdd_true()));
          AssertThat(x.at(1), Is().EqualTo(bdd_false()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_false()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_false()));
          AssertThat(x.at(6), Is().EqualTo(bdd_false()));
          AssertThat(x.at(7), Is().EqualTo(bdd_false()));
        });
      });

      describe("bvec::bvec(begin, end)", []() {
        it("copies over bits from iterator of (010101)", [&]() {
          const std::vector<bdd> vec(
            { bdd_false(), bdd_true(), bdd_false(), bdd_true(), bdd_false(), bdd_true() });
          const bvec x(vec.begin(), vec.end());

          AssertThat(x.size(), Is().EqualTo(6u));
          AssertThat(x.bitlen(), Is().EqualTo(bvec::variadic_bitlen));

          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_true()));
        });
      });

      describe("bvec::bvec(bitlen, begin, end)", []() {
        it("copies over bits from iterator of (010101) with 8 bits", [&]() {
          const std::vector<bdd> vec(
            { bdd_false(), bdd_true(), bdd_false(), bdd_true(), bdd_false(), bdd_true() });
          const bvec x(8, vec.begin(), vec.end());

          AssertThat(x.size(), Is().EqualTo(6u));
          AssertThat(x.bitlen(), Is().EqualTo(8u));

          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_true()));
          AssertThat(x.at(6), Is().EqualTo(bdd_false()));
          AssertThat(x.at(7), Is().EqualTo(bdd_false()));
        });

        it("truncates bits from iterator of (010101) with 5 bits", [&]() {
          const std::vector<bdd> vec(
            { bdd_false(), bdd_true(), bdd_false(), bdd_true(), bdd_false(), bdd_true() });
          const bvec x(5, vec.begin(), vec.end());

          AssertThat(x.size(), Is().EqualTo(4u));
          AssertThat(x.bitlen(), Is().EqualTo(5u));

          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
        });
      });
    });

    describe("bvec_equal", []() {
      it("compares 101 == 101", [&]() {
        const bvec x({ bdd_true(), bdd_false(), bdd_true() });
        const bvec y({ bdd_true(), bdd_false(), bdd_true() });

        AssertThat(bvec_equal(x, y), Is().True());
      });

      it("compares 0101 == 101", [&]() {
        const bvec x({ bdd_true(), bdd_false(), bdd_true() });
        const bvec y({ bdd_true(), bdd_false(), bdd_true(), bdd_false() });

        AssertThat(bvec_equal(x, y), Is().True());
      });

      it("compares 101 == 0101", [&]() {
        const bvec x({ bdd_true(), bdd_false(), bdd_true(), bdd_false() });
        const bvec y({ bdd_true(), bdd_false(), bdd_true() });

        AssertThat(bvec_equal(x, y), Is().True());
      });

      it("compares 101 != 1101", [&]() {
        const bvec x({ bdd_true(), bdd_false(), bdd_true(), bdd_true() });
        const bvec y({ bdd_true(), bdd_false(), bdd_true() });

        AssertThat(bvec_equal(x, y), Is().False());
      });

      it("compares 1101 != 101", [&]() {
        const bvec x({ bdd_true(), bdd_false(), bdd_true() });
        const bvec y({ bdd_true(), bdd_false(), bdd_true(), bdd_true() });

        AssertThat(bvec_equal(x, y), Is().False());
      });

      it("compares 1101 != 1110", [&]() {
        const bvec x({ bdd_true(), bdd_false(), bdd_true(), bdd_true() });
        const bvec y({ bdd_false(), bdd_true(), bdd_true(), bdd_true() });

        AssertThat(bvec_equal(x, y), Is().False());
      });

      it("compares 1100 != 1110", [&]() {
        const bvec x({ bdd_false(), bdd_false(), bdd_true(), bdd_true() });
        const bvec y({ bdd_false(), bdd_true(), bdd_true(), bdd_true() });

        AssertThat(bvec_equal(x, y), Is().False());
      });

      it("compares 1x0 == 1x0", [&]() {
        const bvec x({ bdd_false(), bdd_ithvar(42), bdd_true() });
        const bvec y({ bdd_false(), bdd_ithvar(42), bdd_true() });

        AssertThat(bvec_equal(x, y), Is().True());
      });

      it("compares 1x0 != 1y0", [&]() {
        const bvec x({ bdd_false(), bdd_ithvar(42), bdd_true() });
        const bvec y({ bdd_false(), bdd_ithvar(11), bdd_true() });

        AssertThat(bvec_equal(x, y), Is().False());
      });
    });

    describe("bvec_and", []() {
      describe("constants", []() {
        it("computes 5 & 3 == 1 (101 & 011 == 001)", [&]() {
          const bvec x        = bvec_const(8, 5);
          const bvec y        = bvec_const(8, 3);
          const bvec expected = bvec_const(8, 1);

          const bvec res = bvec_and(x, y);

          AssertThat(res, Is().EqualTo(expected));
        });

        it("computes 0 & 3 == 0 (000 & 011 == 000)", [&]() {
          const bvec x        = bvec_const(8, 0);
          const bvec y        = bvec_const(8, 3);
          const bvec expected = bvec_const(8, 0);

          const bvec res = bvec_and(x, y);

          AssertThat(res, Is().EqualTo(expected));
        });
      });
    });

    describe("bvec_or", []() {
      describe("constants", []() {
        it("computes 5 | 3 == 7 (101 | 011 == 111)", [&]() {
          const bvec x        = bvec_const(8, 5);
          const bvec y        = bvec_const(8, 3);
          const bvec expected = bvec_const(8, 7);

          const bvec res = bvec_or(x, y);

          AssertThat(res, Is().EqualTo(expected));
        });

        it("computes 0 | 3 == 3 (000 | 011 == 011)", [&]() {
          const bvec x        = bvec_const(8, 0);
          const bvec y        = bvec_const(8, 3);
          const bvec expected = bvec_const(8, 3);

          const bvec res = bvec_or(x, y);

          AssertThat(res, Is().EqualTo(expected));
        });
      });
    });

    describe("bvec_xor", []() {
      describe("constants", []() {
        it("computes 5 ^ 3 == 6 (101 ^ 011 == 110)", [&]() {
          const bvec x        = bvec_const(8, 5);
          const bvec y        = bvec_const(8, 3);
          const bvec expected = bvec_const(8, 6);

          const bvec res = bvec_xor(x, y);

          AssertThat(res, Is().EqualTo(expected));
        });

        it("computes 0 ^ 3 == 3 (000 ^ 011 == 011)", [&]() {
          const bvec x        = bvec_const(8, 0);
          const bvec y        = bvec_const(8, 3);
          const bvec expected = bvec_const(8, 3);

          const bvec res = bvec_xor(x, y);

          AssertThat(res, Is().EqualTo(expected));
        });

        it("computes 255 ^ 3 == 252 (11111111 ^ 00000011 == 11111100)", [&]() {
          const bvec x        = bvec_const(8, 255);
          const bvec y        = bvec_const(8, 3);
          const bvec expected = bvec_const(8, 252);

          const bvec res = bvec_xor(x, y);

          AssertThat(res, Is().EqualTo(expected));
        });
      });
    });

    describe("bvec_not", []() {
      describe("constants", []() {
        it("computes ~3 == 252 for bitlength 8 (~00000011 == 11111100)", [&]() {
          const bvec x        = bvec_const(8, 3);
          const bvec expected = bvec_const(8, 252);

          const bvec res = bvec_not(x);

          AssertThat(res, Is().EqualTo(expected));
        });

        it("computes ~3 == (65535 - 3) for bitlength 16 (~0000000000000011 == 1111111111111100)",
           [&]() {
             const bvec x        = bvec_const(16, 3);
             const bvec expected = bvec_const(16, (USHRT_MAX - 3));

             const bvec res = bvec_not(x);

             AssertThat(res, Is().EqualTo(expected));
           });

        it("computes ~2 == 1 for variadic bitlength", [&]() {
          const bvec x   = bvec_const(2);
          const bvec res = bvec_not(x);

          const bvec expected = bvec_const(1);
          AssertThat(res, Is().EqualTo(expected));
        });

        it("computes ~3 == 0 for variadic bitlength", [&]() {
          const bvec x   = bvec_const(3);
          const bvec res = bvec_not(x);

          const bvec expected = bvec_const(0);
          AssertThat(res, Is().EqualTo(expected));
        });
      });
    });

    describe("bvec_add", []() {
      it("computes bvec[8](5) + bvec[8](3)", [&]() {
        const bvec a = bvec_const(8, 5);
        const bvec b = bvec_const(8, 3);

        const bvec expected = bvec_const(8, 8);
        const bvec res      = bvec_add(a, b);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(4u));
      });

      it("computes bvec[8](42) + bvec[8](3)", [&]() {
        const bvec a = bvec_const(8, 42);
        const bvec b = bvec_const(8, 3);

        const bvec expected = bvec_const(8, 45);
        const bvec res      = a + b;
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(6u));
      });

      it("overflows bvec[8](255) + bvec[8](1)", [&]() {
        const bvec a = bvec_const(8, 255);
        const bvec b = bvec_const(8, 1);

        const bvec expected = bvec_const(8, 0);
        const bvec res      = bvec_add(a, b);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(0u));
      });

      it("upcasts bvec[32](5) + bvec[8](3)", [&]() {
        const bvec a = bvec_const(32, 5);
        const bvec b = bvec_const(8, 3);

        const bvec expected = bvec_const(32, 8);
        const bvec res      = bvec_add(a, b);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(32u));
        AssertThat(res.size(), Is().EqualTo(4u));
      });

      it("upcasts bvec[32](0) + bvec[8](3)", [&]() {
        const bvec a = bvec_const(32, 0);
        const bvec b = bvec_const(8, 3);

        const bvec expected = bvec_const(32, 3);
        const bvec res      = bvec_add(a, b);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(32u));
        AssertThat(res.size(), Is().EqualTo(2u));
      });

      it("upcasts bvec[8](0) + bvec[32](3)", [&]() {
        const bvec a = bvec_const(8, 0);
        const bvec b = bvec_const(32, 3);

        const bvec expected = bvec_const(32, 3);
        const bvec res      = bvec_add(a, b);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(32u));
        AssertThat(res.size(), Is().EqualTo(2u));
      });

      it("upcasts bvec[32](17) + bvec[8](42)", [&]() {
        const bvec a = bvec_const(32, 17);
        const bvec b = bvec_const(8, 42);

        const bvec expected = bvec_const(32, 59);
        const bvec res      = a + b;
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(32u));
        AssertThat(res.size(), Is().EqualTo(6u));
      });

      it("fixes size of bvec[_](9) + bvec[8](21)", [&]() {
        const bvec a = bvec_const(9);
        const bvec b = bvec_const(8, 21);

        const bvec expected = bvec_const(8, 30);
        const bvec res      = a + b;
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(5u));
      });

      it("fixes size of bvec[16](2) + bvec[_](4)", [&]() {
        const bvec a = bvec_const(16, 2);
        const bvec b = bvec_const(4);

        const bvec expected = bvec_const(16, 6);
        const bvec res      = a + b;
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(16u));
        AssertThat(res.size(), Is().EqualTo(3u));
      });

      it("makes bvec[8](42) + bvec[_](512) variadic bit length", [&]() {
        const bvec a = bvec_const(8, 42);
        const bvec b = bvec_const(512);

        const bvec expected = bvec_const(554);
        const bvec res      = a + b;
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
        AssertThat(res.size(), Is().EqualTo(10u));
      });

      it("makes bvec[_](320) + bvec[8](5) variadic bit length", [&]() {
        const bvec a = bvec_const(320);
        const bvec b = bvec_const(8, 5);

        const bvec expected = bvec_const(325);
        const bvec res      = a + b;
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(bvec::variadic_bitlen));
        AssertThat(res.size(), Is().EqualTo(9u));
      });
    });

    describe("bvec_sub", []() {
      it("compute bvec[8](42) - bvec[8](3)", [&]() {
        const bvec x = bvec_const(8, 42);
        const bvec y = bvec_const(8, 3);

        const bvec expected = bvec_const(32, 39);
        const bvec res      = bvec_sub(x, y);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(6u));
      });

      it("compute bvec[8](140) - bvec[8](4)", [&]() {
        const bvec x = bvec_const(8, 140);
        const bvec y = bvec_const(8, 4);

        const bvec expected = bvec_const(8, 136);
        const bvec res      = bvec_sub(x, y);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(8u));
      });

      it("underflows bvec[8](0) - bvec[8](1)", [&]() {
        const bvec x = bvec_const(8, 0);
        const bvec y = bvec_const(8, 1);

        const bvec expected = bvec_const(8, -1);
        const bvec res      = bvec_sub(x, y);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(8u));
      });

      it("underflows bvec[8](4) - bvec[8](7)", [&]() {
        const bvec x = bvec_const(8, 4);
        const bvec y = bvec_const(8, 7);

        const bvec expected = bvec_const(8, -3);
        const bvec res      = bvec_sub(x, y);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(8u));
      });

      it("underflows bvec[16](21) - bvec[8](42)", [&]() {
        const bvec x = bvec_const(16, 21);
        const bvec y = bvec_const(8, 42);

        const bvec expected = bvec_const(16, -21);
        const bvec res      = bvec_sub(x, y);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(16u));
        AssertThat(res.size(), Is().EqualTo(16u));
      });

      it("underflows bvec[8](4) - bvec[16](7)", [&]() {
        const bvec x = bvec_const(8, 3);
        const bvec y = bvec_const(16, 9);

        const bvec expected = bvec_const(16, -6);
        const bvec res      = bvec_sub(x, y);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(16u));
        AssertThat(res.size(), Is().EqualTo(16u));
      });
    });

    describe("bvec_truncate", []() {
      it("truncates to bitlen", [&]() {
        const bvec x = bvec_true(32, 32);

        const bvec expected = bvec_true(8, 8);
        const bvec res      = bvec_truncate(8, x);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(8u));
      });

      it("extends to bitlen", [&]() {
        const bvec x = bvec_true(4, 4);
        AssertThat(x.bitlen(), Is().EqualTo(4u));

        const bvec expected = bvec_true(8, 4);
        const bvec res      = bvec_truncate(8, x);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(4u));
      });

      it("truncates false prefix", [&]() {
        const bvec x = bvec_const(18);

        const bvec expected = bvec_const(2);
        const bvec res      = bvec_truncate(4, x);
        AssertThat(res, Is().EqualTo(expected));
        AssertThat(res.bitlen(), Is().EqualTo(4u));
        AssertThat(res.size(), Is().EqualTo(2u));
      });
    });
  });
});
