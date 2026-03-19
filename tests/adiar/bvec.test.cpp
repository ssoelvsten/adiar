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
        it("is the binary encoding of 42", [&]() {
          // 42_10 == 101010_2 (010101 litte-endian)
          const bvec x = bvec_const(42);
          AssertThat(x.at(0), Is().EqualTo(bdd_false()));
          AssertThat(x.at(1), Is().EqualTo(bdd_true()));
          AssertThat(x.at(2), Is().EqualTo(bdd_false()));
          AssertThat(x.at(3), Is().EqualTo(bdd_true()));
          AssertThat(x.at(4), Is().EqualTo(bdd_false()));
          AssertThat(x.at(5), Is().EqualTo(bdd_true()));
        });
      });
    });
  });
});