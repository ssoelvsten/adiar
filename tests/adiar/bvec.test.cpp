#include "../test.h"
#include <adiar/bvec.h>
go_bandit([]() {
  describe("adiar/bvec.h", []() {
    describe("bdd_false()", []() {
      it("has 16 bits when asked", [&]() {
        AssertThat(bvec_false(16).bitlen(), Is().EqualTo(16u));
      });
    });
    describe("bdd_true()", []() {
      it("has 32 bits when asked", [&]() {
        AssertThat(bvec_true(32).bitlen(), Is().EqualTo(32u));
      });
    });
    describe("bdd_const", []() {
      describe("bitlength inference", []() {
        it("has 8 bitlen when constructed with char", [&]() {
          const bvec x = bvec_const((char)3);
          AssertThat(x.bitlen(), Is().EqualTo(8u));
        });
        it("has 16 bitlen when constructed with short", [&]() {
          const bvec x = bvec_const((short)3);
          AssertThat(x.bitlen(), Is().EqualTo(16u));
        });
        it("has 32 bitlen when constructed with int", [&]() {
          const bvec x = bvec_const(3);
          AssertThat(x.bitlen(), Is().EqualTo(32u));
        });
        it("has 64 bitlen when constructed with long", [&]() {
          const bvec x = bvec_const(3l);
          AssertThat(x.bitlen(), Is().EqualTo(64u));
        });
      });

      describe("int encoding", []() {
        it("is the binary encoding of 42 (101010)", [&]() {
          // 
          const bvec x = bvec_const(42);
          // Should initialize to bitlength 32
          AssertThat(x.bitlen(), Is().EqualTo(32u));

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
          // Should initialize to bitlength 32
          AssertThat(x.bitlen(), Is().EqualTo(32u));

          for( size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(x.at(i), Is().EqualTo(bdd_true()));
          }

        });

        it("is the binary encoding of -2^32", [&]() {
          // 
          const bvec x = bvec_const(INT32_MIN);
          // Should initialize to bitlength 32
          AssertThat(x.bitlen(), Is().EqualTo(32u));
          
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
          
          for(size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(res.at(i), Is().EqualTo(expected.at(i)));
          }
        });
        it("computes 0 & 3 == 0 (000 & 011 == 000)", [&]() {
          const bvec x = bvec_const((char)0);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)0); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_and(x,y);
          
          for(size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(res.at(i), Is().EqualTo(expected.at(i)));
          }
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
          
          for(size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(res.at(i), Is().EqualTo(expected.at(i)));
          }
        });
        it("computes 0 | 3 == 3 (000 | 011 == 011)", [&]() {
          const bvec x = bvec_const((char)0);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)3); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_or(x,y);
          
          for(size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(res.at(i), Is().EqualTo(expected.at(i)));
          }
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
          
          for(size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(res.at(i), Is().EqualTo(expected.at(i)));
          }
        });
        it("computes 0 ^ 3 == 3 (000 ^ 011 == 011)", [&]() {
          const bvec x = bvec_const((char)0);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)3); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_xor(x,y);
          
          for(size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(res.at(i), Is().EqualTo(expected.at(i)));
          }
        });
        it("computes 255 ^ 3 == 252 (11111111 ^ 00000011 == 11111100)", [&]() {
          const bvec x = bvec_const((char)255);
          const bvec y = bvec_const((char)3);
          const bvec expected = bvec_const((char)252); //Is this expected, or should we check bdd structure?

          const bvec res = bvec_xor(x,y);
          
          for(size_t i = 0; i < x.bitlen(); i++) {
            AssertThat(res.at(i), Is().EqualTo(expected.at(i)));
          }
        });
      });
    });
  });
});