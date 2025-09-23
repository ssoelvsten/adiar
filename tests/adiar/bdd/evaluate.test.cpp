#include "../../test.h"
#include <iterator>

go_bandit([]() {
  describe("adiar/bdd/evaluate.cpp", []() {
    { // bdd_eval
      bdd::pointer_type terminal_T = bdd::pointer_type(true);
      bdd::pointer_type terminal_F = bdd::pointer_type(false);

      shared_levelized_file<bdd::node_type> bdd;
      /*
      //               1           ---- x0
      //              / \
      //              | 2          ---- x1
      //              |/ \
      //              3   4        ---- x2
      //             / \ / \
      //             F T T 5       ---- x3
      //                  / \
      //                  F T
      */

      node n5 = node(3, 0, terminal_F, terminal_T);
      node n4 = node(2, 1, terminal_T, n5.uid());
      node n3 = node(2, 0, terminal_F, terminal_T);
      node n2 = node(1, 0, n3.uid(), n4.uid());
      node n1 = node(0, 0, n3.uid(), n2.uid());

      { // Garbage collect writer to free write-lock
        node_ofstream nw(bdd);
        nw << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<bdd::node_type> skip_bdd;
      /*
      //             1      ---- x0
      //            / \
      //           /   \    ---- x1
      //           |   |
      //           2   3    ---- x2
      //          / \ / \
      //          F T T |   ---- x3
      //                |
      //                4   ---- x4
      //               / \
      //               F T
      */

      node skip_n4 = node(4, 0, terminal_F, terminal_T);
      node skip_n3 = node(2, 1, terminal_T, skip_n4.uid());
      node skip_n2 = node(2, 0, terminal_F, terminal_T);
      node skip_n1 = node(0, 0, skip_n2.uid(), skip_n3.uid());

      { // Garbage collect writer to free write-lock
        node_ofstream skip_nw(skip_bdd);
        skip_nw << skip_n4 << skip_n3 << skip_n2 << skip_n1;
      }

      shared_levelized_file<bdd::node_type> non_zero_bdd;
      /*
      //                   ---- x0
      //
      //           1       ---- x1
      //          / \
      //          F T
      */

      { // Garbage collect writer to free write-lock
        node_ofstream nw(non_zero_bdd);
        nw << node(1, 0, terminal_F, terminal_T);
      }

      shared_levelized_file<bdd::node_type> bdd_F;
      { // Garbage collect writer to free write-lock
        node_ofstream nw(bdd_F);
        nw << node(false);
      }

      shared_levelized_file<bdd::node_type> bdd_T;
      { // Garbage collect writer to free write-lock
        node_ofstream nw(bdd_T);
        nw << node(true);
      }

      describe("bdd_eval(const bdd&, const generator<...>&)", [&]() {
        // TODO
      });

      describe("bdd_eval(const bdd&, MutIter, MutIter)", [&]() {
        it("returns F on test BDD with assignment (F,F,F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, false }, { 1, false }, { 2, false }, { 3, true }
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns F on test BDD with assignment (F,_,F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = { { 0, false },
                                                           { 2, false },
                                                           { 3, true } };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on test BDD with assignment (F,T,T,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, false }, { 1, true }, { 2, true }, { 3, true }
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().True());
        });

        it("returns F on test BDD with assignment (T,F,F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, false }, { 2, false }, { 3, true }
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on test BDD with assignment (T,F,T,F)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, false }, { 2, true }, { 3, false }
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().True());
        });

        it("returns T on test BDD with assignment (T,T,F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, true }, { 2, false }, { 3, true }
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().True());
        });

        it("returns T on test BDD with assignment (T,T,T,F)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, true }, { 2, true }, { 3, false }
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on test BDD with assignment (T,T,T,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, true }, { 2, true }, { 3, true }
          };

          AssertThat(bdd_eval(bdd, ass.begin(), ass.end()), Is().True());
        });

        it("evaluates BDD that skips level [1]", [&skip_bdd]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, false }, { 1, true }, { 2, false }, { 3, true }, { 4, true }
          };

          AssertThat(bdd_eval(skip_bdd, ass.begin(), ass.end()), Is().False());
        });

        it("evaluates BDD that skips level [2]", [&skip_bdd]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, false }, { 2, true }, { 3, true }, { 4, false }
          };

          AssertThat(bdd_eval(skip_bdd, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on BDD with non-zero root with assignment (F,T)", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = { { 0, false }, { 1, true } };

          AssertThat(bdd_eval(non_zero_bdd, ass.begin(), ass.end()), Is().True());
        });

        it("returns F on F terminal-only BDD", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, false }, { 2, false }, { 3, true }
          };

          AssertThat(bdd_eval(bdd_F, ass.begin(), ass.end()), Is().False());
        });

        it("returns F on F terminal-only BDD with empty assignment", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {};

          AssertThat(bdd_eval(bdd_F, ass.begin(), ass.end()), Is().False());
        });

        it("returns T on T terminal-only BDD", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, true }, { 2, false }, { 3, true }
          };

          AssertThat(bdd_eval(bdd_T, ass.begin(), ass.end()), Is().True());
        });

        it("returns T on T terminal-only BDD with empty assignment", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass;

          AssertThat(bdd_eval(bdd_T, ass.begin(), ass.end()), Is().True());
        });

        it("throws exception when given non-ascending list of assignments", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 2, true }, { 1, true }, { 3, true }, { 4, true }
          };

          AssertThrows(invalid_argument, bdd_eval(skip_bdd, ass.begin(), ass.end()));
        });

        it("throws exception when running out of assignments", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = { { 0, true }, { 1, true } };

          AssertThrows(out_of_range, bdd_eval(skip_bdd, ass.begin(), ass.end()));
        });

        it("throws exception when list is missing a needed assignment", [&]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, true }, { 1, true }, { 3, true }, { 4, true }
          };

          AssertThrows(invalid_argument, bdd_eval(skip_bdd, ass.begin(), ass.end()));
        });

        it("finds path in shifted BDD", [&skip_bdd]() {
          std::vector<pair<bdd::label_type, bool>> ass = {
            { 0, false }, { 1, true }, { 2, false }, { 3, true }, { 4, true }, { 5, false }
          };

          AssertThat(bdd_eval(adiar::bdd(skip_bdd, false, +1), ass.begin(), ass.end()),
                     Is().False());
        });
      });

      describe("bdd_eval(const bdd&, const predicate<...>&)", [&]() {
        it("returns F on test BDD with assignment 'l -> l = 3'", [&]() {
          auto af = [](const bdd::label_type l) { return l == 3; };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment 'l -> l % 2 == 0'", [&]() {
          auto af = [](const bdd::label_type l) { return (l & 1u) == 0; };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns T on test BDD with assignment 'l -> l > 0'", [&]() {
          auto af = [](const bdd::label_type l) { return l > 0; };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on test BDD with assignment 'l -> l == 0 || l == 3'", [&]() {
          auto af = [](const bdd::label_type l) { return l == 0 || l == 3; };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns F on test BDD with assignment 'l -> l % 2 == 1'", [&]() {
          auto af = [](const bdd::label_type l) { return (l & 1) == 1; };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment 'l -> l != 2'", [&]() {
          auto af = [](const bdd::label_type l) { return l != 2; };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on test BDD with assignment 'l -> l < 3'", [&]() {
          auto af = [](const bdd::label_type l) { return l < 3; };
          AssertThat(bdd_eval(bdd, af), Is().False());
        });

        it("returns T on test BDD with assignment '_ -> true'", [&]() {
          auto af = [](const bdd::label_type) { return true; };
          AssertThat(bdd_eval(bdd, af), Is().True());
        });

        it("returns F on BDD that skips with assignment 'l -> l == 1 || l > 2'", [&]() {
          auto af = [](const bdd::label_type l) { return l == 1 || l > 2; };
          AssertThat(bdd_eval(skip_bdd, af), Is().False());
        });

        it("returns F on BDD that skips with assignment 'l -> l != 1 && l < 4'", [&]() {
          auto af = [](const bdd::label_type l) { return l != 1 && l < 4; };
          AssertThat(bdd_eval(skip_bdd, af), Is().False());
        });

        it("returns T on BDD with non-zero root with assignment 'l -> l == 1'", [&]() {
          auto af = [](const bdd::label_type l) { return l == 1; };
          AssertThat(bdd_eval(non_zero_bdd, af), Is().True());
        });

        it("returns F on F terminal-only BDD with assignment '_ -> true'", [&]() {
          auto af = [](const bdd::label_type) { return true; };
          AssertThat(bdd_eval(bdd_F, af), Is().False());
        });

        it("returns T on T terminal-only BDD with assignment '_ -> false'", [&]() {
          auto af = [](const bdd::label_type) { return false; };
          AssertThat(bdd_eval(bdd_T, af), Is().True());
        });

        it("finds path in shifted BDD", [&skip_bdd]() {
          std::vector<int> call_history;
          AssertThat(bdd_eval(adiar::bdd(skip_bdd, false, +1),
                              [&call_history](int x) {
                                call_history.push_back(x);
                                return true;
                              }),
                     Is().True());

          // Check lambda was indeed called on shifted levels
          AssertThat(call_history.size(), Is().EqualTo(3u));

          AssertThat(call_history.at(0), Is().EqualTo(1));
          AssertThat(call_history.at(1), Is().EqualTo(3));
          AssertThat(call_history.at(2), Is().EqualTo(5));
        });
      });
    } // bdd_eval

    { // bdd_satmin, bdd_satmax
      bdd::pointer_type terminal_T = bdd::pointer_type(true);
      bdd::pointer_type terminal_F = bdd::pointer_type(false);

      shared_levelized_file<bdd::node_type> bdd_T;
      {
        node_ofstream nw(bdd_T);
        nw << node(true);
      }

      shared_levelized_file<bdd::node_type> bdd_F;
      {
        node_ofstream nw(bdd_F);
        nw << node(false);
      }

      shared_levelized_file<bdd::node_type> bdd_0;
      /*
      //              1      ---- x0
      //             / \
      //             F T
      */
      {
        node_ofstream nw(bdd_0);
        nw << node(0, bdd::max_id, terminal_F, terminal_T);
      }

      shared_levelized_file<bdd::node_type> bdd_1;
      /*
      //              1      ---- x0
      //             / \
      //             2 |     ---- x1
      //            / \|
      //           3   4     ---- x2
      //          / \ / \
      //          F 5 T F    ---- x3
      //           / \
      //           F T
      */

      {
        node n5 = node(3, bdd::max_id, terminal_F, terminal_T);
        node n4 = node(2, bdd::max_id, terminal_T, terminal_F);
        node n3 = node(2, bdd::max_id - 1, terminal_F, n5.uid());
        node n2 = node(1, bdd::max_id, n3.uid(), n4.uid());
        node n1 = node(0, bdd::max_id, n2.uid(), n4.uid());

        node_ofstream nw(bdd_1);
        nw << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<bdd::node_type> bdd_2;
      /*
      //               1       ---- x0
      //              / \
      //             2   \     ---- x1
      //            / \   \
      //           3   4   5   ---- x2
      //          / \ / \ / \
      //          F | T F | T
      //            \__ __/
      //               6       ---- x3
      //              / \
      //              T F
      */

      { // Garbage collect writer to free write-lock
        node n6 = node(3, bdd::max_id, terminal_T, terminal_F);
        node n5 = node(2, bdd::max_id, n6.uid(), terminal_T);
        node n4 = node(2, bdd::max_id - 1, terminal_T, terminal_F);
        node n3 = node(2, bdd::max_id - 2, terminal_F, n6.uid());
        node n2 = node(1, bdd::max_id, n3.uid(), n4.uid());
        node n1 = node(0, bdd::max_id, n2.uid(), n5.uid());

        node_ofstream nw(bdd_2);
        nw << n6 << n5 << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<bdd::node_type> bdd_3;
      /*
      //                1     ---- x1
      //               / \
      //               2  \   ---- x3
      //              / \ /
      //             3   4    ---- x5
      //            / \ / \
      //            T F F T
      */

      { // Garbage collect writer to free write-lock
        node n4 = node(5, bdd::max_id, terminal_F, terminal_T);
        node n3 = node(5, bdd::max_id - 1, terminal_T, terminal_F);
        node n2 = node(3, bdd::max_id, n3.uid(), n4.uid());
        node n1 = node(1, bdd::max_id, n2.uid(), n4.uid());

        node_ofstream nw(bdd_3);
        nw << n4 << n3 << n2 << n1;
      }

      shared_levelized_file<bdd::node_type> bdd_4;
      /*
      //               1       ---- x0
      //              / \
      //             /   2     ---- x1
      //            /   / \
      //           3   4   5   ---- x2
      //          / \ / \ / \
      //          T | T F | T  ---- x3
      //            \__ __/
      //               6
      //              / \
      //              T F
      */

      { // Garbage collect writer to free write-lock
        node n6 = node(3, bdd::max_id, terminal_T, terminal_F);
        node n5 = node(2, bdd::max_id, n6.uid(), terminal_T);
        node n4 = node(2, bdd::max_id - 1, terminal_T, terminal_F);
        node n3 = node(2, bdd::max_id - 2, terminal_T, n6.uid());
        node n2 = node(1, bdd::max_id, n4.uid(), n5.uid());
        node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

        node_ofstream nw(bdd_4);
        nw << n6 << n5 << n4 << n3 << n2 << n1;
      }

      describe("bdd_satmin(const bdd&)", [&]() {
        it("returns same file for false terminal", [&]() {
          const bdd out = bdd_satmin(bdd_F);
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
        });

        it("returns same file for true terminal", [&]() {
          const bdd out = bdd_satmin(bdd_T);
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
        });

        it("returns minimal BDD cube [0]", [&]() {
          const bdd out = bdd_satmin(bdd_0);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [~0]", [&]() {
          const bdd out = bdd_satmin(bdd(bdd_0, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [1]", [&]() {
          const bdd out = bdd_satmin(bdd_1);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [~1]", [&]() {
          const bdd out = bdd_satmin(bdd(bdd_1, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [2]", [&]() {
          const bdd out = bdd_satmin(bdd_2);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [~2]", [&]() {
          const bdd out = bdd_satmin(bdd(bdd_2, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [3]", [&]() {
          const bdd out = bdd_satmin(bdd_3);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [~3]", [&]() {
          const bdd out = bdd_satmin(bdd(bdd_3, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [3(-1)]", [&]() {
          const bdd out = bdd_satmin(bdd(bdd_3, false, -1));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [3(+1)]", [&]() {
          const bdd out = bdd_satmin(bdd(bdd_3, false, +1));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(6, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [4]", [&]() {
          const bdd out = bdd_satmin(bdd_4);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns minimal BDD cube [~4]", [&]() {
          const bdd out = bdd_satmin(bdd(bdd_4, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("bdd_satmin(const bdd&, const generator<...>&)", [&]() {
        it("returns same file for false terminal", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 2) { return {}; }
            return { x++ };
          };

          const bdd out = bdd_satmin(bdd_F, gen);
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
        });

        it("returns cube for true terminal", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 2) { return {}; }
            return { x++ };
          };

          const bdd out = bdd_satmin(bdd_T, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("adds disjoint domain [0]", [&]() {
          const generator<bdd::label_type> gen = [x = 1]() mutable -> optional<bdd::label_type> {
            if (x > 2) { return {}; }
            return { x++ };
          };

          const bdd out = bdd_satmin(bdd_0, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("adds disjoint domain [3]", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 5) { return {}; }
            return { x += 2 };
          };

          const bdd out = bdd_satmin(bdd_3, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(6, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(6u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(7u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(6u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(7u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(6u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [1]", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 4) { return {}; }
            return { x++ };
          };

          const bdd out = bdd_satmin(bdd_1, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(5u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3]", [&]() {
          const generator<bdd::label_type> gen = [x = -1]() mutable -> optional<bdd::label_type> {
            if (x > 6) { return {}; }
            return { x += 2 };
          };

          const bdd out = bdd_satmin(bdd_3, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(7, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3(-1)]", [&]() {
          const generator<bdd::label_type> gen = [x = -2]() mutable -> optional<bdd::label_type> {
            if (x > 5) { return {}; }
            return { x += 2 };
          };

          const bdd out = bdd_satmin(bdd(bdd_3, false, -1), gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(6, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3(+1)]", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 7) { return {}; }
            return { x += 2 };
          };

          const bdd out = bdd_satmin(bdd(bdd_3, false, +1), gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(6, bdd::max_id, bdd::pointer_type(8, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(6, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("bdd_satmin(const bdd&, ConstIter, ConstIter)", [&]() {
        it("merges with overlapping domain [1]", [&]() {
          const std::vector<int> d = { 0, 1, 2, 3 };

          const bdd out = bdd_satmin(bdd_1, d.cbegin(), d.cend());

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3]", [&]() {
          const std::vector<int> d = { 1, 3, 5, 7 };
          const bdd out            = bdd_satmin(bdd_3, d.cbegin(), d.cend());

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(7, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3(+1)]", [&]() {
          const std::vector<int> d = { 0, 2, 4, 6 };
          const bdd out            = bdd_satmin(bdd(bdd_3, false, +1), d.cbegin(), d.cend());

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(6, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("bdd_satmin(const bdd&, const bdd&)", [&]() {
        it("throws exception for non-cubical input",
           [&]() { AssertThrows(domain_error, bdd_satmin(bdd_1, bdd_2)); });

        it("merges with overlapping domain [1]", [&]() {
          const std::vector<int> d = { 0, 1, 2, -3 };
          const bdd out            = bdd_satmin(bdd_1, bdd_cube(d.rbegin(), d.rend()));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(1, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3]", [&]() {
          const std::vector<int> d = { 1, -3, 5, 7 };
          const bdd out            = bdd_satmin(bdd_3, bdd_cube(d.rbegin(), d.rend()));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(5, bdd::max_id, bdd::pointer_type(7, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, bdd::pointer_type(5, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3]", [&]() {
          const std::vector<int> d = { 0, 2, 4, 6 };
          const bdd out = bdd_satmin(bdd(bdd_3, false, +1), bdd_cube(d.rbegin(), d.rend()));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, bdd::pointer_type(6, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(4, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, bdd::pointer_type(2, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("bdd_satmin(const bdd&, const consumer<...>&)", [&]() {
        it("never calls consumer for false terminal", [&]() {
          size_t calls  = 0u;
          const auto cb = [&calls](pair<bdd::label_type, bool>) { calls++; };

          bdd_satmin(bdd_F, cb);
          AssertThat(calls, Is().EqualTo(0u));
        });

        it("never calls consumer for true terminal", [&]() {
          size_t calls  = 0u;
          const auto cb = [&calls](pair<bdd::label_type, bool>) { calls++; };

          bdd_satmin(bdd_T, cb);
          AssertThat(calls, Is().EqualTo(0u));
        });

        it("calls consumer once with 'x0 == true' for [0]", [&]() {
          size_t calls  = 0u;
          const auto cb = [&calls](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().EqualTo(0u));
            AssertThat(xv.first, Is().EqualTo(0u));
            AssertThat(xv.second, Is().EqualTo(true));

            calls++;
          };

          bdd_satmin(bdd_0, cb);
          AssertThat(calls, Is().EqualTo(1u));
        });

        it("calls consumer once with 'x0 == false' for [~0]", [&]() {
          size_t calls  = 0u;
          const auto cb = [&calls](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().EqualTo(0u));
            AssertThat(xv.first, Is().EqualTo(0u));
            AssertThat(xv.second, Is().EqualTo(true));

            calls++;
          };

          bdd_satmin(bdd_0, cb);
          AssertThat(calls, Is().EqualTo(1u));
        });

        it("calls consumer with minimal truth assignment [1]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = {
            { 0, false }, { 1, false }, { 2, true }, { 3, true }
          };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd_1, cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("calls consumer with minimal truth assignment [~1]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, false },
                                                                     { 1, false },
                                                                     { 2, false } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd(bdd_1, true), cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("calls consumer with minimal truth assignment [2]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = {
            { 0, false }, { 1, false }, { 2, true }, { 3, false }
          };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));

            calls++;
          };

          bdd_satmin(bdd_2, cb);
          AssertThat(calls, Is().EqualTo(4u));
        });

        it("calls consumer with minimal truth assignment [~2]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, false },
                                                                     { 1, false },
                                                                     { 2, false } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));

            calls++;
          };

          bdd_satmin(bdd(bdd_2, true), cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("calls consumer with minimal truth assignment [3]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 1, false },
                                                                     { 3, false },
                                                                     { 5, false } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd_3, cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("calls consumer with minimal truth assignment [~3]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 1, false },
                                                                     { 3, false },
                                                                     { 5, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd(bdd_3, true), cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("calls consumer with minimal truth assignment [3(-1)]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, false },
                                                                     { 2, false },
                                                                     { 4, false } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd(bdd_3, false, -1), cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("calls consumer with minimal truth assignment [3(+1)]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 2, false },
                                                                     { 4, false },
                                                                     { 6, false } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd(bdd_3, false, +1), cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("calls consumer with minimal truth assignment [4]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, false }, { 2, false } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd_4, cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("calls consumer with minimal truth assignment [~4]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, false },
                                                                     { 2, true },
                                                                     { 3, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmin(bdd(bdd_4, true), cb);
          AssertThat(calls, Is().EqualTo(3u));
        });
      });

      describe("bdd_satmin(const bdd&, OutputIter)", [&]() {
        using pair_type     = pair<bdd::label_type, bool>;
        using output_vector = std::vector<pair_type>;

        it("outputs [] for false terminal", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd_F, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(0u));

          // Check state of 'iter'
          iter = { 21, true };
          AssertThat(out.size(), Is().EqualTo(1u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 21, true }));
        });

        it("outputs [] for true terminal", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd_T, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(0u));

          // Check state of 'iter'
          iter = { 42, false };
          AssertThat(out.size(), Is().EqualTo(1u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 42, false }));
        });

        it("outputs [{x0, true}] for [0]", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd_0, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(1u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, true }));

          // Check state of 'iter'
          iter = { 42, false };
          AssertThat(out.size(), Is().EqualTo(2u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, true }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 42, false }));
        });

        it("outputs [{x0, false}] for [~0]", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd(bdd_0, true), std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(1u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, false }));

          // Check state of 'iter'
          iter = { 21, true };
          AssertThat(out.size(), Is().EqualTo(2u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, false }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 21, true }));
        });

        it("outputs [{0, false}, {1, false}, {2, true}, {3, true}] for [1]", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd_1, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(4u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, false }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 1, false }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 2, true }));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 3, true }));

          // Check state of 'iter'
          iter = { 42, true };
          AssertThat(out.size(), Is().EqualTo(5u));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 3, true }));
          AssertThat(out.at(4), Is().EqualTo(pair_type{ 42, true }));
        });

        it("outputs [{0, false}, {1, false}, {2, false}] for [~1]", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd(bdd_1, true), std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(3u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, false }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 1, false }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 2, false }));

          // Check state of 'iter'
          iter = { 21, false };
          AssertThat(out.size(), Is().EqualTo(4u));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 2, false }));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 21, false }));
        });

        it("outputs [{0, false}, {2, false}] for [4]", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd_4, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(2u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, false }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 2, false }));

          // Check state of 'iter'
          iter = { 8, false };
          AssertThat(out.size(), Is().EqualTo(3u));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 2, false }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 8, false }));
        });

        it("outputs [{0, false}, {2, true}, {3, true}] for [~4]", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd(bdd_4, true), std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(3u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, false }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 2, true }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 3, true }));

          // Check state of 'iter'
          iter = { 4, false };
          AssertThat(out.size(), Is().EqualTo(4u));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 3, true }));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 4, false }));
        });

        it("outputs [{1, false}, {3, false}] for [4(+1)]", [&]() {
          output_vector out;
          auto iter = bdd_satmin(bdd(bdd_4, false, +1), std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(2u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 1, false }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 3, false }));

          // Check state of 'iter'
          iter = { 5, false };
          AssertThat(out.size(), Is().EqualTo(3u));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 3, false }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 5, false }));
        });
      });

      describe("bdd_satmax(const bdd&)", [&]() {
        it("returns same file for false terminal", [&]() {
          const bdd out = bdd_satmax(bdd_F);
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
        });

        it("returns same file for true terminal", [&]() {
          const bdd out = bdd_satmax(bdd_T);
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_T));
        });

        it("returns maximal BDD cube [0]", [&]() {
          const bdd out = bdd_satmax(bdd_0);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [~0]", [&]() {
          const bdd out = bdd_satmax(bdd(bdd_0, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [1]", [&]() {
          const bdd out = bdd_satmax(bdd_1);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [~1]", [&]() {
          const bdd out = bdd_satmax(bdd(bdd_1, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [2]", [&]() {
          const bdd out = bdd_satmax(bdd_2);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [~2]", [&]() {
          const bdd out = bdd_satmax(bdd(bdd_2, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [3]", [&]() {
          const bdd out = bdd_satmax(bdd_3);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [~3]", [&]() {
          const bdd out = bdd_satmax(bdd(bdd_3, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, bdd::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [3(-1)]", [&]() {
          const bdd out = bdd_satmax(bdd(bdd_3, false, -1));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [3(+1)]", [&]() {
          const bdd out = bdd_satmax(bdd(bdd_3, false, +1));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(6, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [4]", [&]() {
          const bdd out = bdd_satmax(bdd_4);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("returns maximal BDD cube [~4]", [&]() {
          const bdd out = bdd_satmax(bdd(bdd_4, true));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("bdd_satmax(const bdd&, const generator<...>&)", [&]() {
        it("returns same file for false terminal", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 2) { return {}; }
            return { x++ };
          };

          const bdd out = bdd_satmax(bdd_F, gen);
          AssertThat(out.file_ptr(), Is().EqualTo(bdd_F));
        });

        it("returns cube for true terminal", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 2) { return {}; }
            return { x++ };
          };

          const bdd out = bdd_satmax(bdd_T, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("adds disjoint domain [0]", [&]() {
          const generator<bdd::label_type> gen = [x = 1]() mutable -> optional<bdd::label_type> {
            if (x > 2) { return {}; }
            return { x++ };
          };

          const bdd out = bdd_satmax(bdd_0, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("adds disjoint domain [3]", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 5) { return {}; }
            return { x += 2 };
          };

          const bdd out = bdd_satmax(bdd_3, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(5, bdd::max_id, terminal_F, bdd::pointer_type(6, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, terminal_F, bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(5u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [1]", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 4) { return {}; }
            return { x++ };
          };

          const bdd out = bdd_satmax(bdd_1, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(5u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3]", [&]() {
          const generator<bdd::label_type> gen = [x = -1]() mutable -> optional<bdd::label_type> {
            if (x > 6) { return {}; }
            return { x += 2 };
          };

          const bdd out = bdd_satmax(bdd_3, gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(5, bdd::max_id, terminal_F, bdd::pointer_type(7, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, terminal_F, bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3(-1)]", [&]() {
          const generator<bdd::label_type> gen = [x = -2]() mutable -> optional<bdd::label_type> {
            if (x > 4) { return {}; }
            return { x += 2 };
          };

          const bdd out = bdd_satmax(bdd(bdd_3, false, -1), gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, terminal_F, bdd::pointer_type(6, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3(+1)]", [&]() {
          const generator<bdd::label_type> gen = [x = 0]() mutable -> optional<bdd::label_type> {
            if (x > 7) { return {}; }
            return { x += 2 };
          };

          const bdd out = bdd_satmax(bdd(bdd_3, false, +1), gen);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(8, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(6, bdd::max_id, terminal_F, bdd::pointer_type(8, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, terminal_F, bdd::pointer_type(6, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(8, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("bdd_satmax(const bdd&, ConstIter, ConstIter)", [&]() {
        it("merges with overlapping domain [1]", [&]() {
          const std::vector<int> d = { 0, 1, 2, 3, 4 };
          const bdd out            = bdd_satmax(bdd_1, d.cbegin(), d.cend());

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(5u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3]", [&]() {
          const std::vector<int> d = { 1, 3, 5, 7 };
          const bdd out            = bdd_satmax(bdd_3, d.cbegin(), d.cend());

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(5, bdd::max_id, terminal_F, bdd::pointer_type(7, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, terminal_F, bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3(-1)]", [&]() {
          const std::vector<int> d = { 0, 2, 4, 6 };
          const bdd out            = bdd_satmax(bdd(bdd_3, false, -1), d.cbegin(), d.cend());

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, terminal_F, bdd::pointer_type(6, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("bdd_satmax(const bdd&, const bdd&)", [&]() {
        it("throws exception for non-cubical input",
           [&]() { AssertThrows(domain_error, bdd_satmax(bdd_1, bdd_2)); });

        it("merges with overlapping domain [1]", [&]() {
          const std::vector<int> d = { 0, 1, 2, 3, 4 };
          const bdd out            = bdd_satmax(bdd_1, bdd_cube(d.rbegin(), d.rend()));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(4, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, bdd::pointer_type(3, bdd::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(1, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(5u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3]", [&]() {
          const std::vector<int> d = { 1, 3, 5, 7 };
          const bdd out            = bdd_satmax(bdd_3, bdd_cube(d.rbegin(), d.rend()));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(5, bdd::max_id, terminal_F, bdd::pointer_type(7, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, bdd::max_id, terminal_F, bdd::pointer_type(5, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, bdd::max_id, terminal_F, bdd::pointer_type(3, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });

        it("merges with overlapping domain [3]", [&]() {
          const std::vector<int> d = { 0, 2, 4, 6 };
          const bdd out = bdd_satmax(bdd(bdd_3, false, -1), bdd_cube(d.rbegin(), d.rend()));

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(6, bdd::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(4, bdd::max_id, terminal_F, bdd::pointer_type(6, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(2, bdd::max_id, terminal_F, bdd::pointer_type(4, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(0, bdd::max_id, terminal_F, bdd::pointer_type(2, bdd::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(6, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("bdd_satmax(const bdd&, const consumer<...>&)", [&]() {
        it("never calls consumer for false terminal", [&]() {
          size_t calls  = 0u;
          const auto cb = [&calls](pair<bdd::label_type, bool>) { calls++; };

          bdd_satmax(bdd_F, cb);
          AssertThat(calls, Is().EqualTo(0u));
        });

        it("never calls consumer for true terminal", [&]() {
          size_t calls  = 0u;
          const auto cb = [&calls](pair<bdd::label_type, bool>) { calls++; };

          bdd_satmax(bdd_T, cb);
          AssertThat(calls, Is().EqualTo(0u));
        });

        it("calls consumer once with 'x0 == true' for [0]", [&]() {
          size_t calls  = 0u;
          const auto cb = [&calls](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().EqualTo(0u));
            AssertThat(xv.first, Is().EqualTo(0u));
            AssertThat(xv.second, Is().EqualTo(true));

            calls++;
          };

          bdd_satmax(bdd_0, cb);
          AssertThat(calls, Is().EqualTo(1u));
        });

        it("calls consumer once with 'x0 == false' for [~0]", [&]() {
          size_t calls  = 0u;
          const auto cb = [&calls](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().EqualTo(0u));
            AssertThat(xv.first, Is().EqualTo(0u));
            AssertThat(xv.second, Is().EqualTo(true));

            calls++;
          };

          bdd_satmax(bdd_0, cb);
          AssertThat(calls, Is().EqualTo(1u));
        });

        it("calls consumer with maximal truth assignment [1]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, true }, { 2, false } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_1, cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("calls consumer with maximal truth assignment [~1]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, true }, { 2, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd(bdd_1, true), cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("calls consumer with maximal truth assignment [2]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, true }, { 2, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_2, cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("calls consumer with maximal truth assignment [~2]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, true },
                                                                     { 2, false },
                                                                     { 3, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd(bdd_2, true), cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("calls consumer with maximal truth assignment [3]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 1, true }, { 5, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_3, cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("calls consumer with maximal truth assignment [~3]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 1, true }, { 5, false } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd(bdd_3, true), cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("calls consumer with maximal truth assignment [3(-1)]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, true }, { 4, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd(bdd_3, false, -1), cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("calls consumer with maximal truth assignment [3(+1)]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 2, true }, { 6, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd(bdd_3, false, +1), cb);
          AssertThat(calls, Is().EqualTo(2u));
        });

        it("calls consumer with maximal truth assignment [4]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = { { 0, true },
                                                                     { 1, true },
                                                                     { 2, true } };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd_4, cb);
          AssertThat(calls, Is().EqualTo(3u));
        });

        it("calls consumer with maximal truth assignment [~4]", [&]() {
          size_t calls                                           = 0;
          std::vector<std::pair<bdd::label_type, bool>> expected = {
            { 0, true }, { 1, true }, { 2, false }, { 3, true }
          };

          const auto cb = [&calls, &expected](pair<bdd::label_type, bool> xv) {
            AssertThat(calls, Is().LessThan(expected.size()));
            AssertThat(xv.first, Is().EqualTo(expected.at(calls).first));
            AssertThat(xv.second, Is().EqualTo(expected.at(calls).second));
            calls++;
          };

          bdd_satmax(bdd(bdd_4, true), cb);
          AssertThat(calls, Is().EqualTo(4u));
        });
      });

      describe("bdd_satmax(const bdd&, OutputIter)", [&]() {
        using pair_type     = pair<bdd::label_type, bool>;
        using output_vector = std::vector<pair_type>;

        it("outputs nothing in buffer for false terminal", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd_F, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(0u));

          // Check state of 'iter'
          iter = { 21, true };
          AssertThat(out.size(), Is().EqualTo(1u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 21, true }));
        });

        it("outputs nothing in buffer for true terminal", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd_T, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(0u));

          // Check state of 'iter'
          iter = { 42, true };
          AssertThat(out.size(), Is().EqualTo(1u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 42, true }));
        });

        it("outputs {x0, true} for [0]", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd_0, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(1u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, true }));

          // Check state of 'iter'
          iter = { 42, true };
          AssertThat(out.size(), Is().EqualTo(2u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, true }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 42, true }));
        });

        it("outputs {x0, false} for [~0]", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd(bdd_0, true), std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(1u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, false }));

          // Check state of 'iter'
          iter = { 8, false };
          AssertThat(out.size(), Is().EqualTo(2u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, false }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 8, false }));
        });

        it("outputs expected values into buffer for [2]", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd_2, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(2u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, true }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 2, true }));

          // Check state of 'iter'
          iter = { 4, false };
          AssertThat(out.size(), Is().EqualTo(3u));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 2, true }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 4, false }));
        });

        it("outputs expected values into buffer for [~2]", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd(bdd_2, true), std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(3u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, true }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 2, false }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 3, true }));

          // Check state of 'iter'
          iter = { 4, false };
          AssertThat(out.size(), Is().EqualTo(4u));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 3, true }));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 4, false }));
        });

        it("outputs expected values into buffer for [4]", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd_4, std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(3u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, true }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 1, true }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 2, true }));

          // Check state of 'iter'
          iter = { 3, false };
          AssertThat(out.size(), Is().EqualTo(4u));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 2, true }));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 3, false }));
        });

        it("outputs expected values into buffer for [~4]", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd(bdd_4, true), std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(4u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 0, true }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 1, true }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 2, false }));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 3, true }));

          // Check state of 'iter'
          iter = { 4, false };
          AssertThat(out.size(), Is().EqualTo(5u));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 3, true }));
          AssertThat(out.at(4), Is().EqualTo(pair_type{ 4, false }));
        });

        it("outputs expected values into buffer for [4(+2)]", [&]() {
          output_vector out;
          auto iter = bdd_satmax(bdd(bdd_4, false, +2), std::back_inserter(out));

          // Check state of 'out'
          AssertThat(out.size(), Is().EqualTo(3u));
          AssertThat(out.at(0), Is().EqualTo(pair_type{ 2, true }));
          AssertThat(out.at(1), Is().EqualTo(pair_type{ 3, true }));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 4, true }));

          // Check state of 'iter'
          iter = { 5, true };
          AssertThat(out.size(), Is().EqualTo(4u));
          AssertThat(out.at(2), Is().EqualTo(pair_type{ 4, true }));
          AssertThat(out.at(3), Is().EqualTo(pair_type{ 5, true }));
        });
      });
    } // bdd_satmin, bdd_satmax
  });
});
