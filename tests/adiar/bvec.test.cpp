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
    describe("bvec_equal", []() {
      it("compares 101 == 101", [&]() {
        std::vector<bdd> raw1 = {bdd_true(),bdd_false(),bdd_true()};
        std::vector<bdd> raw2 = {bdd_true(),bdd_false(),bdd_true()};
        
        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().True()); 
      });
      it("compares 0101 == 101", [&]() {
        std::vector<bdd> raw1 = {bdd_true(),bdd_false(),bdd_true()};
        std::vector<bdd> raw2 = {bdd_true(),bdd_false(),bdd_true(),bdd_false()};
        
        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().True());
      });
      it("compares 101 == 0101", [&]() {
        std::vector<bdd> raw1 = {bdd_true(),bdd_false(),bdd_true(),bdd_false()};
        std::vector<bdd> raw2 = {bdd_true(),bdd_false(),bdd_true()};
        
        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().True());
      });
      it("compares 101 != 1101", [&]() {
        std::vector<bdd> raw1 = {bdd_true(),bdd_false(),bdd_true(),bdd_true()};
        std::vector<bdd> raw2 = {bdd_true(),bdd_false(),bdd_true()};
        
        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().False());
      });
      it("compares 1101 != 101", [&]() {
        std::vector<bdd> raw1 = {bdd_true(),bdd_false(),bdd_true()};
        std::vector<bdd> raw2 = {bdd_true(),bdd_false(),bdd_true(),bdd_true()};
        
        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().False());
      });
      it("compares 1101 != 1110", [&]() {
        std::vector<bdd> raw1 = {bdd_true(),bdd_false(),bdd_true(),bdd_true()};
        std::vector<bdd> raw2 = {bdd_false(),bdd_true(),bdd_true(),bdd_true()};
        
        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().False());
      });
      it("compares 1100 != 1110", [&]() {
        std::vector<bdd> raw1 = {bdd_false(),bdd_false(),bdd_true(),bdd_true()};
        std::vector<bdd> raw2 = {bdd_false(),bdd_true(),bdd_true(),bdd_true()};
        
        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().False());
      });
      it("compares 1x0 == 1x0", [&]() {
        std::vector<bdd> raw1 = {bdd_false(),bdd_ithvar(42),bdd_true()};
        std::vector<bdd> raw2 = {bdd_false(),bdd_ithvar(42),bdd_true()};

        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().True());
      });
      it("compares 1x0 != 1y0", [&]() {
        std::vector<bdd> raw1 = {bdd_false(),bdd_ithvar(42),bdd_true()};
        std::vector<bdd> raw2 = {bdd_false(),bdd_ithvar(11),bdd_true()};
        
        AssertThat(bvec_equal(bvec(raw1),bvec(raw2)), Is().False());
      });
    });
    describe("bvec_add", []() {
      it("computes bvec[8](5) + bvec[8](3)", [&]() {
        bvec a = bvec_const((char)5);
        bvec b = bvec_const((char)3);
        // 00000101 +
        // 00000011 =
        // 00001000
        std::vector<bdd> raw_expected = {bdd_false(),bdd_false(),bdd_false(),bdd_true()};
        bvec expected = bvec(raw_expected);
        bvec res = bvec_add(a,b);
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(4u));
      });

      it("computes bvec[32](5) + bvec[8](3)", [&]() {
        bvec a = bvec_const((int)5);
        bvec b = bvec_const((char)3);
        // 00000000000000000000000000000101 +
        //                         00000011 =
        // 00000000000000000000000000001000
        std::vector<bdd> raw_expected = {bdd_false(),bdd_false(),bdd_false(),bdd_true()};
        bvec expected = bvec(raw_expected);
        bvec res = bvec_add(a,b);
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(32u));
        AssertThat(res.size(), Is().EqualTo(4u));
      });

      it("computes bvec[32](0) + bvec[8](3)", [&]() {
        bvec a = bvec_const((int)0);
        bvec b = bvec_const((char)3);
        // 00000000000000000000000000000000 +
        //                         00000011 =
        // 00000000000000000000000000000011
        std::vector<bdd> raw_expected = {bdd_true(),bdd_true()};
        bvec expected = bvec(raw_expected);
        bvec res = bvec_add(a,b);
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(32u));
        AssertThat(res.size(), Is().EqualTo(2u));
      });

      it("computes bvec[8](0) + bvec[32](3)", [&]() {
        bvec a = bvec_const((char)0);
        bvec b = bvec_const((int)3);
        //                         00000000 +
        // 00000000000000000000000000000011 =
        // 00000000000000000000000000000011
        std::vector<bdd> raw_expected = {bdd_true(),bdd_true()};
        bvec expected = bvec(raw_expected);
        bvec res = bvec_add(a,b);
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(32u));
        AssertThat(res.size(), Is().EqualTo(2u));
      });

      it("computes bvec[8](255) + bvec[8](1)", [&]() {
        bvec a = bvec_const((char)255);
        bvec b = bvec_const((char)1);
        // 11111111 +
        // 00000001 =
        // 00000000
        std::vector<bdd> raw_expected = {};
        bvec expected = bvec(raw_expected);
        bvec res = bvec_add(a,b);
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(0u));
      });
      
      it("computes bvec[8](255) + bvec[8](1)", [&]() {
        bvec a = bvec_const((char)255);
        bvec b = bvec_const((char)1);
        // 11111111 +
        // 00000001 =
        // 00000000
        std::vector<bdd> raw_expected = {};
        bvec expected = bvec(raw_expected);
        bvec res = a+b;
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(0u));
      });
      
      it("computes bvec[8](42) + char(3)", [&]() {
        bvec a = bvec_const((char)42);
        char b = 3;
        // 00101010 +
        // 00000011 =
        // 00101101
        bvec expected = bvec_const(45);
        bvec res = a+b;
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(6u));
      });
      
      it("computes int(17) + bvec[8](42)", [&]() {
        int a = 17;
        bvec b = bvec_const((char)42);
        // 00010001 +
        // 00101010 =
        // 00111011
        bvec expected = bvec_const(59);
        bvec res = a+b;
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(32u));
        AssertThat(res.size(), Is().EqualTo(6u));
      });
    });
    describe("bvec_truncate", []() {
      it("truncates to bitlen", [&](){
        bvec x = bvec_true(32);


        bvec expected = bvec_true(8);
        bvec res = bvec_truncate(x,8);
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(8u));
      });
      
      it("extends to bitlen", [&](){
        bvec x = bvec_true(4);
        AssertThat(x.bitlen(), Is().EqualTo(4u));

        bvec expected = bvec_true(4);
        bvec res = bvec_truncate(x,8);
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(8u));
        AssertThat(res.size(), Is().EqualTo(4u));
      });

      it("truncates false prefix", [&](){
        bvec x = bvec_const(18);

        bvec expected = bvec_const(2);
        bvec res = bvec_truncate(x,4);
        AssertThat(res, Is().EqualTo(expected)); 
        AssertThat(res.bitlen(), Is().EqualTo(4u));
        AssertThat(res.size(), Is().EqualTo(2u));
      });
    });
  });
});