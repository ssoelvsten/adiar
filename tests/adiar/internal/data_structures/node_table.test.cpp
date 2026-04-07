#include "../../../test.h"

#include <adiar/internal/data_structures/node_table.h>

go_bandit([]() {
  describe("adiar/internal/node_table.h", []() {
    const ptr_uint64 terminal_F = ptr_uint64(false);
    const ptr_uint64 terminal_T = ptr_uint64(true);

    describe("capacity(), allocated(), size()", [&]() {
      it("initializes for a capacity of 8", []() {
        node_table nt(8);

        AssertThat(nt.capacity(), Is().EqualTo(8u));
        AssertThat(nt.allocated(), Is().EqualTo(8u));
        AssertThat(nt.size(), Is().EqualTo(0u));
      });

      it("initializes for a capacity of 16", []() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));
      });
    });

    describe("insert(), at(), capacity(), allocated(), size()", [&]() {
      it("returns child if suppressed<bdd>", [&]() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));

        const ptr_uint64 ret0 = nt.insert<bdd_policy>(node(0, 0, terminal_T, terminal_T));
        AssertThat(ret0, Is().EqualTo(terminal_T));
        AssertThat(nt.at(ret0), Is().EqualTo(node(true)));

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));
      });

      it("returns child if suppressed<zdd>", [&]() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));

        const ptr_uint64 ret0 = nt.insert<zdd_policy>(node(0, 0, terminal_T, terminal_F));
        AssertThat(ret0, Is().EqualTo(terminal_T));
        AssertThat(nt.at(ret0), Is().EqualTo(node(true)));

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));
      });

      it("inserts x0", [&]() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));

        const ptr_uint64 ret0 = nt.insert<bdd_policy>(node(0, 0, terminal_F, terminal_T));
        AssertThat(ret0, Is().EqualTo(ptr_uint64(0, 0)));
        AssertThat(nt.at(ret0), Is().EqualTo(node(0, 0, terminal_F, terminal_T)));

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(1u));
      });

      it("inserts x1", [&]() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));

        const ptr_uint64 ret0 = nt.insert<bdd_policy>(node(1, 0, terminal_F, terminal_T));
        AssertThat(ret0, Is().EqualTo(ptr_uint64(1, 0)));
        AssertThat(nt.at(ret0), Is().EqualTo(node(1, 0, terminal_F, terminal_T)));

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(1u));
      });

      it("inserts x0, x1", [&]() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));

        const ptr_uint64 ret0 = nt.insert<bdd_policy>(node(0, 0, terminal_F, terminal_T));
        AssertThat(ret0, Is().EqualTo(ptr_uint64(0, 0)));
        AssertThat(nt.at(ret0), Is().EqualTo(node(0, 0, terminal_F, terminal_T)));

        const ptr_uint64 ret1 = nt.insert<bdd_policy>(node(1, 1, terminal_F, terminal_T));
        AssertThat(ret1, Is().EqualTo(ptr_uint64(1, 1)));
        AssertThat(nt.at(ret1), Is().EqualTo(node(1, 1, terminal_F, terminal_T)));

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(2u));
      });

      it("inserts x0, ~x0, x0", [&]() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));

        const ptr_uint64 ret0 = nt.insert<bdd_policy>(node(0, 0, terminal_F, terminal_T));
        AssertThat(ret0, Is().EqualTo(ptr_uint64(0, 0)));
        AssertThat(nt.at(ret0), Is().EqualTo(node(0, 0, terminal_F, terminal_T)));

        const ptr_uint64 ret1 = nt.insert<bdd_policy>(node(0, 1, terminal_T, terminal_F));
        AssertThat(ret1, Is().EqualTo(ptr_uint64(0, 1)));
        AssertThat(nt.at(ret1), Is().EqualTo(node(0, 1, terminal_T, terminal_F)));

        const ptr_uint64 ret2 = nt.insert<bdd_policy>(node(0, 2, terminal_F, terminal_T));
        AssertThat(ret2, Is().EqualTo(ptr_uint64(0, 0)));
        AssertThat(nt.at(ret2), Is().EqualTo(node(0, 0, terminal_F, terminal_T)));

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(2u));
      });

      it("inserts ~x1, x0, ~x1", [&]() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));

        const ptr_uint64 ret0 = nt.insert<bdd_policy>(node(1, 0, terminal_T, terminal_F));
        AssertThat(ret0, Is().EqualTo(ptr_uint64(1, 0)));
        AssertThat(nt.at(ret0), Is().EqualTo(node(1, 0, terminal_T, terminal_F)));

        const ptr_uint64 ret1 = nt.insert<bdd_policy>(node(0, 1, terminal_F, terminal_T));
        AssertThat(ret1, Is().EqualTo(ptr_uint64(0, 1)));
        AssertThat(nt.at(ret1), Is().EqualTo(node(0, 1, terminal_F, terminal_T)));

        const ptr_uint64 ret2 = nt.insert<bdd_policy>(node(1, 2, terminal_T, terminal_F));
        AssertThat(ret2, Is().EqualTo(ptr_uint64(1, 0)));
        AssertThat(nt.at(ret2), Is().EqualTo(node(1, 0, terminal_T, terminal_F)));

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(2u));
      });

      it("can construct x0 ^ x1", [&]() {
        node_table nt(16);

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(0u));

        const ptr_uint64 ret0 = nt.insert<bdd_policy>(node(1, 0, terminal_F, terminal_T));
        AssertThat(ret0, Is().EqualTo(ptr_uint64(1, 0)));
        AssertThat(nt.at(ret0), Is().EqualTo(node(1, 0, terminal_F, terminal_T)));

        const ptr_uint64 ret1 = nt.insert<bdd_policy>(node(1, 0, terminal_T, terminal_F));
        AssertThat(ret1, Is().EqualTo(ptr_uint64(1, 1)));
        AssertThat(nt.at(ret1), Is().EqualTo(node(1, 1, terminal_T, terminal_F)));

        const ptr_uint64 ret2 = nt.insert<bdd_policy>(node(0, 0, ret0, ret1));
        AssertThat(ret2, Is().EqualTo(ptr_uint64(0, 2)));
        AssertThat(nt.at(ret2), Is().EqualTo(node(0, 2, ret0, ret1)));

        AssertThat(nt.capacity(), Is().EqualTo(16u));
        AssertThat(nt.allocated(), Is().EqualTo(16u));
        AssertThat(nt.size(), Is().EqualTo(3u));
      });
    });
  });
 });
