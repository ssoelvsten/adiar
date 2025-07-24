#include "../../../test.h"

#include <adiar/internal/data_types/level_info.h>

go_bandit([]() {
  describe("adiar/internal/data_types/level_info.h", []() {
    it("should be a POD", []() { AssertThat(std::is_pod<level_info>::value, Is().True()); });

    it("should take up 16 bytes of memory", [&]() {
      const level_info li(42, 1024);
      AssertThat(sizeof(li), Is().EqualTo(2u * 8u));
    });

    describe(".level(), .label()", [&] {
      it("creates and retrieves from (0,1)", [&]() {
        const level_info li(0, 1);
        AssertThat(li.level(), Is().EqualTo(0u));
        AssertThat(li.label(), Is().EqualTo(0u));
      });

      it("creates and retrieves from (0,2)", [&]() {
        const level_info li(0, 2);
        AssertThat(li.level(), Is().EqualTo(0u));
        AssertThat(li.label(), Is().EqualTo(0u));
      });

      it("creates and retrieves from (42,8)", [&]() {
        const level_info li(42, 8);
        AssertThat(li.level(), Is().EqualTo(42u));
        AssertThat(li.label(), Is().EqualTo(42u));
      });

      it("creates and retrieves from (max,1)", [&]() {
        const level_info li(ptr_uint64::max_label, 1);
        AssertThat(li.level(), Is().EqualTo(ptr_uint64::max_label));
        AssertThat(li.level(), Is().EqualTo(ptr_uint64::max_label));
      });

      it("creates and retrieves from (max,2)", [&]() {
        const level_info li(ptr_uint64::max_label, 2);
        AssertThat(li.level(), Is().EqualTo(ptr_uint64::max_label));
        AssertThat(li.level(), Is().EqualTo(ptr_uint64::max_label));
      });
    });

    describe(".width()", [&] {
      it("creates and retrieves from (0,1)", [&]() {
        const level_info li(0, 1);
        AssertThat(li.width(), Is().EqualTo(1u));
      });

      it("creates and retrieves from (0,2)", [&]() {
        const level_info li(0, 2);
        AssertThat(li.width(), Is().EqualTo(2u));
      });

      it("creates and retrieves from (42,max)", [&]() {
        const level_info li(42, ptr_uint64::max_id);
        AssertThat(li.width(), Is().EqualTo(ptr_uint64::max_id));
      });

      it("creates and retrieves from (42,max)", [&]() {
        const level_info li(42, ptr_uint64::max_id + 1);
        AssertThat(li.width(), Is().EqualTo(ptr_uint64::max_id + 1));
      });
    });

    describe("operator ==", [&] {
      it("checks (0,1) == (0,1)", [&]() {
        const level_info li_1(0, 1);
        const level_info li_2(0, 1);
        AssertThat(li_1, Is().EqualTo(li_2));
      });

      it("checks (2,1) == (2,1)", [&]() {
        const level_info li_1(2, 1);
        const level_info li_2(2, 1);
        AssertThat(li_1, Is().EqualTo(li_2));
      });

      it("checks (1,2) == (1,2)", [&]() {
        const level_info li_1(1, 2);
        const level_info li_2(1, 2);
        AssertThat(li_1, Is().EqualTo(li_2));
      });

      it("checks (42,21) == (42,21)", [&]() {
        const level_info li_1(42, 21);
        const level_info li_2(42, 21);
        AssertThat(li_1, Is().EqualTo(li_2));
      });

      it("checks (max,max+1) == (max,max+1)", [&]() {
        const level_info li_1(ptr_uint64::max_label, ptr_uint64::max_id + 1);
        const level_info li_2(ptr_uint64::max_label, ptr_uint64::max_id + 1);
        AssertThat(li_1, Is().EqualTo(li_2));
      });

      it("checks (2,1) != (2,2)", [&]() {
        const level_info li_1(2, 1);
        const level_info li_2(2, 2);
        AssertThat(li_1, Is().Not().EqualTo(li_2));
      });

      it("checks (1,2) != (2,2)", [&]() {
        const level_info li_1(1, 2);
        const level_info li_2(2, 2);
        AssertThat(li_1, Is().Not().EqualTo(li_2));
      });

      it("checks (1,2) != (2,1)", [&]() {
        const level_info li_1(1, 2);
        const level_info li_2(2, 1);
        AssertThat(li_1, Is().Not().EqualTo(li_2));
      });
    });

    describe("shift_replace(const level_info&, ...)", [&] {
      it("shifts (0,1) [+1]", [&]() {
        AssertThat(shift_replace(level_info(0, 1), +1), Is().EqualTo(level_info(1, 1)));
      });

      it("shifts (0,1) [+2]", [&]() {
        AssertThat(shift_replace(level_info(0, 1), +2), Is().EqualTo(level_info(2, 1)));
      });

      it("shifts (2,1) [+1]", [&]() {
        AssertThat(shift_replace(level_info(2, 1), +1), Is().EqualTo(level_info(3, 1)));
      });

      it("shifts (2,2) [+1]", [&]() {
        AssertThat(shift_replace(level_info(2, 2), +1), Is().EqualTo(level_info(3, 2)));
      });

      it("shifts (2,1) [-1]", [&]() {
        AssertThat(shift_replace(level_info(2, 1), -1), Is().EqualTo(level_info(1, 1)));
      });

      it("shifts (2,2) [-1]", [&]() {
        AssertThat(shift_replace(level_info(2, 2), -1), Is().EqualTo(level_info(1, 2)));
      });

      it("shifts (0,1) [+max]", [&]() {
        AssertThat(shift_replace(level_info(0, 1), +ptr_uint64::max_label),
                   Is().EqualTo(level_info(ptr_uint64::max_label, 1)));
      });

      it("shifts (max,1) [-max]", [&]() {
        AssertThat(
          shift_replace(level_info(ptr_uint64::max_label, 1),
                        -static_cast<level_info::signed_level_type>(ptr_uint64::max_label)),
          Is().EqualTo(level_info(0, 1)));
      });
    });
  });
});
