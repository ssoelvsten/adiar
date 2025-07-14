#ifndef ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H
#define ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H

#include <adiar/exec_policy.h>
#include <adiar/functional.h>
#include <adiar/statistics.h>

#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_structures/sorter.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/iofstream.h>
#include <adiar/internal/io/node_raccess.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Name space for helper functions and classes for the `nested_sweep` algorithm below.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  namespace nested_sweeping
  {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Struct to hold statistics
    extern statistics::nested_sweeping_t stats;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   A faster alternative to `__reduce_level`.
    ///
    /// \details The `__reduce_level` algorithm merges duplicate subtrees by use of two sorting
    ///          steps. Yet, these cost I/Os and computation time. This is an alternative, that does
    ///          not apply any reduction rules. This is only useful, if one can already guarantee
    ///          the unreduced input in fact already is reduced, e.g. variable reordering.
    ///
    /// \warning Where `__reduce_level` only requires the priority queue can contain a 1-level cut
    ///          (since it processes all nodes of a level in bulk), this variation induces a 2-level
    ///          cut (since it processes one node at a time).
    ///
    /// \see __reduce_level
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename dd_policy, typename pq_t, typename arc_ifstream_t>
    void
    __reduce_level__fast(arc_ifstream_t& arcs,
                         const typename dd_policy::label_type in_label,
                         const typename dd_policy::label_type out_label,
                         pq_t& pq,
                         node_ofstream& out,
                         [[maybe_unused]] statistics::reduce_t& stats = internal::stats_reduce)
    {
      // Count number of arcs that cross this level. Since all or no levels are reduced with this
      // logic, no node should suppressed and its parents arcs tainted.
      cuts_t local_1level_cut = { { 0u, 0u, 0u, 0u } };

      __reduce_cut_add(local_1level_cut,
                       pq.size_without_terminals(),
                       pq.terminals(false) + arcs.unread_terminals(false),
                       pq.terminals(true) + arcs.unread_terminals(true));

      // Pull out all nodes from pq and terminal_arcs for this level
      typename dd_policy::id_type out_id = dd_policy::max_id;

      while (pq.can_pull()
             || (arcs.can_pull_terminal() && arcs.peek_terminal().source().label() == in_label)) {
        // TODO (MDD / QMDD):
        //   Use __reduce_get_next node_type::outdegree times to create a node_type::children_type.
        const arc e_high = __reduce_get_next(pq, arcs);
        const arc e_low  = __reduce_get_next(pq, arcs);
        adiar_assert(essential(e_low.source()) == essential(e_high.source()),
                     "Obtained arcs for the same node");
        adiar_assert(!e_low.target().is_flagged(),
                     "No previous nodes have been suppressed and their arcs tainted");
        adiar_assert(!e_high.target().is_flagged(),
                     "No previous nodes have been suppressed and their arcs tainted");

        // Output node
        adiar_assert(out_id > 0, "Should still have more ids left");
        const typename dd_policy::node_type out_node(
          out_label, out_id--, e_low.target(), e_high.target());
        out.unsafe_push(out_node);

        // Forward resulting node to parents
        const node::pointer_type e_src = e_low.source();
        adiar_assert(e_src == essential(e_src));

        const node::pointer_type t = out_node.uid();
        adiar_assert(t.is_terminal() || t.out_idx() == false, "Created target is without an index");
        adiar_assert(!t.is_flagged(), "Target does not have suppression taint");

        while (arcs.can_pull_internal() && arcs.peek_internal().target() == e_src) {
          // The out_idx is included in arc.source() pulled from the internal arcs.
          const node::pointer_type s = arcs.pull_internal().source();
          pq.push(arc(s, t));
        }
      }

      // Update with new possible maximum 1-level cut.
      out.unsafe_max_1level_cut(local_1level_cut);

      // Add number of nodes to level information, if any nodes were pushed to the output.
      // Furthermore, mark as unsorted if at least two nodes were output (their order might
      // very much have been wrong).
      if (out_id != dd_policy::max_id) {
        const size_t width = dd_policy::max_id - out_id;
        out.unsafe_push(level_info(out_label, width));

        if (width > 1u) { out.unsafe_set_sorted(false); }
      }

      // Set up priority queue for next level
      constexpr bool terminal_value = false; // <-- NOTE: Dummy value
      __reduce_level__epilogue<>(arcs, pq, out, terminal_value);
    }

    template <typename dd_policy, typename pq_t, typename arc_ifstream_t>
    void
    __reduce_level__fast(arc_ifstream_t& arcs,
                         const typename dd_policy::label_type label,
                         pq_t& pq,
                         node_ofstream& out,
                         statistics::reduce_t& stats = internal::stats_reduce)
    {
      return __reduce_level__fast<dd_policy, pq_t, arc_ifstream_t>(
        arcs, label, label, pq, out, stats);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Algorithms and Decorators for the Outer sweep.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    namespace outer
    {
      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Sorter for root requests to inner sweep
      ///
      /// \details This is to be used as the bridge from the *outer* bottom-up sweep to the *inner*
      ///          top-down sweep.
      ///
      /// \see sorter nested_sweep
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <memory_mode MemoryMode, typename element_t, typename element_comp_t>
      class roots_sorter
      {
      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using value_type = element_t;

        static_assert(value_type::inputs == 1 && value_type::sorted_target,
                      "Request should be on a single input, and hence sorted");

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using value_comp_type = element_comp_t;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Memory mode of sorter.
        ////////////////////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode mem_mode = MemoryMode;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the sorter.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using sorter_t = sorter<mem_mode, value_type, value_comp_type>;

        ////////////////////////////////////////////////////////////////////////////////////////////
        static constexpr typename value_type::label_type no_level =
          static_cast<typename value_type::label_type>(-1);

        static constexpr size_t data_structures = sorter_t::data_structures;

      private:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Resetabble sorter for elements
        ////////////////////////////////////////////////////////////////////////////////////////////
        unique_ptr<sorter_t> _sorter_ptr;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Maximum source seen
        ////////////////////////////////////////////////////////////////////////////////////////////
        typename value_type::pointer_type _max_source = value_type::pointer_type::nil();

        ////////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: There is not '_terminals[2]' like in the priority queue, since there is as an
        //       invariant that the target is never a terminal.

        ////////////////////////////////////////////////////////////////////////////////////////////
        const size_t _memory_bytes;
        const size_t _no_arcs;

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        static size_t
        memory_usage(size_t no_elements)
        {
          return sorter_t::memory_usage(no_elements);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        static size_t
        memory_fits(size_t memory_bytes)
        {
          return sorter_t::memory_fits(memory_bytes);
        }

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        roots_sorter(size_t memory_bytes, size_t no_arcs)
          : _sorter_ptr(sorter_t::make_unique(memory_bytes, no_arcs))
          , _memory_bytes(memory_bytes)
          , _no_arcs(no_arcs)
        {}

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether a request can be pushed.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_push()
        {
          return _sorter_ptr->can_push();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Push request (marked as from outer sweep).
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        push(const value_type& v)
        {
          adiar_assert(v.target.first().is_node(),
                       "Requests should have at least one internal node");

          _max_source = _max_source.is_nil() ? v.data.source : std::max(_max_source, v.data.source);

          // TODO: support requests with more than just the source
          _sorter_ptr->push({ v.target, {}, { flag(v.data.source) } });
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Convert arc to request (and mark it as from outer sweep).
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        push(const reduce_arc& a)
        {
          adiar_assert(!a.target().is_terminal(),
                       "Arcs to terminals always reside in the outer PQ");

          // TODO: support requests with more than just the source

          // TODO: Is there a better way to explicitly set the remainders of
          //       target to nil?

          if constexpr (value_type::cardinality == 1u) {
            push(value_type({ a.target() }, {}, { a.source() }));
          } else if constexpr (value_type::cardinality == 2u) {
            push(value_type({ a.target(), value_type::pointer_type::nil() }, {}, { a.source() }));
          } else {
            static_assert(value_type::cardinality <= 2u,
                          "Missing implementation for larger than binary combinators");
          }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        sort()
        {
          _sorter_ptr->sort();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_top() /*const*/
        {
          return _sorter_ptr->can_pull();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        value_type
        top() /*const*/
        {
          return _sorter_ptr->top();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_pull() /*const*/
        {
          return has_top();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        value_type
        pull()
        {
          // TODO: decrement terminal count (is this number ever relevant before it already is
          //       'reset()'?)
          return _sorter_ptr->pull();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        reset()
        {
          sorter_t::reset_unique(_sorter_ptr, _memory_bytes, _no_arcs);
          _max_source = value_type::pointer_type::nil();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Take ownership of another root sorter's content.
        ///
        /// \details This leaves the other in an illegal state that requires a call to `reset()`.
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        move(roots_sorter<mem_mode, element_t, element_comp_t>& o)
        {
          _sorter_ptr = std::move(o._sorter_ptr);
          _max_source = o._max_source;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        size() /*const*/
        {
          return _sorter_ptr->size();
        }

        bool
        empty() /*const*/
        {
          return _sorter_ptr->empty();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Level of the deepest source
        ////////////////////////////////////////////////////////////////////////////////////////////
        typename value_type::label_type
        deepest_source()
        {
          return _max_source.is_nil() ? no_level : _max_source.label();
        }
      };

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Default priority queue for the Outer Up Sweep.
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <size_t look_ahead, memory_mode mem_mode>
      using up__pq_t = reduce_priority_queue<look_ahead, mem_mode>;

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Decorator for the Reduce priority queue that either forwards to itself or to a
      ///        sorter, depending on the level of the source.
      ///
      /// \details This is to be used in the *outer* sweep when forwarding information to a level
      ///          above the current one.
      ///
      /// \see pq_t, levelized_priority_queue, nested_sweep
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <typename outer_pq_t, typename outer_roots_t>
      class up__pq_decorator
      {
      public:
        ////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////////////////////////
        using value_type = typename outer_pq_t::value_type;

        ////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////////////////////////
        using value_comp_type = typename outer_pq_t::value_comp_type;

        ////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Memory mode (same as decorated priority queue).
        ////////////////////////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode mem_mode = outer_pq_t::mem_mode;

      private:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the outer sweep's (levelized) priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        // TODO (optimisation): public inheritance?
        outer_pq_t& _outer_pq;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the sorter in the outer sweep that contains the root requests
        ///        generated for the next inner sweep.
        ////////////////////////////////////////////////////////////////////////////////////////////
        outer_roots_t& _outer_roots;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of a level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using level_type = typename value_type::pointer_type::label_type;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief The level of the next inner sweep;
        ////////////////////////////////////////////////////////////////////////////////////////////
        // TODO: turn into signed value to allow using decorator above last legal value of
        //       '_next_inner' (abusing negative numbers).
        const level_type _next_inner;

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Value to reflect 'out of levels'.
        ////////////////////////////////////////////////////////////////////////////////////////////////
        static constexpr level_type no_label = outer_pq_t::no_label;

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Instantiate decorator.
        ////////////////////////////////////////////////////////////////////////////////////////////
        up__pq_decorator(outer_pq_t& outer_pq,
                         outer_roots_t& outer_roots,
                         const level_type next_inner)
          : _outer_pq(outer_pq)
          , _outer_roots(outer_roots)
          , _next_inner(next_inner)
        {}

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Number of terminals (of each type) placed within the priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        terminals(const bool terminal_value) const
        {
          return _outer_pq.terminals(terminal_value);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Numer of elements in both the priority queue and the sorter.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        size() const
        {
          return _outer_pq.size() + _outer_roots.size();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Total number of arcs (across all levels) ignoring terminals.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        size_without_terminals() const
        {
          return size() - terminals(false) - terminals(true);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the current level is empty.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        empty_level() const
        {
          return _outer_pq.empty_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are more elements to pull for this level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_pull() const
        {
          return _outer_pq.can_pull();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain and remove the next element from this level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        reduce_arc
        pull()
        {
          return _outer_pq.pull();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there is a top element in the priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_top() const
        {
          return _outer_pq.has_top();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next element from this level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        reduce_arc
        top()
        {
          return _outer_pq.top();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Remove the top arc on the current level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        pop()
        {
          _outer_pq.pull();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Adds an element to the priority queue or the sorter, depending on the level.
        ///        Terminals always stay in the priority queue
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        push(const reduce_arc& a)
        {
          if (a.source().is_nil() || (a.source().label() < _next_inner && a.target().is_node())) {
#ifdef ADIAR_STATS
            nested_sweeping::stats.inner_down.requests.preserving += 1u;
#endif
            _outer_roots.push(a);
          } else {
#ifdef ADIAR_STATS
            nested_sweeping::stats.inner_down.requests.terminals +=
              static_cast<int>(a.target().is_terminal());
#endif
            _outer_pq.push(a);
          }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Push requests created at `_next_inner` to the priority queue if it only has
        ///        terminals; otherwise pushes to roots for nested sweep.
        ///
        /// \param e
        ///    The request with some `target` and `data.source`
        ///
        /// \pre The request's source must be above `_next_inner` or `nil`.
        ///
        /// \see request request_with_data
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        push(const typename outer_roots_t::value_type& e)
        {
          adiar_assert(e.data.source.is_nil() || e.data.source.label() < _next_inner);
          if (e.target.first().is_terminal()) {
#ifdef ADIAR_STATS
            nested_sweeping::stats.inner_down.requests.terminals += 1u;
#endif
            _outer_pq.push({ e.data.source, e.target.first() });
          } else {
#ifdef ADIAR_STATS
            const size_t modifying = e.targets() > 1; // 0 or 1
            nested_sweeping::stats.inner_down.requests.modifying += modifying;
            nested_sweeping::stats.inner_down.requests.preserving += 1 - modifying; // !modifying
#endif
            _outer_roots.push(e);
          }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue has a current level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_current_level() const
        {
          return _outer_pq.has_current_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the current level from the priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        level_type
        current_level() const
        {
          return _outer_pq.current_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Set up the next nonempty level in the priority queue and the sorter (down to the
        ///        given `stop_level`).
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        setup_next_level(level_type stop_level = no_label)
        {
          _outer_pq.setup_next_level(stop_level);
          adiar_assert(_next_inner <= _outer_pq.current_level(),
                       "Outer PQ stays below next Inner Level");
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue and the sorter are empty.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        empty() const
        {
          return size() == 0u;
        }
      };

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Class to obtain from the Inner Down Sweep what is the next level (ahead of the
      ///        current one being processed).
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <typename nesting_policy>
      class inner_iterator
      {
      public:
        using level_type = typename nesting_policy::pointer_type::label_type;

      public:
        ////////////////////////////////////////////////////////////////////////
        static size_t
        memory_usage()
        {
          return level_info_ifstream<>::memory_usage();
        }

      private:
        level_info_ifstream<> _lis;
        nesting_policy& _policy_impl;

      public:
        inner_iterator(const typename nesting_policy::shared_arc_file_type& dag,
                       nesting_policy& policy_impl)
          : _lis(dag)
          , _policy_impl(policy_impl)
        {}

      public:
        static constexpr level_type end = static_cast<level_type>(-1);

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next level to do an inner sweep to pull.
        ///
        /// \returns The next inner level that should be recursed on (or `end` if none are left)
        ////////////////////////////////////////////////////////////////////////////////////////////
        level_type
        next_inner()
        {
          while (_lis.can_pull()) {
            const level_type l = _lis.pull().level();
            if (_policy_impl.has_sweep(l)) { return l; }
          }
          return end;
        }
      };
    } // namespace outer

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Algorithms and Decorators for the Inner sweeps.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    namespace inner
    {
      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief   Decorator for a (levelized) priority queue that merges the requests created
      ///          within the inner sweep with the (sorted) list of requests from outer sweep.
      ///
      /// \details This is to be used as the (levelized) priority queue for the *inner* top-down
      ///          sweep.
      ///
      /// \see levelized_priority_queue, nested_sweep
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <typename inner_pq_t, typename outer_roots_t>
      class down__pq_decorator
      {
      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using value_type = typename inner_pq_t::value_type;

        // TODO: static_assert(inner_pq_t::value_type == outer_roots_t::value_type);

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using value_comp_type = typename inner_pq_t::value_comp_type;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Memory mode (same as decorated priority queue).
        ////////////////////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode mem_mode = inner_pq_t::mem_mode;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements in the priority queue / sorter.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using level_type = typename value_type::pointer_type::label_type;

      private:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the inner sweep's (levelized) priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        // TODO (optimisation): public inheritance?
        inner_pq_t& _inner_pq;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Instantiation of the comparator between elements.
        ////////////////////////////////////////////////////////////////////////////////////////////
        value_comp_type _v_comparator = value_comp_type();

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the sorter in the outer sweep that contains the root requests
        ///        generated for the next inner sweep.
        ////////////////////////////////////////////////////////////////////////////////////////////
        outer_roots_t& _outer_roots;

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Value to reflect 'out of levels'.
        ////////////////////////////////////////////////////////////////////////////////////////////
        static constexpr level_type no_label = inner_pq_t::no_label;

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Instantiate decorator.
        ////////////////////////////////////////////////////////////////////////////////////////////
        down__pq_decorator(inner_pq_t& inner_pq, outer_roots_t& outer_roots)
          : _inner_pq(inner_pq)
          , _outer_roots(outer_roots)
        {}

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there is any current level to pull elements from.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_current_level() const
        {
          return _inner_pq.has_current_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief The label of the current level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        level_type
        current_level() const
        {
          return _inner_pq.current_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are any more (possibly all empty) levels.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_next_level() /*const*/
        {
          return _inner_pq.has_next_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief The label of the next (possibly empty) level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        level_type
        next_level() /*const*/
        {
          return _inner_pq.next_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether one can push elements.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_push() /*const*/
        {
          return _inner_pq.can_push();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Push request to inner priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        push(const value_type& e)
        {
          _inner_pq.push(e);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Set up the next nonempty level in the priority queue and the sorter (down to the
        ///        given `stop_level`).
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        setup_next_level(level_type stop_level = no_label)
        {
          if (_outer_roots.can_pull()) {
            stop_level = std::min(stop_level, _outer_roots.top().level());
          }
          _inner_pq.setup_next_level(stop_level);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether this level is empty of requests.
        ///
        /// \pre has_current_level
        ///
        /// \see can_pull, empty
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        empty_level() const
        {
          return _inner_pq.empty_level()
            && (!_outer_roots.can_pull() || _outer_roots.top().level() != current_level());
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there is a top element in the priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_top() const
        {
          return !empty_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the top request of this level.
        ///
        /// \pre `has_top() == true`
        ////////////////////////////////////////////////////////////////////////////////////////////
        value_type
        top()
        {
          return __pq_first() ? _inner_pq.top() : __essential(_outer_roots.top());
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the top request of this level.
        ///
        /// \pre `has_top() == true`
        ////////////////////////////////////////////////////////////////////////////////////////////
        value_type
        peek()
        {
          return top();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are more requests to pull for this level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_pull() const
        {
          return has_top();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain and remove the top request of this level.
        ///
        /// \pre `can_pull() == true` (or `has_top() == true`)
        ////////////////////////////////////////////////////////////////////////////////////////////
        value_type
        pull()
        {
          return __pq_first() ? _inner_pq.pull() : __essential(_outer_roots.pull());
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Remove the top arc on the current level.
        ///
        /// \pre `can_pull() == true` (or `has_top() == true`)
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        pop()
        {
          pull();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief The number of requests in the priority queue and the sorter.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        size() /*const*/
        {
          return _inner_pq.size() + _outer_roots.size();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue and the sorter are empty.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        empty() /*const*/
        {
          return _inner_pq.empty() && _outer_roots.empty();
        }

      private:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Predicate whether to take the next element from the priority queue or not. If
        ///        not, it should be take from the sorter.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        __pq_first()
        {
          if (_inner_pq.empty_level()) { return false; }
          if (!_outer_roots.can_pull()) { return true; }

          const level_type _outer_roots_level = _outer_roots.top().level();
          if (_outer_roots_level != current_level()) { return true; }

          return _v_comparator(_inner_pq.top(), _outer_roots.top());
        }

        value_type
        __essential(const value_type& r)
        {
          // TODO: generalize into a `map(r.target, essential)` in
          //       <adiar/internal/data_types/tuple.h>. Ensure everything can be inlined at
          //       compile-time.
          static_assert(value_type::cardinality <= 2,
                        "No need to support higher cardinality (yet)");

          typename value_type::target_t r_tgt;
          if constexpr (value_type::cardinality == 1) {
            r_tgt = typename value_type::target_t(essential(r.target[0]));
          } else { // if constexpr (value_type::cardinality == 2) {
            r_tgt = typename value_type::target_t(essential(r.target[0]), essential(r.target[1]));
          }
          return value_type(r_tgt, {}, r.data);
        }
      };

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Start the nested sweep given initial recursions in `outer_roots` on DAG in
      ///        `outer_file`.
      ///
      /// \par Side Effects:
      ///      Sorts and resets the given `outer_roots` sorter.
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <typename nesting_policy, typename outer_roots_t>
      typename nesting_policy::__dd_type
      down(const exec_policy& ep,
           nesting_policy& policy_impl,
           const typename nesting_policy::shared_node_file_type& outer_file,
           outer_roots_t& outer_roots,
           const size_t inner_memory)
      {
        adiar_assert(outer_roots.size() > 0, "Nested Sweep needs one or more roots");

        outer_roots.sort();

        // Since we do not know anything about policy_impl, we cannot be sure of what type for its
        // priority queue is optimal at this point in time. For example, it might choose to use
        // random access which changes the sorting predicate (and hence the type).
        //
        // To avoid having to do the boiler-plate yourself, use `down__sweep_switch` below (assuming
        // your algorithm fits).
        const typename nesting_policy::__dd_type res =
          policy_impl.sweep(ep, outer_file, outer_roots, inner_memory);

        outer_roots.reset();
        return res;
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Implements the default boiler-plate behaviour in the Inner Down Sweep to (1) switch
      ///        based on `memory_mode` and (2) decorate the priority queue and the `roots_sorter`.
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <typename nesting_policy, typename outer_roots_t>
      inline typename nesting_policy::__dd_type
      down__sweep_switch(const exec_policy& ep,
                         nesting_policy& policy_impl,
                         const typename nesting_policy::shared_node_file_type& outer_file,
                         outer_roots_t& outer_roots,
                         const size_t inner_memory,
                         statistics::__alg_base::__lpq_t& lpq_stats)
      {
        // -----------------------------------------------------------------------------------------

        // Compute amount of memory available for auxiliary data structures after having opened all
        // streams.
        //
        // We then may derive an upper bound on the size of auxiliary data structures and check
        // whether we can run them with a faster internal memory variant.
        const size_t inner_stream_memory = nesting_policy::stream_memory();
        adiar_assert(inner_stream_memory <= inner_memory,
                     "There should be enough memory to include all streams");

        // -----------------------------------------------------------------------------------------
        // Determine memory for priority queue depending on whether random access can also be used.
        const size_t minimum_inner_pq_memory =
          policy_impl.pq_memory(inner_memory - inner_stream_memory);
        adiar_assert(minimum_inner_pq_memory <= inner_memory - inner_stream_memory,
                     "There should be enough memory to include all streams and priority queue");

        const size_t inner_ra_memory = policy_impl.ra_memory(outer_file);
        const bool use_random_access =
          ep.template get<exec_policy::access>() == exec_policy::access::Random_Access
          || (inner_ra_memory <= inner_memory - inner_stream_memory - minimum_inner_pq_memory
              && ep.template get<exec_policy::access>() == exec_policy::access::Auto
              && outer_file->indexable);

        if (use_random_access) {
#ifdef ADIAR_STATS
          stats.inner_down.ra_runs += 1u;
#endif
        } else {
#ifdef ADIAR_STATS
          stats.inner_down.pq_runs += 1u;
#endif
        }

        const size_t inner_pq_memory = use_random_access
          ? inner_memory - inner_stream_memory - inner_ra_memory
          : minimum_inner_pq_memory;

        const size_t inner_remaining_memory = inner_memory - inner_stream_memory - inner_pq_memory;

        // -----------------------------------------------------------------------------------------
        // Set up cross-level priority queue for either type of sweep
        const size_t inner_pq_bound = policy_impl.pq_bound(outer_file, outer_roots.size());

        const size_t inner_pq_fits =
          nesting_policy::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(
            inner_pq_memory);

        const bool external_only =
          ep.template get<exec_policy::memory>() == exec_policy::memory::External;
        const bool internal_only =
          ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;

        const size_t inner_pq_max_size =
          internal_only ? std::min(inner_pq_fits, inner_pq_bound) : inner_pq_bound;

        // TODO (bdd_compose): ask 'nesting_policy' implementation for the initalizer list
        if (!external_only
            && inner_pq_max_size <= no_lookahead_bound(outer_roots_t::value_type::cardinality)) {
#ifdef ADIAR_STATS
          lpq_stats.unbucketed += 1u;
#endif
          adiar_assert(inner_pq_max_size <= inner_pq_fits,
                       "'no_lookahead' implies it should (in practice) satisfy the '<='");

          using inner_pq_t = typename nesting_policy::template pq_t<0, memory_mode::Internal>;
          inner_pq_t inner_pq({ typename nesting_policy::dd_type(outer_file) },
                              inner_pq_memory,
                              inner_pq_max_size,
                              lpq_stats);

          using decorator_t = down__pq_decorator<inner_pq_t, outer_roots_t>;
          decorator_t decorated_pq(inner_pq, outer_roots);

          return use_random_access
            ? policy_impl.sweep_ra(ep, outer_file, decorated_pq, inner_remaining_memory)
            : policy_impl.sweep_pq(ep, outer_file, decorated_pq, inner_remaining_memory);
        } else if (!external_only && inner_pq_max_size <= inner_pq_fits) {
#ifdef ADIAR_STATS
          lpq_stats.internal += 1u;
#endif
          using inner_pq_t =
            typename nesting_policy::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>;
          inner_pq_t inner_pq({ typename nesting_policy::dd_type(outer_file) },
                              inner_pq_memory,
                              inner_pq_max_size,
                              lpq_stats);

          using decorator_t = down__pq_decorator<inner_pq_t, outer_roots_t>;
          decorator_t decorated_pq(inner_pq, outer_roots);

          return use_random_access
            ? policy_impl.sweep_ra(ep, outer_file, decorated_pq, inner_remaining_memory)
            : policy_impl.sweep_pq(ep, outer_file, decorated_pq, inner_remaining_memory);
        } else {
#ifdef ADIAR_STATS
          lpq_stats.external += 1u;
#endif
          using inner_pq_t =
            typename nesting_policy::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>;
          inner_pq_t inner_pq({ typename nesting_policy::dd_type(outer_file) },
                              inner_pq_memory,
                              inner_pq_max_size,
                              lpq_stats);

          using decorator_t = down__pq_decorator<inner_pq_t, outer_roots_t>;
          decorator_t decorated_pq(inner_pq, outer_roots);

          return use_random_access
            ? policy_impl.sweep_ra(ep, outer_file, decorated_pq, inner_remaining_memory)
            : policy_impl.sweep_pq(ep, outer_file, decorated_pq, inner_remaining_memory);
        }
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Default priority queue for the Inner Up Sweep.
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <size_t look_ahead, memory_mode mem_mode>
      using up__pq_t = outer::up__pq_t<look_ahead, mem_mode>;

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief   Decorator for a (levelized) priority queue that either forwards it to an outer
      ///          one or to itself depending on whether the node is generated from the inner or the
      ///          outer sweep.
      ///
      /// \details This is to be used as the (levelized) priority queue for the *inner* bottom-down
      ///          reduce sweep.
      ///
      /// \see levelized_priority_queue, nested_sweep
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <typename inner_pq_t, typename outer_pq_t>
      class up__pq_decorator
      {
      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using value_type = typename inner_pq_t::value_type;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using value_comp_type = typename inner_pq_t::value_comp_type;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Memory mode (same as decorated priority queue).
        ////////////////////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode mem_mode = inner_pq_t::mem_mode;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements in the priority queue / sorter.
        ////////////////////////////////////////////////////////////////////////////////////////////
        using level_type = typename value_type::pointer_type::label_type;

      private:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the inner sweep's (levelized) priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        // TODO (optimisation): public inheritance?
        inner_pq_t& _inner_pq;

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Instantiation of the comparator between elements.
        ////////////////////////////////////////////////////////////////////////////////////////////
        value_comp_type _v_comparator = value_comp_type();

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the outer sweep's (levelized) priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        outer_pq_t& _outer_pq;

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Value to reflect 'out of levels'.
        ////////////////////////////////////////////////////////////////////////////////////////////
        static constexpr level_type no_label = inner_pq_t::no_label;

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Instantiate decorator.
        ////////////////////////////////////////////////////////////////////////////////////////////
        up__pq_decorator(inner_pq_t& inner_pq, outer_pq_t& outer_pq)
          : _inner_pq(inner_pq)
          , _outer_pq(outer_pq)
        {}

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there is any current level to pull elements from.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_current_level() const
        {
          return _inner_pq.has_current_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief The label of the current level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        level_type
        current_level() const
        {
          return _inner_pq.current_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are any more (possibly all empty) levels.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_next_level() /*const*/
        {
          return _inner_pq.has_next_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief The label of the next (possibly empty) level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        level_type
        next_level() /*const*/
        {
          return _inner_pq.next_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether one can push elements.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_push() const
        {
          return true;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Push request to inner priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        push(const value_type& v)
        {
          if (v.source().is_flagged()) {
#ifdef ADIAR_STATS
            stats.inner_up.outer_arcs += 1u;
#endif
            _outer_pq.push(arc(unflag(v.source()), v.target()));
          } else {
#ifdef ADIAR_STATS
            stats.inner_up.inner_arcs += 1u;
#endif
            _inner_pq.push(v);
          }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Set up the next nonempty level in the priority queue and the sorter (down to the
        ///        given `stop_level`).
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        setup_next_level(level_type stop_level = no_label)
        {
          _inner_pq.setup_next_level(stop_level);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether this level is empty of requests.
        ///
        /// \pre has_current_level
        ///
        /// \see has_top, can_pull, empty
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        empty_level() const
        {
          return _inner_pq.empty_level();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there is a top element in the priority queue.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        has_top() const
        {
          return _inner_pq.can_pull();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the top request of this level.
        ///
        /// \pre `has_top() == true`
        ////////////////////////////////////////////////////////////////////////////////////////////
        value_type
        top()
        {
          return _inner_pq.top();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the top request of this level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////////////////////////
        value_type
        peek()
        {
          return _inner_pq.peek();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are more requests to pull for this level.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_pull() const
        {
          return has_top();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain and remove the top request of this level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////////////////////////
        value_type
        pull()
        {
          return _inner_pq.pull();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Remove the top arc on the current level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////////////////////////
        void
        pop()
        {
          _inner_pq.pop();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Number of terminals (of each type) placed within either of the priority queues.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        terminals(const bool terminal_value) const
        {
          return _outer_pq.terminals(terminal_value) + _inner_pq.terminals(terminal_value);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Number of elements in both priority queues.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        size() const
        {
          return _outer_pq.size() + _inner_pq.size();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Total number of arcs (across all levels) ignoring terminals
        ///        in both priority queues.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        size_without_terminals() const
        {
          return _outer_pq.size_without_terminals() + _inner_pq.size_without_terminals();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue and the sorter are empty.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        empty() /*const*/
        {
          return _inner_pq.empty();
        }
      };

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Decorator for inner and outer unreduced files to provide access to the arcs from
      ///        inner sweep's file while still providing the meta data of both.
      //////////////////////////////////////////////////////////////////////////////////////////////
      class up__arc_ifstream__decorator
      {
      private:
        arc_ifstream<>& _inner_arcs;
        const arc_ifstream<>& _outer_arcs;

      public:
        up__arc_ifstream__decorator(arc_ifstream<>& inner_arcs, const arc_ifstream<>& outer_arcs)
          : _inner_arcs(inner_arcs)
          , _outer_arcs(outer_arcs)
        {}

      public:
        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the stream contains more internal arcs.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_pull_internal() const
        {
          return _inner_arcs.can_pull_internal();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next internal arc (and move the read head).
        ////////////////////////////////////////////////////////////////////////////////////////////
        const arc
        pull_internal()
        {
          return _inner_arcs.pull_internal();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next internal arc (but do not move the read head).
        ////////////////////////////////////////////////////////////////////////////////////////////
        const arc
        peek_internal()
        {
          return _inner_arcs.peek_internal();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the number of unread terminals.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        unread_terminals() const
        {
          return _outer_arcs.unread_terminals() + _inner_arcs.unread_terminals();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the number of unread terminals of a specific value.
        ////////////////////////////////////////////////////////////////////////////////////////////
        size_t
        unread_terminals(const bool terminal_value) const
        {
          return _outer_arcs.unread_terminals(terminal_value)
            + _inner_arcs.unread_terminals(terminal_value);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the stream contains more terminal arcs.
        ////////////////////////////////////////////////////////////////////////////////////////////
        bool
        can_pull_terminal() const
        {
          return _inner_arcs.can_pull_terminal();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next arc (and move the read head).
        ////////////////////////////////////////////////////////////////////////////////////////////
        const arc
        pull_terminal()
        {
          return _inner_arcs.pull_terminal();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next arc (but do not move the read head).
        ////////////////////////////////////////////////////////////////////////////////////////////
        const arc
        peek_terminal()
        {
          return _inner_arcs.peek_terminal();
        }
      };

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Execute the Inner Up Sweep (part 2).
      ///
      /// \see nested_sweep
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <typename nesting_policy, typename inner_pq_t, typename outer_pq_t>
      inline void
      up(const exec_policy& /*ep*/,
         const arc_ifstream<>& outer_arcs,
         outer_pq_t& outer_pq,
         node_ofstream& outer_ofstream,
         const typename nesting_policy::shared_arc_file_type& inner_arcs_file,
         const size_t inner_pq_memory,
         const size_t inner_pq_max_size,
         const size_t inner_sorters_memory,
         const bool is_last_inner)
      {
#ifdef ADIAR_STATS
        stats.inner_up.sum_node_arcs += inner_arcs_file->size(0);
        stats.inner_up.sum_terminal_arcs += inner_arcs_file->size(1) + inner_arcs_file->size(2);
#endif

        // Set up input
        arc_ifstream<> inner_arcs(inner_arcs_file);
        up__arc_ifstream__decorator decorated_arcs(inner_arcs, outer_arcs);

        level_info_ifstream<> inner_levels(inner_arcs_file);

        // Set up (decorated) priority queue
        inner_pq_t inner_pq({ inner_arcs_file },
                            inner_pq_memory,
                            inner_pq_max_size,
                            nested_sweeping::stats.inner_up.lpq);

        using decorator_t = up__pq_decorator<inner_pq_t, outer_pq_t>;
        decorator_t decorated_pq(inner_pq, outer_pq);

        const size_t internal_sorter_can_fit =
          internal_sorter<node>::memory_fits(inner_sorters_memory / 2);

        // Process bottom-up each level
        while (inner_levels.can_pull()) {
          adiar_assert(decorated_arcs.can_pull_terminal() || !decorated_pq.empty(),
                       "If there is a level, then there should also be something for it.");
          const level_info inner_level_info = inner_levels.pull();

          const typename nesting_policy::label_type level = inner_level_info.level();

          adiar_assert(!decorated_pq.has_current_level() || level == decorated_pq.current_level(),
                       "level and priority queue should be in sync");

#ifdef ADIAR_STATS
          nested_sweeping::stats.inner_up.reduced_levels += 1u;
#endif

          if ((!nesting_policy::final_canonical || !is_last_inner) && nesting_policy::fast_reduce) {
#ifdef ADIAR_STATS
            nested_sweeping::stats.inner_up.reduced_levels__fast += 1u;
#endif

            nested_sweeping::__reduce_level__fast<nesting_policy>(
              decorated_arcs, level, decorated_pq, outer_ofstream, stats.inner_up);
          } else {
            const size_t unreduced_width = inner_level_info.width();

            if (unreduced_width <= internal_sorter_can_fit) {
              __reduce_level<nesting_policy, internal_sorter>(decorated_arcs,
                                                              level,
                                                              decorated_pq,
                                                              outer_ofstream,
                                                              inner_sorters_memory,
                                                              unreduced_width,
                                                              stats.inner_up);
            } else {
              __reduce_level<nesting_policy, external_sorter>(decorated_arcs,
                                                              level,
                                                              decorated_pq,
                                                              outer_ofstream,
                                                              inner_sorters_memory,
                                                              unreduced_width,
                                                              stats.inner_up);
            }
          }
        }

        // Forward arcs from outer sweep that collapsed to a sink.
        while (inner_arcs.can_pull_terminal()) {
          const arc a = inner_arcs.pull_terminal();
          adiar_assert(a.source().is_flagged(),
                       "Left-over terminal arcs are meant for outer sweep");
          outer_pq.push(arc(unflag(a.source()), a.target()));
        }
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief Execute the Inner Up Sweep (part 1).
      ///
      /// \see nested_sweep
      //////////////////////////////////////////////////////////////////////////////////////////////
      template <typename nesting_policy, typename outer_pq_t>
      void
      up(const exec_policy& ep,
         const arc_ifstream<>& outer_arcs,
         outer_pq_t& outer_pq,
         node_ofstream& outer_ofstream,
         const typename nesting_policy::shared_arc_file_type& inner_arcs_file,
         const size_t inner_memory,
         const bool is_last_inner)
      {
        // Compute amount of memory available for auxiliary data structures after having opened all
        // streams.
        //
        // We then may derive an upper bound on the size of auxiliary data structures and check
        // whether we can run them with a faster internal memory variant.

        const size_t inner_aux_available_memory =
          inner_memory - arc_ifstream<>::memory_usage() - level_info_ifstream<>::memory_usage();

        const size_t inner_pq_memory = inner_aux_available_memory / 2;
        const size_t inner_sorters_memory =
          inner_aux_available_memory - inner_pq_memory - iofstream<mapping>::memory_usage();

        const size_t inner_pq_memory_fits =
          up__pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(inner_pq_memory);

        // TODO (optimization):
        //   Resolve `inner_arcs_file->max_1level_cut == 0` in a separate while-loop.
        size_t inner_pq_bound = std::max<size_t>(inner_arcs_file->max_1level_cut, 1);

        // If 'fast_reduce' will be used, then the priority queue is a 2-level cut
        if constexpr (nesting_policy::fast_reduce) {
          if (!nesting_policy::final_canonical || !is_last_inner) {
            inner_pq_bound = (inner_pq_bound * 3u) / 2u;
          }
        }

        const size_t inner_pq_max_size =
          ep.template get<exec_policy::memory>() == exec_policy::memory::Internal
          ? std::min(inner_pq_memory_fits, inner_pq_bound)
          : inner_pq_bound;

        const bool external_only =
          ep.template get<exec_policy::memory>() == exec_policy::memory::External;

        if (!external_only && inner_pq_max_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
          stats.inner_up.lpq.unbucketed += 1u;
#endif
          using inner_pq_t = up__pq_t<0, memory_mode::Internal>;
          return up<nesting_policy, inner_pq_t>(ep,
                                                outer_arcs,
                                                outer_pq,
                                                outer_ofstream,
                                                inner_arcs_file,
                                                inner_pq_memory,
                                                inner_pq_max_size,
                                                inner_sorters_memory,
                                                is_last_inner);

        } else if (!external_only && inner_pq_max_size <= inner_pq_memory_fits) {
#ifdef ADIAR_STATS
          stats.inner_up.lpq.internal += 1u;
#endif
          using inner_pq_t = up__pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>;
          return up<nesting_policy, inner_pq_t>(ep,
                                                outer_arcs,
                                                outer_pq,
                                                outer_ofstream,
                                                inner_arcs_file,
                                                inner_pq_memory,
                                                inner_pq_max_size,
                                                inner_sorters_memory,
                                                is_last_inner);
        } else {
#ifdef ADIAR_STATS
          stats.inner_up.lpq.external += 1u;
#endif
          using inner_pq_t = up__pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>;
          return up<nesting_policy, inner_pq_t>(ep,
                                                outer_arcs,
                                                outer_pq,
                                                outer_ofstream,
                                                inner_arcs_file,
                                                inner_pq_memory,
                                                inner_pq_max_size,
                                                inner_sorters_memory,
                                                is_last_inner);
        }
      }
    } // namespace inner
  } // namespace nested_sweeping

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Primary logic for the framework for I/O-efficient translation of recursive functions
  ///        that re-recurse on (a single) intermediate result.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename nesting_policy, size_t outer_look_ahead, memory_mode outer_mem_mode>
  typename nesting_policy::dd_type
  __nested_sweep(const exec_policy& ep,
                 const typename nesting_policy::shared_arc_file_type& dag,
                 nesting_policy& policy_impl,
                 const size_t outer_pq_memory,
                 const size_t outer_roots_memory,
                 const size_t outer_pq_roots_max,
                 const size_t inner_memory)
  {
    using level_type            = typename nesting_policy::label_type;
    using reduced_t             = typename nesting_policy::dd_type;
    using unreduced_t           = typename nesting_policy::__dd_type;
    using request_t             = typename nesting_policy::request_t;
    using request_pred_t        = typename nesting_policy::request_pred_t;
    using shared_arc_file_type  = typename nesting_policy::shared_arc_file_type;
    using shared_node_file_type = typename nesting_policy::shared_node_file_type;

    using inner_iter_t = nested_sweeping::outer::inner_iterator<nesting_policy>;
    inner_iter_t inner_iter(dag, policy_impl);

    level_type next_inner = inner_iter.next_inner();

    // If there are no levels to do an inner sweep, then bail out with the classic Reduce sweep.
    if (next_inner == inner_iter_t::end) {
#ifdef ADIAR_STATS
      nested_sweeping::stats.skips += 1u;
#endif
      return reduce(ep, policy_impl, typename nesting_policy::__dd_type(dag, ep));
    }
#ifdef ADIAR_STATS
    nested_sweeping::stats.runs += 1u;

    nested_sweeping::stats.outer_up.sum_node_arcs += dag->size(0);
    nested_sweeping::stats.outer_up.sum_terminal_arcs += dag->size(1) + dag->size(2);
#endif

    // Set up input
    arc_ifstream<> outer_arcs(dag);

    // Set up (intermediate) output
    shared_node_file_type outer_file = __reduce_init_output<nesting_policy>();

    node_ofstream outer_ofstream(outer_file);

    // Outer Up Sweep: Obtain access to the levels to get the inputs width.
    level_info_ifstream outer_levels(dag);

    // Outer Up Sweep: Instantiate the (levelized) priority queue and other Reduce state variables,
    //                 e.g. i-level cuts.
    adiar_assert(outer_pq_memory + outer_roots_memory + inner_memory < memory_available(),
                 "Enough memory should be left priority queue and inner sweep");

    using outer_pq_t = nested_sweeping::outer::up__pq_t<outer_look_ahead, outer_mem_mode>;
    outer_pq_t outer_pq(
      { dag }, outer_pq_memory, outer_pq_roots_max, nested_sweeping::stats.outer_up.lpq);

    // Outer Up Sweep: Use the 'inner_memory' for the per-level data structures
    // and streams. This is safe, since these data structures are only
    // instantiated when the Inner Sweep is not.
    const size_t outer_sorters_memory = inner_memory - iofstream<mapping>::memory_usage();

    const size_t outer_internal_sorter_can_fit =
      internal_sorter<node>::memory_fits(outer_sorters_memory / 2);

    // TODO: assert enough memory is still available for the remaining things

    // Inner Down Sweep:
    //
    // 1. We need to create requests for roots of the Inner Down Sweep in (i) the Outer Down Sweep
    //    and (ii) the Inner Up Sweep.
    //
    // 2. We want to change the memory type based on i-level cuts if possible. To keep this at the
    //    compile-time level, we cannot instantiate the priority queue at this level of the
    //    algorithm.
    //
    // Hence, we will have a special sorter of all nested root requests which is then merged with
    // the recursion requests within the Inner Down Sweep.
    //
    // The arcs in 'outer_roots' are always between nodes that cross some level of the input. That
    // is, these arcs always constitute a 1-level cut of the input.
    using outer_roots_t =
      nested_sweeping::outer::roots_sorter<outer_mem_mode, request_t, request_pred_t>;
    outer_roots_t outer_roots(outer_roots_memory, outer_pq_roots_max);

    // Decorator that acts as 'outer_pq' but moves arcs to 'outer_roots'.
    using outer_pq_decorator_t =
      nested_sweeping::outer::up__pq_decorator<outer_pq_t, outer_roots_t>;

    while (outer_levels.can_pull()) {
      adiar_assert(outer_arcs.can_pull_terminal() || !outer_pq.empty(),
                   "If there is a level, then there should also be something for it.");

      // Set up next level for outer reduce

      const level_info outer_level = outer_levels.pull();

      adiar_assert(!outer_pq.has_current_level() || outer_level.level() == outer_pq.current_level(),
                   "level and priority queue should be in sync");
      adiar_assert(next_inner == inner_iter_t::end || next_inner <= outer_level.level(),
                   "next_inner level should (if it exists) be above current level (inclusive).");

      const level_type out_level = policy_impl.map_level(outer_level.level());

      // -------------------------------------------------------------------------------------------
      // CASE: Unnested Level with no nested sweep above:
      //   Reduce this level (without decorators).
      if (next_inner == inner_iter_t::end) {
#ifdef ADIAR_STATS
        nested_sweeping::stats.outer_up.reduced_levels += 1u;
#endif

        if constexpr (!nesting_policy::final_canonical && nesting_policy::fast_reduce) {
#ifdef ADIAR_STATS
          nested_sweeping::stats.outer_up.reduced_levels__fast += 1u;
#endif

          nested_sweeping::__reduce_level__fast<nesting_policy>(outer_arcs,
                                                                outer_level.level(),
                                                                out_level,
                                                                outer_pq,
                                                                outer_ofstream,
                                                                nested_sweeping::stats.outer_up);
        } else {
          const size_t unreduced_width = outer_level.width();

          if (unreduced_width <= outer_internal_sorter_can_fit) {
            __reduce_level<nesting_policy, internal_sorter>(outer_arcs,
                                                            outer_level.level(),
                                                            out_level,
                                                            outer_pq,
                                                            outer_ofstream,
                                                            outer_sorters_memory,
                                                            unreduced_width,
                                                            nested_sweeping::stats.outer_up);
          } else {
            __reduce_level<nesting_policy, external_sorter>(outer_arcs,
                                                            outer_level.level(),
                                                            out_level,
                                                            outer_pq,
                                                            outer_ofstream,
                                                            outer_sorters_memory,
                                                            unreduced_width,
                                                            nested_sweeping::stats.outer_up);
          }
        }

        continue;
      }

      // -------------------------------------------------------------------------------------------
      // CASE: Unnested Level with a nested sweep above:
      //   Reduce this level (but with a decorator to redirect requests).
      if (next_inner < outer_level.level()) {
        outer_pq_decorator_t outer_pq_decorator(outer_pq, outer_roots, next_inner);

#ifdef ADIAR_STATS
        nested_sweeping::stats.outer_up.reduced_levels += 1u;
#endif

        if constexpr (nesting_policy::fast_reduce) {
#ifdef ADIAR_STATS
          nested_sweeping::stats.outer_up.reduced_levels__fast += 1u;
#endif

          nested_sweeping::__reduce_level__fast<nesting_policy>(outer_arcs,
                                                                outer_level.level(),
                                                                out_level,
                                                                outer_pq_decorator,
                                                                outer_ofstream,
                                                                nested_sweeping::stats.outer_up);
        } else {
          const size_t unreduced_width = outer_level.width();

          if (unreduced_width <= outer_internal_sorter_can_fit) {
            __reduce_level<nesting_policy, internal_sorter>(outer_arcs,
                                                            outer_level.level(),
                                                            out_level,
                                                            outer_pq_decorator,
                                                            outer_ofstream,
                                                            outer_sorters_memory,
                                                            unreduced_width,
                                                            nested_sweeping::stats.outer_up);
          } else {
            __reduce_level<nesting_policy, external_sorter>(outer_arcs,
                                                            outer_level.level(),
                                                            out_level,
                                                            outer_pq_decorator,
                                                            outer_ofstream,
                                                            outer_sorters_memory,
                                                            unreduced_width,
                                                            nested_sweeping::stats.outer_up);
          }
        }

        continue;
      }

      // -------------------------------------------------------------------------------------------
      // CASE Nested Level:
      //   Sweep down re-reduce it back up to this level.
      adiar_assert(outer_level.level() == next_inner, "'next_inner' level is not skipped");

      // -----------------------------------------------------------------------
      // Collect all recursions for this level

      // Keep track of whether a non-GC request has been made.
      bool non_gc_request = false;
      {
        outer_pq_decorator_t outer_pq_decorator(outer_pq, outer_roots, next_inner);

        // TODO (bdd_compose):
        //   Support creating nodes on this level.

        while ((outer_arcs.can_pull_terminal()
                && outer_arcs.peek_terminal().source().label() == outer_level.level())
               || outer_pq.can_pull()) {

          const arc e_high = __reduce_get_next(outer_pq, outer_arcs);
          const arc e_low  = __reduce_get_next(outer_pq, outer_arcs);

          const node n = node_of(e_low, e_high);

          // Apply Reduction rule 1?
          const node::pointer_type reduction_rule_ret = nesting_policy::reduction_rule(n);
          if (reduction_rule_ret != n.uid()) {
            // If so, preserve child in inner sweep
            if (!outer_levels.can_pull()) {
              adiar_assert(!outer_arcs.can_pull_internal(),
                           "Should not have any parents at top-most level");

              if (reduction_rule_ret.is_terminal()) {
                return reduced_t(reduction_rule_ret.value());
              }
#ifdef ADIAR_STATS
              nested_sweeping::stats.inner_down.removed_by_rule_1 += 1u;
#endif
              outer_pq_decorator.push(arc(node::pointer_type::nil(), flag(reduction_rule_ret)));
            } else {
              do {
#ifdef ADIAR_STATS
                nested_sweeping::stats.inner_down.removed_by_rule_1 += 1u;
#endif
                outer_pq_decorator.push(
                  arc(outer_arcs.pull_internal().source(), flag(reduction_rule_ret)));
              } while (outer_arcs.can_pull_internal()
                       && outer_arcs.peek_internal().target() == n.uid());
            }
          } else {
            // Otherwise, create request
            if (!outer_levels.can_pull()) {
              adiar_assert(!outer_arcs.can_pull_internal(),
                           "Should not have any parents at top-most level");

              const request_t r = policy_impl.request_from_node(n, node::pointer_type::nil());

              adiar_assert(r.targets() > 0, "Requests are always to something");
              non_gc_request |= r.targets() > 1;

              if (r.target.first().is_terminal()) { return reduced_t(r.target.first().value()); }
              outer_pq_decorator.push(r);
            } else {
              do {
                const request_t r =
                  policy_impl.request_from_node(n, outer_arcs.pull_internal().source());

                adiar_assert(r.targets() > 0, "Requests are always to something");
                non_gc_request |= r.targets() > 1;

                outer_pq_decorator.push(r);
              } while (outer_arcs.can_pull_internal()
                       && outer_arcs.peek_internal().target() == n.uid());
            }
          }
        }
      }

      // -----------------------------------------------------------------------
      // Obtain the next level to sweep nestedly on.
      next_inner = inner_iter.next_inner();

      const bool is_last_inner = next_inner == inner_iter_t::end;

      const bool run_inner =
        // We have some requests to sweep on
        outer_roots.size() > 0
        // There are some requests that change the subgraph, or this is the last
        // down sweep (meaning we have to go down to ensure dead subtrees are
        // garbage collected and nodes are canonical)
        && (non_gc_request || is_last_inner);

      if (run_inner) {
        // ---------------------------------------------------------------------
        // Inner Down Sweep
#ifdef ADIAR_STATS
        nested_sweeping::stats.outer_up.nested_levels += 1u;
#endif
        adiar_assert(outer_roots.size() > 0, "Nested Sweep needs some number of requests");

        outer_ofstream.close();

#ifdef ADIAR_STATS
        nested_sweeping::stats.inner_down.inputs.acc_size += outer_file->size();
        nested_sweeping::stats.inner_down.inputs.max_size =
          std::max<uintwide>(nested_sweeping::stats.inner_down.inputs.max_size, outer_file->size());
        nested_sweeping::stats.inner_down.inputs.acc_width += outer_file->width;
        nested_sweeping::stats.inner_down.inputs.max_width =
          std::max<uintwide>(nested_sweeping::stats.inner_down.inputs.max_width, outer_file->width);
        nested_sweeping::stats.inner_down.inputs.acc_levels += outer_file->levels();
        nested_sweeping::stats.inner_down.inputs.max_levels = std::max<uintwide>(
          nested_sweeping::stats.inner_down.inputs.max_levels, outer_file->levels());
#endif

        // TODO (optimisation): is_last_inner && !non_gc_request
        //   Use a simpler (and hence faster?) algorithm for a GC-only sweep.

        const unreduced_t inner_unreduced =
          nested_sweeping::inner::down(ep, policy_impl, outer_file, outer_roots, inner_memory);

        if (inner_unreduced.template has<shared_node_file_type>()) {
          adiar_assert(!outer_levels.can_pull(),
                       "Should only collapse to a node file case when at the very top-level.");
          adiar_assert(inner_unreduced.template get<shared_node_file_type>()->is_terminal(),
                       "Should have collapsed to a terminal.");

          return inner_unreduced.template get<shared_node_file_type>();
        }

        // ---------------------------------------------------------------------
        // Inner Up Sweep
        adiar_assert(!inner_unreduced.template has<no_file>(), "Inner Sweep returned something");

        const shared_arc_file_type inner_arcs =
          inner_unreduced.template get<shared_arc_file_type>();

        outer_file = __reduce_init_output<nesting_policy>();
        outer_ofstream.open(outer_file);

        if (is_last_inner) {
          nested_sweeping::inner::up<nesting_policy>(
            ep, outer_arcs, outer_pq, outer_ofstream, inner_arcs, inner_memory, is_last_inner);
        } else {
          adiar_assert(next_inner < outer_level.level(),
                       "If 'next_inner' is not illegal, then it is above current level");

          outer_pq_decorator_t outer_pq_decorator(outer_pq, outer_roots, next_inner);

          nested_sweeping::inner::up<nesting_policy>(ep,
                                                     outer_arcs,
                                                     outer_pq_decorator,
                                                     outer_ofstream,
                                                     inner_arcs,
                                                     inner_memory,
                                                     is_last_inner);
        }
      } else if (outer_roots.size() > 0) {
        // ---------------------------------------------------------------------
        // Bail out requests for a GC-only Inner Sweep (postponing doing so for
        // a later sweep)
#ifdef ADIAR_STATS
        nested_sweeping::stats.outer_up.skipped_nested_levels += 1u;
#endif
        adiar_assert(next_inner <= nesting_policy::pointer_type::max_label,
                     "Has another later sweep to do possible garbage collection");

        adiar_assert(outer_roots_memory <= inner_memory,
                     "Enough memory is left for a temporary root sorter");

        outer_roots_t tmp_outer_roots(outer_roots_memory, outer_pq_roots_max);
        outer_pq_decorator_t outer_pq_decorator(outer_pq, tmp_outer_roots, next_inner);

        outer_roots.sort();
        while (outer_roots.can_pull()) {
          const request_t r = outer_roots.pull();
          adiar_assert(r.targets() == 1, "Has exactly one child");

          outer_pq_decorator.push(arc(unflag(r.data.source), r.target.first()));
        }

        outer_roots.move(tmp_outer_roots);
      } else { // if (outer_roots.size() == 0) {
               // ---------------------------------------------------------------------
               // Nothing within 'outer_file' should survive
#ifdef ADIAR_STATS
        nested_sweeping::stats.outer_up.skipped_nested_levels += 1u;
        nested_sweeping::stats.outer_up.skipped_nested_levels__prune += 1u;
#endif
        if (outer_ofstream.size() != 0) {
          outer_ofstream.close();
          outer_file = __reduce_init_output<nesting_policy>();
          outer_ofstream.open(outer_file);
        }
      }

      // -----------------------------------------------------------------------
      // Set up next level in Outer PQ
      if (!outer_pq.empty() || !outer_roots.empty()) {
        adiar_assert(!outer_arcs.can_pull_terminal()
                       || outer_arcs.peek_terminal().source().label() < outer_level.level(),
                     "All terminal arcs for 'label' should be processed");

        adiar_assert(!outer_arcs.can_pull_internal()
                       || outer_arcs.peek_internal().target().label() < outer_level.level(),
                     "All internal arcs for 'label' should be processed");

        adiar_assert(outer_pq.empty() || !outer_pq.can_pull(),
                     "All forwarded arcs for 'label' should be processed");

        const size_t terminal_stop_level = outer_arcs.can_pull_terminal()
          ? outer_arcs.peek_terminal().source().label()
          : outer_pq_t::no_label;

        const size_t outer_roots_stop_level =
          !outer_roots.empty() ? outer_roots.deepest_source() : outer_pq_t::no_label;

        adiar_assert(terminal_stop_level != outer_pq_t::no_label
                       || outer_roots_stop_level != outer_pq_t::no_label || !outer_pq.empty(),
                     "There must be some (known) level ready to be forwarded to.");

        const size_t stop_level = terminal_stop_level == outer_pq_t::no_label
          ? outer_roots_stop_level
          : outer_roots_stop_level == outer_pq_t::no_label
          ? terminal_stop_level
          : std::max(terminal_stop_level, outer_roots_stop_level);

        adiar_assert(stop_level != outer_pq_t::no_label || !outer_pq.empty(),
                     "There must be some (known) level ready to be forwarded to.");

        outer_pq.setup_next_level(stop_level);
      } else if (outer_file->is_terminal()) {
#ifdef ADIAR_STATS
        nested_sweeping::stats.outer_up.collapse_to_terminal += 1u;
#endif
        return outer_file;
      }
    }

    // Return (now not anymore 'intermediate') output
    outer_ofstream.close();
    return outer_file;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Framework for I/O-efficient translation of recursive functions that re-recurse on
  ///        intermediate results.
  ///
  /// \tparam nesting_policy Policy that includes (a) the semantics of the decision diagram, (b) at
  ///                        run-time a predicate for the levels to sweep on (may abuse it is only
  ///                        ever asked once per level and bottom-up), and (c) provides the logic
  ///                        for each inner downwards sweep that manipulates the graph.
  ///
  /// \param input           A possibly reduced input. If it has collapsed to a terminal, then that
  ///                        one is returned. If it is reduced, then it will be transposed before
  ///                        computation starts.
  ///
  /// \param policy_impl     Provides the non-static parts of the inner logic that determines when
  ///                        to start the nested sweep.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename nesting_policy>
  typename nesting_policy::dd_type
  nested_sweep(const exec_policy& ep,
               const typename nesting_policy::__dd_type& input,
               nesting_policy& policy_impl)
  {
    using reduced_t             = typename nesting_policy::dd_type;
    using request_t             = typename nesting_policy::request_t;
    using request_pred_t        = typename nesting_policy::request_pred_t;
    using shared_arc_file_type  = typename nesting_policy::shared_arc_file_type;
    using shared_node_file_type = typename nesting_policy::shared_node_file_type;

    adiar_assert(!input.empty(), "Input for Nested Sweeping should always be non-empty");

    // Is it a terminal?
    if (input.template has<shared_node_file_type>()
        && input.template get<shared_node_file_type>()->is_terminal()) {
      return reduced_t(input.template get<shared_node_file_type>(), input._negate);
    }

    // Otherwise obtain the semi-transposed DAG (construct it if necessary)
    const shared_arc_file_type dag = input.template has<shared_arc_file_type>()
      ? input.template get<shared_arc_file_type>()
      : transpose(reduced_t(input.template get<shared_node_file_type>(), input._negate));

    // Compute amount of memory available for auxiliary data structures after having opened all
    // streams.
    //
    // We then may derive an upper bound on the size of auxiliary data structures for the Outer Up
    // Sweep. If small enough, we can run them with a faster internal memory variant.
    //
    // Here, we still need to keep in mind, that the available memory needs to be distributed across
    // (up to) four priority queues!
    const size_t total_memory = memory_available();

    const size_t outer_memory = total_memory / 2;
    const size_t inner_memory = total_memory - outer_memory;

    const size_t aux_outer_memory = outer_memory
      // Input streams
      - arc_ifstream<>::memory_usage()
      - level_info_ifstream<>::memory_usage()
      // Inner Iterator
      - nested_sweeping::outer::inner_iterator<nesting_policy>::memory_usage()
      // Output streams
      - node_ofstream::memory_usage();

    using outer_default_lpq_t =
      nested_sweeping::outer::up__pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>;

    constexpr size_t data_structures_in_pq = outer_default_lpq_t::data_structures;

    using internal_roots_sorter_t =
      nested_sweeping::outer::roots_sorter<memory_mode::Internal, request_t, request_pred_t>;

    constexpr size_t data_structures_in_roots = internal_roots_sorter_t::data_structures;

    const size_t outer_pq_memory =
      (aux_outer_memory / (data_structures_in_pq + data_structures_in_roots))
      * data_structures_in_pq;

    const size_t outer_roots_memory = aux_outer_memory - outer_pq_memory;

    const size_t outer_pq_memory_fits = outer_default_lpq_t::memory_fits(outer_pq_memory);

    const size_t outer_roots_memory_fits = internal_roots_sorter_t::memory_fits(outer_pq_memory);

    const size_t pq_roots_bound = (dag->max_1level_cut * 3u) / 2u;

    const size_t outer_pq_roots_max =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal
      ? std::min({ outer_pq_memory_fits, outer_roots_memory_fits, pq_roots_bound })
      : pq_roots_bound;

    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;
    if (!external_only && outer_pq_roots_max <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      nested_sweeping::stats.outer_up.lpq.unbucketed += 1u;
#endif
      return __nested_sweep<nesting_policy, 0, memory_mode::Internal>(ep,
                                                                      dag,
                                                                      policy_impl,
                                                                      outer_pq_memory,
                                                                      outer_roots_memory,
                                                                      outer_pq_roots_max,
                                                                      inner_memory);
    } else if (!external_only && outer_pq_roots_max <= outer_pq_memory_fits
               && outer_pq_roots_max <= outer_roots_memory_fits) {
#ifdef ADIAR_STATS
      nested_sweeping::stats.outer_up.lpq.internal += 1u;
#endif
      return __nested_sweep<nesting_policy, ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>(
        ep,
        dag,
        policy_impl,
        outer_pq_memory,
        outer_roots_memory,
        outer_pq_roots_max,
        inner_memory);
    } else {
#ifdef ADIAR_STATS
      nested_sweeping::stats.outer_up.lpq.external += 1u;
#endif
      return __nested_sweep<nesting_policy, ADIAR_LPQ_LOOKAHEAD, memory_mode::External>(
        ep,
        dag,
        policy_impl,
        outer_pq_memory,
        outer_roots_memory,
        outer_pq_roots_max,
        inner_memory);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H
