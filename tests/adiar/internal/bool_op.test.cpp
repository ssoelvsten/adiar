#include "../../test.h"

#include <adiar/internal/bool_op.h>

go_bandit([]() {
  describe("adiar/internal/bool_op.h", []() {
    describe("predicate<bool, bool> predicates", []() {
      describe("can_shortcut", []() {
        it("is correct for 'adiar::and_op'", []() {
          AssertThat(can_left_shortcut(adiar::and_op, false), Is().True());
          AssertThat(can_left_shortcut(adiar::and_op, true), Is().False());

          AssertThat(can_right_shortcut(adiar::and_op, false), Is().True());
          AssertThat(can_right_shortcut(adiar::and_op, true), Is().False());
        });

        it("is correct for 'adiar::nand_op'", []() {
          AssertThat(can_left_shortcut(adiar::nand_op, false), Is().True());
          AssertThat(can_left_shortcut(adiar::nand_op, true), Is().False());

          AssertThat(can_right_shortcut(adiar::nand_op, false), Is().True());
          AssertThat(can_right_shortcut(adiar::nand_op, true), Is().False());
        });

        it("is correct for 'adiar::or_op'", []() {
          AssertThat(can_left_shortcut(adiar::or_op, false), Is().False());
          AssertThat(can_left_shortcut(adiar::or_op, true), Is().True());

          AssertThat(can_right_shortcut(adiar::or_op, false), Is().False());
          AssertThat(can_right_shortcut(adiar::or_op, true), Is().True());
        });

        it("is correct for 'adiar::nor_op'", []() {
          AssertThat(can_left_shortcut(adiar::nor_op, false), Is().False());
          AssertThat(can_left_shortcut(adiar::nor_op, true), Is().True());

          AssertThat(can_right_shortcut(adiar::nor_op, false), Is().False());
          AssertThat(can_right_shortcut(adiar::nor_op, true), Is().True());
        });

        it("is correct for 'adiar::xor_op'", []() {
          AssertThat(can_left_shortcut(adiar::xor_op, false), Is().False());
          AssertThat(can_left_shortcut(adiar::xor_op, true), Is().False());

          AssertThat(can_right_shortcut(adiar::xor_op, false), Is().False());
          AssertThat(can_right_shortcut(adiar::xor_op, true), Is().False());
        });

        it("is correct for 'adiar::xnor_op'", []() {
          AssertThat(can_left_shortcut(adiar::xnor_op, false), Is().False());
          AssertThat(can_left_shortcut(adiar::xnor_op, true), Is().False());

          AssertThat(can_right_shortcut(adiar::xnor_op, false), Is().False());
          AssertThat(can_right_shortcut(adiar::xnor_op, true), Is().False());
        });

        it("is correct for 'adiar::imp_op'", []() {
          AssertThat(can_left_shortcut(adiar::imp_op, false), Is().True());
          AssertThat(can_left_shortcut(adiar::imp_op, true), Is().False());

          AssertThat(can_right_shortcut(adiar::imp_op, false), Is().False());
          AssertThat(can_right_shortcut(adiar::imp_op, true), Is().True());
        });

        it("is correct for 'adiar::diff_op'", []() {
          AssertThat(can_left_shortcut(adiar::diff_op, false), Is().True());
          AssertThat(can_left_shortcut(adiar::diff_op, true), Is().False());

          AssertThat(can_right_shortcut(adiar::diff_op, false), Is().False());
          AssertThat(can_right_shortcut(adiar::diff_op, true), Is().True());
        });
      });

      describe("is_idempotent", []() {
        it("is correct for 'adiar::and_op'", []() {
          AssertThat(is_left_idempotent(adiar::and_op, false), Is().False());
          AssertThat(is_left_idempotent(adiar::and_op, true), Is().True());

          AssertThat(is_right_idempotent(adiar::and_op, false), Is().False());
          AssertThat(is_right_idempotent(adiar::and_op, true), Is().True());
        });

        it("is correct for 'adiar::nand_op'", []() {
          AssertThat(is_left_idempotent(adiar::nand_op, false), Is().False());
          AssertThat(is_left_idempotent(adiar::nand_op, true), Is().False());

          AssertThat(is_right_idempotent(adiar::nand_op, false), Is().False());
          AssertThat(is_right_idempotent(adiar::nand_op, true), Is().False());
        });

        it("is correct for 'adiar::or_op'", []() {
          AssertThat(is_left_idempotent(adiar::or_op, false), Is().True());
          AssertThat(is_left_idempotent(adiar::or_op, true), Is().False());

          AssertThat(is_right_idempotent(adiar::or_op, false), Is().True());
          AssertThat(is_right_idempotent(adiar::or_op, true), Is().False());
        });

        it("is correct for 'adiar::nor_op'", []() {
          AssertThat(is_left_idempotent(adiar::nor_op, false), Is().False());
          AssertThat(is_left_idempotent(adiar::nor_op, true), Is().False());

          AssertThat(is_right_idempotent(adiar::nor_op, false), Is().False());
          AssertThat(is_right_idempotent(adiar::nor_op, true), Is().False());
        });

        it("is correct for 'adiar::xor_op'", []() {
          AssertThat(is_left_idempotent(adiar::xor_op, false), Is().True());
          AssertThat(is_left_idempotent(adiar::xor_op, true), Is().False());

          AssertThat(is_right_idempotent(adiar::xor_op, false), Is().True());
          AssertThat(is_right_idempotent(adiar::xor_op, true), Is().False());
        });

        it("is correct for 'adiar::xnor_op'", []() {
          AssertThat(is_left_idempotent(adiar::xnor_op, false), Is().False());
          AssertThat(is_left_idempotent(adiar::xnor_op, true), Is().True());

          AssertThat(is_right_idempotent(adiar::xnor_op, false), Is().False());
          AssertThat(is_right_idempotent(adiar::xnor_op, true), Is().True());
        });

        it("is correct for 'adiar::imp_op'", []() {
          AssertThat(is_left_idempotent(adiar::imp_op, false), Is().False());
          AssertThat(is_left_idempotent(adiar::imp_op, true), Is().True());

          AssertThat(is_right_idempotent(adiar::imp_op, false), Is().False());
          AssertThat(is_right_idempotent(adiar::imp_op, true), Is().False());
        });

        it("is correct for 'adiar::diff_op'", []() {
          AssertThat(is_left_idempotent(adiar::diff_op, false), Is().False());
          AssertThat(is_left_idempotent(adiar::diff_op, true), Is().False());

          AssertThat(is_right_idempotent(adiar::diff_op, false), Is().True());
          AssertThat(is_right_idempotent(adiar::diff_op, true), Is().False());
        });
      });

      describe("is_negating", []() {
        it("is correct for 'adiar::and_op'", []() {
          AssertThat(is_left_negating(adiar::and_op, false), Is().False());
          AssertThat(is_left_negating(adiar::and_op, true), Is().False());

          AssertThat(is_right_negating(adiar::and_op, false), Is().False());
          AssertThat(is_right_negating(adiar::and_op, true), Is().False());
        });

        it("is correct for 'adiar::nand_op'", []() {
          AssertThat(is_left_negating(adiar::nand_op, false), Is().False());
          AssertThat(is_left_negating(adiar::nand_op, true), Is().True());

          AssertThat(is_right_negating(adiar::nand_op, false), Is().False());
          AssertThat(is_right_negating(adiar::nand_op, true), Is().True());
        });

        it("is correct for 'adiar::or_op'", []() {
          AssertThat(is_left_negating(adiar::or_op, false), Is().False());
          AssertThat(is_left_negating(adiar::or_op, true), Is().False());

          AssertThat(is_right_negating(adiar::or_op, false), Is().False());
          AssertThat(is_right_negating(adiar::or_op, true), Is().False());
        });

        it("is correct for 'adiar::nor_op'", []() {
          AssertThat(is_left_negating(adiar::nor_op, false), Is().True());
          AssertThat(is_left_negating(adiar::nor_op, true), Is().False());

          AssertThat(is_right_negating(adiar::nor_op, false), Is().True());
          AssertThat(is_right_negating(adiar::nor_op, true), Is().False());
        });

        it("is correct for 'adiar::xor_op'", []() {
          AssertThat(is_left_negating(adiar::xor_op, false), Is().False());
          AssertThat(is_left_negating(adiar::xor_op, true), Is().True());

          AssertThat(is_right_negating(adiar::xor_op, false), Is().False());
          AssertThat(is_right_negating(adiar::xor_op, true), Is().True());
        });

        it("is correct for 'adiar::xnor_op'", []() {
          AssertThat(is_left_negating(adiar::xnor_op, false), Is().True());
          AssertThat(is_left_negating(adiar::xnor_op, true), Is().False());

          AssertThat(is_right_negating(adiar::xnor_op, false), Is().True());
          AssertThat(is_right_negating(adiar::xnor_op, true), Is().False());
        });

        it("is correct for 'adiar::imp_op'", []() {
          AssertThat(is_left_negating(adiar::imp_op, false), Is().False());
          AssertThat(is_left_negating(adiar::imp_op, true), Is().False());

          AssertThat(is_right_negating(adiar::imp_op, false), Is().True());
          AssertThat(is_right_negating(adiar::imp_op, true), Is().False());
        });

        it("is correct for 'adiar::invimp_op'", []() {
          AssertThat(is_left_negating(adiar::invimp_op, false), Is().True());
          AssertThat(is_left_negating(adiar::invimp_op, true), Is().False());

          AssertThat(is_right_negating(adiar::invimp_op, false), Is().False());
          AssertThat(is_right_negating(adiar::invimp_op, true), Is().False());
        });

        it("is correct for 'adiar::diff_op'", []() {
          AssertThat(is_left_negating(adiar::diff_op, false), Is().False());
          AssertThat(is_left_negating(adiar::diff_op, true), Is().True());

          AssertThat(is_right_negating(adiar::diff_op, false), Is().False());
          AssertThat(is_right_negating(adiar::diff_op, true), Is().False());
        });
      });

      describe("is_commutative(...)", []() {
        it("is true for 'adiar::and_op'",
           []() { AssertThat(is_commutative(adiar::and_op), Is().True()); });

        it("is true for 'adiar::nand_op'",
           []() { AssertThat(is_commutative(adiar::nand_op), Is().True()); });

        it("is true for 'adiar::or_op'",
           []() { AssertThat(is_commutative(adiar::or_op), Is().True()); });

        it("is true for 'adiar::nor_op'",
           []() { AssertThat(is_commutative(adiar::nor_op), Is().True()); });

        it("is true for 'adiar::xor_op'",
           []() { AssertThat(is_commutative(adiar::xor_op), Is().True()); });

        it("is true for 'adiar::xnor_op'",
           []() { AssertThat(is_commutative(adiar::xnor_op), Is().True()); });

        it("is true for 'adiar::equiv_op'",
           []() { AssertThat(is_commutative(adiar::equiv_op), Is().True()); });

        it("is false for 'adiar::imp_op'",
           []() { AssertThat(is_commutative(adiar::imp_op), Is().False()); });

        it("is false for 'adiar::invimp_op'",
           []() { AssertThat(is_commutative(adiar::invimp_op), Is().False()); });

        it("is false for 'adiar::diff_op'",
           []() { AssertThat(is_commutative(adiar::diff_op), Is().False()); });

        it("is false for 'adiar::less_op'",
           []() { AssertThat(is_commutative(adiar::less_op), Is().False()); });
      });

      describe("flip(...)", []() {
        it("resolves commutative operator 'adiar::and_op'", []() {
          const predicate<bool, bool>& flip_and_op = flip(adiar::and_op);

          AssertThat(flip_and_op(true, true), Is().EqualTo(true));
          AssertThat(flip_and_op(true, false), Is().EqualTo(false));
          AssertThat(flip_and_op(false, true), Is().EqualTo(false));
          AssertThat(flip_and_op(false, false), Is().EqualTo(false));
        });

        it("resolves non-commutative operator 'adiar::imp_op'", []() {
          const predicate<bool, bool>& flip_imp_op = flip(adiar::imp_op);

          AssertThat(flip_imp_op(true, true), Is().EqualTo(true));
          AssertThat(flip_imp_op(true, false), Is().EqualTo(true));
          AssertThat(flip_imp_op(false, true), Is().EqualTo(false));
          AssertThat(flip_imp_op(false, false), Is().EqualTo(true));
        });
      });
    });

    const ptr_uint64 ptr_false(false);
    const ptr_uint64 ptr_true(true);

    const ptr_uint64 ptr_internal__min(0, 0);
    const ptr_uint64 ptr_internal__max =
      flag(ptr_uint64(ptr_uint64::max_label, ptr_uint64::max_id));

    const ptr_uint64 ptr_nil = ptr_uint64::nil();

    describe("binary_op<predicate<bool, bool>>", [&]() {
      it("uses expected number of bytes",
         []() { AssertThat(sizeof(binary_op<predicate<bool, bool>>), Is().EqualTo(17u)); });

      describe("operator(ptr_uint64, ptr_uint64)", [&]() {
        it("has (pre-computed) values for 'adiar::and_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
        });

        it("has (pre-computed) values for 'adiar::or_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
        });

        it("has (pre-computed) values for 'adiar::xor_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_false));
        });

        it("has (pre-computed) values for 'adiar::imp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
        });

        it("has (pre-computed) values for 'adiar::invimp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
        });

        it("has (pre-computed) values for 'adiar::diff_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::diff_op);

          AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
          AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_false));
        });
      });

      describe(".can_shortcut(bool)", []() {
        it("has (pre-computed) values for 'adiar::and_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.can_left_shortcut(false), Is().True());
          AssertThat(op.can_right_shortcut(false), Is().True());

          AssertThat(op.can_left_shortcut(true), Is().False());
          AssertThat(op.can_right_shortcut(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::or_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);

          AssertThat(op.can_left_shortcut(false), Is().False());
          AssertThat(op.can_right_shortcut(false), Is().False());

          AssertThat(op.can_left_shortcut(true), Is().True());
          AssertThat(op.can_right_shortcut(true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::xor_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.can_left_shortcut(false), Is().False());
          AssertThat(op.can_right_shortcut(false), Is().False());

          AssertThat(op.can_left_shortcut(true), Is().False());
          AssertThat(op.can_right_shortcut(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::imp_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.can_left_shortcut(false), Is().True());
          AssertThat(op.can_right_shortcut(false), Is().False());

          AssertThat(op.can_left_shortcut(true), Is().False());
          AssertThat(op.can_right_shortcut(true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::invimp_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);

          AssertThat(op.can_left_shortcut(false), Is().False());
          AssertThat(op.can_right_shortcut(false), Is().True());

          AssertThat(op.can_left_shortcut(true), Is().True());
          AssertThat(op.can_right_shortcut(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::diff_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::diff_op);

          AssertThat(op.can_left_shortcut(false), Is().True());
          AssertThat(op.can_right_shortcut(false), Is().False());

          AssertThat(op.can_left_shortcut(true), Is().False());
          AssertThat(op.can_right_shortcut(true), Is().True());
        });
      });

      describe(".can_shortcut(const ptr_uint64&)", [&]() {
        it("has (pre-computed) values for terminals in 'adiar::and_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.can_left_shortcut(ptr_false), Is().True());
          AssertThat(op.can_right_shortcut(ptr_false), Is().True());

          AssertThat(op.can_left_shortcut(ptr_true), Is().False());
          AssertThat(op.can_right_shortcut(ptr_true), Is().False());
        });

        it("has (pre-computed) values for terminals 'adiar::xor_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.can_left_shortcut(ptr_false), Is().False());
          AssertThat(op.can_right_shortcut(ptr_false), Is().False());

          AssertThat(op.can_left_shortcut(ptr_true), Is().False());
          AssertThat(op.can_right_shortcut(ptr_true), Is().False());
        });

        it("has (pre-computed) values for terminals 'adiar::imp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.can_left_shortcut(ptr_false), Is().True());
          AssertThat(op.can_right_shortcut(ptr_false), Is().False());

          AssertThat(op.can_left_shortcut(ptr_true), Is().False());
          AssertThat(op.can_right_shortcut(ptr_true), Is().True());
        });

        it("is false for (unflagged) internal node", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.can_left_shortcut(ptr_internal__min), Is().False());
          AssertThat(op.can_right_shortcut(ptr_internal__min), Is().False());
        });

        it("is false for (flagged) internal node", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.can_left_shortcut(ptr_internal__max), Is().False());
          AssertThat(op.can_right_shortcut(ptr_internal__max), Is().False());
        });

        it("is false for nil", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);
          AssertThat(op.can_left_shortcut(ptr_nil), Is().False());
          AssertThat(op.can_right_shortcut(ptr_nil), Is().False());
        });
      });

      describe(".is_idempotent(bool)", []() {
        it("has (pre-computed) values for 'adiar::and_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.is_left_idempotent(false), Is().False());
          AssertThat(op.is_right_idempotent(false), Is().False());

          AssertThat(op.is_left_idempotent(true), Is().True());
          AssertThat(op.is_right_idempotent(true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::or_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);

          AssertThat(op.is_left_idempotent(false), Is().True());
          AssertThat(op.is_right_idempotent(false), Is().True());

          AssertThat(op.is_left_idempotent(true), Is().False());
          AssertThat(op.is_right_idempotent(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::xor_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.is_left_idempotent(false), Is().True());
          AssertThat(op.is_right_idempotent(false), Is().True());

          AssertThat(op.is_left_idempotent(true), Is().False());
          AssertThat(op.is_right_idempotent(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::imp_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.is_left_idempotent(false), Is().False());
          AssertThat(op.is_right_idempotent(false), Is().False());

          AssertThat(op.is_left_idempotent(true), Is().True());
          AssertThat(op.is_right_idempotent(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::invimp_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);

          AssertThat(op.is_left_idempotent(false), Is().False());
          AssertThat(op.is_right_idempotent(false), Is().False());

          AssertThat(op.is_left_idempotent(true), Is().False());
          AssertThat(op.is_right_idempotent(true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::diff_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::diff_op);

          AssertThat(op.is_left_idempotent(false), Is().False());
          AssertThat(op.is_right_idempotent(false), Is().True());

          AssertThat(op.is_left_idempotent(true), Is().False());
          AssertThat(op.is_right_idempotent(true), Is().False());
        });
      });

      describe(".is_idempotent(const ptr_uint64&)", [&]() {
        it("has (pre-computed) values for 'adiar::and_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.is_left_idempotent(ptr_false), Is().False());
          AssertThat(op.is_right_idempotent(ptr_false), Is().False());

          AssertThat(op.is_left_idempotent(ptr_true), Is().True());
          AssertThat(op.is_right_idempotent(ptr_true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::xor_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.is_left_idempotent(ptr_false), Is().True());
          AssertThat(op.is_right_idempotent(ptr_false), Is().True());

          AssertThat(op.is_left_idempotent(ptr_true), Is().False());
          AssertThat(op.is_right_idempotent(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::imp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.is_left_idempotent(ptr_false), Is().False());
          AssertThat(op.is_right_idempotent(ptr_false), Is().False());

          AssertThat(op.is_left_idempotent(ptr_true), Is().True());
          AssertThat(op.is_right_idempotent(ptr_true), Is().False());
        });

        it("is false for (unflagged) internal node", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.is_left_idempotent(ptr_internal__min), Is().False());
          AssertThat(op.is_right_idempotent(ptr_internal__min), Is().False());
        });

        it("is false for (flagged) internal node", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.is_left_idempotent(ptr_internal__max), Is().False());
          AssertThat(op.is_right_idempotent(ptr_internal__max), Is().False());
        });

        it("is false for nil", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);
          AssertThat(op.is_left_idempotent(ptr_nil), Is().False());
          AssertThat(op.is_right_idempotent(ptr_nil), Is().False());
        });
      });

      describe(".is_negating(bool)", []() {
        it("has (pre-computed) values for 'adiar::and_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.is_left_negating(false), Is().False());
          AssertThat(op.is_right_negating(false), Is().False());

          AssertThat(op.is_left_negating(true), Is().False());
          AssertThat(op.is_right_negating(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::or_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);

          AssertThat(op.is_left_negating(false), Is().False());
          AssertThat(op.is_right_negating(false), Is().False());

          AssertThat(op.is_left_negating(true), Is().False());
          AssertThat(op.is_right_negating(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::xor_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.is_left_negating(false), Is().False());
          AssertThat(op.is_right_negating(false), Is().False());

          AssertThat(op.is_left_negating(true), Is().True());
          AssertThat(op.is_right_negating(true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::imp_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.is_left_negating(false), Is().False());
          AssertThat(op.is_right_negating(false), Is().True());

          AssertThat(op.is_left_negating(true), Is().False());
          AssertThat(op.is_right_negating(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::invimp_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);

          AssertThat(op.is_left_negating(false), Is().True());
          AssertThat(op.is_right_negating(false), Is().False());

          AssertThat(op.is_left_negating(true), Is().False());
          AssertThat(op.is_right_negating(true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::diff_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::diff_op);

          AssertThat(op.is_left_negating(false), Is().False());
          AssertThat(op.is_right_negating(false), Is().False());

          AssertThat(op.is_left_negating(true), Is().True());
          AssertThat(op.is_right_negating(true), Is().False());
        });
      });

      describe(".is_negating(const ptr_uint64&)", [&]() {
        it("has (pre-computed) values for 'adiar::and_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.is_left_negating(ptr_false), Is().False());
          AssertThat(op.is_right_negating(ptr_false), Is().False());

          AssertThat(op.is_left_negating(ptr_true), Is().False());
          AssertThat(op.is_right_negating(ptr_true), Is().False());
        });

        it("has (pre-computed) values for 'adiar::xor_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.is_left_negating(ptr_false), Is().False());
          AssertThat(op.is_right_negating(ptr_false), Is().False());

          AssertThat(op.is_left_negating(ptr_true), Is().True());
          AssertThat(op.is_right_negating(ptr_true), Is().True());
        });

        it("has (pre-computed) values for 'adiar::imp_op'", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);

          AssertThat(op.is_left_negating(ptr_false), Is().False());
          AssertThat(op.is_right_negating(ptr_false), Is().True());

          AssertThat(op.is_left_negating(ptr_true), Is().False());
          AssertThat(op.is_right_negating(ptr_true), Is().False());
        });

        it("is false for (unflagged) internal node", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);

          AssertThat(op.is_left_negating(ptr_internal__min), Is().False());
          AssertThat(op.is_right_negating(ptr_internal__min), Is().False());
        });

        it("is false for (flagged) internal node", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);

          AssertThat(op.is_left_negating(ptr_internal__max), Is().False());
          AssertThat(op.is_right_negating(ptr_internal__max), Is().False());
        });

        it("is false for nil", [&]() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);
          AssertThat(op.is_left_negating(ptr_nil), Is().False());
          AssertThat(op.is_right_negating(ptr_nil), Is().False());
        });
      });

      describe(".is_commutative()", [&]() {
        it("has (pre-computed) values for 'adiar::and_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::and_op);
          AssertThat(op.is_commutative(), Is().True());
        });

        it("has (pre-computed) value for 'adiar::or_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::or_op);
          AssertThat(op.is_commutative(), Is().True());
        });

        it("has (pre-computed) value for 'adiar::xor_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::xor_op);
          AssertThat(op.is_commutative(), Is().True());
        });

        it("has (pre-computed) value for 'adiar::imp_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::imp_op);
          AssertThat(op.is_commutative(), Is().False());
        });

        it("has (pre-computed) value for 'adiar::invimp_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::invimp_op);
          AssertThat(op.is_commutative(), Is().False());
        });

        it("has (pre-computed) value for 'adiar::diff_op'", []() {
          binary_op<predicate<bool, bool>> op(adiar::diff_op);
          AssertThat(op.is_commutative(), Is().False());
        });
      });

      describe(".flip()", [&]() {
        it("flips correctly 'adiar::and_op'", [&]() {
          const binary_op<predicate<bool, bool>> op(adiar::and_op);
          const typename binary_op<predicate<bool, bool>>::flipped_operator flipped_op = op.flip();

          AssertThat(flipped_op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(flipped_op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
          AssertThat(flipped_op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(flipped_op(ptr_true, ptr_true), Is().EqualTo(ptr_true));

          AssertThat(flipped_op.can_left_shortcut(ptr_false), Is().True());
          AssertThat(flipped_op.can_left_shortcut(ptr_true), Is().False());
          AssertThat(flipped_op.can_right_shortcut(ptr_false), Is().True());
          AssertThat(flipped_op.can_right_shortcut(ptr_true), Is().False());

          AssertThat(flipped_op.is_left_idempotent(ptr_false), Is().False());
          AssertThat(flipped_op.is_left_idempotent(ptr_true), Is().True());
          AssertThat(flipped_op.is_right_idempotent(ptr_false), Is().False());
          AssertThat(flipped_op.is_right_idempotent(ptr_true), Is().True());

          AssertThat(flipped_op.is_left_negating(ptr_false), Is().False());
          AssertThat(flipped_op.is_left_negating(ptr_true), Is().False());
          AssertThat(flipped_op.is_right_negating(ptr_false), Is().False());
          AssertThat(flipped_op.is_right_negating(ptr_true), Is().False());

          AssertThat(flipped_op.is_commutative(), Is().True());
        });

        it("flips correctly 'adiar::imp_op'", [&]() {
          const binary_op<predicate<bool, bool>> op(adiar::imp_op);
          const typename binary_op<predicate<bool, bool>>::flipped_operator flipped_op = op.flip();

          AssertThat(flipped_op(ptr_false, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(flipped_op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
          AssertThat(flipped_op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
          AssertThat(flipped_op(ptr_true, ptr_true), Is().EqualTo(ptr_true));

          AssertThat(flipped_op.can_left_shortcut(ptr_false), Is().False());
          AssertThat(flipped_op.can_left_shortcut(ptr_true), Is().True());
          AssertThat(flipped_op.can_right_shortcut(ptr_false), Is().True());
          AssertThat(flipped_op.can_right_shortcut(ptr_true), Is().False());

          AssertThat(flipped_op.is_left_idempotent(ptr_false), Is().False());
          AssertThat(flipped_op.is_left_idempotent(ptr_true), Is().False());
          AssertThat(flipped_op.is_right_idempotent(ptr_false), Is().False());
          AssertThat(flipped_op.is_right_idempotent(ptr_true), Is().True());

          AssertThat(flipped_op.is_left_negating(ptr_false), Is().True());
          AssertThat(flipped_op.is_left_negating(ptr_true), Is().False());
          AssertThat(flipped_op.is_right_negating(ptr_false), Is().False());
          AssertThat(flipped_op.is_right_negating(ptr_true), Is().False());

          AssertThat(flipped_op.is_commutative(), Is().False());
        });

        it("flips correctly 'adiar::diff_op'", [&]() {
          const binary_op<predicate<bool, bool>> op(adiar::diff_op);
          const typename binary_op<predicate<bool, bool>>::flipped_operator flipped_op = op.flip();

          AssertThat(flipped_op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(flipped_op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
          AssertThat(flipped_op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
          AssertThat(flipped_op(ptr_true, ptr_true), Is().EqualTo(ptr_false));

          AssertThat(flipped_op.can_left_shortcut(ptr_false), Is().False());
          AssertThat(flipped_op.can_left_shortcut(ptr_true), Is().True());
          AssertThat(flipped_op.can_right_shortcut(ptr_false), Is().True());
          AssertThat(flipped_op.can_right_shortcut(ptr_true), Is().False());

          AssertThat(flipped_op.is_left_idempotent(ptr_false), Is().True());
          AssertThat(flipped_op.is_left_idempotent(ptr_true), Is().False());
          AssertThat(flipped_op.is_right_idempotent(ptr_false), Is().False());
          AssertThat(flipped_op.is_right_idempotent(ptr_true), Is().False());

          AssertThat(flipped_op.is_left_negating(ptr_false), Is().False());
          AssertThat(flipped_op.is_left_negating(ptr_true), Is().False());
          AssertThat(flipped_op.is_right_negating(ptr_false), Is().False());
          AssertThat(flipped_op.is_right_negating(ptr_true), Is().True());

          AssertThat(flipped_op.is_commutative(), Is().False());
        });
      });
    });

    describe("and_op", [&]() {
      const adiar::internal::and_op op;

      it("operator() (const bool, const bool)", [&]() {
        AssertThat(op(false, false), Is().EqualTo(false));
        AssertThat(op(false, true), Is().EqualTo(false));
        AssertThat(op(true, false), Is().EqualTo(false));
        AssertThat(op(true, true), Is().EqualTo(true));
      });

      it("operator() (const Pointer&, const Pointer&)", [&]() {
        AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
      });

      it(".can_left_shortcut(bool)", [&]() {
        AssertThat(op.can_left_shortcut(false), Is().True());
        AssertThat(op.can_left_shortcut(true), Is().False());
      });

      it(".can_left_shortcut(const Pointer&) [terminals]", [&]() {
        AssertThat(op.can_left_shortcut(ptr_false), Is().True());
        AssertThat(op.can_left_shortcut(ptr_true), Is().False());
      });

      it(".can_left_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__min), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__max), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_nil), Is().False()); });

      it(".can_right_shortcut(bool)", [&]() {
        AssertThat(op.can_right_shortcut(false), Is().True());
        AssertThat(op.can_right_shortcut(true), Is().False());
      });

      it(".can_right_shortcut(const Pointer&) [terminals]", [&]() {
        AssertThat(op.can_right_shortcut(ptr_false), Is().True());
        AssertThat(op.can_right_shortcut(ptr_true), Is().False());
      });

      it(".can_right_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__min), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__max), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_nil), Is().False()); });

      it(".is_left_idempotent(bool)", [&]() {
        AssertThat(op.is_left_idempotent(false), Is().False());
        AssertThat(op.is_left_idempotent(true), Is().True());
      });

      it(".is_left_idempotent(const Pointer&) [terminals]", [&]() {
        AssertThat(op.is_left_idempotent(ptr_false), Is().False());
        AssertThat(op.is_left_idempotent(ptr_true), Is().True());
      });

      it(".is_left_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__min), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__max), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_nil), Is().False()); });

      it(".is_right_idempotent(bool)", [&]() {
        AssertThat(op.is_right_idempotent(false), Is().False());
        AssertThat(op.is_right_idempotent(true), Is().True());
      });

      it(".is_right_idempotent(const Pointer&) [terminals]", [&]() {
        AssertThat(op.is_right_idempotent(ptr_false), Is().False());
        AssertThat(op.is_right_idempotent(ptr_true), Is().True());
      });

      it(".is_right_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__min), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__max), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_nil), Is().False()); });

      it(".is_left_negating(bool)", [&]() {
        AssertThat(op.is_left_negating(false), Is().False());
        AssertThat(op.is_left_negating(true), Is().False());
      });

      it(".is_left_negating(const Pointer&) [terminals]", [&]() {
        AssertThat(op.is_left_negating(ptr_false), Is().False());
        AssertThat(op.is_left_negating(ptr_true), Is().False());
      });

      it(".is_left_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__min), Is().False()); });

      it(".is_left_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__max), Is().False()); });

      it(".is_left_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_negating(ptr_nil), Is().False()); });

      it(".is_right_negating(bool)", [&]() {
        AssertThat(op.is_right_negating(false), Is().False());
        AssertThat(op.is_right_negating(true), Is().False());
      });

      it(".is_right_negating(const Pointer&) [terminals]", [&]() {
        AssertThat(op.is_right_negating(ptr_false), Is().False());
        AssertThat(op.is_right_negating(ptr_true), Is().False());
      });

      it(".is_right_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__min), Is().False()); });

      it(".is_right_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__max), Is().False()); });

      it(".is_right_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_negating(ptr_nil), Is().False()); });

      it(".is_commutative()", [&]() { AssertThat(op.is_commutative(), Is().True()); });
    });

    describe("nand_op", [&]() {
      const adiar::internal::nand_op op;

      it("operator() (const bool, const bool)", [&]() {
        AssertThat(op(false, false), Is().EqualTo(true));
        AssertThat(op(false, true), Is().EqualTo(true));
        AssertThat(op(true, false), Is().EqualTo(true));
        AssertThat(op(true, true), Is().EqualTo(false));
      });

      it("operator() (const Pointer&, const Pointer&)", [&]() {
        AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_false));
      });

      it(".can_left_shortcut(bool)", [&]() {
        AssertThat(op.can_left_shortcut(false), Is().True());
        AssertThat(op.can_left_shortcut(true), Is().False());
      });

      it(".can_left_shortcut(const Pointer&) [terminals]", [&]() {
        AssertThat(op.can_left_shortcut(ptr_false), Is().True());
        AssertThat(op.can_left_shortcut(ptr_true), Is().False());
      });

      it(".can_left_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__min), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__max), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_nil), Is().False()); });

      it(".can_right_shortcut(bool)", [&]() {
        AssertThat(op.can_right_shortcut(false), Is().True());
        AssertThat(op.can_right_shortcut(true), Is().False());
      });

      it(".can_right_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_right_shortcut(ptr_false), Is().True());
        AssertThat(op.can_right_shortcut(ptr_true), Is().False());
      });

      it(".can_right_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__min), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__max), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_nil), Is().False()); });

      it(".is_left_idempotent(bool)", [&]() {
        AssertThat(op.is_left_idempotent(false), Is().False());
        AssertThat(op.is_left_idempotent(true), Is().False());
      });

      it(".is_left_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_left_idempotent(ptr_false), Is().False());
        AssertThat(op.is_left_idempotent(ptr_true), Is().False());
      });

      it(".is_left_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__min), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__max), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_nil), Is().False()); });

      it(".is_right_idempotent(bool)", [&]() {
        AssertThat(op.is_right_idempotent(false), Is().False());
        AssertThat(op.is_right_idempotent(true), Is().False());
      });

      it(".is_right_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_right_idempotent(ptr_false), Is().False());
        AssertThat(op.is_right_idempotent(ptr_true), Is().False());
      });

      it(".is_right_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__min), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__max), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_nil), Is().False()); });

      it(".is_left_negating(bool)", [&]() {
        AssertThat(op.is_left_negating(false), Is().False());
        AssertThat(op.is_left_negating(true), Is().True());
      });

      it(".is_left_negating(const Pointer&)", [&]() {
        AssertThat(op.is_left_negating(ptr_false), Is().False());
        AssertThat(op.is_left_negating(ptr_true), Is().True());
      });

      it(".is_left_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__min), Is().False()); });

      it(".is_left_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__max), Is().False()); });

      it(".is_left_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_negating(ptr_nil), Is().False()); });

      it(".is_right_negating(bool)", [&]() {
        AssertThat(op.is_right_negating(false), Is().False());
        AssertThat(op.is_right_negating(true), Is().True());
      });

      it(".is_right_negating(const Pointer&)", [&]() {
        AssertThat(op.is_right_negating(ptr_false), Is().False());
        AssertThat(op.is_right_negating(ptr_true), Is().True());
      });

      it(".is_right_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__min), Is().False()); });

      it(".is_right_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__max), Is().False()); });

      it(".is_right_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_negating(ptr_nil), Is().False()); });

      it(".is_commutative()", [&]() { AssertThat(op.is_commutative(), Is().True()); });
    });

    describe("or_op", [&]() {
      const adiar::internal::or_op op;

      it("operator() (const bool, const bool)", [&]() {
        AssertThat(op(false, false), Is().EqualTo(false));
        AssertThat(op(false, true), Is().EqualTo(true));
        AssertThat(op(true, false), Is().EqualTo(true));
        AssertThat(op(true, true), Is().EqualTo(true));
      });

      it("operator() (const Pointer&, const Pointer&)", [&]() {
        AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
      });

      it(".can_left_shortcut(bool)", [&]() {
        AssertThat(op.can_left_shortcut(false), Is().False());
        AssertThat(op.can_left_shortcut(true), Is().True());
      });

      it(".can_left_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_left_shortcut(ptr_false), Is().False());
        AssertThat(op.can_left_shortcut(ptr_true), Is().True());
      });

      it(".can_left_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__min), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__max), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_nil), Is().False()); });

      it(".can_right_shortcut(bool)", [&]() {
        AssertThat(op.can_right_shortcut(false), Is().False());
        AssertThat(op.can_right_shortcut(true), Is().True());
      });

      it(".can_right_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_right_shortcut(ptr_false), Is().False());
        AssertThat(op.can_right_shortcut(ptr_true), Is().True());
      });

      it(".can_right_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__min), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__max), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_nil), Is().False()); });

      it(".is_left_idempotent(bool)", [&]() {
        AssertThat(op.is_left_idempotent(false), Is().True());
        AssertThat(op.is_left_idempotent(true), Is().False());
      });

      it(".is_left_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_left_idempotent(ptr_false), Is().True());
        AssertThat(op.is_left_idempotent(ptr_true), Is().False());
      });

      it(".is_left_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__min), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__max), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_nil), Is().False()); });

      it(".is_right_idempotent(bool)", [&]() {
        AssertThat(op.is_right_idempotent(false), Is().True());
        AssertThat(op.is_right_idempotent(true), Is().False());
      });

      it(".is_right_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_right_idempotent(ptr_false), Is().True());
        AssertThat(op.is_right_idempotent(ptr_true), Is().False());
      });

      it(".is_right_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__min), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__max), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_nil), Is().False()); });

      it(".is_left_negating(bool)", [&]() {
        AssertThat(op.is_left_negating(false), Is().False());
        AssertThat(op.is_left_negating(true), Is().False());
      });

      it(".is_left_negating(const Pointer&)", [&]() {
        AssertThat(op.is_left_negating(ptr_false), Is().False());
        AssertThat(op.is_left_negating(ptr_true), Is().False());
      });

      it(".is_left_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__min), Is().False()); });

      it(".is_left_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__max), Is().False()); });

      it(".is_left_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_negating(ptr_nil), Is().False()); });

      it(".is_right_negating(bool)", [&]() {
        AssertThat(op.is_right_negating(false), Is().False());
        AssertThat(op.is_right_negating(true), Is().False());
      });

      it(".is_right_negating(const Pointer&)", [&]() {
        AssertThat(op.is_right_negating(ptr_false), Is().False());
        AssertThat(op.is_right_negating(ptr_true), Is().False());
      });

      it(".is_right_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__min), Is().False()); });

      it(".is_right_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__max), Is().False()); });

      it(".is_right_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_negating(ptr_nil), Is().False()); });

      it(".is_commutative()", [&]() { AssertThat(op.is_commutative(), Is().True()); });
    });

    describe("nor_op", [&]() {
      const adiar::internal::nor_op op;

      it("operator() (const bool, const bool)", [&]() {
        AssertThat(op(false, false), Is().EqualTo(true));
        AssertThat(op(false, true), Is().EqualTo(false));
        AssertThat(op(true, false), Is().EqualTo(false));
        AssertThat(op(true, true), Is().EqualTo(false));
      });

      it("operator() (const Pointer&, const Pointer&)", [&]() {
        AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_false));
      });

      it(".can_left_shortcut(bool)", [&]() {
        AssertThat(op.can_left_shortcut(false), Is().False());
        AssertThat(op.can_left_shortcut(true), Is().True());
      });

      it(".can_left_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_left_shortcut(ptr_false), Is().False());
        AssertThat(op.can_left_shortcut(ptr_true), Is().True());
      });

      it(".can_left_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__min), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__max), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_nil), Is().False()); });

      it(".can_right_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_right_shortcut(ptr_false), Is().False());
        AssertThat(op.can_right_shortcut(ptr_true), Is().True());
      });

      it(".can_right_shortcut(bool)", [&]() {
        AssertThat(op.can_right_shortcut(false), Is().False());
        AssertThat(op.can_right_shortcut(true), Is().True());
      });

      it(".can_right_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__min), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__max), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_nil), Is().False()); });

      it(".is_left_idempotent(bool)", [&]() {
        AssertThat(op.is_left_idempotent(false), Is().False());
        AssertThat(op.is_left_idempotent(true), Is().False());
      });

      it(".is_left_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_left_idempotent(ptr_false), Is().False());
        AssertThat(op.is_left_idempotent(ptr_true), Is().False());
      });

      it(".is_left_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__min), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__max), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_nil), Is().False()); });

      it(".is_right_idempotent(bool)", [&]() {
        AssertThat(op.is_right_idempotent(false), Is().False());
        AssertThat(op.is_right_idempotent(true), Is().False());
      });

      it(".is_right_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_right_idempotent(ptr_false), Is().False());
        AssertThat(op.is_right_idempotent(ptr_true), Is().False());
      });

      it(".is_right_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__min), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__max), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_nil), Is().False()); });

      it(".is_left_negating(bool)", [&]() {
        AssertThat(op.is_left_negating(false), Is().True());
        AssertThat(op.is_left_negating(true), Is().False());
      });

      it(".is_left_negating(const Pointer&)", [&]() {
        AssertThat(op.is_left_negating(ptr_false), Is().True());
        AssertThat(op.is_left_negating(ptr_true), Is().False());
      });

      it(".is_left_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__min), Is().False()); });

      it(".is_left_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__max), Is().False()); });

      it(".is_left_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_negating(ptr_nil), Is().False()); });

      it(".is_right_negating(bool)", [&]() {
        AssertThat(op.is_right_negating(false), Is().True());
        AssertThat(op.is_right_negating(true), Is().False());
      });

      it(".is_right_negating(const Pointer&)", [&]() {
        AssertThat(op.is_right_negating(ptr_false), Is().True());
        AssertThat(op.is_right_negating(ptr_true), Is().False());
      });

      it(".is_right_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__min), Is().False()); });

      it(".is_right_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__max), Is().False()); });

      it(".is_right_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_negating(ptr_nil), Is().False()); });

      it(".is_commutative()", [&]() { AssertThat(op.is_commutative(), Is().True()); });
    });

    describe("xor_op", [&]() {
      const adiar::internal::xor_op op;

      it("operator() (const bool, const bool)", [&]() {
        AssertThat(op(false, false), Is().EqualTo(false));
        AssertThat(op(false, true), Is().EqualTo(true));
        AssertThat(op(true, false), Is().EqualTo(true));
        AssertThat(op(true, true), Is().EqualTo(false));
      });

      it("operator() (const Pointer&, const Pointer&)", [&]() {
        AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_false));
      });

      it(".can_left_shortcut(bool)", [&]() {
        AssertThat(op.can_left_shortcut(false), Is().False());
        AssertThat(op.can_left_shortcut(true), Is().False());
      });

      it(".can_left_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_left_shortcut(ptr_false), Is().False());
        AssertThat(op.can_left_shortcut(ptr_true), Is().False());
      });

      it(".can_left_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__min), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__max), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_nil), Is().False()); });

      it(".can_right_shortcut(bool)", [&]() {
        AssertThat(op.can_right_shortcut(false), Is().False());
        AssertThat(op.can_right_shortcut(true), Is().False());
      });

      it(".can_right_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_right_shortcut(ptr_false), Is().False());
        AssertThat(op.can_right_shortcut(ptr_true), Is().False());
      });

      it(".can_right_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__min), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__max), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_nil), Is().False()); });

      it(".is_left_idempotent(bool)", [&]() {
        AssertThat(op.is_left_idempotent(false), Is().True());
        AssertThat(op.is_left_idempotent(true), Is().False());
      });

      it(".is_left_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_left_idempotent(ptr_false), Is().True());
        AssertThat(op.is_left_idempotent(ptr_true), Is().False());
      });

      it(".is_left_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__min), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__max), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_nil), Is().False()); });

      it(".is_right_idempotent(bool)", [&]() {
        AssertThat(op.is_right_idempotent(false), Is().True());
        AssertThat(op.is_right_idempotent(true), Is().False());
      });

      it(".is_right_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_right_idempotent(ptr_false), Is().True());
        AssertThat(op.is_right_idempotent(ptr_true), Is().False());
      });

      it(".is_right_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__min), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__max), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_nil), Is().False()); });

      it(".is_left_negating(bool)", [&]() {
        AssertThat(op.is_left_negating(false), Is().False());
        AssertThat(op.is_left_negating(true), Is().True());
      });

      it(".is_left_negating(const Pointer&)", [&]() {
        AssertThat(op.is_left_negating(ptr_false), Is().False());
        AssertThat(op.is_left_negating(ptr_true), Is().True());
      });

      it(".is_left_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__min), Is().False()); });

      it(".is_left_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__max), Is().False()); });

      it(".is_left_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_negating(ptr_nil), Is().False()); });

      it(".is_right_negating(bool)", [&]() {
        AssertThat(op.is_right_negating(false), Is().False());
        AssertThat(op.is_right_negating(true), Is().True());
      });

      it(".is_right_negating(const Pointer&)", [&]() {
        AssertThat(op.is_right_negating(ptr_false), Is().False());
        AssertThat(op.is_right_negating(ptr_true), Is().True());
      });

      it(".is_right_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__min), Is().False()); });

      it(".is_right_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__max), Is().False()); });

      it(".is_right_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_negating(ptr_nil), Is().False()); });

      it(".is_commutative()", [&]() { AssertThat(op.is_commutative(), Is().True()); });
    });

    describe("xnor_op, equiv_op", [&]() {
      const adiar::internal::xnor_op op;

      it("operator() (const bool, const bool)", [&]() {
        AssertThat(op(false, false), Is().EqualTo(true));
        AssertThat(op(false, true), Is().EqualTo(false));
        AssertThat(op(true, false), Is().EqualTo(false));
        AssertThat(op(true, true), Is().EqualTo(true));
      });

      it("operator() (const Pointer&, const Pointer&)", [&]() {
        AssertThat(op(ptr_false, ptr_false), Is().EqualTo(ptr_true));
        AssertThat(op(ptr_false, ptr_true), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_true, ptr_false), Is().EqualTo(ptr_false));
        AssertThat(op(ptr_true, ptr_true), Is().EqualTo(ptr_true));
      });

      it(".can_left_shortcut(bool)", [&]() {
        AssertThat(op.can_left_shortcut(false), Is().False());
        AssertThat(op.can_left_shortcut(true), Is().False());
      });

      it(".can_left_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_left_shortcut(ptr_false), Is().False());
        AssertThat(op.can_left_shortcut(ptr_true), Is().False());
      });

      it(".can_left_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__min), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_internal__max), Is().False()); });

      it(".can_left_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_left_shortcut(ptr_nil), Is().False()); });

      it(".can_right_shortcut(bool)", [&]() {
        AssertThat(op.can_right_shortcut(false), Is().False());
        AssertThat(op.can_right_shortcut(true), Is().False());
      });

      it(".can_right_shortcut(const Pointer&)", [&]() {
        AssertThat(op.can_right_shortcut(ptr_false), Is().False());
        AssertThat(op.can_right_shortcut(ptr_true), Is().False());
      });

      it(".can_right_shortcut(const Pointer&) [internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__min), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_internal__max), Is().False()); });

      it(".can_right_shortcut(const Pointer&) [nil]",
         [&]() { AssertThat(op.can_right_shortcut(ptr_nil), Is().False()); });

      it(".is_left_idempotent(bool)", [&]() {
        AssertThat(op.is_left_idempotent(false), Is().False());
        AssertThat(op.is_left_idempotent(true), Is().True());
      });

      it(".is_left_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_left_idempotent(ptr_false), Is().False());
        AssertThat(op.is_left_idempotent(ptr_true), Is().True());
      });

      it(".is_left_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__min), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_internal__max), Is().False()); });

      it(".is_left_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_idempotent(ptr_nil), Is().False()); });

      it(".is_right_idempotent(bool)", [&]() {
        AssertThat(op.is_right_idempotent(false), Is().False());
        AssertThat(op.is_right_idempotent(true), Is().True());
      });

      it(".is_right_idempotent(const Pointer&)", [&]() {
        AssertThat(op.is_right_idempotent(ptr_false), Is().False());
        AssertThat(op.is_right_idempotent(ptr_true), Is().True());
      });

      it(".is_right_idempotent(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__min), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_internal__max), Is().False()); });

      it(".is_right_idempotent(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_idempotent(ptr_nil), Is().False()); });

      it(".is_left_negating(bool)", [&]() {
        AssertThat(op.is_left_negating(false), Is().True());
        AssertThat(op.is_left_negating(true), Is().False());
      });

      it(".is_left_negating(const Pointer&)", [&]() {
        AssertThat(op.is_left_negating(ptr_false), Is().True());
        AssertThat(op.is_left_negating(ptr_true), Is().False());
      });

      it(".is_left_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__min), Is().False()); });

      it(".is_left_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_left_negating(ptr_internal__max), Is().False()); });

      it(".is_left_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_left_negating(ptr_nil), Is().False()); });

      it(".is_right_negating(bool)", [&]() {
        AssertThat(op.is_right_negating(false), Is().True());
        AssertThat(op.is_right_negating(true), Is().False());
      });

      it(".is_right_negating(const Pointer&)", [&]() {
        AssertThat(op.is_right_negating(ptr_false), Is().True());
        AssertThat(op.is_right_negating(ptr_true), Is().False());
      });

      it(".is_right_negating(const Pointer&) [internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__min), Is().False()); });

      it(".is_right_negating(const Pointer&) [flagged internal]",
         [&]() { AssertThat(op.is_right_negating(ptr_internal__max), Is().False()); });

      it(".is_right_negating(const Pointer&) [nil]",
         [&]() { AssertThat(op.is_right_negating(ptr_nil), Is().False()); });

      it(".is_commutative()", [&]() { AssertThat(op.is_commutative(), Is().True()); });
    });
  });
});
