#include "../../../test.h"

go_bandit([]() {
  // TODO: tests are missing cut sizes

  describe("adiar/internal/algorithms/convert.h", []() {
    const std::vector<int> dom_empty = {};
    const std::vector<int> dom_0     = { 0 };
    const std::vector<int> dom_012   = { 0, 1, 2 };
    const std::vector<int> dom_0123  = { 0, 1, 2, 3 };
    const std::vector<int> dom_024   = { 0, 2, 4 };

    shared_levelized_file<dd::node_type> nf_F;
    {
      node_ofstream w(nf_F);
      w << node(false);
    }

    shared_levelized_file<dd::node_type> nf_T;
    {
      node_ofstream w(nf_T);
      w << node(true);
    }

    const ptr_uint64 terminal_F = ptr_uint64(false);
    const ptr_uint64 terminal_T = ptr_uint64(true);

    shared_levelized_file<dd::node_type> nf_x0;
    {
      node_ofstream nw(nf_x0);
      nw << node(0, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<dd::node_type> nf_x1;
    {
      node_ofstream nw(nf_x1);
      nw << node(1, node::max_id, terminal_F, terminal_T);
    }

    shared_levelized_file<dd::node_type> nf_x2;
    {
      node_ofstream nw(nf_x2);
      nw << node(2, node::max_id, terminal_F, terminal_T);
    }

    zdd zdd_F(nf_F);
    zdd zdd_T(nf_T);

    zdd zdd_x0(nf_x0);
    zdd zdd_x1(nf_x1);
    zdd zdd_x2(nf_x2);

    shared_levelized_file<dd::node_type> nf_x0_null;
    {
      node_ofstream nw(nf_x0_null);
      nw << node(0, node::max_id, terminal_T, terminal_T);
    }
    zdd zdd_x0_null(nf_x0_null);

    shared_levelized_file<dd::node_type> nf_x1_null;
    {
      node_ofstream nw(nf_x1_null);
      nw << node(1, node::max_id, terminal_T, terminal_T);
    }
    zdd zdd_x1_null(nf_x1_null);

    shared_levelized_file<dd::node_type> nf_x2_null;
    {
      node_ofstream nw(nf_x2_null);
      nw << node(2, node::max_id, terminal_T, terminal_T);
    }
    zdd zdd_x2_null(nf_x2_null);

    // Fig. 5 from Minato: "Zero-suppressed BDDs and their applications". This is the ZDD version of
    // Fig. 3 in the same paper.
    shared_levelized_file<zdd::node_type> nf_minato_fig5;
    {
      const node n2 = node(1, zdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, zdd::max_id, n2.uid(), terminal_T);

      node_ofstream nw(nf_minato_fig5);
      nw << n2 << n1;
    }
    zdd zdd_minato_fig5(nf_minato_fig5);

    // Fig. 3 (left) from Minato: "Zero-suppressed BDDs and their applications". This is the ZDD
    // version of Fig. 5 in the same paper.
    shared_levelized_file<bdd::node_type> nf_minato_fig3_a;
    {
      const node n4 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n3 = node(1, bdd::max_id, n4.uid(), terminal_F);
      const node n2 = node(1, bdd::max_id - 1, terminal_F, n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(nf_minato_fig3_a);
      nw << n4 << n3 << n2 << n1;
    }
    bdd bdd_minato_fig3_a(nf_minato_fig3_a);

    shared_levelized_file<bdd::node_type> nf_minato_fig3_b;
    {
      const node n5 = node(3, bdd::max_id, terminal_T, terminal_F);
      const node n4 = node(2, bdd::max_id, n5.uid(), terminal_F);
      const node n3 = node(1, bdd::max_id, n4.uid(), terminal_F);
      const node n2 = node(1, bdd::max_id - 1, terminal_F, n4.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(nf_minato_fig3_b);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    bdd bdd_minato_fig3_b(nf_minato_fig3_b);

    bdd bdd_F(nf_F);
    bdd bdd_T(nf_T);

    bdd bdd_x0(nf_x0);
    bdd bdd_x2(nf_x2);

    describe("bdd_from(const zdd&, const generator<...>&)", [&]() {
      // TODO
    });

    describe("bdd_from(const zdd&, ForwardIt, ForwardIt)", [&]() {
      it("returns same file for F terminal on Ø with dom = Ø", [&]() {
        __bdd out = bdd_from(zdd_F, dom_empty.begin(), dom_empty.end());

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(zdd_F.file_ptr()));
        AssertThat(out._negate, Is().False());
        AssertThat(out._shift, Is().EqualTo(0));
      });

      it("returns same file for T terminal on { Ø } with dom = Ø", [&]() {
        __bdd out = bdd_from(zdd_T, dom_empty.begin(), dom_empty.end());

        AssertThat(out.get<__bdd::shared_node_file_type>(), Is().EqualTo(zdd_T.file_ptr()));
        AssertThat(out._negate, Is().False());
        AssertThat(out._shift, Is().EqualTo(0));
      });

      it("returns F terminal on Ø with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_F, dom_012.begin(), dom_012.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns check-false chain to T terminal on { Ø } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_T, dom_012.begin(), dom_012.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, ptr_uint64(2, ptr_uint64::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("adds pre-root false-chain on { { 2 } } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_x2, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ ptr_uint64(0, 0), ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ ptr_uint64(1, 0), ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("adds post-nodes false-chain on { { 0 } } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_x0, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("kills root and returns T terminal on { Ø, { 0 } } with dom = { 0 }", [&]() {
        __bdd out = bdd_from(zdd_x0_null, dom_0.begin(), dom_0.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("kills root on { Ø, { 0 } } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_x0_null, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("kills root and bridges over it on { Ø, { 1 } } with dom = { 0,1,2 }", [&]() {
        __bdd out = bdd_from(zdd_x1_null, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("kills root and bridges over it on { Ø, { 2 } } with dom = { 0,2,4 }", [&]() {
        __bdd out = bdd_from(zdd_x2_null, dom_024.begin(), dom_024.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(4, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(4, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(4, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("collapses to T terminal on pow(dom) with dom = { 0,1,2 }", [&]() {
        shared_levelized_file<bdd::node_type> nf_pow_dom;
        {
          const node n2 = node(2, node::max_id, terminal_T, terminal_T);
          const node n1 = node(1, node::max_id, n2.uid(), n2.uid());
          const node n0 = node(0, node::max_id, n1.uid(), n1.uid());

          node_ofstream nw(nf_pow_dom);
          nw << n2 << n1 << n0;
        }
        zdd zdd_pow(nf_pow_dom);

        __bdd out = bdd_from(zdd_pow, dom_012.begin(), dom_012.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("collapses to T terminal on pow(dom) with dom = { 0,2,4 }", [&]() {
        shared_levelized_file<zdd::node_type> nf_pow_dom;
        {
          const node n2 = node(4, node::max_id, terminal_T, terminal_T);
          const node n1 = node(2, node::max_id, n2.uid(), n2.uid());
          const node n0 = node(0, node::max_id, n1.uid(), n1.uid());

          node_ofstream nw(nf_pow_dom);
          nw << n2 << n1 << n0;
        }
        zdd zdd_pow(nf_pow_dom);

        __bdd out = bdd_from(zdd_pow, dom_024.begin(), dom_024.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("bridges internal arcs on { { 0,2 }, { 1,2 } { 0,1,2 } } with dom = { 0,1,2 } ", [&]() {
        /*
        //          _1_     ---- x0
        //         /   \
        //         2   3    ---- x1
        //        / \ //
        //        F  4      ---- x2
        //          / \
        //          F T
        */

        shared_levelized_file<zdd::node_type> nf;
        {
          const node n4 = node(2, node::max_id, terminal_F, terminal_T);
          const node n3 = node(1, node::max_id, n4.uid(), n4.uid());
          const node n2 = node(1, node::max_id - 1, terminal_F, n4.uid());
          const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

          node_ofstream nw(nf);
          nw << n4 << n3 << n2 << n1;
        }
        zdd in(nf);

        __bdd out = bdd_from(in, dom_012.begin(), dom_012.end());
        /*
        //        _1_     ---- x0
        //       /   \
        //       2   /    ---- x1
        //      / \ /
        //      F  4      ---- x2
        //        / \
        //        F T
        */

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      // Minato examples
      it("converts [Minato] Fig. 5 into Fig. 3 with dom = { 0,1,2 } ", [&]() {
        __bdd out = bdd_from(zdd_minato_fig5, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("converts [Minato] Fig. 5 into Fig. 3 with dom = { 0,1,2,3 }", [&]() {
        __bdd out = bdd_from(zdd_minato_fig5, dom_0123.begin(), dom_0123.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(4u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("converts [Minato] Fig. 11 (dom = { 0,1,2,3 })", [&]() {
        shared_levelized_file<zdd::node_type> nf;
        {
          const node n3 = node(3, node::max_id, terminal_T, terminal_T);
          const node n2 = node(2, node::max_id, n3.uid(), n3.uid());
          const node n1 = node(1, node::max_id, n2.uid(), terminal_T);

          node_ofstream nw(nf);
          nw << n3 << n2 << n1;
        }

        __bdd out = bdd_from(nf, dom_0123.begin(), dom_0123.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // produced out-of-order
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("converts [Minato] Fig. 15 (dom = { 0,1,2 })", [&]() {
        shared_levelized_file<zdd::node_type> nf;
        {
          const node n3 = node(2, node::max_id, terminal_F, terminal_T);
          const node n2 = node(1, node::max_id, n3.uid(), terminal_T);
          const node n1 = node(0, node::max_id, n3.uid(), n2.uid());

          node_ofstream nw(nf);
          nw << n3 << n2 << n1;
        }

        __bdd out = bdd_from(nf, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1) -> (2)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (1) -- * -> (3)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2) -> (3)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // * -> (3)
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // (2) -- * -> T
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 1), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(3u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(3u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      // Other large cases
      it(
        "converts { Ø, { 0,1 }, { 0,2 }, { 1,2 } } with dom = { 0,1,2 } only adding true-chain",
        [&]() {
          shared_levelized_file<zdd::node_type> nf_in;
          // In dom = { 0,1,2 }
          /*
          //      _1_      ---- x0
          //     /   \
          //     2   3     ---- x1
          //    / \ / \
          //    T  4  T    ---- x2
          //      / \
          //      F T
          */
          {
            const node n4 = node(2, node::max_id, terminal_F, terminal_T);
            const node n3 = node(1, node::max_id, n4.uid(), terminal_T);
            const node n2 = node(1, node::max_id - 1, terminal_T, n4.uid());
            const node n1 = node(0, node::max_id, n2.uid(), n3.uid());

            node_ofstream nw(nf_in);
            nw << n4 << n3 << n2 << n1;
          }

          zdd in(nf_in);

          __bdd out = bdd_from(in, dom_012.begin(), dom_012.end());

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), false, ptr_uint64(2, 0) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(),
                     Is().EqualTo(arc{ ptr_uint64(1, 1), true, ptr_uint64(2, 1) }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(),
                     Is().EqualTo(arc{ ptr_uint64(2, 1), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 1), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(4u));

          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                     Is().EqualTo(2u));
          AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                     Is().EqualTo(2u));
        });

      it("bridges over root and others, and creates pre and post chains", [&]() {
        std::vector<int> dom = { 0, 1, 2, 3, 4, 5, 6 };

        shared_levelized_file<zdd::node_type> in;
        /*
        //                      ---- x0
        //
        //             1        ---- x1
        //            / \
        //            | |       ---- x2
        //            \ /
        //            _2_       ---- x3
        //           /   \
        //           3   4      ---- x4
        //          / \ / \
        //          5  6  T     ---- x5
        //         / \//
        //         F  7         ---- x6
        //           / \
        //           T T
        */
        {
          const node n7 = node(6, node::max_id, terminal_T, terminal_T);
          const node n6 = node(5, node::max_id, n7.uid(), n7.uid());
          const node n5 = node(5, node::max_id - 1, terminal_F, n7.uid());
          const node n4 = node(4, node::max_id, n6.uid(), terminal_T);
          const node n3 = node(4, node::max_id - 1, n5.uid(), n6.uid());
          const node n2 = node(3, node::max_id, n3.uid(), n4.uid());
          const node n1 = node(1, node::max_id, n2.uid(), n2.uid());

          node_ofstream w(in);
          w << n7 << n6 << n5 << n4 << n3 << n2 << n1;
        }

        __bdd out = bdd_from(in, dom.begin(), dom.end());
        /*
        //              1       ---- x0
        //             / \
        //             | F      ---- x1
        //             |
        //            _2_       ---- x2
        //           /   \
        //          _3_  F      ---- x3
        //         /   \
        //         4   5        ---- x4
        //        / \ / \
        //        6 T T 7       ---- x5
        //       / \   / \
        //       F T   8 F      ---- x6
        //            / \
        //            T F
        */

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True()); // 2
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 3
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 4
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(3, 0), false, ptr_uint64(4, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 5
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(3, 0), true, ptr_uint64(4, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 6
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(4, 0), false, ptr_uint64(5, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 7
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(4, 1), true, ptr_uint64(5, 1) }));

        AssertThat(arcs.can_pull_internal(), Is().True()); // 8
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(5, 1), false, ptr_uint64(6, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 1
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 2
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 4
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(4, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 5
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(4, 1), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 6
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(5, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(5, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 7
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(5, 1), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True()); // 8
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(6, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(6, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(4, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(5, 2u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(6, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(5u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(4u));
      });
    });

    describe("bdd_from(const zdd&)", [&]() {
      it("returns check-false chain to T terminal on { Ø } with set dom = { 0,1,2 }", [&]() {
        domain_set(dom_012.begin(), dom_012.end());

        __bdd out = bdd_from(zdd_T);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, ptr_uint64(2, ptr_uint64::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id), terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("kills root and bridges over it on { Ø, { 2 } } with set dom = { 0,2,4 }", [&]() {
        domain_set(dom_024.begin(), dom_024.end());

        __bdd out = bdd_from(zdd_x2_null);

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(4, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(4, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(4, 0), true, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__bdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(1u));

        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(2u));
        AssertThat(out.get<__bdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });
    });

    describe("zdd_from(const bdd&, const generator<...>&)", [&]() {
      // TODO
    });

    describe("zdd_from(const bdd&, ForwardIt, ForwardIt)", [&]() {
      it("returns Ø on F terminal with dom = Ø", [&]() {
        __zdd out = zdd_from(bdd_F, dom_empty.begin(), dom_empty.end());

        AssertThat(out.get<__zdd::shared_node_file_type>(), Is().EqualTo(bdd_F.file_ptr()));
        AssertThat(out._negate, Is().False());
        AssertThat(out._shift, Is().EqualTo(0));
      });

      it("returns { Ø } on T terminal with dom = Ø", [&]() {
        __zdd out = zdd_from(bdd_T, dom_empty.begin(), dom_empty.end());

        AssertThat(out.get<__zdd::shared_node_file_type>(), Is().EqualTo(bdd_T.file_ptr()));
        AssertThat(out._negate, Is().False());
        AssertThat(out._shift, Is().EqualTo(0));
      });

      it("returns Ø on shifted F terminal with dom = Ø", [&]() {
        __zdd out = zdd_from(bdd(nf_F, false, +2), dom_empty.begin(), dom_empty.end());

        AssertThat(out.get<__zdd::shared_node_file_type>(), Is().EqualTo(nf_F));
        AssertThat(out._negate, Is().False());
        AssertThat(out._shift, Is().EqualTo(0));
      });

      it("returns { Ø } on shifted T terminal with dom = Ø", [&]() {
        __zdd out = zdd_from(bdd(nf_T, false, +42), dom_empty.begin(), dom_empty.end());

        AssertThat(out.get<__zdd::shared_node_file_type>(), Is().EqualTo(nf_T));
        AssertThat(out._negate, Is().False());
        AssertThat(out._shift, Is().EqualTo(0));
      });

      it("returns Ø on negated F terminal with dom = Ø", [&]() {
        __zdd out = zdd_from(bdd_not(bdd_F), dom_empty.begin(), dom_empty.end());

        AssertThat(out.get<__zdd::shared_node_file_type>(), Is().EqualTo(bdd_F.file_ptr()));
        AssertThat(out._negate, Is().True());
        AssertThat(out._shift, Is().EqualTo(0));

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns { Ø } on negated T terminal with dom = Ø", [&]() {
        __zdd out = zdd_from(bdd_not(bdd_T), dom_empty.begin(), dom_empty.end());

        AssertThat(out.get<__zdd::shared_node_file_type>(), Is().EqualTo(bdd_T.file_ptr()));
        AssertThat(out._negate, Is().True());
        AssertThat(out._shift, Is().EqualTo(0));

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns Ø on F terminal with dom = { 0,1,2 }", [&]() {
        __zdd out = zdd_from(bdd_F, dom_012.begin(), dom_012.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("returns pow(dom) on T terminal with dom = { 0,1,2 }", [&]() {
        __zdd out = zdd_from(bdd_T, dom_012.begin(), dom_012.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     ptr_uint64(2, ptr_uint64::max_id),
                                     ptr_uint64(2, ptr_uint64::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     ptr_uint64(1, ptr_uint64::max_id),
                                     ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("adds don't care chain before root on x2 terminal with dom = { 0,1,2 }", [&]() {
        __zdd out = zdd_from(bdd_x2, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(1u));
      });

      it("adds don't care chain after root on x0 terminal with dom = { 0,1,2 }", [&]() {
        __zdd out = zdd_from(bdd_x0, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("kills and bridges root on ~x1 with dom = { 0,1,2 }", [&]() {
        shared_levelized_file<bdd::node_type> nf;
        {
          node_ofstream nw(nf);
          nw << node(1, node::max_id, terminal_T, terminal_F);
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut, Is().EqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("kills root into { Ø } on ~x0 into with dom = { 0 }", [&]() {
        shared_levelized_file<bdd::node_type> nf;
        {
          node_ofstream nw(nf);
          nw << node(0, node::max_id, terminal_T, terminal_F);
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_0.begin(), dom_0.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      it("collapses false-chain into { Ø } on with dom = { 0,2,4 }", [&]() {
        shared_levelized_file<bdd::node_type> nf;
        {
          const node n3 = node(4, node::max_id, terminal_T, terminal_F);
          const node n2 = node(2, node::max_id, n3.uid(), terminal_F);
          const node n1 = node(0, node::max_id, n2.uid(), terminal_F);

          node_ofstream nw(nf);
          nw << n3 << n2 << n1;
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_024.begin(), dom_024.end());

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });

      // Minato examples
      it("converts [Minato] Fig. 3 into Fig. 5 with dom = { 0,1,2 } ", [&]() {
        __zdd out = zdd_from(bdd_minato_fig3_a, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("converts [Minato] Fig. 3 into Fig. 5 with dom = { 0,1,2,3 } ", [&]() {
        __zdd out = zdd_from(bdd_minato_fig3_b, dom_0123.begin(), dom_0123.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ ptr_uint64(0, 0), ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(0, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("converts [Minato] shifted Fig. 3 with dom = { 0,1,2 } + { 3 } ", [&]() {
        __zdd out = zdd_from(bdd(nf_minato_fig3_a, false, +1), dom_0123.begin(), dom_0123.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });

      it("converts [Minato] Fig. 11 with dom = { 0,1,2,3 } ", [&]() {
        shared_levelized_file<bdd::node_type> nf;
        {
          const node n4 = node(3, node::max_id, terminal_T, terminal_F);
          const node n3 = node(2, node::max_id, n4.uid(), terminal_F);
          const node n2 = node(1, node::max_id - 1, terminal_T, n3.uid());
          const node n1 = node(0, node::max_id, n2.uid(), terminal_F);

          node_ofstream nw(nf);
          nw << n4 << n3 << n2 << n1;
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_0123.begin(), dom_0123.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), false, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(2, 0), true, ptr_uint64(3, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), false, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(3, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(0u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(3u));
      });

      it("converts [Minato] Fig. 15 with dom = { 0,1,2 } ", [&]() {
        shared_levelized_file<bdd::node_type> nf;
        {
          const node n5 = node(2, node::max_id, terminal_F, terminal_T);
          const node n4 = node(2, node::max_id - 1, terminal_T, terminal_F);
          const node n3 = node(1, node::max_id, n5.uid(), terminal_F);
          const node n2 = node(1, node::max_id - 1, n5.uid(), n4.uid());
          const node n1 = node(0, node::max_id, n3.uid(), n2.uid());

          node_ofstream nw(nf);
          nw << n5 << n4 << n3 << n2 << n1;
        }
        bdd in(nf);

        __zdd out = zdd_from(in, dom_012.begin(), dom_012.end());

        arc_test_ifstream arcs(out);

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), true, ptr_uint64(1, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(0, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().True());
        AssertThat(arcs.pull_internal(),
                   Is().EqualTo(arc{ ptr_uint64(1, 0), false, ptr_uint64(2, 0) }));

        AssertThat(arcs.can_pull_internal(), Is().False());

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(1, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), false, terminal_F }));

        AssertThat(arcs.can_pull_terminal(), Is().True());
        AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ ptr_uint64(2, 0), true, terminal_T }));

        AssertThat(arcs.can_pull_terminal(), Is().False());

        level_info_test_ifstream levels(out);

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(levels.can_pull(), Is().True());
        AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(levels.can_pull(), Is().False());

        AssertThat(out.get<__zdd::shared_arc_file_type>()->width, Is().EqualTo(1u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->max_1level_cut,
                   Is().GreaterThanOrEqualTo(2u));

        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[false],
                   Is().EqualTo(1u));
        AssertThat(out.get<__zdd::shared_arc_file_type>()->number_of_terminals[true],
                   Is().EqualTo(2u));
      });
    });

    describe("zdd_from(const bdd&)", [&]() {
      it("returns pow(dom) on T terminal with set dom = { 0,1,2 }", [&]() {
        domain_set(dom_012.begin(), dom_012.end());

        __zdd out = zdd_from(bdd_T);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     ptr_uint64(2, ptr_uint64::max_id),
                                     ptr_uint64(2, ptr_uint64::max_id))));

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     ptr_uint64(1, ptr_uint64::max_id),
                                     ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(ms.can_pull(), Is().True());
        AssertThat(ms.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(ms.can_pull(), Is().False());
      });

      it("collapses false-chain into { Ø } on with set dom = { 0,2,4 }", [&]() {
        shared_levelized_file<bdd::node_type> nf;
        {
          const node n3 = node(4, node::max_id, terminal_T, terminal_F);
          const node n2 = node(2, node::max_id, n3.uid(), terminal_F);
          const node n1 = node(0, node::max_id, n2.uid(), terminal_F);

          node_ofstream nw(nf);
          nw << n3 << n2 << n1;
        }
        bdd in(nf);

        domain_set(dom_024.begin(), dom_024.end());

        __zdd out = zdd_from(in);

        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream ms(out);
        AssertThat(ms.can_pull(), Is().False());
      });
    });
  });
});
