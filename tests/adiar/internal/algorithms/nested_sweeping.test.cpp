#include "../../../test.h"

#include <adiar/bdd/bdd_policy.h>
#include <adiar/statistics.h>

#include <adiar/internal/algorithms/nested_sweeping.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/request.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Policy that provides the most simplistic top-down sweep to test the Inner Down Sweep in
/// isolation. The algorithm within is a simple 'mark and sweep' GC that somewhat negates the entire
/// DAG (but kills the nodes on the nesting level).
///
/// \tparam FinalCanonical Whether the final BDD should be canonical (see `FastReduce`).
///
/// \tparam FastReduce     Whether to use the 'fast reduce' algorithm.
///
/// \tparam OnlyGC         When set to true, then each request created will only have a single
///                        non-nil member, i.e. it is a request preserving a subtree but
///                        (presumably) not changing it.
////////////////////////////////////////////////////////////////////////////////////////////////////
template <bool FinalCanonical = true, bool FastReduce = false, bool OnlyGC = false>
class test_not_sweep
  : public bdd_policy
  , public statistics::__alg_base::__lpq_t
{
private:
  const size_t _nesting_modulo;

public:
  // HACK: The request is for a 'tuple' to allow us to 'lie' to the Nested
  //       Sweeping framework that we are not "just" preserving some node
  //       underneath or inversely that we are not "changing" subgraph.
  using request_t = request_data<2, with_parent, 0, 1>;

  using request_pred_t = request_data_first_lt<request_t>;

  template <size_t look_ahead, memory_mode mem_mode>
  using pq_t = levelized_node_priority_queue<request_t, request_pred_t, look_ahead, mem_mode>;

public:
  test_not_sweep(const size_t nm)
    : _nesting_modulo(nm)
  {}

public:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  static size_t
  stream_memory()
  {
    return node_ifstream<>::memory_usage() + arc_ofstream::memory_usage();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  static size_t
  pq_memory(const size_t inner_memory)
  {
    return inner_memory;
  }

  static size_t
  ra_memory(const shared_levelized_file<node>& /*outer_file*/)
  {
    return std::numeric_limits<size_t>::max();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  static size_t
  pq_bound(const shared_levelized_file<node>& /*outer_file*/, const size_t /*outer_roots*/)
  {
    return 8;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  constexpr inline bdd::label_type
  map_level(bdd::label_type x) const
  {
    return x;
  }

public:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The Sweep Logic for the PQ access mode case.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename inner_pq_t>
  __bdd
  sweep_pq(const exec_policy& ep,
           const shared_levelized_file<node>& outer_file,
           inner_pq_t& inner_pq,
           const size_t /*inner_remaining_memory == 0*/)
  {
    node_ifstream<> ns(outer_file);

    shared_levelized_file<arc> af;
    af->max_1level_cut = 1;

    arc_ofstream aw(af);

    while (!inner_pq.empty()) {
      inner_pq.setup_next_level();

      const node::label_type level_label = inner_pq.current_level();
      node::id_type level_size           = 0u;

      while (!inner_pq.empty_level()) {
        // Give node a new name
        const node::uid_type u(level_label, level_size++);

        // Get target of next request
        adiar_assert(!inner_pq.top().target.first().is_flagged(),
                     "Double checking decorator indeed hides taint");

        const node::pointer_type next = inner_pq.top().target.first();

        // Seek node in stream and forward its out-going edges
        const node n = ns.seek(next);
        forward_arc<false>(inner_pq, aw, u, n.low());
        forward_arc<true>(inner_pq, aw, u, n.high());

        // Output all in-going edges that match the target
        while (inner_pq.can_pull() && inner_pq.top().target.first() == next) {
          request_t req = inner_pq.pull();
          if (req.data.source.is_nil()) { continue; }

          aw.push_internal({ req.data.source, u });
        }
      }

      aw.push(level_info(level_label, level_size));
      af->max_1level_cut = std::max(af->max_1level_cut, inner_pq.size());
    }

    return __bdd(af, ep);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The Sweep Logic for the RA access mode case.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename inner_pq_t>
  __bdd
  sweep_ra(const exec_policy& ep,
           const shared_levelized_file<node>& outer_file,
           inner_pq_t& inner_pq,
           const size_t inner_remaining_memory)
  {
    return sweep_pq(ep, outer_file, inner_pq, inner_remaining_memory);
  }

private:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <bool is_high, typename inner_pq_t>
  void
  forward_arc(inner_pq_t& inner_pq,
              arc_ofstream& aw,
              const node::uid_type& uid,
              const node::pointer_type& c)
  {
    if (c.is_terminal()) {
      aw.push_terminal({ uid, is_high, !c });
    } else {
      inner_pq.push({ { c, node::pointer_type::nil() }, {}, { uid.as_ptr(is_high) } });
    }
  }

public:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether it wants to sweep on some level.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  has_sweep(node::pointer_type::label_type l) const
  {
    return (l % _nesting_modulo) == 0u;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Pick PQ type and Run Sweep.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename outer_roots_t>
  __bdd
  sweep(const exec_policy& ep,
        const shared_levelized_file<node>& outer_file,
        outer_roots_t& outer_roots,
        const size_t inner_memory)
  {
    return nested_sweeping::inner::down__sweep_switch(
      ep, *this, outer_file, outer_roots, inner_memory, *this);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create request
  //////////////////////////////////////////////////////////////////////////////////////////////////
  static inline request_t
  request_from_node(const node& n, const ptr_uint64& parent)
  {
    // Always pick high child
    return request_t({ n.high(), OnlyGC ? node::pointer_type::nil() : n.high() }, {}, { parent });
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  static constexpr bool final_canonical = FinalCanonical;
  static constexpr bool fast_reduce     = FastReduce;
};

class test_terminal_sweep
  : public bdd_policy
  , public statistics::__alg_base::__lpq_t
{
private:
  const size_t _nesting_modulo;

public:
  // HACK: The request is for a 'tuple' to allow us to tell the Nested Sweeping
  //       framework we are indeed changing subgraph.
  using request_t      = request_data<2, with_parent, 0, 1>;
  using request_pred_t = request_first_lt<request_t>;

  template <size_t look_ahead, memory_mode mem_mode>
  using pq_t = levelized_node_priority_queue<request_t, request_pred_t, look_ahead, mem_mode>;

public:
  test_terminal_sweep(const size_t nm)
    : _nesting_modulo(nm)
  {}

public:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  static size_t
  stream_memory()
  {
    return 0u;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  static size_t
  pq_memory(const size_t inner_memory)
  {
    return inner_memory;
  }

  static size_t
  ra_memory(const shared_levelized_file<node>& /*outer_file*/)
  {
    return std::numeric_limits<size_t>::max();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  static size_t
  pq_bound(const shared_levelized_file<node>& /*outer_file*/, const size_t /*outer_roots*/)
  {
    return 1u;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  constexpr inline node::label_type
  map_level(node::label_type x) const
  {
    return x;
  }

public:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The Sweep Logic for the PQ access mode case.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename inner_pq_t>
  __bdd
  sweep_pq(const exec_policy& ep,
           const shared_levelized_file<node>& outer_file,
           inner_pq_t& inner_pq,
           const size_t /*inner_remaining_memory == 0*/)
  {
    node_ifstream<> ns(outer_file);

    shared_levelized_file<arc> af;
    af->max_1level_cut = 1;

    arc_ofstream aw(af);

    while (!inner_pq.empty()) {
      inner_pq.setup_next_level();

      const node::label_type level_label = inner_pq.current_level();
      node::id_type level_size           = 0u;

      while (!inner_pq.empty_level()) {
        // Get target of next request
        const node::pointer_type next = inner_pq.top().target.first();
        node::uid_type t;

        if (next.label() % _nesting_modulo == 1) {
          // Collapse immediately to a terminal
          t = node::uid_type(level_label % (_nesting_modulo + 1) > 0);
        } else {
          // Create a simple (i, _, _) node with two terminals.
          t = node::uid_type(level_label, level_size++);

          const node::pointer_type t0((level_label) % (_nesting_modulo) > 1);
          aw.push_terminal({ t, false, t0 });

          const node::pointer_type t1((level_label + 1) % (_nesting_modulo) > 1);
          aw.push_terminal({ t, true, t1 });
        }

        // Output all in-going edges that match the target
        while (inner_pq.can_pull() && inner_pq.top().target.first() == next) {
          request_t req = inner_pq.pull();
          if (req.data.source.is_nil()) {
            if (t.is_terminal()) { return bdd(t.value()); }
          } else {
            aw.push({ req.data.source, t });
          }
        }
      }
      if (level_size > 0) { aw.push(level_info(level_label, level_size)); }
    }

    return __bdd(af, ep);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The Sweep Logic for the RA access mode case.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename inner_pq_t>
  __bdd
  sweep_ra(const exec_policy& ep,
           const shared_levelized_file<node>& outer_file,
           inner_pq_t& inner_pq,
           const size_t inner_remaining_memory)
  {
    return sweep_pq(ep, outer_file, inner_pq, inner_remaining_memory);
  }

public:
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether it wants to sweep on some level.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  bool
  has_sweep(node::pointer_type::label_type l) const
  {
    return (l % _nesting_modulo) == 0u;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Pick PQ type and Run Sweep.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename outer_roots_t>
  __bdd
  sweep(const exec_policy& ep,
        const shared_levelized_file<node>& outer_file,
        outer_roots_t& outer_roots,
        const size_t inner_memory)
  {
    return nested_sweeping::inner::down__sweep_switch(
      ep, *this, outer_file, outer_roots, inner_memory, *this);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create request
  //////////////////////////////////////////////////////////////////////////////////////////////////
  static inline request_t
  request_from_node(const node& n, const ptr_uint64& parent)
  {
    // Always pick low child
    return request_t({ n.low() }, {}, { parent });
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  static constexpr bool final_canonical = true;
  static constexpr bool fast_reduce     = false;
};

go_bandit([]() {
  describe("adiar/internal/algorithms/nested_sweeping.h", []() {
    // Outer PQ
    using outer_test_pq_t = nested_sweeping::outer::up__pq_t<1, memory_mode::Internal>;
    /*
    //       1         ---- x0
    //      / \
    //      F 2        ---- x1
    //       / \
    //       F 3       ---- x2
    //        / \
    //        F 4      ---- x3
    //         / \
    //         F 5     ---- x4
    //          / \
    //          F T
    */

    const uid_uint64 terminal_F(false);
    const uid_uint64 terminal_T(true);

    const uid_uint64 outer_n1(0, 0);
    const uid_uint64 outer_n2(1, 0);
    const uid_uint64 outer_n3(2, 0);
    const uid_uint64 outer_n4(3, 0);
    const uid_uint64 outer_n5(4, 0);

    shared_levelized_file<arc> outer_dag;

    { // Garbage collect writer to free write-lock
      arc_ofstream aw(outer_dag);

      aw.push_internal({ outer_n1, true, outer_n2 });
      aw.push_internal({ outer_n2, true, outer_n3 });
      aw.push_internal({ outer_n3, true, outer_n4 });
      aw.push_internal({ outer_n4, true, outer_n5 });

      aw.push_terminal({ outer_n1, false, terminal_F });
      aw.push_terminal({ outer_n2, false, terminal_F });
      aw.push_terminal({ outer_n3, false, terminal_F });
      aw.push_terminal({ outer_n4, false, terminal_F });
      aw.push_terminal({ outer_n5, false, terminal_F });
      aw.push_terminal({ outer_n5, true, terminal_T });

      aw.push(level_info(0, 1u));
      aw.push(level_info(1, 1u));
      aw.push(level_info(2, 1u));
      aw.push(level_info(3, 1u));
      aw.push(level_info(4, 1u));
    }

    describe("nested_sweeping:: _ ::decorators", [&]() {
      describe("outer::roots_sorter", []() {
        it("can sort pushed 'requests_t' [cardinality = 2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          const test_request_t i1({ arc::pointer_type(1, 1), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(0, 0, false) });
          s.push(i1);

          const test_request_t i2({ arc::pointer_type(1, 0), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(0, 0, true) });
          s.push(i2);

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o1 = s.pull();

          AssertThat(o1.target, Is().EqualTo(i2.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(i2.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o2 = s.pull();

          AssertThat(o2.target, Is().EqualTo(i1.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(i1.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can convert and sort 'reduced_arc' [cardinality == 1]", []() {
          using test_request_t = request_data<1, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          {
            const reduce_arc i1 = arc(arc::uid_type(0, 0), false, arc::pointer_type(1, 1));
            s.push(i1);

            const reduce_arc i2 = arc(arc::uid_type(0, 0), true, arc::pointer_type(1, 0));
            s.push(i2);
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t e1(
            { arc::pointer_type(1, 0) }, {}, { arc::pointer_type(0, 0, true) });
          const test_request_t o1 = s.pull();

          AssertThat(o1.target, Is().EqualTo(e1.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(e1.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t e2(
            { arc::pointer_type(1, 1) }, {}, { arc::pointer_type(0, 0, false) });
          const test_request_t o2 = s.pull();

          AssertThat(o2.target, Is().EqualTo(e2.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(e2.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can convert and sort 'reduced_arc' [cardinality == 2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          {
            const reduce_arc a1 = arc(arc::uid_type(0, 0), false, arc::pointer_type(1, 1));
            s.push(a1);

            const reduce_arc a2 = arc(arc::uid_type(0, 0), true, arc::pointer_type(1, 0));
            s.push(a2);
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t e1({ arc::pointer_type(1, 0), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(0, 0, true) });
          const test_request_t o1 = s.pull();

          AssertThat(o1.target, Is().EqualTo(e1.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(e1.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t e2({ arc::pointer_type(1, 1), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(0, 0, false) });
          const test_request_t o2 = s.pull();

          AssertThat(o2.target, Is().EqualTo(e2.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(e2.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can mix insertion of 'request_t' and 'reduced_arc' [cardinality == 2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          const test_request_t i1({ arc::pointer_type(3, 3), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(2, 0, false) });
          const test_request_t i2({ arc::pointer_type(3, 1), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(2, 0, true) });
          const test_request_t i3({ arc::pointer_type(3, 2), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(2, 1, false) });
          const test_request_t i4({ arc::pointer_type(3, 4), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(2, 1, true) });

          {
            s.push(reduce_arc(arc(i1.data.source, i1.target[0])));
            s.push(i2);
            s.push(i3);
            s.push(reduce_arc(arc(i4.data.source, i4.target[0])));
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o1 = s.pull();

          AssertThat(o1.target, Is().EqualTo(i2.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(i2.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o2 = s.pull();

          AssertThat(o2.target, Is().EqualTo(i3.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(i3.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o3 = s.pull();

          AssertThat(o3.target, Is().EqualTo(i1.target));
          AssertThat(o3.data.source, Is().EqualTo(flag(i1.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o4 = s.pull();

          AssertThat(o4.target, Is().EqualTo(i4.target));
          AssertThat(o4.data.source, Is().EqualTo(flag(i4.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can reset after sorting", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          const test_request_t i1({ arc::pointer_type(3, 3), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(2, 0, false) });
          const test_request_t i2({ arc::pointer_type(3, 2), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(2, 0, true) });
          const test_request_t i3({ arc::pointer_type(3, 2), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(2, 1, false) });
          const test_request_t i4({ arc::pointer_type(3, 1), arc::pointer_type::nil() },
                                  {},
                                  { arc::pointer_type(2, 1, true) });

          {
            s.push(reduce_arc(arc(i1.data.source, i1.target[0])));
            s.push(i2);
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o1 = s.pull();

          AssertThat(o1.target, Is().EqualTo(i2.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(i2.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o2 = s.pull();

          AssertThat(o2.target, Is().EqualTo(i1.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(i1.data.source)));

          AssertThat(s.can_pull(), Is().False());

          s.reset();

          {
            s.push(i3);
            s.push(reduce_arc(arc(i4.data.source, i4.target[0])));
          }

          s.sort();

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o3 = s.pull();

          AssertThat(o3.target, Is().EqualTo(i4.target));
          AssertThat(o3.data.source, Is().EqualTo(flag(i4.data.source)));

          AssertThat(s.can_pull(), Is().True());
          const test_request_t o4 = s.pull();

          AssertThat(o4.target, Is().EqualTo(i3.target));
          AssertThat(o4.data.source, Is().EqualTo(flag(i3.data.source)));

          AssertThat(s.can_pull(), Is().False());
        });

        it("can move from another sorter", []() {
          using test_request_t = request_data<1, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s1(1024, 16);
          test_roots_sorter_t s2(1024, 16);

          const test_request_t i1(
            { arc::pointer_type(3, 2) }, {}, { arc::pointer_type(2, 0, false) });
          const test_request_t i2(
            { arc::pointer_type(3, 1) }, {}, { arc::pointer_type(2, 1, true) });

          s1.push(i1);
          s1.push(i2);

          s2.move(s1); // From hereon forward s1 is in an illegal state
          s2.sort();

          AssertThat(s2.can_pull(), Is().True());
          const test_request_t o1 = s2.pull();

          AssertThat(o1.target, Is().EqualTo(i2.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(i2.data.source)));

          AssertThat(s2.can_pull(), Is().True());
          const test_request_t o2 = s2.pull();

          AssertThat(o2.target, Is().EqualTo(i1.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(i1.data.source)));

          AssertThat(s2.can_pull(), Is().False());
        });

        it("can move from another sorter and push more elements", []() {
          using test_request_t = request_data<1, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s1(1024, 16);
          test_roots_sorter_t s2(1024, 16);

          const test_request_t i1(
            { arc::pointer_type(3, 3) }, {}, { arc::pointer_type(2, 0, false) });
          const test_request_t i2(
            { arc::pointer_type(3, 1) }, {}, { arc::pointer_type(2, 1, true) });

          s1.push(i1);
          s1.push(i2);

          s2.move(s1); // From hereon forward s1 is in an illegal state

          const test_request_t i3(
            { arc::pointer_type(3, 2) }, {}, { arc::pointer_type(2, 1, false) });
          s2.push(i3);

          s2.sort();

          AssertThat(s2.can_pull(), Is().True());
          const test_request_t o1 = s2.pull();

          AssertThat(o1.target, Is().EqualTo(i2.target));
          AssertThat(o1.data.source, Is().EqualTo(flag(i2.data.source)));

          AssertThat(s2.can_pull(), Is().True());
          const test_request_t o2 = s2.pull();

          AssertThat(o2.target, Is().EqualTo(i3.target));
          AssertThat(o2.data.source, Is().EqualTo(flag(i3.data.source)));

          AssertThat(s2.can_pull(), Is().True());
          const test_request_t o3 = s2.pull();

          AssertThat(o3.target, Is().EqualTo(i1.target));
          AssertThat(o3.data.source, Is().EqualTo(flag(i1.data.source)));

          AssertThat(s2.can_pull(), Is().False());
        });

        it("updates its 'size'", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.size(), Is().EqualTo(0u));

          s.push(arc(arc::uid_type(2, 0), false, arc::pointer_type(3, 0)));
          AssertThat(s.size(), Is().EqualTo(1u));

          s.push({ { arc::pointer_type(3, 0) }, {}, { arc::pointer_type(2, 0, true) } });
          AssertThat(s.size(), Is().EqualTo(2u));

          s.reset();
          AssertThat(s.size(), Is().EqualTo(0u));

          s.push({ { arc::pointer_type(1, 0) }, {}, { arc::pointer_type(0, 0, true) } });
          AssertThat(s.size(), Is().EqualTo(1u));
        });

        it("provides the deepest source from arcs [1]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::no_level));

          const reduce_arc a1 = arc(arc::uid_type(0, 0), false, arc::pointer_type(1, 1));
          s.push(a1);

          AssertThat(s.deepest_source(), Is().EqualTo(0u));

          const reduce_arc a2 = arc(arc::uid_type(1, 0), false, arc::pointer_type(2, 0));
          s.push(a2);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));

          const reduce_arc a3 = arc(arc::uid_type(0, 0), true, arc::pointer_type(1, 0));
          s.push(a3);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));
        });

        it("provides the deepest source from arcs [2]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::no_level));

          const reduce_arc a1 = arc(arc::uid_type(2, 0), false, arc::pointer_type(3, 1));
          s.push(a1);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));

          const reduce_arc a2 = arc(arc::uid_type(1, 0), false, arc::pointer_type(4, 0));
          s.push(a2);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));
        });

        it("provides the deepest source from requests [1]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::no_level));

          const test_request_t r1(
            { arc::pointer_type(1, 1) }, {}, { arc::pointer_type(0, 0, false) });
          s.push(r1);

          AssertThat(s.deepest_source(), Is().EqualTo(0u));

          const test_request_t r2(
            { arc::pointer_type(2, 0) }, {}, { arc::pointer_type(1, 0, false) });
          s.push(r2);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));

          const test_request_t r3(
            { arc::pointer_type(1, 0) }, {}, { arc::pointer_type(0, 0, true) });
          s.push(r3);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));
        });

        it("provides the deepest source from requests [1]", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::no_level));

          const test_request_t r1(
            { arc::pointer_type(3, 1) }, {}, { arc::pointer_type(2, 0, false) });
          s.push(r1);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));

          const test_request_t r2(
            { arc::pointer_type(4, 0) }, {}, { arc::pointer_type(1, 0, false) });
          s.push(r2);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));
        });

        it("provides a different deepest source after reset", []() {
          using test_request_t = request_data<2, with_parent, 0, 1>;

          using test_roots_sorter_t = nested_sweeping::outer::
            roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

          test_roots_sorter_t s(1024, 16);

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::no_level));

          const reduce_arc a1 = arc(arc::uid_type(2, 0), false, arc::pointer_type(1, 1));
          s.push(a1);

          AssertThat(s.deepest_source(), Is().EqualTo(2u));

          s.reset();

          AssertThat(s.deepest_source(), Is().EqualTo(test_roots_sorter_t::no_level));

          const reduce_arc a2 = arc(arc::uid_type(1, 0), false, arc::pointer_type(2, 0));
          s.push(a2);

          AssertThat(s.deepest_source(), Is().EqualTo(1u));
        });
      });

      describe("outer::up__pq_decorator", [&]() {
        using test_request_t = request_data<2, with_parent, 0, 1>;

        using test_roots_sorter_t = nested_sweeping::outer::
          roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

        using test_pq_t = nested_sweeping::outer::up__pq_t<1, memory_mode::Internal>;

        using test_decorator =
          nested_sweeping::outer::up__pq_decorator<test_pq_t, test_roots_sorter_t>;

        it("forwards internal arcs to PQ when below threshold", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n4, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards internal arcs to PQ when at threshold", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n3, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards internal arcs to Sorter when above threshold", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n2, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(0u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });

        it("forwards terminal arcs to PQ even if at threshold", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n3, false, terminal_F));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards terminal arcs to PQ even if above threshold", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          d.push(arc(outer_n2, false, terminal_T));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards correctly with multiple arcs", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal below threshold
          d.push(arc(outer_n4, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // terminal at threshold
          d.push(arc(outer_n3, false, terminal_F));

          AssertThat(pq.size(), Is().EqualTo(2u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // internal at threshold
          d.push(arc(outer_n3, true, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(3u));
          AssertThat(sorter.size(), Is().EqualTo(0u));

          // internal above threshold
          d.push(arc(outer_n2, false, outer_n5));

          AssertThat(pq.size(), Is().EqualTo(3u));
          AssertThat(sorter.size(), Is().EqualTo(1u));

          // terminal above threshold
          d.push(arc(outer_n1, false, terminal_T));

          AssertThat(pq.size(), Is().EqualTo(4u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });

        it("provides combined size (w and w/o terminals)", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          AssertThat(d.empty(), Is().True());

          // internal below threshold
          d.push(arc(outer_n4, false, outer_n5));
          // terminal at threshold
          d.push(arc(outer_n3, false, terminal_F));
          // internal at threshold
          d.push(arc(outer_n3, true, outer_n5));
          // internal above threshold
          d.push(arc(outer_n2, false, outer_n5));
          // terminal above threshold
          d.push(arc(outer_n1, false, terminal_T));

          AssertThat(d.empty(), Is().False());
          AssertThat(d.size(), Is().EqualTo(5u));
          AssertThat(d.terminals(false), Is().EqualTo(1u));
          AssertThat(d.terminals(true), Is().EqualTo(1u));
          AssertThat(d.size_without_terminals(), Is().EqualTo(3u));
        });

        it("forwards internal request to PQ", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal at threshold
          d.push(test_request_t({ outer_n5 }, {}, { outer_n1.as_ptr(true) }));

          AssertThat(pq.size(), Is().EqualTo(0u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });

        it("forwards terminal request to PQ", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(4u);

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal at threshold
          d.push(test_request_t({ terminal_T }, {}, { outer_n1 }));

          AssertThat(pq.size(), Is().EqualTo(1u));
          AssertThat(sorter.size(), Is().EqualTo(0u));
        });

        it("forwards arc with nil source to sorter", [&]() {
          test_pq_t pq({ outer_dag }, memory_available(), 16);
          pq.setup_next_level(0u); // <-- set up pq to have no extra levels.

          AssertThat(pq.has_next_level(), Is().False()); // sanity check

          test_roots_sorter_t sorter(1024, 16);

          test_decorator d(pq, sorter, 2);

          // internal at threshold
          d.push(arc(arc::pointer_type::nil(), arc::pointer_type(42, 0)));

          AssertThat(pq.size(), Is().EqualTo(0u));
          AssertThat(sorter.size(), Is().EqualTo(1u));
        });
      });

      describe("outer::inner_iterator", [&terminal_F, &terminal_T]() {
        using test_iter_t = nested_sweeping::outer::inner_iterator<test_not_sweep<>>;

        it("provides {end} for {3,2,1} % 4", [&]() {
          /*
          //
          //         1       ---- x1
          //        / \
          //        F 2      ---- x2
          //         / \
          //         F 3     ---- x3
          //          / \
          //          F T
          */
          const node::uid_type n1(1, 0);
          const node::uid_type n2(2, 0);
          const node::uid_type n3(3, 0);

          shared_levelized_file<arc> dag;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(dag);

            aw.push_internal({ n1, true, n2 });
            aw.push_internal({ n2, true, n3 });

            aw.push_terminal({ n1, false, terminal_F });
            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n3, true, terminal_T });

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 1u));
            aw.push(level_info(3, 1u));
          }

          test_not_sweep<> inner_impl(4);
          test_iter_t inner_iter(dag, inner_impl);

          AssertThat(inner_iter.next_inner(), Is().EqualTo(test_iter_t::end));
        });

        it("provides {2, end} for {3,2,1} % 2", [&]() {
          /*
          //
          //         1       ---- x1
          //        / \
          //        F 2      ---- x2
          //         / \
          //         F 3     ---- x3
          //          / \
          //          F T
          */
          const node::uid_type n1(1, 0);
          const node::uid_type n2(2, 0);
          const node::uid_type n3(3, 0);

          shared_levelized_file<arc> dag;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(dag);

            aw.push_internal({ n1, true, n2 });
            aw.push_internal({ n2, true, n3 });

            aw.push_terminal({ n1, false, terminal_F });
            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n3, true, terminal_T });

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 1u));
            aw.push(level_info(3, 1u));
          }

          test_not_sweep<> inner_impl(2);
          test_iter_t inner_iter(dag, inner_impl);

          AssertThat(inner_iter.next_inner(), Is().EqualTo(2u));
          AssertThat(inner_iter.next_inner(), Is().EqualTo(test_iter_t::end));
        });

        it("provides {4, 2, 0, end} for {4,3,2,0} % 2", [&]() {
          /*
          //
          //         1       ---- x0
          //        / \
          //        F 2      ---- x2
          //         / \
          //         F 3     ---- x3
          //          / \
          //          F 4    ---- x4
          //           / \
          //           F T
          */
          const node::uid_type n1(0, 0);
          const node::uid_type n2(2, 0);
          const node::uid_type n3(3, 0);
          const node::uid_type n4(4, 0);

          shared_levelized_file<arc> dag;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(dag);

            aw.push_internal({ n1, true, n2 });
            aw.push_internal({ n2, true, n3 });
            aw.push_internal({ n3, true, n4 });

            aw.push_terminal({ n1, false, terminal_F });
            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n4, false, terminal_F });
            aw.push_terminal({ n4, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(2, 1u));
            aw.push(level_info(3, 1u));
            aw.push(level_info(4, 1u));
          }

          test_not_sweep<> inner_impl(2);
          test_iter_t inner_iter(dag, inner_impl);

          AssertThat(inner_iter.next_inner(), Is().EqualTo(4u));
          AssertThat(inner_iter.next_inner(), Is().EqualTo(2u));
          AssertThat(inner_iter.next_inner(), Is().EqualTo(0u));
          AssertThat(inner_iter.next_inner(), Is().EqualTo(test_iter_t::end));
        });
      });

      describe("inner::down__pq_decorator", [&terminal_F, &terminal_T]() {
        const node::uid_type inner_n1(3, 0);
        const node::uid_type inner_n2(3, 1);
        const node::uid_type inner_n3(4, 0);
        const node::uid_type inner_n4(4, 1);
        const node::uid_type inner_n5(5, 0);
        const node::uid_type inner_n6(5, 1);

        shared_levelized_file<node> dag;

        /*
        //        1   2        ---- x3
        //       / \ / \
        //       3  4   \      ---- x4
        //      / \/ \   \
        //      F T  5   6     ---- x5
        //          / \ / \
        //          T F F T
        */

        { // Garbage collect writer to free write-lock
          node_ofstream nw(dag);

          nw << node(inner_n6, terminal_F, terminal_T) << node(inner_n5, terminal_T, terminal_F)
             << node(inner_n4, terminal_T, inner_n5) << node(inner_n3, terminal_F, terminal_T)
             << node(inner_n2, inner_n4, inner_n6) << node(inner_n1, inner_n3, inner_n4);
        }

        using test_request_t = request_data<1, with_parent, 0, 1>;

        using test_roots_sorter_t = nested_sweeping::outer::
          roots_sorter<memory_mode::Internal, test_request_t, request_first_lt<test_request_t>>;

        const test_request_t root1({ inner_n1 }, {}, { node::pointer_type(1, 0, false) });
        const test_request_t root2({ inner_n2 }, {}, { node::pointer_type(1, 0, true) });
        const test_request_t root3({ inner_n6 }, {}, { node::pointer_type(1, 1, false) });

        /*
        //
        //       n1p/n2p  n6p
        //        /   \    |
        //        |   |    |   ---- x2 (nested sweep level)
        //        |   |    |
        //        1   2    |   ---- x3
        //                 |
        //                 |   ---- x4
        //                 |
        //                 6   ---- x5
        */

        test_roots_sorter_t sorter(1024, 16);

        before_each([&]() {
          sorter.reset();

          sorter.push(root1);
          sorter.push(root2);
          sorter.push(root3);

          sorter.sort();
        });

        statistics::levelized_priority_queue_t lpq_stats;

        using test_pq_t =
          levelized_node_priority_queue<test_request_t,
                                        request_first_lt<test_request_t>,
                                        1,
                                        memory_mode::Internal,
                                        1,
                                        0 /* <-- this is important for nested sweeping */>;

        using test_decorator =
          nested_sweeping::inner::down__pq_decorator<test_pq_t, test_roots_sorter_t>;

        it("initializes with levels of PQ but with size of Sorter", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());

          AssertThat(dec.has_next_level(), Is().True());
          AssertThat(dec.next_level(), Is().EqualTo(3u));

          AssertThat(dec.empty(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));
        });

        it("merges size of PQ and Sorter", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);

          const test_request_t root4({ inner_n2 }, {}, { node::pointer_type(1, 2, false) });
          pq.push(root4);

          test_decorator dec(pq, sorter);

          AssertThat(dec.size(), Is().EqualTo(4u));
        });

        it("setup_next_level() forward to first Sorter level", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());
          dec.setup_next_level();

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(3u));
        });

        it("setup_next_level(k) forward to Sorter's level if k is larger", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());
          dec.setup_next_level(5);

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(3u));
        });

        it("setup_next_level(k) forward to level k if k is smaller", [&]() {
          sorter.pull();
          sorter.pull(); // Next is at level 6

          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          AssertThat(dec.has_current_level(), Is().False());
          dec.setup_next_level(4);

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(4u));
        });

        it("can pull requests from Sorter", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.pull();
          AssertThat(r2.target, Is().EqualTo(root2.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can pull merge of PQ and Sorter [tie on 'n2']", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({ inner_n2 }, {}, { node::pointer_type(1, 2, false) });
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.pull();
          AssertThat(r2.target, Is().EqualTo(root2.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r3 = dec.pull();
          AssertThat(r3.target, Is().EqualTo(root4.target));
          AssertThat(r3.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can pull merge of PQ and Sorter [tie on 'n1']", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({ inner_n1 }, {}, { node::pointer_type(1, 2, false) });
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r2 = dec.pull();
          AssertThat(r2.target, Is().EqualTo(root4.target));
          AssertThat(r2.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r3 = dec.pull();
          AssertThat(r3.target, Is().EqualTo(root2.target));
          AssertThat(r3.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can pull when Sorter is empty", [&]() {
          // empty out sorter
          AssertThat(sorter.size(), Is().EqualTo(3u));
          sorter.pull();
          sorter.pull();
          sorter.pull();
          AssertThat(sorter.size(), Is().EqualTo(0u));
          AssertThat(sorter.empty(), Is().True());

          // Start test
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({ inner_n1 }, {}, { node::pointer_type(1, 2, false) });
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.can_pull(), Is().True());
          const auto r = dec.pull();
          AssertThat(r.target, Is().EqualTo(root4.target));
          AssertThat(r.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(0u));

          AssertThat(dec.can_pull(), Is().False());
        });

        it("can top the same request from Sorter multiple times", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          // Top the sorter
          AssertThat(dec.has_top(), Is().True());
          const auto r1 = dec.top();
          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          // Top it again
          AssertThat(dec.has_top(), Is().True());
          const auto r2 = dec.top();
          AssertThat(r2.target, Is().EqualTo(root1.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));
        });

        it("can top the same request from PQ multiple times", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);

          const test_request_t root4({ inner_n1 }, {}, { node::pointer_type(1, 2, false) });
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          // Skip over the one request from the sorter (but check it truly is that one)
          AssertThat(dec.has_top(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          // Top the priority queue once
          AssertThat(dec.has_top(), Is().True());
          const auto r2 = dec.top();
          AssertThat(r2.target, Is().EqualTo(root4.target));
          AssertThat(r2.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          // Top it again
          AssertThat(dec.has_top(), Is().True());
          const auto r3 = dec.top();
          AssertThat(r3.target, Is().EqualTo(root4.target));
          AssertThat(r3.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));
        });

        it("can top and pop requests from Sorter", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.has_top(), Is().True());
          const auto r1 = dec.top();
          dec.pop();

          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.has_top(), Is().True());
          const auto r2 = dec.top();
          dec.pop();

          AssertThat(r2.target, Is().EqualTo(root2.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.has_top(), Is().False());
        });

        it("can top and pop merge of PQ and Sorter [tie on 'n2']", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({ inner_n2 }, {}, { node::pointer_type(1, 2, false) });
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          AssertThat(dec.has_top(), Is().True());
          const auto r1 = dec.top();
          dec.pop();

          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.has_top(), Is().True());
          const auto r2 = dec.top();
          dec.pop();

          AssertThat(r2.target, Is().EqualTo(root2.target));
          AssertThat(r2.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.has_top(), Is().True());
          const auto r3 = dec.top();
          dec.pop();

          AssertThat(r3.target, Is().EqualTo(root4.target));
          AssertThat(r3.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.has_top(), Is().False());
        });

        it("can top and pop merge of PQ and Sorter [tie on 'n1']", [&]() {
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({ inner_n1 }, {}, { node::pointer_type(1, 2, false) });
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(4u));

          AssertThat(dec.has_top(), Is().True());
          const auto r1 = dec.top();
          dec.pop();

          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.has_top(), Is().True());
          const auto r2 = dec.top();
          dec.pop();

          AssertThat(r2.target, Is().EqualTo(root4.target));
          AssertThat(r2.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));

          AssertThat(dec.has_top(), Is().True());
          const auto r3 = dec.top();
          dec.pop();

          AssertThat(r3.target, Is().EqualTo(root2.target));
          AssertThat(r3.data.source, Is().EqualTo(flag(root2.data.source)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.has_top(), Is().False());
        });

        it("can top and pop when Sorter is empty", [&]() {
          // empty out sorter
          AssertThat(sorter.size(), Is().EqualTo(3u));
          sorter.pull();
          sorter.pull();
          sorter.pull();
          AssertThat(sorter.size(), Is().EqualTo(0u));
          AssertThat(sorter.empty(), Is().True());

          // Start test
          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);

          // Add new request to tie with sorter
          const test_request_t root4({ inner_n1 }, {}, { node::pointer_type(1, 2, false) });
          pq.push(root4);

          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(1u));

          AssertThat(dec.has_top(), Is().True());
          const auto r = dec.top();
          dec.pop();

          AssertThat(r.target, Is().EqualTo(root4.target));
          AssertThat(r.data.source, Is().EqualTo(root4.data.source));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(0u));

          AssertThat(dec.has_top(), Is().False());
        });

        it("cleans taint flags on targets from outer roots", [&]() {
          sorter.reset(); // ignore 'before_each' and do our own setup

          const test_request_t root1_taint(
            { flag(inner_n1) }, {}, { node::pointer_type(1, 0, false) });
          sorter.push(root1_taint);
          sorter.push(root2);
          sorter.push(root3);

          sorter.sort();

          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(3u));

          AssertThat(dec.has_top(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(2u));
        });

        it("cleans taint flags on targets from outer roots [cardinality == 1]", [&]() {
          sorter.reset(); // ignore 'before_each' and do our own setup

          const test_request_t root1_taint(
            { flag(inner_n1) }, {}, { node::pointer_type(1, 0, false) });
          sorter.push(root1_taint);

          sorter.sort();

          test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
          test_decorator dec(pq, sorter);

          dec.setup_next_level();

          AssertThat(dec.has_top(), Is().True());
          const auto r1 = dec.pull();
          AssertThat(r1.target, Is().EqualTo(root1.target));
          AssertThat(r1.data.source, Is().EqualTo(flag(root1.data.source)));
        });

        it("cleans taint flags on targets from outer roots [cardinality == 2]",
           [&dag, &inner_n1, &inner_n2, &lpq_stats]() {
             using test_request_t = request_data<2, with_parent, 0, 1>;

             using test_roots_sorter_t =
               nested_sweeping::outer::roots_sorter<memory_mode::Internal,
                                                    test_request_t,
                                                    request_first_lt<test_request_t>>;

             using test_pq_t =
               levelized_node_priority_queue<test_request_t,
                                             request_first_lt<test_request_t>,
                                             1,
                                             memory_mode::Internal,
                                             1,
                                             0 /* <-- this is important for nested sweeping */>;

             using test_decorator =
               nested_sweeping::inner::down__pq_decorator<test_pq_t, test_roots_sorter_t>;

             test_roots_sorter_t sorter(1024, 16);

             const test_request_t req(
               { flag(inner_n1), flag(inner_n2) }, {}, { node::pointer_type(1, 0, false) });
             sorter.push(req);

             sorter.sort();

             test_pq_t pq({ bdd(dag) }, memory_available(), 16, lpq_stats);
             test_decorator dec(pq, sorter);

             dec.setup_next_level();

             AssertThat(dec.can_pull(), Is().True());
             const auto r1 = dec.pull();
             AssertThat(r1.target, Is().EqualTo(test_request_t::target_t(inner_n1, inner_n2)));
             AssertThat(r1.data.source, Is().EqualTo(flag(req.data.source)));
           });
      });

      describe("inner::up__pq_decorator", [&]() {
        // Inner PQ
        using inner_test_pq_t = nested_sweeping::inner::up__pq_t<1, memory_mode::Internal>;
        /*
        //
        //         3       ---- x2
        //        / \
        //        F 4      ---- x3
        //         / \
        //         F 5     ---- x4
        //          / \
        //          F T
        */

        shared_levelized_file<arc> inner_dag;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(inner_dag);

          aw.push_internal({ outer_n3, true, outer_n4 });
          aw.push_internal({ outer_n4, true, outer_n5 });

          aw.push_terminal({ outer_n3, false, terminal_F });
          aw.push_terminal({ outer_n4, false, terminal_F });
          aw.push_terminal({ outer_n5, false, terminal_F });
          aw.push_terminal({ outer_n5, true, terminal_T });

          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 1u));
          aw.push(level_info(4, 1u));
        }

        using test_decorator =
          nested_sweeping::inner::up__pq_decorator<inner_test_pq_t, outer_test_pq_t>;

        const size_t pq_mem = memory_available() / 2;

        it("forwards unflagged terminal to Inner PQ", [&]() {
          outer_test_pq_t outer_pq({ outer_dag }, pq_mem, 16);
          inner_test_pq_t inner_pq({ inner_dag }, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(outer_n4, false, terminal_F));

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(1u));
        });

        it("forwards unflagged internal node to Inner PQ", [&]() {
          outer_test_pq_t outer_pq({ outer_dag }, pq_mem, 16);
          inner_test_pq_t inner_pq({ inner_dag }, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(outer_n4, true, outer_n5));

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(1u));
        });

        it("forwards flagged terminal to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({ outer_dag }, pq_mem, 16);
          inner_test_pq_t inner_pq({ inner_dag }, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(flag(outer_n4.as_ptr(false)), terminal_F));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));
        });

        it("forwards flagged internal node to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({ outer_dag }, pq_mem, 16);
          inner_test_pq_t inner_pq({ inner_dag }, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          AssertThat(outer_pq.size(), Is().EqualTo(0u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));

          dec.push(arc(flag(outer_n4.as_ptr(true)), outer_n5));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          AssertThat(inner_pq.size(), Is().EqualTo(0u));
        });

        it("unflags source when handing over to Outer PQ", [&]() {
          outer_test_pq_t outer_pq({ outer_dag }, pq_mem, 16);
          inner_test_pq_t inner_pq({ inner_dag }, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);
          dec.push(arc(flag(outer_n4.as_ptr(true)), outer_n5));

          AssertThat(outer_pq.size(), Is().EqualTo(1u));
          outer_pq.setup_next_level();

          AssertThat(outer_pq.can_pull(), Is().True());
          AssertThat(outer_pq.pull(), Is().EqualTo(arc(outer_n4, true, outer_n5)));
        });

        it("sets up next level only based on Inner PQ", [&]() {
          outer_test_pq_t outer_pq({ outer_dag }, pq_mem, 16);
          inner_test_pq_t inner_pq({ inner_dag }, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(outer_n3.as_ptr(false), terminal_F));    // inner (x2)
          dec.push(arc(flag(outer_n4.as_ptr(true)), outer_n5)); // outer (x3)
          dec.push(arc(outer_n3.as_ptr(true), outer_n5));       // inner (x2)

          dec.setup_next_level();

          AssertThat(dec.has_current_level(), Is().True());
          AssertThat(dec.current_level(), Is().EqualTo(2u));
        });

        it("counts arcs from both Inner and Outer PQ", [&]() {
          outer_test_pq_t outer_pq({ outer_dag }, pq_mem, 16);
          inner_test_pq_t inner_pq({ inner_dag }, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(outer_n4.as_ptr(false), terminal_F));    // inner (x3)
          dec.push(arc(flag(outer_n4.as_ptr(true)), outer_n5)); // outer (x3)
          dec.push(arc(outer_n4.as_ptr(true), outer_n5));       // inner (x3)

          AssertThat(outer_pq.size(), Is().EqualTo(1u));

          AssertThat(dec.size(), Is().EqualTo(3u));
          AssertThat(dec.empty(), Is().False());

          dec.setup_next_level();

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.has_top(), Is().True());
          AssertThat(dec.top(), Is().EqualTo(arc(outer_n4, true, outer_n5)));
          AssertThat(dec.can_pull(), Is().True());
          AssertThat(dec.pull(), Is().EqualTo(arc(outer_n4, true, outer_n5)));

          AssertThat(dec.size(), Is().EqualTo(2u));
          AssertThat(dec.empty(), Is().False());

          AssertThat(dec.empty_level(), Is().False());
          AssertThat(dec.has_top(), Is().True());
          AssertThat(dec.top(), Is().EqualTo(arc(outer_n4, false, terminal_F)));
          AssertThat(dec.can_pull(), Is().True());
          AssertThat(dec.pull(), Is().EqualTo(arc(outer_n4, false, terminal_F)));

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.has_top(), Is().False());
          AssertThat(dec.can_pull(), Is().False());
          AssertThat(dec.size(), Is().EqualTo(1u));
          AssertThat(dec.empty(), Is().True());

          dec.push(arc(flag(outer_n3.as_ptr(false)), terminal_F)); // outer
          dec.push(arc(outer_n3.as_ptr(true), outer_n5));          // inner

          AssertThat(dec.empty_level(), Is().True());
          AssertThat(dec.size(), Is().EqualTo(3u));
          AssertThat(dec.empty(), Is().False());
        });

        it("empty() is independent of Outer PQ size", [&]() {
          outer_test_pq_t outer_pq({ outer_dag }, pq_mem, 16);
          inner_test_pq_t inner_pq({ inner_dag }, pq_mem, 16);

          test_decorator dec(inner_pq, outer_pq);

          dec.push(arc(flag(outer_n4.as_ptr(true)), outer_n5)); // outer (x3)

          AssertThat(dec.size(), Is().EqualTo(1u));
          AssertThat(dec.empty(), Is().True());
        });
      });

      describe("inner::up__arc_ifstream__decorator", [&terminal_F, &terminal_T]() {
        const arc::uid_type n0(0, 0);
        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);

        shared_levelized_file<arc> outer;
        { // Garbage collect writer to free write-lock
          arc_ofstream aw(outer);

          aw.push_internal({ n0, false, n1 });
          aw.push_terminal({ n0, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));

          outer->max_1level_cut = 1;
        }

        shared_levelized_file<arc> inner;
        { // Garbage collect writer to free write-lock
          arc_ofstream aw(inner);

          aw.push_internal({ n1, true, n2 });

          aw.push_terminal({ n1, false, terminal_T });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));

          inner->max_1level_cut = 1;
        }

        it("only reads from inner stream", [&]() {
          const arc_ifstream<> outer_ifstream(outer);
          arc_ifstream<> inner_ifstream(inner);

          nested_sweeping::inner::up__arc_ifstream__decorator dec(inner_ifstream, outer_ifstream);

          AssertThat(dec.can_pull_internal(), Is().True());
          AssertThat(dec.peek_internal(), Is().EqualTo(arc(n1, true, n2)));
          AssertThat(dec.pull_internal(), Is().EqualTo(arc(n1, true, n2)));
          AssertThat(dec.can_pull_internal(), Is().False());

          AssertThat(dec.can_pull_terminal(), Is().True());
          AssertThat(dec.peek_terminal(), Is().EqualTo(arc(n2, true, terminal_T)));
          AssertThat(dec.pull_terminal(), Is().EqualTo(arc(n2, true, terminal_T)));
          AssertThat(dec.can_pull_terminal(), Is().True());
          AssertThat(dec.peek_terminal(), Is().EqualTo(arc(n2, false, terminal_F)));
          AssertThat(dec.pull_terminal(), Is().EqualTo(arc(n2, false, terminal_F)));
          AssertThat(dec.can_pull_terminal(), Is().True());
          AssertThat(dec.peek_terminal(), Is().EqualTo(arc(n1, false, terminal_T)));
          AssertThat(dec.pull_terminal(), Is().EqualTo(arc(n1, false, terminal_T)));
          AssertThat(dec.can_pull_terminal(), Is().False());
        });

        it("sums unread_terminals from both streams", [&outer, &inner]() {
          const arc_ifstream<> outer_ifstream(outer);
          arc_ifstream<> inner_ifstream(inner);

          nested_sweeping::inner::up__arc_ifstream__decorator dec(inner_ifstream, outer_ifstream);

          AssertThat(dec.unread_terminals(), Is().EqualTo(4u));
          AssertThat(dec.unread_terminals(false), Is().EqualTo(2u));
          AssertThat(dec.unread_terminals(true), Is().EqualTo(2u));

          AssertThat(dec.can_pull_terminal(), Is().True());
          dec.pull_terminal();

          AssertThat(dec.unread_terminals(), Is().EqualTo(3u));
          AssertThat(dec.unread_terminals(false), Is().EqualTo(2u));
          AssertThat(dec.unread_terminals(true), Is().EqualTo(1u));

          AssertThat(dec.can_pull_terminal(), Is().True());
          dec.pull_terminal();

          AssertThat(dec.unread_terminals(), Is().EqualTo(2u));
          AssertThat(dec.unread_terminals(false), Is().EqualTo(1u));
          AssertThat(dec.unread_terminals(true), Is().EqualTo(1u));

          AssertThat(dec.can_pull_terminal(), Is().True());
          dec.pull_terminal();

          AssertThat(dec.unread_terminals(), Is().EqualTo(1u));
          AssertThat(dec.unread_terminals(false), Is().EqualTo(1u));
          AssertThat(dec.unread_terminals(true), Is().EqualTo(0u));
        });
      });
    });

    describe("nested_sweeping::aux algorithms", []() {
      describe("__reduce_level__fast(..., label, ...)", []() {
        using pq_t = reduce_priority_queue<1, memory_mode::Internal>;

        it("does not suppress an entire level of redundant nodes", []() {
          /*
          //      _1_     ---- x0
          //     /   \
          //     2   3    ---- x1 (being reduced)
          //     \\ //
          //       4      ---- x2
          //      / \
          //      F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);
          const arc::uid_type n4(2, 0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });
            aw.push_internal({ n2, false, n4 });
            aw.push_internal({ n2, true, n4 });
            aw.push_internal({ n3, false, n4 });
            aw.push_internal({ n3, true, n4 });

            aw.push_terminal({ n4, false, terminal_F });
            aw.push_terminal({ n4, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
            aw.push(level_info(2, 1u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Simulate reduction of x2
          arcs.pull_terminal(); // 4 ---> T
          arcs.pull_terminal(); // 4 - -> F

          out_ofstream.unsafe_push(node(2, node::max_id, terminal_F, terminal_T));
          out_ofstream.unsafe_push(level_info(2, 1));
          out_ofstream.unsafe_max_1level_cut({ 0, 1, 1, 2 });

          // 3 ---> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));
          // 3 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));
          // 2 ---> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));
          // 2 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));

          pq.setup_next_level(1);

          // Reduce level x1
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 1, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       node::pointer_type(2, node::max_id),
                                       node::pointer_type(2, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id - 1,
                                       node::pointer_type(2, node::max_id),
                                       node::pointer_type(2, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("outputs non-redundant nodes [bottom level]", []() {
          /*
          //      _1_     ---- x0
          //     /   \
          //     2   3    ---- x1 (being reduced)
          //    / \ / \
          //    T F F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });

            aw.push_terminal({ n2, false, terminal_T });
            aw.push_terminal({ n2, true, terminal_F });
            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n3, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Reduce level x1
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 1, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, node::max_id - 1, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("does not canonically sort output [bottom level]", []() {
          /*
          //      _1_     ---- x0
          //     /   \
          //     2   3    ---- x1 (being reduced)
          //    / \ / \
          //    F T T F
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });

            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n2, true, terminal_T });
            aw.push_terminal({ n3, false, terminal_T });
            aw.push_terminal({ n3, true, terminal_F });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Reduce level x1
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 1, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1, node::max_id - 1, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("does not canonically sort output [other levels]", []() {
          /*
          //          _1_        ---- x0
          //         /   \
          //         2   3       ---- x1 (being reduced)
          //        / \ / \
          //        | T | |
          //         \ /  |
          //          4   5      ---- x2
          //         / \ / \
          //         T F F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);
          const arc::uid_type n4(2, 0);
          const arc::uid_type n5(2, 1);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });
            aw.push_internal({ n2, false, n4 });
            aw.push_internal({ n3, false, n4 });
            aw.push_internal({ n3, true, n5 });

            aw.push_terminal({ n2, true, terminal_T });
            aw.push_terminal({ n4, false, terminal_T });
            aw.push_terminal({ n4, true, terminal_F });
            aw.push_terminal({ n5, false, terminal_F });
            aw.push_terminal({ n5, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
            aw.push(level_info(2, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Simulate reduction of x2
          arcs.pull_terminal(); // 5 ---> T
          arcs.pull_terminal(); // 5 - -> F
          out_ofstream.unsafe_push(node(2, node::max_id, terminal_F, terminal_T));

          arcs.pull_terminal(); // 4 ---> F
          arcs.pull_terminal(); // 4 - -> T
          out_ofstream.unsafe_push(node(2, node::max_id - 1, terminal_T, terminal_F));

          out_ofstream.unsafe_push(level_info(2, 2));
          out_ofstream.unsafe_max_1level_cut({ 0, 2, 3, 5 });

          // 3 ---> 5
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));
          // 3 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id - 1)));
          // 2 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id - 1)));

          pq.setup_next_level(1);

          // Reduce level x1
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 1, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, node::max_id - 1, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       node::pointer_type(2, node::max_id - 1),
                                       node::pointer_type(2, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       1, node::max_id - 1, node::pointer_type(2, node::max_id - 1), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("also includes unsuppressed contribution to 1-level cut [internal arc]", []() {
          /*
          //          _1_      ---- x0
          //         /   \
          //         2   3     ---- x1 (being reduced)
          //        / \ //
          //        | T ||
          //         \ //
          //          4       ---- x2
          //         / \
          //         F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);
          const arc::uid_type n4(2, 0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });
            aw.push_internal({ n2, false, n4 });
            aw.push_internal({ n3, false, n4 });
            aw.push_internal({ n3, true, n4 });

            aw.push_terminal({ n2, true, terminal_T });
            aw.push_terminal({ n4, false, terminal_T });
            aw.push_terminal({ n4, true, terminal_F });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
            aw.push(level_info(2, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Simulate reduction of x2
          arcs.pull_terminal(); // 4 ---> T
          arcs.pull_terminal(); // 4 - -> F
          out_ofstream.unsafe_push(node(2, node::max_id, terminal_F, terminal_T));

          out_ofstream.unsafe_push(level_info(2, 1));
          out_ofstream.unsafe_max_1level_cut({ 0, 1, 2, 2 });

          // 3 ---> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));
          // 3 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));
          // 2 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));

          pq.setup_next_level(1);

          // Reduce level x1
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 1, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       node::pointer_type(2, node::max_id),
                                       node::pointer_type(2, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, node::max_id - 1, node::pointer_type(2, node::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("also includes unsuppressed contribution to 1-level cut [terminal]", []() {
          /*
          //          _1_      ---- x0
          //         /   \
          //         2   3     ---- x1 (being reduced)
          //        / \ / \
          //        4 T F F    ---- x2
          //       / \
          //       F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);
          const arc::uid_type n4(2, 0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });
            aw.push_internal({ n2, false, n4 });

            aw.push_terminal({ n2, true, terminal_T });
            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n3, true, terminal_F });
            aw.push_terminal({ n4, false, terminal_T });
            aw.push_terminal({ n4, true, terminal_F });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
            aw.push(level_info(2, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Simulate reduction of x2
          arcs.pull_terminal(); // 4 ---> T
          arcs.pull_terminal(); // 4 - -> F
          out_ofstream.unsafe_push(node(2, node::max_id, terminal_F, terminal_T));

          out_ofstream.unsafe_push(level_info(2, 1));

          // HACK: do not set 1-level cut to see desired behaviour

          // 2 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));

          pq.setup_next_level(1);

          // Reduce level x1
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 1, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(1, node::max_id - 1, node::pointer_type(2, node::max_id), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("does not merge (non-adjacent) duplicate nodes", []() {
          /*
          //        _1_        ---- x0
          //       /   \
          //      _2_   \      ---- x1
          //     /   \   \
          //     3   4   5     ---- x2 (being reduced)
          //    / \ / \ / \
          //    F | F T F |
          //      \___ ___/    ---- x3
          //          6
          //         / \
          //         F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(2, 0);
          const arc::uid_type n4(2, 1);
          const arc::uid_type n5(2, 2);
          const arc::uid_type n6(3, 0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n2, false, n3 });
            aw.push_internal({ n2, true, n4 });
            aw.push_internal({ n1, true, n5 });
            aw.push_internal({ n3, true, n6 });
            aw.push_internal({ n5, true, n6 });

            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n4, false, terminal_F });
            aw.push_terminal({ n4, true, terminal_T });
            aw.push_terminal({ n5, false, terminal_F });
            aw.push_terminal({ n6, false, terminal_F });
            aw.push_terminal({ n6, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 3u));
            aw.push(level_info(3, 1u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Simulate reduction of x3
          arcs.pull_terminal(); // 6 ---> T
          arcs.pull_terminal(); // 6 - -> F
          out_ofstream.unsafe_push(node(3, node::max_id, terminal_F, terminal_T));

          out_ofstream.unsafe_push(level_info(3, 1));
          out_ofstream.unsafe_max_1level_cut({ 0, 1, 1, 2 });

          // 5 ---> 6
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(3, arc::max_id)));
          // 3 ---> 6
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(3, arc::max_id)));

          pq.setup_next_level(2);

          // Reduce level x1
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 2, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(3u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(
              2, node::max_id, node::pointer_type(false), node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       2, node::max_id - 1, node::pointer_type(false), node::pointer_type(true))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2,
                                       node::max_id - 2,
                                       node::pointer_type(false),
                                       node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("does not merge adjacent duplicate nodes", []() {
          /*
          //      _1_     ---- x0
          //     /   \
          //     2_ _3    ---- x1 (being reduced)
          //     |_X_|
          //     4   5    ---- x2
          //    / \ / \
          //    T F F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);
          const arc::uid_type n4(2, 0);
          const arc::uid_type n5(2, 1);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });
            aw.push_internal({ n2, false, n4 });
            aw.push_internal({ n3, false, n4 });
            aw.push_internal({ n2, true, n5 });
            aw.push_internal({ n3, true, n5 });

            aw.push_terminal({ n4, false, terminal_T });
            aw.push_terminal({ n4, true, terminal_F });
            aw.push_terminal({ n5, false, terminal_F });
            aw.push_terminal({ n5, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
            aw.push(level_info(2, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Simulate reduction of x2
          arcs.pull_terminal(); // 5 ---> T
          arcs.pull_terminal(); // 5 - -> F
          out_ofstream.unsafe_push(node(2, node::max_id, terminal_F, terminal_T));

          arcs.pull_terminal(); // 4 ---> F
          arcs.pull_terminal(); // 4 - -> T
          out_ofstream.unsafe_push(node(2, node::max_id - 1, terminal_T, terminal_F));

          out_ofstream.unsafe_push(level_info(2, 2));
          out_ofstream.unsafe_max_1level_cut({ 0, 2, 2, 4 });

          // 3 ---> 5
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));
          // 2 ---> 5
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id)));
          // 3 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id - 1)));
          // 2 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(2, arc::max_id - 1)));

          pq.setup_next_level(1);

          // Reduce level x1
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 1, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, node::max_id - 1, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       node::pointer_type(2, node::max_id - 1),
                                       node::pointer_type(2, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id - 1,
                                       node::pointer_type(2, node::max_id - 1),
                                       node::pointer_type(2, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("does not merge adjacent duplicate nodes (with a suppressed one in-between)", []() {
          /* input
          //        _1_         ---- x0
          //       /   \
          //       2    \       ---- x1
          //      / \    \
          //     3   4   5      ---- x2 (being reduced)
          //    / \  || / \
          //    F T  F  F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(2, 0);
          const arc::uid_type n4(2, 1);
          const arc::uid_type n5(2, 2);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n2, false, n3 });
            aw.push_internal({ n2, true, n5 });
            aw.push_internal({ n1, true, n5 });

            aw.push_terminal({ n3, false, terminal_F });
            aw.push_terminal({ n3, true, terminal_T });
            aw.push_terminal({ n4, false, terminal_F });
            aw.push_terminal({ n4, true, terminal_F });
            aw.push_terminal({ n5, false, terminal_F });
            aw.push_terminal({ n5, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 3u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          out_ofstream.unsafe_max_1level_cut({ 0, 0, 0, 0 });

          pq.setup_next_level(2);

          // Reduce level x2
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 2, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(3u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(4u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, node::max_id - 1, terminal_F, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, node::max_id - 2, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("does not collapse to a terminal [single level]", []() {
          /*
          //      1     ---- x0  (being reduced)
          //     / \
          //     T T
          */
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_terminal({ n1, false, terminal_T });
            aw.push_terminal({ n1, true, terminal_T });

            aw.push(level_info(0, 1u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Reduce level x0
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 0, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().True());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("does not collapse to a terminal [multiple levels]", []() {
          /*
          //      _1_      ---- x0 (being reduced)
          //     /   \
          //     2   3     ---- x1
          //     ||  ||
          //     T   T
          */
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });

            aw.push_terminal({ n2, false, terminal_T });
            aw.push_terminal({ n2, true, terminal_T });
            aw.push_terminal({ n3, false, terminal_T });
            aw.push_terminal({ n3, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Simulate reduction of x1
          arcs.pull_terminal(); // 3 ---> T
          arcs.pull_terminal(); // 3 - -> F

          arcs.pull_terminal(); // 2 ---> F
          arcs.pull_terminal(); // 2 - -> T

          // 1 ---> 3 (T)
          pq.push(arc(arcs.pull_internal().source(), terminal_T));
          // 1 - -> 2 (T)
          pq.push(arc(arcs.pull_internal().source(), terminal_T));

          pq.setup_next_level(0);

          // Reduce level x0
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 0, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->sorted, Is().True());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().True());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(0, node::max_id, terminal_T, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });
      });

      describe("__reduce_level__fast(..., in_label, out_label, ...)", []() {
        using pq_t = reduce_priority_queue<1, memory_mode::Internal>;

        it("does not canonically sort output [bottom level]", []() {
          /*
          //      _1_     ---- x0
          //     /   \
          //     2   3    ---- x1 (being reduced and mapped to x2)
          //    / \ / \
          //    F T T F
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(1, 0);
          const arc::uid_type n3(1, 1);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });

            aw.push_terminal({ n2, false, terminal_F });
            aw.push_terminal({ n2, true, terminal_T });
            aw.push_terminal({ n3, false, terminal_T });
            aw.push_terminal({ n3, true, terminal_F });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Reduce level x1 (mapping it to x2)
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 1, 2, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(2, node::max_id - 1, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });

        it("does not canonically sort output [other levels]", []() {
          /*
          //          _1_        ---- x0
          //         /   \
          //         2   3       ---- x2 (being reduced and mapped to x1)
          //        / \ / \
          //        | T | |
          //         \ /  |
          //          4   5      ---- x3
          //         / \ / \
          //         T F F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(0, 0);
          const arc::uid_type n2(2, 0);
          const arc::uid_type n3(2, 1);
          const arc::uid_type n4(3, 0);
          const arc::uid_type n5(3, 1);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });
            aw.push_internal({ n2, false, n4 });
            aw.push_internal({ n3, false, n4 });
            aw.push_internal({ n3, true, n5 });

            aw.push_terminal({ n2, true, terminal_T });
            aw.push_terminal({ n4, false, terminal_T });
            aw.push_terminal({ n4, true, terminal_F });
            aw.push_terminal({ n5, false, terminal_F });
            aw.push_terminal({ n5, true, terminal_T });

            aw.push(level_info(0, 1u));
            aw.push(level_info(2, 2u));
            aw.push(level_info(3, 2u));
          }

          // Outer state
          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();

          node_ofstream out_ofstream(out);

          arc_ifstream arcs(in);

          pq_t pq({ in }, memory_available(), 16);

          // Simulate reduction of x2
          arcs.pull_terminal(); // 5 ---> T
          arcs.pull_terminal(); // 5 - -> F
          out_ofstream.unsafe_push(node(3, node::max_id, terminal_F, terminal_T));

          arcs.pull_terminal(); // 4 ---> F
          arcs.pull_terminal(); // 4 - -> T
          out_ofstream.unsafe_push(node(3, node::max_id - 1, terminal_T, terminal_F));

          out_ofstream.unsafe_push(level_info(3, 2));
          out_ofstream.unsafe_max_1level_cut({ 0, 2, 3, 5 });

          // 3 ---> 5
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(3, arc::max_id)));
          // 3 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(3, arc::max_id - 1)));
          // 2 - -> 4
          pq.push(arc(arcs.pull_internal().source(), node::pointer_type(3, arc::max_id - 1)));

          pq.setup_next_level(1);

          // Reduce level x2 (mapping it to x1)
          nested_sweeping::__reduce_level__fast<bdd_policy>(arcs, 2, 1, pq, out_ofstream);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3, node::max_id - 1, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       node::pointer_type(3, node::max_id - 1),
                                       node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(
                       1, node::max_id - 1, node::pointer_type(3, node::max_id - 1), terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(out_meta.can_pull(), Is().False());
        });
      });
    });

    describe("nested_sweeping:: _ ::sweeps", [&terminal_F, &terminal_T]() {
      describe("inner::down<...>(...)", [&]() {
        using inner_down_sweep = test_not_sweep<>;
        using inner_roots_t =
          nested_sweeping::outer::roots_sorter<memory_mode::Internal,
                                               inner_down_sweep::request_t,
                                               request_first_lt<inner_down_sweep::request_t>>;

        /*
        //       ?   ?         ---- x0
        // -   -   -   -   -
        //       2   3   4     ---- x1
        //      / \ / \ / \
        //      F T F  5  T    ---- x2
        //            / \
        //            F T
        */

        const node n5 = node(2, 0, terminal_F, terminal_T);
        const node n4 = node(1, 2, n5.uid(), terminal_F);
        const node n3 = node(1, 1, terminal_F, n5.uid());
        const node n2 = node(1, 0, terminal_F, terminal_T);

        shared_levelized_file<node> outer_file;
        { // Garbage collect writer to free write-lock
          node_ofstream nw(outer_file);
          nw << n5 << n4 << n3 << n2;
        }

        it("can be run from a single root", [&]() {
          inner_down_sweep test_policy(2);

          /*
          //         _1_         ---- x0
          // -   -  / - \   -
          //        2   3   4    ---- x1
          */
          inner_roots_t inner_roots(1024, 8);
          const node::uid_type u1 = node::uid_type(0, 0);

          inner_roots.push({ { n2.uid() }, {}, { u1.as_ptr(false) } });
          inner_roots.push({ { n3.uid() }, {}, { u1.as_ptr(true) } });

          const shared_levelized_file<arc> out =
            nested_sweeping::inner::down(
              exec_policy(), test_policy, outer_file, inner_roots, memory_available())
              .template get<shared_levelized_file<arc>>();

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ flag(u1.as_ptr(false)), n2.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ flag(u1.as_ptr(true)), n3.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n3.uid(), true, n5.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5.uid(), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out->max_1level_cut, Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));

          // Side effects on `inner_roots`.
          AssertThat(inner_roots.size(), Is().EqualTo(0u));
          AssertThat(inner_roots.can_push(), Is().True());
        });

        it("can be run from multiple roots", [&]() {
          inner_down_sweep test_policy(2);

          /*
          //       0   1        ---- x0
          // -   -/_\_/-\   -
          //      2 3   |       ---- x1
          //            |
          //            5       ---- x2
          //           / \
          //           F T
          */

          inner_roots_t inner_roots(1024, 8);
          const node::uid_type u0 = node::uid_type(0, 0);
          const node::uid_type u1 = node::uid_type(0, 1);

          inner_roots.push({ { n2.uid() }, {}, { u0.as_ptr(false) } });
          inner_roots.push({ { n3.uid() }, {}, { u0.as_ptr(true) } });

          inner_roots.push({ { n2.uid() }, {}, { u1.as_ptr(false) } });
          inner_roots.push({ { n5.uid() }, {}, { u1.as_ptr(true) } });

          const shared_levelized_file<arc> out =
            nested_sweeping::inner::down(
              exec_policy(), test_policy, outer_file, inner_roots, memory_available())
              .template get<shared_levelized_file<arc>>();

          arc_test_ifstream arcs(out);

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ flag(u0.as_ptr(false)), n2.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ flag(u1.as_ptr(false)), n2.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ flag(u0.as_ptr(true)), n3.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True()); // From sorter
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ flag(u1.as_ptr(true)), n5.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().True());
          AssertThat(arcs.pull_internal(), Is().EqualTo(arc{ n3.uid(), true, n5.uid() }));

          AssertThat(arcs.can_pull_internal(), Is().False());

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n2.uid(), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n3.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5.uid(), false, terminal_T }));

          AssertThat(arcs.can_pull_terminal(), Is().True());
          AssertThat(arcs.pull_terminal(), Is().EqualTo(arc{ n5.uid(), true, terminal_F }));

          AssertThat(arcs.can_pull_terminal(), Is().False());

          level_info_test_ifstream levels(out);

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(1, 2u)));

          AssertThat(levels.can_pull(), Is().True());
          AssertThat(levels.pull(), Is().EqualTo(level_info(2, 1u)));

          AssertThat(levels.can_pull(), Is().False());

          AssertThat(out->max_1level_cut, Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));

          // Side effects on `inner_roots`.
          AssertThat(inner_roots.size(), Is().EqualTo(0u));
          AssertThat(inner_roots.can_push(), Is().True());
        });

        it("can collapse to a terminal", []() {
          test_terminal_sweep test_policy(2);

          /*
          //  nil
          //   |
          //   1     ---- x1
          //  / \
          //  F T
          */
          const node n1(1, node::max_id, node::pointer_type(false), node::pointer_type(true));

          shared_levelized_file<node> outer_file;
          { // Garbage collect writer to free write-lock
            node_ofstream nw(outer_file);
            nw << n1;
          }

          inner_roots_t inner_roots(1024, 8);

          inner_roots.push({ { n1.uid() }, {}, { node::pointer_type::nil() } });

          const shared_levelized_file<node> out =
            nested_sweeping::inner::down(
              exec_policy(), test_policy, outer_file, inner_roots, memory_available())
              .template get<shared_levelized_file<node>>();

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("inner::up<...>(...)", []() {
        using outer_pq_t = nested_sweeping::outer::up__pq_t<1, memory_mode::Internal>;

        {
          /* input for the next two unit tests
          //
          //         _0__              ---- x0
          //        /    \
          //       1     _2_           ---- x1
          //  -   - \ - / - \  -   -   -    -
          //        3   4   5          ---- x2
          //       / \ / \ / \
          //       T | F T T |
          //         \___ ___/
          //             6             ---- x3
          //            / \
          //            T F
          */

          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n0(0, 0);
          const arc::uid_type n1(1, 0);
          const arc::uid_type n2(1, 1);
          const arc::uid_type n3(2, 0);
          const arc::uid_type n4(2, 1);
          const arc::uid_type n5(2, 2);
          const arc::uid_type n6(3, 0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_outer);

            aw.push_internal({ n0, false, n1 });
            aw.push_internal({ n0, true, n2 });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
            aw.push(level_info(2, 3u));

            in_outer->max_1level_cut = 3;
          }
          arc_ifstream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_inner);

            aw.push_internal({ flag(n1.as_ptr(true)), n3 });
            aw.push_internal({ flag(n2.as_ptr(false)), n4 });
            aw.push_internal({ flag(n2.as_ptr(true)), n5 });
            aw.push_internal({ n3.as_ptr(true), n6 });
            aw.push_internal({ n5.as_ptr(true), n6 });

            aw.push_terminal({ n3.as_ptr(false), terminal_T });
            aw.push_terminal({ n4.as_ptr(false), terminal_F });
            aw.push_terminal({ n4.as_ptr(true), terminal_T });
            aw.push_terminal({ n5.as_ptr(false), terminal_T });
            aw.push_terminal({ n6.as_ptr(false), terminal_T });
            aw.push_terminal({ n6.as_ptr(true), terminal_F });

            // NOTE: 'level_info(0,1u)' is not a processable part of the forest;
            // NOTE: 'level_info(1,2u)' is not a processable part of the forest;
            aw.push(level_info(2, 3u));
            aw.push(level_info(3, 1u));

            in_inner->max_1level_cut = 2;
          }

          it("reduces forest and pushes roots back out", [&]() {
            using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ false>;

            shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
            node_ofstream out_ofstream(out);

            const size_t available_memory = memory_available();

            outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
            out_pq.setup_next_level(2);

            /* output
            //      1   2           ---- x1
            //  -   -\ _X_  -   -   -    -
            //        3   4         ---- x2
            //       / \ / \
            //       T 6 F T        ---- x3
            //        / \
            //        T F
            */
            nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                    stream_outer,
                                                    out_pq,
                                                    out_ofstream,
                                                    in_inner,
                                                    available_memory / 2,
                                                    false);

            // Check meta variables before detach computations
            AssertThat(out->width, Is().EqualTo(2u));

            AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
            AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));

            // Over-approximation, since T-terminal from level (2) is removed
            AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
            AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
            AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
            AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(6u));

            AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
            AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));

            // Check node and meta files are correct
            out_ofstream.close();

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // 6
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(3, node::max_id, terminal_T, terminal_F)));

            AssertThat(out_nodes.can_pull(), Is().True()); // 5
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

            AssertThat(out_nodes.can_pull(), Is().True()); // 3
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         2, node::max_id - 1, terminal_T, node::pointer_type(3, node::max_id))));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

            AssertThat(out_meta.can_pull(), Is().False());

            // Check outer priority queue is correct
            AssertThat(out_pq.size(), Is().EqualTo(3u));

            out_pq.setup_next_level();

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n2, true, arc::pointer_type(2, arc::max_id - 1))));

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n2, false, arc::pointer_type(2, arc::max_id))));

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n1, true, arc::pointer_type(2, arc::max_id - 1))));

            AssertThat(out_pq.can_pull(), Is().False());
          });

          it("uses fast reduce with non-negative value [non-adjacent duplicates]", [&]() {
            using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ true>;

            shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
            node_ofstream out_ofstream(out);

            const size_t available_memory = memory_available();

            outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
            out_pq.setup_next_level(2);

            /* output
            //       1     _2_           ---- x1
            //  -   - \ - / - \  -   -   -    -
            //        3   4   5          ---- x2
            //       / \ / \ / \
            //       T | F T T |
            //         \___ ___/
            //             6             ---- x3
            //            / \
            //            T F
            */
            nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                    stream_outer,
                                                    out_pq,
                                                    out_ofstream,
                                                    in_inner,
                                                    available_memory / 2,
                                                    false);

            // Check meta variables before detach computations
            AssertThat(out->width, Is().EqualTo(3u));

            AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
            AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
            AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(5u));
            AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

            AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
            AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));

            // Check node and meta files are correct
            out_ofstream.close();

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // 6
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(3, node::max_id, terminal_T, terminal_F)));

            AssertThat(out_nodes.can_pull(), Is().True()); // 5
            AssertThat(
              out_nodes.pull(),
              Is().EqualTo(node(2, node::max_id, terminal_T, node::pointer_type(3, node::max_id))));

            AssertThat(out_nodes.can_pull(), Is().True()); // 4
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, node::max_id - 1, terminal_F, terminal_T)));

            AssertThat(out_nodes.can_pull(), Is().True()); // 3
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         2, node::max_id - 2, terminal_T, node::pointer_type(3, node::max_id))));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 3u)));

            AssertThat(out_meta.can_pull(), Is().False());

            // Check outer priority queue is correct
            AssertThat(out_pq.size(), Is().EqualTo(3u));

            out_pq.setup_next_level();

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n2, true, arc::pointer_type(2, arc::max_id))));

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n2, false, arc::pointer_type(2, arc::max_id - 1))));

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n1, true, arc::pointer_type(2, arc::max_id - 2))));

            AssertThat(out_pq.can_pull(), Is().False());
          });

          it("reduces forest canonically anyway if 'is_last_inner == true'", [&]() {
            using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ true>;

            shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
            node_ofstream out_ofstream(out);

            const size_t available_memory = memory_available();

            outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
            out_pq.setup_next_level(2);

            /* output
            //      1   2           ---- x1
            //  -   -\ _X_  -   -   -    -
            //        3   4         ---- x2
            //       / \ / \
            //       T 6 F T        ---- x3
            //        / \
            //        T F
            */
            nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                    stream_outer,
                                                    out_pq,
                                                    out_ofstream,
                                                    in_inner,
                                                    available_memory / 2,
                                                    true);

            // Check meta variables before detach computations
            AssertThat(out->width, Is().EqualTo(2u));

            AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
            AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));

            // Over-approximation, since T-terminal from level (2) is removed
            AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
            AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
            AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
            AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(6u));

            AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
            AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));

            // Check node and meta files are correct
            out_ofstream.close();

            node_test_ifstream out_nodes(out);

            AssertThat(out_nodes.can_pull(), Is().True()); // 6
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(3, node::max_id, terminal_T, terminal_F)));

            AssertThat(out_nodes.can_pull(), Is().True()); // 5
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

            AssertThat(out_nodes.can_pull(), Is().True()); // 3
            AssertThat(out_nodes.pull(),
                       Is().EqualTo(node(
                         2, node::max_id - 1, terminal_T, node::pointer_type(3, node::max_id))));

            AssertThat(out_nodes.can_pull(), Is().False());

            level_info_test_ifstream out_meta(out);

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

            AssertThat(out_meta.can_pull(), Is().True());
            AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 2u)));

            AssertThat(out_meta.can_pull(), Is().False());

            // Check outer priority queue is correct
            AssertThat(out_pq.size(), Is().EqualTo(3u));

            out_pq.setup_next_level();

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n2, true, arc::pointer_type(2, arc::max_id - 1))));

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n2, false, arc::pointer_type(2, arc::max_id))));

            AssertThat(out_pq.can_pull(), Is().True());
            AssertThat(out_pq.pull(),
                       Is().EqualTo(arc(n1, true, arc::pointer_type(2, arc::max_id - 1))));

            AssertThat(out_pq.can_pull(), Is().False());
          });

          it("reduces forest non-canonically despite 'is_last_inner == true' [non-adjacent "
             "duplicates]",
             [&]() {
               using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ true>;

               shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
               node_ofstream out_ofstream(out);

               const size_t available_memory = memory_available();

               outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
               out_pq.setup_next_level(2);

               /* output
               //       1     _2_           ---- x1
               //  -   - \ - / - \  -   -   -    -
               //        3   4   5          ---- x2
               //       / \ / \ / \
               //       T | F T T |
               //         \___ ___/
               //             6             ---- x3
               //            / \
               //            T F
               */
               nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                       stream_outer,
                                                       out_pq,
                                                       out_ofstream,
                                                       in_inner,
                                                       available_memory / 2,
                                                       false);

               // Check meta variables before detach computations
               AssertThat(out->width, Is().EqualTo(3u));

               AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
               AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
               AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(5u));
               AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

               AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
               AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));

               // Check node and meta files are correct
               out_ofstream.close();

               node_test_ifstream out_nodes(out);

               AssertThat(out_nodes.can_pull(), Is().True()); // 6
               AssertThat(out_nodes.pull(),
                          Is().EqualTo(node(3, node::max_id, terminal_T, terminal_F)));

               AssertThat(out_nodes.can_pull(), Is().True()); // 5
               AssertThat(out_nodes.pull(),
                          Is().EqualTo(node(
                            2, node::max_id, terminal_T, node::pointer_type(3, node::max_id))));

               AssertThat(out_nodes.can_pull(), Is().True()); // 4
               AssertThat(out_nodes.pull(),
                          Is().EqualTo(node(2, node::max_id - 1, terminal_F, terminal_T)));

               AssertThat(out_nodes.can_pull(), Is().True()); // 3
               AssertThat(out_nodes.pull(),
                          Is().EqualTo(node(
                            2, node::max_id - 2, terminal_T, node::pointer_type(3, node::max_id))));

               AssertThat(out_nodes.can_pull(), Is().False());

               level_info_test_ifstream out_meta(out);

               AssertThat(out_meta.can_pull(), Is().True());
               AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

               AssertThat(out_meta.can_pull(), Is().True());
               AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 3u)));

               AssertThat(out_meta.can_pull(), Is().False());

               // Check outer priority queue is correct
               AssertThat(out_pq.size(), Is().EqualTo(3u));

               out_pq.setup_next_level();

               AssertThat(out_pq.can_pull(), Is().True());
               AssertThat(out_pq.pull(),
                          Is().EqualTo(arc(n2, true, arc::pointer_type(2, arc::max_id))));

               AssertThat(out_pq.can_pull(), Is().True());
               AssertThat(out_pq.pull(),
                          Is().EqualTo(arc(n2, false, arc::pointer_type(2, arc::max_id - 1))));

               AssertThat(out_pq.can_pull(), Is().True());
               AssertThat(out_pq.pull(),
                          Is().EqualTo(arc(n1, true, arc::pointer_type(2, arc::max_id - 2))));

               AssertThat(out_pq.can_pull(), Is().False());
             });
        }

        it("uses fast reduce with non-negative value [adjacent duplicates]", []() {
          using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ true>;

          /* input (same as above but with nodes 4 and 5 swapped)
          //
          //         _0__              ---- x0
          //        /    \
          //       1     _2_           ---- x1
          //  -   - \ - / - \  -   -   -    -
          //        3   4   5          ---- x2
          //       / \ / \ / \
          //       T | T | F T
          //         \_ _/
          //           6               ---- x3
          //          / \
          //          T F
          */

          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n0(0, 0);
          const arc::uid_type n1(1, 0);
          const arc::uid_type n2(1, 1);
          const arc::uid_type n3(2, 0);
          const arc::uid_type n4(2, 1);
          const arc::uid_type n5(2, 2);
          const arc::uid_type n6(3, 0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_outer);

            aw.push_internal({ n0, false, n1 });
            aw.push_internal({ n0, true, n2 });

            aw.push(level_info(0, 1u));
            aw.push(level_info(1, 2u));
            aw.push(level_info(2, 3u));

            in_outer->max_1level_cut = 3;
          }
          arc_ifstream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_inner);

            aw.push_internal({ flag(n1.as_ptr(true)), n3 });
            aw.push_internal({ flag(n2.as_ptr(false)), n4 });
            aw.push_internal({ flag(n2.as_ptr(true)), n5 });
            aw.push_internal({ n3.as_ptr(true), n6 });
            aw.push_internal({ n4.as_ptr(true), n6 });

            aw.push_terminal({ n3.as_ptr(false), terminal_T });
            aw.push_terminal({ n4.as_ptr(false), terminal_T });
            aw.push_terminal({ n5.as_ptr(false), terminal_F });
            aw.push_terminal({ n5.as_ptr(true), terminal_T });
            aw.push_terminal({ n6.as_ptr(false), terminal_T });
            aw.push_terminal({ n6.as_ptr(true), terminal_F });

            // NOTE: 'level_info(0,1u)' is not a processable part of the forest;
            // NOTE: 'level_info(1,2u)' is not a processable part of the forest;
            aw.push(level_info(2, 3u));
            aw.push(level_info(3, 1u));

            in_inner->max_1level_cut = 2;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_ofstream out_ofstream(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2);

          /* output
          //       1     _2_           ---- x1
          //  -   - \ - / - \  -   -   -    -
          //        3   4   5          ---- x2
          //       / \ / \ / \
          //       T | T | F T
          //         \_ _/
          //           6               ---- x3
          //          / \
          //          T F
          */
          nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                  stream_outer,
                                                  out_pq,
                                                  out_ofstream,
                                                  in_inner,
                                                  available_memory / 2,
                                                  false);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(3u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(4u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True()); // 6
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_T, terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().True()); // 5
          AssertThat(out_nodes.pull(), Is().EqualTo(node(2, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().True()); // 4
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(2, node::max_id - 1, terminal_T, node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().True()); // 3
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(2, node::max_id - 2, terminal_T, node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 3u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(), Is().EqualTo(3u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n2, true, arc::pointer_type(2, arc::max_id))));

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(),
                     Is().EqualTo(arc(n2, false, arc::pointer_type(2, arc::max_id - 1))));

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(),
                     Is().EqualTo(arc(n1, true, arc::pointer_type(2, arc::max_id - 2))));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("includes outer_pq terminals in cut size", []() {
          using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ false>;

          /* input
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    |   2           ---- x3
          //    |  / \
          //    F  F T
           */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(1, 0);
          const arc::uid_type n2(3, 0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_outer);

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 1u));
            aw.push(level_info(3, 1u));

            in_outer->max_1level_cut = 2;
          }
          arc_ifstream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_inner);

            aw.push_internal({ flag(n1.as_ptr(true)), n2 });

            aw.push_terminal({ n2.as_ptr(false), terminal_F });
            aw.push_terminal({ n2.as_ptr(true), terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3, 1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_ofstream out_ofstream(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
          out_pq.push({ n1, false, terminal_F });

          out_pq.setup_next_level(2u);

          /* output
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    |   2           ---- x3
          //    |  / \
          //    F  F T
          */
          nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                  stream_outer,
                                                  out_pq,
                                                  out_ofstream,
                                                  in_inner,
                                                  available_memory / 2,
                                                  false);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(), Is().EqualTo(2u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, arc::pointer_type(3, arc::max_id))));

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, false, terminal_F)));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("includes outer_arcs terminals in cut size", []() {
          using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ false>;

          /* input
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    F   2           ---- x3
          //       / \
          //       F T
           */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(1, 0);
          const arc::uid_type n2(3, 0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_outer);

            aw.push_terminal({ n1, false, terminal_F });

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 1u));

            in_outer->max_1level_cut = 1;
          }
          arc_ifstream<> stream_outer(in_outer);
          AssertThat(stream_outer.unread_terminals(false), Is().EqualTo(1u));
          AssertThat(stream_outer.unread_terminals(true), Is().EqualTo(0u));

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_inner);

            aw.push_internal({ flag(n1.as_ptr(true)), n2 });

            aw.push_terminal({ n2.as_ptr(false), terminal_F });
            aw.push_terminal({ n2.as_ptr(true), terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            // NOTE: 'level_info(2,?u)' is not a processable part of the forest;
            aw.push(level_info(3, 1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_ofstream out_ofstream(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2u);

          /* output
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    F   2           ---- x3
          //       / \
          //       F T
          */
          nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                  stream_outer,
                                                  out_pq,
                                                  out_ofstream,
                                                  in_inner,
                                                  available_memory / 2,
                                                  false);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(), Is().EqualTo(1u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, arc::pointer_type(3, arc::max_id))));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("includes account for tainted arcs of 1-level cut", []() {
          using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ false>;

          /* input
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        2           ---- x3
          //       / \
          //       F 3          ---- x4
          //        / \
          //        T T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(1, 0);
          const arc::uid_type n2(3, 0);
          const arc::uid_type n3(4, 0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_outer);

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 1u));

            in_outer->max_1level_cut = 1;
          }
          arc_ifstream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_inner);

            aw.push_internal({ flag(n1.as_ptr(true)), n2 });
            aw.push_internal({ n2.as_ptr(true), n3 });

            aw.push_terminal({ n2.as_ptr(false), terminal_F });
            aw.push_terminal({ n3.as_ptr(false), terminal_T });
            aw.push_terminal({ n3.as_ptr(true), terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3, 1u));
            aw.push(level_info(4, 1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_ofstream out_ofstream(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2);

          /* output
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        2           ---- x3
          //       / \
          //       F T <-- T tainted in global cut
          */
          nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                  stream_outer,
                                                  out_pq,
                                                  out_ofstream,
                                                  in_inner,
                                                  available_memory / 2,
                                                  false);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(2u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(), Is().EqualTo(1u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True());
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, arc::pointer_type(3, arc::max_id))));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("leaves file empty if everything collapsed to a terminal", []() {
          using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ false>;

          /* input
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        2           ---- x3
          //       / \
          //       T T
          */
          const uid_uint64 terminal_F(false);
          const uid_uint64 terminal_T(true);

          const uid_uint64 n1(1, 0);
          const uid_uint64 n2(3, 0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_outer);

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 1u));

            in_outer->max_1level_cut = 1;
          }
          arc_ifstream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_inner);

            aw.push_internal({ flag(n1.as_ptr(true)), n2 });

            aw.push_terminal({ n2.as_ptr(false), terminal_T });
            aw.push_terminal({ n2.as_ptr(true), terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3, 1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_ofstream out_ofstream(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2);

          /* output
          //      1_            ---- x1
          //  -   - \ -   -   -    -
          //        T <-- in 'out_pq', not in 'out'
          */
          nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                  stream_outer,
                                                  out_pq,
                                                  out_ofstream,
                                                  in_inner,
                                                  available_memory / 2,
                                                  false);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(0u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(0u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(), Is().EqualTo(1u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True()); // Edge is tainted by reduction rule 1
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, true, flag(terminal_T))));

          AssertThat(out_pq.can_pull(), Is().False());
        });

        it("can deal with some root arcs go to a terminal", []() {
          using test_policy = test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ false>;

          /* input
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    |   2           ---- x3
          //    |  / \
          //    F  F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(1, 0);
          const arc::uid_type n2(3, 0);

          shared_levelized_file<arc> in_outer;
          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_outer);

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 1u));

            in_outer->max_1level_cut = 3;
          }
          arc_ifstream<> stream_outer(in_outer);

          shared_levelized_file<arc> in_inner;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in_inner);

            aw.push_internal({ flag(n1.as_ptr(true)), n2 });

            aw.push_terminal({ flag(n1.as_ptr(false)), terminal_F });
            aw.push_terminal({ n2.as_ptr(false), terminal_F });
            aw.push_terminal({ n2.as_ptr(true), terminal_T });

            // NOTE: 'level_info(1,1u)' is not a processable part of the forest;
            aw.push(level_info(3, 1u));

            in_inner->max_1level_cut = 1;
          }

          shared_levelized_file<node> out = __reduce_init_output<bdd_policy>();
          node_ofstream out_ofstream(out);

          const size_t available_memory = memory_available();

          outer_pq_t out_pq({ in_outer }, available_memory / 2, in_outer->max_1level_cut);
          out_pq.setup_next_level(2);

          /* output
          //     _1_            ---- x1
          //  - / - \ -   -   -    -
          //    |   2           ---- x3
          //    |  / \
          //    F  F T
          */
          nested_sweeping::inner::up<test_policy>(exec_policy(),
                                                  stream_outer,
                                                  out_pq,
                                                  out_ofstream,
                                                  in_inner,
                                                  available_memory / 2,
                                                  false);

          // Check meta variables before detach computations
          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));

          // Check node and meta files are correct
          out_ofstream.close();

          node_test_ifstream out_nodes(out);

          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          // Check outer priority queue is correct
          AssertThat(out_pq.size(), Is().EqualTo(2u));

          out_pq.setup_next_level();

          AssertThat(out_pq.can_pull(), Is().True()); // Edge is tainted by reduction rule 1
          AssertThat(out_pq.pull(),
                     Is().EqualTo(arc(n1, true, node::pointer_type(3, arc::max_id))));
          AssertThat(out_pq.can_pull(), Is().True()); // Edge is tainted by reduction rule 1
          AssertThat(out_pq.pull(), Is().EqualTo(arc(n1, false, terminal_F)));

          AssertThat(out_pq.can_pull(), Is().False());
        });
      });
    });

    describe("nested_sweep<...>(...)", []() {
      it("returns input if it is a terminal", []() {
        /* input
        //    T
        */
        shared_levelized_file<node> in;
        {
          node_ofstream nw(in);
          nw << node(true);
        }

        test_not_sweep<> inner_impl(2);

        const bdd out = nested_sweep<>(exec_policy(), in, inner_impl);

        AssertThat(out.file_ptr(), Is().EqualTo(in));
      });

      it("accumulates a single nested sweep [non-root level] [1]", []() {
        /* input
        //     _1_          ---- x1
        //    /   \
        //    T  _2_         ---- x2 (%2)
        //      /   \
        //      3   4       ---- x3
        //     / \ / \
        //     T T F T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(3, 0);
        const arc::uid_type n4(3, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n2, true, n4 });

          aw.push_terminal({ n1, false, terminal_T });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 2u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     1      ---- x1
        //    / \
        //    T |     ---- x2 (%2)
        //      |
        //      4     ---- x3
        //     / \
        //     T F
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_T, terminal_F)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, terminal_T, node::pointer_type(3, node::max_id))));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("accumulates a single nested sweep [non-root level] [2]", []() {
        /* input
        //     _1_          ---- x1
        //    /   \
        //    2   3         ---- x2 (%2)
        //   / \ / \
        //   F  4   5       ---- x3
        //     / \ / \
        //     F T T T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(2, 1);
        const arc::uid_type n4(3, 0);
        const arc::uid_type n5(3, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true, n5 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_T });
          aw.push_terminal({ n5, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 2u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //       1     ---- x1
        //      / \
        //      | T    ---- x2 (%2)
        //      |
        //      4      ---- x3
        //     / \
        //     T F
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_T, terminal_F)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(3, node::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));

        // NOTE: Since the outer priority queue includes a tainted arc to T, it
        //       is in fact counted twice: first it is counted in the inner
        //       sweep to then be counted once more in the outer
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("accumulates a single nested sweep [root level]", []() {
        /* input
        //          1       ---- x0 (%2)
        //         / \
        //         F 2      ---- x1
        //          / \
        //          F T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //             ---- x0
        //
        //     2       ---- x1
        //    / \
        //    T F
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n2
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

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

      it("accumulates multiple nested sweeps [excl. root]", []() {
        /* input
        //     _1_          ---- x1
        //    /   \
        //    2   3         ---- x2 (%2)
        //   / \ / \
        //   F  4   5       ---- x3
        //     / \  ||
        //     F |  6       ---- x4 (%2)
        //       \ / \
        //        7  8      ---- x5
        //       / \/ \
        //       F  T F
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(2, 1);
        const arc::uid_type n4(3, 0);
        const arc::uid_type n5(3, 1);
        const arc::uid_type n6(4, 0);
        const arc::uid_type n7(5, 0);
        const arc::uid_type n8(5, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true, n5 });
          aw.push_internal({ n5, false, n6 });
          aw.push_internal({ n5, true, n6 });
          aw.push_internal({ n4, true, n7 });
          aw.push_internal({ n6, false, n7 });
          aw.push_internal({ n6, true, n8 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n7, false, terminal_F });
          aw.push_terminal({ n7, true, terminal_T });
          aw.push_terminal({ n8, false, terminal_T });
          aw.push_terminal({ n8, true, terminal_F });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 2u));
          aw.push(level_info(4, 1u));
          aw.push(level_info(5, 2u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output (note, the deepest ones are doubly-negated!)
        //       1     ---- x1
        //      / \
        //     /   \   ---- x2
        //     |   |
        //     4   /   ---- x3
        //    / \ /
        //    T  X     ---- x4
        //      / \
        //      7  8   ---- x5
        //     / \/ \
        //     T F  T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n8
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_F, terminal_T)));

        // n7
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(5, node::max_id - 1, terminal_T, terminal_F)));

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, node::max_id, terminal_T, node::pointer_type(5, node::max_id))));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id),
                                     node::pointer_type(5, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("accumulates multiple nested sweeps [incl. root]", []() {
        /* input
        //         _1_       ---- x0 (%2)
        //        /   \
        //        2   3      ---- x1
        //       / \ / \
        //       T  4   5    ---- x2 (%2)
        //         / \ / \
        //         T  6  F   ---- x3
        //           / \
        //           F T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);
        const arc::uid_type n3(1, 1);
        const arc::uid_type n4(2, 0);
        const arc::uid_type n5(2, 1);
        const arc::uid_type n6(3, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n3, true, n5 });
          aw.push_internal({ n4, true, n6 });
          aw.push_internal({ n5, false, n6 });

          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n5, true, terminal_F });
          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 2u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 1u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //                    ---- x0
        //
        //        _3_         ---- x1
        //       /   \
        //       |   T        ---- x2
        //       |
        //       6            ---- x3
        //      / \
        //      F T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n6
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(3, node::max_id, terminal_F, terminal_T)));

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(1, node::max_id, node::pointer_type(3, node::max_id), terminal_T)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("accumulates multiple nested sweeps on (transposed) node file", []() {
        /* input
        // See 'accumulates multiple nested sweeps [excl. root]' above but with
        // (5) removed since it would realistically be reduced away.
        */
        const node::uid_type terminal_F(false);
        const node::uid_type terminal_T(true);

        // In canonical order (realistic output from Reduce)
        const node n7(5, node::max_id, terminal_F, terminal_T);
        const node n8(5, node::max_id - 1, terminal_T, terminal_F);
        const node n6(4, node::max_id, n7.uid(), n8.uid());
        // const node n5(3,_,            n6.uid(),   n6.uid());
        const node n4(3, node::max_id, terminal_F, n7.uid());
        const node n3(2, node::max_id, n4.uid(), n6.uid());
        const node n2(2, node::max_id - 1, terminal_F, n4.uid());
        const node n1(1, node::max_id, n2.uid(), n3.uid());

        shared_levelized_file<node> in;

        { // Garbage collect writer to free write-lock
          node_ofstream nw(in);
          nw << n7 << n8 << n6 /*<< n5*/ << n4 << n3 << n2 << n1;
        }

        // Just a sanity check we created the input as intended
        AssertThat(in->is_canonical(), Is().True());

        test_not_sweep<> inner_impl(2);

        /* output
        // See 'accumulates multiple nested sweeps [excl. root]' above.
        */
        const bdd out = nested_sweep<>(exec_policy(), in, inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n8
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_F, terminal_T)));

        // n7
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(5, node::max_id - 1, terminal_T, terminal_F)));

        // n4
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, node::max_id, terminal_T, node::pointer_type(5, node::max_id))));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id),
                                     node::pointer_type(5, node::max_id - 1))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 2u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(2u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(5u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
      });

      it("accumulates multiple nested sweeps with arcs that span multiple sweeps", []() {
        // Here, we have an arc that spans multiple existing levels where an
        // inner sweep has to be started on.

        /* input
        //           _1_        ---- x1
        //          /   \
        //          2    \      ---- x2 (%2)
        //         / \   |
        //         | 3   |      ---- x3
        //         |/ \  /
        //         4_ F /       ---- x4 (%2)
        //        /  \ /
        //        T   5         ---- x5
        //           / \
        //           F T
        */

        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(3, 0);
        const arc::uid_type n4(4, 0);
        const arc::uid_type n5(5, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n2, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n1, true, n5 });
          aw.push_internal({ n4, true, n5 });

          aw.push_terminal({ n3, true, terminal_F });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 1u));
          aw.push(level_info(4, 1u));
          aw.push(level_info(5, 1u));

          in->max_1level_cut = 3;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //         _1_       ---- x1
        //        /   \
        //        |   |      ---- x2
        //        |   |
        //        3   |      ---- x3
        //       / \  /
        //       \ T /       ---- x4
        //        \ /
        //         5         ---- x5
        //        / \
        //        F T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n5
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_F, terminal_T)));

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(3, node::max_id, node::pointer_type(5, node::max_id), terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(3, node::max_id),
                                     node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("accumulates multiple nested sweeps on renamed outer sweep", []() {
        // Repetition of 'accumulates multiple nested sweeps with arcs that span multiple sweeps'
        // with all output nodes shifted by one.
        //
        // By repeating this very test, we also make sure that variables are renamed in the outer
        // sweep rather than the inner (If so, the deepest nodes would be shifted twice).
        class test_shifted_not_sweep : public test_not_sweep<>
        {
        private:
          int _shift;

        public:
          test_shifted_not_sweep(const size_t nm, int shift)
            : test_not_sweep<>(nm)
            , _shift(shift)
          {}

          ////////////////////////////////////////////////////////////////////////////////////////////////
          constexpr inline bdd::label_type
          map_level(bdd::label_type x) const
          {
            return x + this->_shift;
          }
        };

        /* input
        //           _1_        ---- x1
        //          /   \
        //          2    \      ---- x2 (%2)
        //         / \   |
        //         | 3   |      ---- x3
        //         |/ \  /
        //         4_ F /       ---- x4 (%2)
        //        /  \ /
        //        T   5         ---- x5
        //           / \
        //           F T
        */

        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(3, 0);
        const arc::uid_type n4(4, 0);
        const arc::uid_type n5(5, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n2, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n1, true, n5 });
          aw.push_internal({ n4, true, n5 });

          aw.push_terminal({ n3, true, terminal_F });
          aw.push_terminal({ n4, false, terminal_T });
          aw.push_terminal({ n5, false, terminal_F });
          aw.push_terminal({ n5, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 1u));
          aw.push(level_info(4, 1u));
          aw.push(level_info(5, 1u));

          in->max_1level_cut = 3;
        }

        test_shifted_not_sweep inner_impl(2, -1);

        /* output
        //         _1_       ---- x0
        //        /   \
        //        |   |
        //        |   |
        //        3   |      ---- x2
        //       / \  /
        //       \ T /
        //        \ /
        //         5         ---- x4
        //        / \
        //        F T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n5
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_F, terminal_T)));

        // n3
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, node::max_id, node::pointer_type(4, node::max_id), terminal_T)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(0,
                                     node::max_id,
                                     node::pointer_type(2, node::max_id),
                                     node::pointer_type(4, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(0, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(3u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("handles all root arcs collapsing into terminals", []() {
        // Here, the Inner Down Sweep collapses each arc to a terminal, but
        // the nodes above it in the Outer Sweep will remain alive.

        /* input
        //         _1_        ---- x1
        //        /   \
        //        2   3       ---- x2 (%2)
        //       / \ / \
        //       |  4  |      ---- x3
        //       | / \ |
        //       | F T |      ---- x4
        //       |     |
        //       5     6      ---- x5
        //      / \   / \
        //      T F   F T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(2, 1);
        const arc::uid_type n4(3, 0);
        const arc::uid_type n5(5, 0);
        const arc::uid_type n6(5, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n3, false, n4 });
          aw.push_internal({ n2, false, n5 });
          aw.push_internal({ n3, true, n6 });

          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });
          aw.push_terminal({ n5, false, terminal_T });
          aw.push_terminal({ n5, true, terminal_F });
          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 1u));
          aw.push(level_info(5, 2u));

          in->max_1level_cut = 4;
        }

        test_terminal_sweep inner_impl(2);

        /* output
        //        1            ---- x1
        //       / \
        //       T F
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

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

      it("handles some root arcs collapsing into terminals", []() {
        // Here, the Inner Down Sweep collapses one arc to a terminal but
        // creates another node.

        /* input
        //         _1_        ---- x2
        //        /   \
        //        2   |       ---- x3 (%3)
        //       / \  |
        //       3 T  |       ---- x4 (collapse to terminal)
        //      / \   |
        //      F T   4       ---- x5 (create node)
        //           / \
        //           F T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(2, 0);
        const arc::uid_type n2(3, 0);
        const arc::uid_type n3(4, 0);
        const arc::uid_type n4(5, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n1, true, n4 });

          aw.push_terminal({ n2, true, terminal_F });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });
          aw.push_terminal({ n4, false, terminal_F });
          aw.push_terminal({ n4, true, terminal_T });

          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 1u));
          aw.push(level_info(4, 1u));
          aw.push(level_info(5, 1u));

          in->max_1level_cut = 2;
        }

        test_terminal_sweep inner_impl(3);

        /* output
        //         _1_        ---- x2
        //        /   \
        //       /    |       ---- x3 (%3)
        //       |    |
        //       F    |       ---- x4
        //            |
        //            4'      ---- x5
        //           / \
        //           T F
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n4'
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_T, terminal_F)));

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, node::max_id, terminal_F, node::pointer_type(5, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));
        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

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

      it("can garbage collect temp solution when no parent references it [non-root level]", []() {
        /* input
        //     1       ---- x1
        //    / \
        //    F 2      ---- x2 (%2)
        //     / \
        //     3 T     ---- x3
        //    / \
        //    F T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(3, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n2, false, n3 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_T });
          aw.push_terminal({ n3, false, terminal_F });
          aw.push_terminal({ n3, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     1       ---- x1
        //    / \
        //    F T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

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

      it("can suppress nesting-level node into a terminal [non-root level]", []() {
        /* input
        //     1       ---- x1
        //    / \
        //    F 2      ---- x2 (%2)
        //     / \
        //     3 T     ---- x3
        //    / \
        //    T T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(3, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n2, false, n3 });

          aw.push_terminal({ n1, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_T });
          aw.push_terminal({ n3, false, terminal_T });
          aw.push_terminal({ n3, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     1       ---- x1
        //    / \
        //    F T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n1
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_F, terminal_T)));
        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

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

      it("can suppress nesting-level node into a terminal [root level]", []() {
        /* input
        //      1      ---- x0 (%2)
        //     / \
        //     2 T     ---- x1
        //    / \
        //    T T
        */
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });

          aw.push_terminal({ n1, true, terminal_T });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can suppress nesting-level node that induces a 2-level cut", []() {
        /* input
        //       1_       ---- x1
        //      /  \
        //      2  3      ---- x2
        //     / \/ \
        //     4 5  6     ---- x3 (%3)
        //     |X| / \
        //     7 8 F T    ---- x4
        //    /| |\
        //    TF FT
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(2, 1);
        const arc::uid_type n4(3, 0);
        const arc::uid_type n5(3, 1);
        const arc::uid_type n6(3, 2);
        const arc::uid_type n7(4, 0);
        const arc::uid_type n8(4, 1);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n3 });
          aw.push_internal({ n2, false, n4 });
          aw.push_internal({ n2, true, n5 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n3, true, n6 });
          aw.push_internal({ n4, false, n7 });
          aw.push_internal({ n5, false, n7 });
          aw.push_internal({ n4, true, n8 });
          aw.push_internal({ n5, true, n8 });

          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true, terminal_T });
          aw.push_terminal({ n7, false, terminal_T });
          aw.push_terminal({ n7, true, terminal_F });
          aw.push_terminal({ n8, false, terminal_F });
          aw.push_terminal({ n8, true, terminal_T });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 2u));
          aw.push(level_info(3, 3u));
          aw.push(level_info(4, 2u));

          in->max_1level_cut = 4;
        }

        test_not_sweep<> inner_impl(3);

        /* output
        //     _1_       ---- x1
        //    /   \
        //    |   3      ---- x2
        //    |  / \
        //    \ /  T
        //     8         ---- x4
        //    / \
        //    T F
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        // n8
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(4, node::max_id, terminal_T, terminal_F)));

        // n3'
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(
          out_nodes.pull(),
          Is().EqualTo(node(2, node::max_id, node::pointer_type(4, node::max_id), terminal_T)));

        // n1'
        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(),
                   Is().EqualTo(node(1,
                                     node::max_id,
                                     node::pointer_type(4, node::max_id),
                                     node::pointer_type(2, node::max_id))));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(4, 1u)));
        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(2, 1u)));
        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));
        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
        // Over-approximation, since F terminal from level x3 is removed?
        AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(4u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(2u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(3u));
        AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(4u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
      });

      it("can preserve subtree for suppressed root with inner sweep", []() {
        /* input
        //          1      ---- x0 (%2)
        //          ||
        //          2      ---- x1
        //         / \
        //         F T
        */

        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });
          aw.push_internal({ n1, true, n2 });

          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));

          in->max_1level_cut = 2;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     2         ---- x1
        //    / \
        //    T F
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(1, node::max_id, terminal_T, terminal_F)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().True());
        AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(1u));

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

      it("can solve root-level nesting request directly to a terminal", []() {
        /* input
        //      1      ---- x0 (%2)
        //     / \
        //     2 T     ---- x1
        //    / \
        //    F T
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });

          aw.push_terminal({ n1, true, terminal_T });
          aw.push_terminal({ n2, false, terminal_F });
          aw.push_terminal({ n2, true, terminal_T });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can collapse to a terminal [outer up]", []() {
        /* input
        //      1      ---- x1
        //     / \
        //     2 F     ---- x2 (%2)
        //    / \
        //    T F
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });

          aw.push_terminal({ n1, true, terminal_F });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true, terminal_F });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));

          in->max_1level_cut = 1;
        }

        test_not_sweep<> inner_impl(2);

        /* output
        //     F
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(false)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(1u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(0u));
      });

      it("can collapse to a terminal [inner down]", []() {
        /* input
        //      1      ---- x0 (%2)
        //     / \
        //     2 T     ---- x1
        //    / \
        //    T F
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(1, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });

          aw.push_terminal({ n1, true, terminal_T });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(1, 1u));

          in->max_1level_cut = 1;
        }

        test_terminal_sweep inner_impl(2);

        /* output
        //     T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      it("can collapse to a terminal [inner up]", []() {
        /* input
        //      1      ---- x0 (%4)
        //     / \
        //     | T     ---- x1
        //     |
        //     2       ---- x2
        //    / \
        //    T F
        */
        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        const arc::uid_type n1(0, 0);
        const arc::uid_type n2(2, 0);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, false, n2 });

          aw.push_terminal({ n1, true, terminal_T });
          aw.push_terminal({ n2, false, terminal_T });
          aw.push_terminal({ n2, true, terminal_F });

          aw.push(level_info(0, 1u));
          aw.push(level_info(2, 1u));

          in->max_1level_cut = 1;
        }

        test_terminal_sweep inner_impl(4);
        /* output
        //     T   <-- reduced from  2'    ---- x2
        //                          / \
        //                          T T
        */
        const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

        // Check it looks all right
        node_test_ifstream out_nodes(out);

        AssertThat(out_nodes.can_pull(), Is().True());
        AssertThat(out_nodes.pull(), Is().EqualTo(node(true)));

        AssertThat(out_nodes.can_pull(), Is().False());

        level_info_test_ifstream out_meta(out);

        AssertThat(out_meta.can_pull(), Is().False());

        AssertThat(out->width, Is().EqualTo(0u));

        AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_False], Is().EqualTo(0u));
        AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
        AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(1u));

        AssertThat(out->number_of_terminals[false], Is().EqualTo(0u));
        AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
      });

      describe("bail-out optimisation", []() {
        it("bails out of inner sweep when all requests are subtree-preserving", []() {
          /* input
          //         _1_       ---- x1
          //        /   \
          //        2   3      ---- x2 (%2)
          //       / \ / \
          //       T  4   5    ---- x3
          //         / \ / \
          //         T  6  F   ---- x4 (%2) <-- skipped
          //           / \
          //           F 7     ---- x5
          //            / \
          //            F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(1, 0);
          const arc::uid_type n2(2, 0);
          const arc::uid_type n3(2, 1);
          const arc::uid_type n4(3, 0);
          const arc::uid_type n5(3, 1);
          const arc::uid_type n6(4, 0);
          const arc::uid_type n7(5, 0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });
            aw.push_internal({ n2, true, n4 });
            aw.push_internal({ n3, false, n4 });
            aw.push_internal({ n3, true, n5 });
            aw.push_internal({ n4, true, n6 });
            aw.push_internal({ n5, false, n6 });
            aw.push_internal({ n6, true, n7 });

            aw.push_terminal({ n2, false, terminal_T });
            aw.push_terminal({ n4, false, terminal_T });
            aw.push_terminal({ n5, true, terminal_F });
            aw.push_terminal({ n6, false, terminal_F });
            aw.push_terminal({ n7, false, terminal_F });
            aw.push_terminal({ n7, true, terminal_T });

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 2u));
            aw.push(level_info(3, 2u));
            aw.push(level_info(4, 1u));
            aw.push(level_info(5, 1u));

            in->max_1level_cut = 3;
          }

          test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ false, /*OnlyGC*/ true> inner_impl(
            2);

          /* output
          //        _1_      ---- x1
          //       /   \
          //       |   |     ---- x2
          //       |   |
          //       4   5     ---- x3
          //      / \ / \
          //      F | |  T    ---- x4
          //        \ /
          //         7       ---- x5
          //        / \
          //        T F      <--- NOTE: only negated once!
          */
          const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n7
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_T, terminal_F)));

          // n5
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, node::max_id, node::pointer_type(5, node::max_id), terminal_T)));

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(
                       node(3, node::max_id - 1, terminal_F, node::pointer_type(5, node::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       node::pointer_type(3, node::max_id - 1),
                                       node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(3u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(4u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(5u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(2u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(2u));
        });

        it("handles node suppression cut-taint when bailing out", []() {
          /* input
          //         __1__      ---- x1
          //        /     \
          //        2     3     ---- x2 (%2)
          //       / \    ||
          //       T 4    5     ---- x3
          //        / \  / \
          //        T 6  F 7    ---- x4 (%2) <-- skipped
          //         / \  / \
          //         F 8  T F   ---- x5
          //          / \
          //          F T
          */
          const arc::uid_type terminal_F(false);
          const arc::uid_type terminal_T(true);

          const arc::uid_type n1(1, 0);
          const arc::uid_type n2(2, 0);
          const arc::uid_type n3(2, 1);
          const arc::uid_type n4(3, 0);
          const arc::uid_type n5(3, 1);
          const arc::uid_type n6(4, 0);
          const arc::uid_type n7(4, 1);
          const arc::uid_type n8(5, 0);

          shared_levelized_file<arc> in;

          { // Garbage collect writer to free write-lock
            arc_ofstream aw(in);

            aw.push_internal({ n1, false, n2 });
            aw.push_internal({ n1, true, n3 });
            aw.push_internal({ n2, true, n4 });
            aw.push_internal({ n3, false, n5 });
            aw.push_internal({ n3, true, n5 });
            aw.push_internal({ n4, true, n6 });
            aw.push_internal({ n5, true, n7 });
            aw.push_internal({ n6, true, n8 });

            aw.push_terminal({ n2, false, terminal_T });
            aw.push_terminal({ n4, false, terminal_T });
            aw.push_terminal({ n5, false, terminal_F });
            aw.push_terminal({ n6, false, terminal_F });
            aw.push_terminal({ n7, false, terminal_T });
            aw.push_terminal({ n7, true, terminal_F });
            aw.push_terminal({ n8, false, terminal_F });
            aw.push_terminal({ n8, true, terminal_T });

            aw.push(level_info(1, 1u));
            aw.push(level_info(2, 2u));
            aw.push(level_info(3, 2u));
            aw.push(level_info(4, 2u));
            aw.push(level_info(5, 1u));

            in->max_1level_cut = 3;
          }

          test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ false, /*OnlyGC*/ true> inner_impl(
            2);

          /* output
          //        _1_      ---- x1
          //       /   \
          //       |   F     ---- x2
          //       |
          //       4         ---- x3
          //      / \
          //      F |        ---- x4
          //        |
          //        7        ---- x5
          //       / \
          //       T F       <--- NOTE: only negated once!
          */
          const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n7
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_T, terminal_F)));

          // n5
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(3, node::max_id, terminal_F, node::pointer_type(5, node::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(
            out_nodes.pull(),
            Is().EqualTo(node(1, node::max_id, node::pointer_type(3, node::max_id), terminal_F)));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 1u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(1u));

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(1u));

          // NOTE: Since the outer priority queue includes a tainted arc to F, it
          //       is in fact counted twice: first it is counted in the inner
          //       sweep to then be counted once more in the outer
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_1level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_1level_cut[cut::All], Is().LessThanOrEqualTo(5u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(1u));

          // NOTE: See above
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().EqualTo(1u));
          AssertThat(out->max_2level_cut[cut::All], Is().GreaterThanOrEqualTo(4u));
          AssertThat(out->max_2level_cut[cut::All], Is().LessThanOrEqualTo(4u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(1u));
        });
      });

      describe("'final_canonical' switch (fast-reduce optimisation)", []() {
        /* input
        //           1            ---- x1
        //          / \
        //         /   \
        //         |   |
        //         |   2          ---- x2 (%2) <-- Negates (4) subtree to prove nesting
        //         |  / \
        //         | /   \
        //         |/     \
        //         3      4       ---- x3      <-- (3) is suppressable if (5) and (7) are merged.
        //        / \     ||                       (4) is suppressable either way.
        //       /   \    ||
        //       |   |    ||
        //       5   7    6       ---- x5      <-- (5) and (7) should be merged but (6) blocks the
        //      / \ / \  / \                       view even for a fancy fast-reduce with reduction
        //      |  X  |  F T                       rules applied (if possible without sorting)
        //      \ / \ /                            NOTE: out-of-order (for sake of clarity)
        //       8   9            ---- x6 (%2)
        //      / \ / \
        //      F | T  \
        //        10   11         ---- x7      <-- out-of-order
        //       /  \ /  \
        //       F  T T  F
        */
        const arc::uid_type n1(1, 0);
        const arc::uid_type n2(2, 0);
        const arc::uid_type n3(3, 0);
        const arc::uid_type n4(3, 1);
        const arc::uid_type n5(5, 0);
        const arc::uid_type n6(5, 1);
        const arc::uid_type n7(5, 2);
        const arc::uid_type n8(6, 0);
        const arc::uid_type n9(6, 1);
        const arc::uid_type n10(7, 0);
        const arc::uid_type n11(7, 1);

        const arc::uid_type terminal_F(false);
        const arc::uid_type terminal_T(true);

        shared_levelized_file<arc> in;

        { // Garbage collect writer to free write-lock
          arc_ofstream aw(in);

          aw.push_internal({ n1, true, n2 });
          aw.push_internal({ n1, false, n3 });
          aw.push_internal({ n2, false, n3 });
          aw.push_internal({ n2, true, n4 });
          aw.push_internal({ n3, false, n5 });
          aw.push_internal({ n4, false, n6 });
          aw.push_internal({ n4, true, n6 });
          aw.push_internal({ n3, true, n7 });
          aw.push_internal({ n5, false, n8 });
          aw.push_internal({ n7, false, n8 });
          aw.push_internal({ n5, true, n9 });
          aw.push_internal({ n7, true, n9 });
          aw.push_internal({ n8, true, n10 });
          aw.push_internal({ n9, true, n11 });

          aw.push_terminal({ n6, false, terminal_F });
          aw.push_terminal({ n6, true, terminal_T });
          aw.push_terminal({ n8, false, terminal_F });
          aw.push_terminal({ n9, false, terminal_T });
          aw.push_terminal({ n10, false, terminal_F });
          aw.push_terminal({ n10, true, terminal_T });
          aw.push_terminal({ n11, false, terminal_T });
          aw.push_terminal({ n11, true, terminal_F });

          aw.push(level_info(1, 1u));
          aw.push(level_info(2, 1u));
          aw.push(level_info(3, 2u));
          aw.push(level_info(5, 3u));
          aw.push(level_info(6, 2u));
          aw.push(level_info(7, 2u));

          in->max_1level_cut = 4;
        }

        it("outputs a canonical BDD if set to 'true'", [&]() {
          /* input
          //           1             ---- x1
          //          / \
          //         /   \
          //         |    \
          //         |     \         ---- x2
          //         |      \
          //         |       \
          //         |       |
          //         |       |       ---- x3
          //         |       |
          //         |       |
          //         |       |
          //        5/6      7       ---- x5
          //       /   \    / \
          //      /     \   F T
          //      |     |
          //      |     |            ---- x6
          //      |     |
          //      |     |
          //     11     10           ---- x7
          //    /  \   /  \
          //    T  F   F  T
          */

          test_not_sweep</*FinalCanonical*/ true, /*FastReduce*/ true> inner_impl(2);

          const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n10 (doubly-negated)
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id, terminal_F, terminal_T)));

          // n11 (doubly-negated)
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(7, node::max_id - 1, terminal_T, terminal_F)));

          // n7 (singly-negated)
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(5, node::max_id, terminal_T, terminal_F)));

          // n5/6
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5,
                                       node::max_id - 1,
                                       node::pointer_type(7, node::max_id),
                                       node::pointer_type(7, node::max_id - 1))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       node::pointer_type(5, node::max_id - 1),
                                       node::pointer_type(5, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(2u));

          AssertThat(out->is_canonical(), Is().True());

          // Over-approximation:
          //   5 and 7 are only merged after the second inner sweep for x2. This
          //   propagates to 3 only in the last sweep, which in turn taints the
          //   arc 1 ---> 5/7 which is added onto the final cut-size.
          //
          //   Furthermore, 4 is first suppressed during the second inner sweep for x2,
          //   which in turn taints the arc 2 ---> 6 which is added onto the final cut size.
          AssertThat(out->max_1level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));

          // Over-approximation:
          //   See above.
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().LessThanOrEqualTo(5u));

          // Over-approximation:
          //   See above.
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().LessThanOrEqualTo(5u));

          // Over-approximation:
          //   Despite the above, there is still the saving grace of 5 BDD Nodes +1.
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal], Is().LessThanOrEqualTo(4u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(2u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(6u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(3u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(6u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
        });

        it("outputs an unordered, unmerged, and unsuppressed BDD if set to 'false'", [&]() {
          test_not_sweep</*FinalCanonical*/ false, /*FastReduce*/ true> inner_impl(2);

          const bdd out = nested_sweep<>(exec_policy(), __bdd(in, exec_policy()), inner_impl);

          // Check it looks all right
          node_test_ifstream out_nodes(out);

          // n11 (doubly-negated)
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(), Is().EqualTo(node(7, node::max_id, terminal_T, terminal_F)));

          // n10 (doubly-negated)
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(7, node::max_id - 1, terminal_F, terminal_T)));

          // n7
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5,
                                       node::max_id,
                                       node::pointer_type(7, node::max_id - 1),
                                       node::pointer_type(7, node::max_id))));

          // n6 (singly-negated)
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5, node::max_id - 1, terminal_T, terminal_F)));

          // n5
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(5,
                                       node::max_id - 2,
                                       node::pointer_type(7, node::max_id - 1),
                                       node::pointer_type(7, node::max_id))));

          // n4
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3,
                                       node::max_id,
                                       node::pointer_type(5, node::max_id - 1),
                                       node::pointer_type(5, node::max_id - 1))));

          // n3
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(3,
                                       node::max_id - 1,
                                       node::pointer_type(5, node::max_id - 2),
                                       node::pointer_type(5, node::max_id))));

          // n1
          AssertThat(out_nodes.can_pull(), Is().True());
          AssertThat(out_nodes.pull(),
                     Is().EqualTo(node(1,
                                       node::max_id,
                                       node::pointer_type(3, node::max_id - 1),
                                       node::pointer_type(3, node::max_id))));

          AssertThat(out_nodes.can_pull(), Is().False());

          level_info_test_ifstream out_meta(out);

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(7, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(5, 3u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(3, 2u)));

          AssertThat(out_meta.can_pull(), Is().True());
          AssertThat(out_meta.pull(), Is().EqualTo(level_info(1, 1u)));

          AssertThat(out_meta.can_pull(), Is().False());

          AssertThat(out->width, Is().EqualTo(3u));

          AssertThat(out->sorted, Is().False());
          AssertThat(out->indexable, Is().True());
          AssertThat(out->is_canonical(), Is().False());

          AssertThat(out->max_1level_cut[cut::Internal], Is().EqualTo(4u));
          AssertThat(out->max_1level_cut[cut::Internal_False], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::Internal_True], Is().EqualTo(5u));
          AssertThat(out->max_1level_cut[cut::All], Is().EqualTo(6u));

          AssertThat(out->max_2level_cut[cut::Internal], Is().EqualTo(6u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().GreaterThanOrEqualTo(6u));
          AssertThat(out->max_2level_cut[cut::Internal_False], Is().LessThanOrEqualTo(7u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().GreaterThanOrEqualTo(6u));
          AssertThat(out->max_2level_cut[cut::Internal_True], Is().LessThanOrEqualTo(7u));
          AssertThat(out->max_2level_cut[cut::All], Is().EqualTo(8u));

          AssertThat(out->number_of_terminals[false], Is().EqualTo(3u));
          AssertThat(out->number_of_terminals[true], Is().EqualTo(3u));
        });
      });
    });
  });
});
