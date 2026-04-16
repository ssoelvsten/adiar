#include "../test.h"
#include <adiar/bvec.h>
go_bandit([]() {
  describe("adiar/bvec.h", []() {
    describe("bvec_false()", []() {
      it("has 16 bits when asked", [&]() {
        AssertThat(bvec_false(16).bitlen(), Is().EqualTo(16u));
      });
    });
    describe("bvec_true()", []() {
      it("has 32 bits when asked", [&]() {
        AssertThat(bvec_true(32).bitlen(), Is().EqualTo(32u));
      });
    });
    describe("bvec_const", []() {
      describe("bitlength inference", []() {
        it("has 32 bitlen 0 size when constructed with 0", [&]() {
          const bvec x = bvec_const(0);
          AssertThat(x.size(), Is().EqualTo(0u));
          AssertThat(x.bitlen(), Is().EqualTo(32u));
        });
        it("has 8 size and bitlen when constructed with max unsigned char", [&]() {
          const bvec x = bvec_const((unsigned char)255);
          AssertThat(x.size(), Is().EqualTo(8u));
          AssertThat(x.bitlen(), Is().EqualTo(8u));
        });
        it("has 8 size and bitlen when constructed with overflowing char", [&]() {
          const bvec x = bvec_const((char)253);
          AssertThat(x.size(), Is().EqualTo(8u));
          AssertThat(x.bitlen(), Is().EqualTo(8u));
        });
        it("has 8 size and bitlen when constructed with negative char", [&]() {
          const bvec x = bvec_const((char)-52);
          AssertThat(x.size(), Is().EqualTo(8u));
          AssertThat(x.bitlen(), Is().EqualTo(8u));
        });
        it("has 7 size and 8 bitlen when constructed with max signed char", [&]() {
          const bvec x = bvec_const((char)127);
          AssertThat(x.size(), Is().EqualTo(7u));
          AssertThat(x.bitlen(), Is().EqualTo(8u));
        });
      });

      describe("int encoding", []() {
        it("is the binary encoding of 42 (101010)", [&]() {
          // 
          const bvec x = bvec_const(42);

          // Check that encoding is 101010
          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_true()));

          // Check that the rest of the bits are zero
          for( size_t i = 6; i < x.bitlen(); i++) {
            AssertThat(x.at(i), Is().EqualTo(bdd_false()));
          }
        });

        it("is the binary encoding of -1 ", [&]() {
          // 
          const bvec x = bvec_const(-1);

          for( size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(x.at(i), Is().EqualTo(bdd_true()));
          }

        });

        it("is the binary encoding of -2^32", [&]() {
          // 
          const bvec x = bvec_const(INT32_MIN);
          
          for( size_t i = 0; i < x.bitlen()-1; i++) {
            AssertThat(x.at(i), Is().EqualTo(bdd_false()));
          }

          AssertThat(x.at(x.bitlen()-1), Is().EqualTo(bdd_true()));
        });
      });
    });

    describe("bvec_and", []() {
      describe("constants", []() {
        it("computes 5 & 3 == 1 (101 & 011 == 001)", [&]() {
          const bvec x = bvec_const((char)5);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)1); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_and(x,y);
          
          AssertThat(res, Is().EqualTo(expected));
          
        });
        it("computes 0 & 3 == 0 (000 & 011 == 000)", [&]() {
          const bvec x = bvec_const((char)0);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)0); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_and(x,y);
          
          AssertThat(res, Is().EqualTo(expected));
        });
      });
    });

    describe("bvec_or", []() {
      describe("constants", []() {
        it("computes 5 | 3 == 7 (101 | 011 == 111)", [&]() {
          const bvec x = bvec_const((char)5);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)7); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_or(x,y);
          
          AssertThat(res, Is().EqualTo(expected));
        });
        it("computes 0 | 3 == 3 (000 | 011 == 011)", [&]() {
          const bvec x = bvec_const((char)0);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)3); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_or(x,y);
          
          AssertThat(res, Is().EqualTo(expected));
        });
      });
    });

    describe("bvec_xor", []() {
      describe("constants", []() {
        it("computes 5 ^ 3 == 6 (101 ^ 011 == 110)", [&]() {
          const bvec x = bvec_const((char)5);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)6); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_xor(x,y);
        
          AssertThat(res, Is().EqualTo(expected));
          
        });
        it("computes 0 ^ 3 == 3 (000 ^ 011 == 011)", [&]() {
          const bvec x = bvec_const((char)0);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)3); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_xor(x,y);
          
          AssertThat(res, Is().EqualTo(expected));
        });
        it("computes 255 ^ 3 == 252 (11111111 ^ 00000011 == 11111100)", [&]() {
          const bvec x = bvec_const((char)255);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)252); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_xor(x,y);
          
          AssertThat(res, Is().EqualTo(expected));
        });
      });
    });

    describe("bvec_not", []() {
      describe("constants", []() {
        it("computes ~3 == 252 for bitlength 8 (~00000011 == 11111100)", [&]() {
          const bvec x = bvec_const((char)3);
          const bvec expected = bvec_const((u_char)252); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_not(x);
          
            AssertThat(res, Is().EqualTo(expected));
        });
        it("computes ~3 == (65535 - 3) for bitlength 16 (~0000000000000011 == 1111111111111100)", [&]() {
          const bvec x = bvec_const((short)3);
          const bvec expected = bvec_const((short)(USHRT_MAX-3)); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_not(x);
          
          
          AssertThat(res, Is().EqualTo(expected));
          
        });
      });
    });
  });
});