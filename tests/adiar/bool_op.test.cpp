#include "../test.h"

go_bandit([]() {
  describe("adiar/bool_op.h", []() {
    it("and_op", [&]() {
      AssertThat(adiar::and_op(true, true), Is().EqualTo(true));
      AssertThat(adiar::and_op(true, false), Is().EqualTo(false));
      AssertThat(adiar::and_op(false, true), Is().EqualTo(false));
      AssertThat(adiar::and_op(false, false), Is().EqualTo(false));
    });

    it("nand_op", [&]() {
      AssertThat(adiar::nand_op(true, true), Is().EqualTo(false));
      AssertThat(adiar::nand_op(true, false), Is().EqualTo(true));
      AssertThat(adiar::nand_op(false, true), Is().EqualTo(true));
      AssertThat(adiar::nand_op(false, false), Is().EqualTo(true));
    });

    it("or_op", [&]() {
      AssertThat(adiar::or_op(true, true), Is().EqualTo(true));
      AssertThat(adiar::or_op(true, false), Is().EqualTo(true));
      AssertThat(adiar::or_op(false, true), Is().EqualTo(true));
      AssertThat(adiar::or_op(false, false), Is().EqualTo(false));
    });

    it("nor_op", [&]() {
      AssertThat(adiar::nor_op(true, true), Is().EqualTo(false));
      AssertThat(adiar::nor_op(true, false), Is().EqualTo(false));
      AssertThat(adiar::nor_op(false, true), Is().EqualTo(false));
      AssertThat(adiar::nor_op(false, false), Is().EqualTo(true));
    });

    it("xor_op", [&]() {
      AssertThat(adiar::xor_op(true, true), Is().EqualTo(false));
      AssertThat(adiar::xor_op(true, false), Is().EqualTo(true));
      AssertThat(adiar::xor_op(false, true), Is().EqualTo(true));
      AssertThat(adiar::xor_op(false, false), Is().EqualTo(false));
    });

    it("xnor_op", [&]() {
      AssertThat(adiar::xnor_op(true, true), Is().EqualTo(true));
      AssertThat(adiar::xnor_op(true, false), Is().EqualTo(false));
      AssertThat(adiar::xnor_op(false, true), Is().EqualTo(false));
      AssertThat(adiar::xnor_op(false, false), Is().EqualTo(true));
    });

    it("imp_op", [&]() {
      AssertThat(adiar::imp_op(true, true), Is().EqualTo(true));
      AssertThat(adiar::imp_op(true, false), Is().EqualTo(false));
      AssertThat(adiar::imp_op(false, true), Is().EqualTo(true));
      AssertThat(adiar::imp_op(false, false), Is().EqualTo(true));
    });

    it("invimp_op", [&]() {
      AssertThat(adiar::invimp_op(true, true), Is().EqualTo(true));
      AssertThat(adiar::invimp_op(true, false), Is().EqualTo(true));
      AssertThat(adiar::invimp_op(false, true), Is().EqualTo(false));
      AssertThat(adiar::invimp_op(false, false), Is().EqualTo(true));
    });

    it("equiv_op", [&]() {
      AssertThat(adiar::equiv_op(true, true), Is().EqualTo(true));
      AssertThat(adiar::equiv_op(true, false), Is().EqualTo(false));
      AssertThat(adiar::equiv_op(false, true), Is().EqualTo(false));
      AssertThat(adiar::equiv_op(false, false), Is().EqualTo(true));
    });

    it("diff_op", [&]() {
      AssertThat(adiar::diff_op(true, true), Is().EqualTo(false));
      AssertThat(adiar::diff_op(true, false), Is().EqualTo(true));
      AssertThat(adiar::diff_op(false, true), Is().EqualTo(false));
      AssertThat(adiar::diff_op(false, false), Is().EqualTo(false));
    });

    it("less_op", [&]() {
      AssertThat(adiar::less_op(true, true), Is().EqualTo(false));
      AssertThat(adiar::less_op(true, false), Is().EqualTo(false));
      AssertThat(adiar::less_op(false, true), Is().EqualTo(true));
      AssertThat(adiar::less_op(false, false), Is().EqualTo(false));
    });
  });
});
