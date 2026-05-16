#ifndef ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
#define ADIAR_INTERNAL_ALGORITHMS_REPLACE_H

#include <type_traits>

#include <adiar/exception.h>
#include <adiar/functional.h>
#include <adiar/type_traits.h>
#include <adiar/types.h>

#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_structures/priority_queue.h>
#include <adiar/internal/data_structures/vector.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/levelized_ifstream.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_ifstream.h>
#include <adiar/internal/io/node_ofstream.h>
#include <adiar/internal/io/node_raccess.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::replace_t stats_replace;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Types

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief A total mapping function.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  using replace_func = function<typename Policy::level_type(typename Policy::level_type)>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Algorithms: `Shift`
  //
  // If the decision diagram is already fully reduced and the variable ordering is a mere `Shift`,
  // i.e. the levels are offset by the same constant amount, then we can reuse the original file by
  // merely deferring the level replacement until it is read later. This saves an expensive O(N/B)
  // copy operation and disk space otherwise done for the `Monotone` case below.
  /*
  //         a           a        | x -> x+c
  //        / \     =>  / \
  //        b c         b c       | y -> y+c
  */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace the level in constant time
  ///
  /// \remark This requires that the mapping, `m`, is *monotonic* and *affine*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::dd_type
  replace__shift(const typename Policy::dd_type& dd, const replace_func<Policy>& m)
  {
    adiar_assert(!dd->is_terminal());

    const typename Policy::signed_level_type topvar         = dd_topvar(dd);
    const typename Policy::signed_level_type shifted_topvar = m(topvar);

    return typename Policy::dd_type(
      dd.file_ptr(), dd.is_negated(), dd.shift() + (shifted_topvar - topvar));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Algorithms: `Monotone`
  //
  // If the variable ordering is monotone, i.e. the levels still follow the same relative ordering,
  // then we can apply the level replacement node-for-node or as part of the reduce algorithm (which
  // has to be run anyways).
  /*
  //         a            a'        | x -> x'
  //        / \     =>   /  \
  //        b c         b'  c'      | y -> y'
  */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace the level of all nodes in a single linear scan.
  ///
  /// \remark This requires that the mapping, `m`, is *monotonic*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::dd_type
  replace__monotone(const typename Policy::dd_type& dd, const replace_func<Policy>& m)
  {
    adiar_assert(!dd->is_terminal());

    // Set up outputs
    shared_levelized_file<typename Policy::node_type> out_file;
    node_ofstream out(out_file);

    out.unsafe_set_sorted(dd->sorted);
    out.unsafe_set_indexable(dd->indexable);

    out.unsafe_set_1level_cut(
      { dd->max_1level_cut[cut::Internal],
        dd->max_1level_cut[dd.is_negated() ? cut::Internal_True : cut::Internal_False],
        dd->max_1level_cut[dd.is_negated() ? cut::Internal_False : cut::Internal_True],
        dd->max_1level_cut[cut::All] });

    { // Copy over nodes (in "reverse" to still follow the same order on disk)
      node_ifstream<true> in_nodes(dd);
      while (in_nodes.can_pull()) { out.unsafe_push(replace(in_nodes.pull(), m)); }
    }
    { // Copy over levels (also in "reverse")
      level_info_ifstream<true> in_levels(dd);
      while (in_levels.can_pull()) {
        const level_info li = in_levels.pull();
        out.unsafe_push(level_info(m(li.level()), li.width()));
      }
    }

    return out_file;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace the level of all nodes as part of the bottom-up reduce sweep.
  ///
  /// \remark This requires that the mapping, `m`, is *monotonic*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::dd_type
  replace__monotone(const exec_policy& ep,
                    const typename Policy::__dd_type& __dd,
                    const replace_func<Policy>& m)
  {
    class reduce_policy : public Policy
    {
    private:
      const replace_func<Policy>& _m;

    public:
      reduce_policy(const replace_func<Policy>& m)
        : _m(m)
      {}

      constexpr inline typename Policy::level_type
      map_level(typename Policy::level_type x) const
      {
        return this->_m(x);
      }
    };

    reduce_policy policy(m);
    return reduce(ep, policy, std::move(__dd));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Algorithms: `Jump_Down` (+ `Swap_Adjacent`)
  //
  // Top-down 2-ary product construction which works as a `prod2u` extended with logic in
  // `intercut` to move levels down.
  /*
  //             ____ O ____                      __ O __                | ...
  //            /           \                    /       \
  //          _a_          _b_                  /         \              | x -> y
  //         /   \        /   \      =>        /           \
  //         c   c'       d   d'            (c,c')        (d,d')         | ...
  //        / \ / \      / \ / \            /    \        /    \
  //        | | |  \     | | |  \        (e,e') (f,f') (g,g') (h,h')     | y
  //        | | |  |     | | |  |         /  \   /  \   /  \   /  \
  //        e f e' f'    g h g' h'        e  e' f   f'  g  g'  h  h'     | ...
  */
  // The most basic version assumes the target level is empty. This can be (thereby also supporting
  // `Swap_Adjacent`) by doubling levels in the levelized priority queue; input and untouched levels
  // are even whereas target levels are odd.
  //
  // This sweep is guaranteed to preserve the reducedness of the input! That is, if one does not
  // care about the output being *sorted*, then one can use the fast `reduce` operation.

  namespace __replace
  {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Request for jump down sweeps.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <uint8_t NodesCarried>
    class jdown_request : public request_data<2, with_parent_and_level, NodesCarried>
    {
      using base_type = request_data<2, with_parent_and_level, NodesCarried>;

    public:
      // Reuse constructors from parent
      using base_type::base_type;

      //////////////////////////////////////////////////////////////////////////////////////////////
      /// \brief   The level at which this request should be resolved.
      ///
      /// \details To support pushing levels to an occupied level, we double the levels; odd values
      ///          are then treated as target levels "in-between". Since one cannot jump from 0 to
      ///          0, we treat the odd *prior* level as the target. This, for example, allows one to
      ///          jump from 0->2 and 2->4 without having any overlaps.
      //////////////////////////////////////////////////////////////////////////////////////////////
      typename base_type::level_type
      level() const
      {
        const typename base_type::level_type base_level = base_type::level();
        return base_level - (this->data.level() < this->target.level());
      }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Wrapper of a `replace_func` into a policy.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Template with a comparator to also support bottom-up `Jump_Up` sweeps. Furthermore,
    //       depending on the comparator, we should +1 or -1 on the target level.
    template <typename Policy>
    class jump_policy : public Policy
    {
    private:
      using level_type    = typename Policy::level_type;
      using vector_type   = internal_vector<level_type>;
      using iterator_type = typename vector_type::iterator;

      /// \brief List of all levels prior to replacement.
      vector_type _before;

      /// \brief Single-read Iterator of `_before` to infer whether a level needs to jump.
      iterator_type _before_iter;

      /// \brief List of all levels after replacement.
      vector_type _after;

      /// \brief Single-read Iterator of `_after` to infer where a level needs to jump to.
      iterator_type _after_iter;

      /// \brief Sorted list of all levels after replacement; this only contains the ones that were
      ///        replaced with something other than itself.
      vector_type _jump_targets;

    public:
      static size_t
      memory_usage(typename Policy::dd_type& dd)
      {
        return 3 * vector_type::memory_usage(dd->levels());
      }

      static constexpr level_type no_level = Policy::pointer_type::nil_level;

    public:
      jump_policy(const typename Policy::dd_type& dd, const replace_func<Policy>& m)
        : _before(dd->levels())
        , _after(dd->levels())
        , _jump_targets(dd->levels())
      {
        level_info_ifstream ls(dd);

        while (ls.can_pull()) {
          this->_before.push_back(ls.pull().level());
          this->_after.push_back(m(this->_before.back()));
        }
        this->_before_iter = this->_before.begin();
        this->_after_iter  = this->_after.begin();

        iterator_type i = this->_after.begin();
        if (i != this->_after.end()) {
          for (; i + 1 != this->_after.end(); ++i) {
            if (*i < *(i + 1)) { continue; }
            this->_jump_targets.push_back(*i);
          }
        }
        std::sort(this->_jump_targets.begin(), this->_jump_targets.end());
      }

      /// \brief Creates access to the (offset) levels for the levelized priority queue.
      template <typename PriorityQueue, typename T>
      std::array<typename PriorityQueue::level_input_type, 2>
      pq_levels(const T&) const
      {
        const generator<level_type> before_gen =
          [_begin = this->_before.begin(),
           _end   = this->_before.end()]() mutable -> optional<level_type> {
          if (_begin == _end) { return {}; }
          return 2 * (*_begin++);
        };

        const generator<level_type> jumps_gen =
          [_begin = this->_jump_targets.begin(),
           _end   = this->_jump_targets.end()]() mutable -> optional<level_type> {
          if (_begin == _end) { return {}; }
          return 2 * (*_begin++) - 1;
        };

        return { before_gen, jumps_gen };
      }

      /// \brief Whether the current level (from the levelized priority queue) is a target level
      ///        for a jump.
      bool
      is_jump_target(const level_type& x)
      {
        return x % 2 != 0;
      }

      /// \brief Whether the current level (from the levelized priority queue) needs to be moved
      ///        with a jump (or can merely be remapped).
      ///
      /// \details This together with `map_level` may only be called in order of the levels.
      ///
      /// \pre `is_jump_target() == false`
      bool
      needs_jump(const level_type& x)
      {
        adiar_assert(!this->is_jump_target(x));
        adiar_assert(this->_before_iter != this->_before.end());
        adiar_assert(this->_after_iter != this->_after.end());

        const level_type unshifted_level = x / 2;
        while (unshifted_level != *this->_before_iter) {
          this->_before_iter++;
          this->_after_iter++;

          adiar_assert(this->_before_iter != this->_before.end());
          adiar_assert(this->_after_iter != this->_after.end());
        }

        const typename vector_type::iterator curr = this->_after_iter;
        const typename vector_type::iterator next = this->_after_iter + 1;

        return next == this->_after.end() ? false : *next < *curr;
      }

      /// \brief Convert the level (from levelized priority queue) back to its intended output
      ///        level.
      ///
      /// \pre `needs_jump(x)` has already been called to forward the iterators to `x`.
      level_type
      map_level(const level_type& x)
      {
        const bool is_jump_target = this->is_jump_target(x);
        const level_type unshifted_level = x / 2 + is_jump_target;
        // If it is the target of a jump, compute the result directly from `x`.
        if (is_jump_target) { return unshifted_level; }

        // Otherwise, find the level in `_before` and `_after`.
        adiar_assert(this->_before_iter != this->_before.end());
        adiar_assert(*this->_before_iter == unshifted_level);
        /*
        while (unshifted_level != *this->_before_iter) {
          this->_before_iter++;
          this->_after_iter++;
        }
        adiar_assert(this->_before_iter != this->_before.end());
        adiar_assert(this->_after_iter != this->_after.end());
        */

        return *this->_after_iter;
      }
    };

    /// \brief Type of the primary priority queue.
    template <size_t LookAhead, memory_mode MemoryMode>
    using jdown_pq1_type = levelized_node_priority_queue<jdown_request<0>,
                                                         request_data_first_lt<jdown_request<0>>,
                                                         LookAhead,
                                                         MemoryMode,
                                                         2,
                                                         0>;

    /// \brief Type of the secondary priority queue to further forward requests across a level.
    template <memory_mode MemoryMode>
    using jdown_pq2_type =
      priority_queue<MemoryMode, jdown_request<1>, request_data_second_lt<jdown_request<1>>>;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Upper bound on the primary priority queue for the `Jump_Down` case
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename Policy, typename Cut, size_t ConstSizeInc, typename In>
    size_t
    jdown__ilevel_upper_bound(const In& in)
    {
      const safe_size_t internal       = Cut::get(in, cut::type::Internal);
      const safe_size_t internal_true  = Cut::get(in, cut::type::Internal_True);
      const safe_size_t internal_false = Cut::get(in, cut::type::Internal_False);

      const safe_size_t false_only = internal_false - internal;
      const safe_size_t true_only  = internal_true - internal;

      return to_size(internal * internal + 2 * false_only * true_only
                     + 2 * (false_only + true_only) * internal + ConstSizeInc);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Upper bound on the primary priority queue for the `Adjacent_Swap` case
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename Policy, typename Cut, size_t ConstSizeInc, typename In>
    size_t
    adjswap__ilevel_upper_bound(const In& in)
    {
      return to_size(2 * Cut::get(in, cut::type::All) + ConstSizeInc);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename Policy, typename PriorityQueue_1, typename PriorityQueue_2>
    inline typename Policy::__dd_type
    jdown_pq(const exec_policy& /*ep*/,
             const typename Policy::dd_type& dd,
             Policy& /*policy*/,
             const PriorityQueue_1& /*pq1*/,
             const PriorityQueue_2& /*pq2*/)
    {
      // TODO: Sweep logic!

      return dd;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename Policy, typename PriorityQueue_1, typename PriorityQueue_2>
    inline typename Policy::__dd_type
    jdown_pq(const exec_policy& ep,
             const typename Policy::dd_type& dd,
             const replace_func<Policy>& m,
             const size_t pq1_memory,
             const size_t pq1_max_size,
             const size_t pq2_memory,
             const size_t pq2_max_size)
    {
      jump_policy<Policy> policy(dd, m);

      PriorityQueue_1 pq1(policy.template pq_levels<PriorityQueue_1>(dd),
                          pq1_memory,
                          pq1_max_size,
                          stats_replace.lpq);
      // TODO: Root request!

      PriorityQueue_2 pq2(pq2_memory, pq2_max_size);

      return jdown_pq(ep, dd, policy, pq1, pq2);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename Policy>
    inline typename Policy::__dd_type
    jdown_pq(const exec_policy& ep,
             const typename Policy::dd_type& dd,
             const replace_func<Policy>& m,
             const size_t pq1_bound,
             const size_t pq2_bound)
    {
      // Compute amount of memory available for auxiliary data structures after having opened all
      // streams.
      //
      // We then may derive an upper bound on the size of auxiliary data structures and check
      // whether we can run them with a faster internal memory variant.
      const size_t aux_available_memory = memory_available()
        // Input stream
        - node_ifstream<>::memory_usage()
        // Output stream
        - arc_ofstream::memory_usage();

      constexpr size_t data_structures_in_pq_1 =
        jdown_pq1_type<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

      constexpr size_t data_structures_in_pq_2 =
        jdown_pq2_type<memory_mode::Internal>::data_structures;

      const size_t pq_1_internal_memory =
        (aux_available_memory / (data_structures_in_pq_1 + data_structures_in_pq_2))
        * data_structures_in_pq_1;

      const size_t pq_1_memory_fits =
        jdown_pq1_type<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(
          pq_1_internal_memory);

      const size_t pq_2_internal_memory = aux_available_memory - pq_1_internal_memory;

      const size_t pq_2_memory_fits =
        jdown_pq2_type<memory_mode::Internal>::memory_fits(pq_2_internal_memory);

      const bool internal_only =
        ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
      const bool external_only =
        ep.template get<exec_policy::memory>() == exec_policy::memory::External;

      const size_t max_pq_1_size =
        internal_only ? std::min(pq_1_memory_fits, pq1_bound) : pq1_bound;

      const size_t max_pq_2_size =
        internal_only ? std::min(pq_2_memory_fits, pq2_bound) : pq2_bound;

      if (!external_only && max_pq_1_size <= no_lookahead_bound(2)) {
#ifdef ADIAR_STATS
        stats_replace.lpq.unbucketed += 1u;
#endif
        using PriorityQueue_1 = jdown_pq1_type<0, memory_mode::Internal>;
        using PriorityQueue_2 = jdown_pq2_type<memory_mode::Internal>;

        return jdown_pq<Policy, PriorityQueue_1, PriorityQueue_2>(
          ep, dd, m, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      } else if (!external_only && max_pq_1_size <= pq_1_memory_fits
                 && max_pq_2_size <= pq_2_memory_fits) {
#ifdef ADIAR_STATS
        stats_replace.lpq.internal += 1u;
#endif
        using PriorityQueue_1 = jdown_pq1_type<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>;
        using PriorityQueue_2 = jdown_pq2_type<memory_mode::Internal>;

        return jdown_pq<Policy, PriorityQueue_1, PriorityQueue_2>(
          ep, dd, m, pq_1_internal_memory, max_pq_1_size, pq_2_internal_memory, max_pq_2_size);
      } else {
#ifdef ADIAR_STATS
        stats_replace.lpq.external += 1u;
#endif
        using PriorityQueue_1 = jdown_pq1_type<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>;
        using PriorityQueue_2 = jdown_pq2_type<memory_mode::External>;

        const size_t pq_1_memory = aux_available_memory / 2;
        const size_t pq_2_memory = pq_1_memory;

        return jdown_pq<Policy, PriorityQueue_1, PriorityQueue_2>(
          ep, dd, m, pq_1_memory, max_pq_1_size, pq_2_memory, max_pq_2_size);
      }
    }

    // TODO: Random-access optimization

    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename Policy>
    inline typename Policy::__dd_type
    jdown(const exec_policy& ep,
          const typename Policy::dd_type& dd,
          const replace_func<Policy>& m,
          const size_t pq1_bound,
          const size_t pq2_bound)
    {
      // -------------------------------------------------------------------------------------------
      // Case: Terminal
      adiar_assert(!dd->is_terminal());

      // -------------------------------------------------------------------------------------------
      // Case: Do the product construction (with random access)
      //
      // Use random access if requested or the width fits half(ish) of the memory otherwise
      // dedicated to the secondary priority queue.

      constexpr size_t data_structures_in_pq_2 =
        jdown_pq2_type<memory_mode::Internal>::data_structures;

      constexpr size_t data_structures_in_pqs = data_structures_in_pq_2
        + jdown_pq1_type<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::data_structures;

      const size_t ra_threshold =
        (memory_available() * data_structures_in_pq_2) / 2 * (data_structures_in_pqs);

      if ( // If user has forced Random Access
        ep.template get<exec_policy::access>() == exec_policy::access::Random_Access
        || ( // Heuristically, if it is indexable and it fits
          ep.template get<exec_policy::access>() == exec_policy::access::Auto && dd->indexable
          && node_raccess::memory_usage(dd->width) <= ra_threshold)) {
        // TODO
        /*
          #ifdef ADIAR_STATS
          stats_replace.jump_down.ra.runs += 1u;
          #endif
          return jdown_ra<jdown_pq1_type>(ep, dd, policy, pq1_bound)
        */
      }

      // -------------------------------------------------------------------------------------------
      // Case: Do the product construction (with priority queues)

#ifdef ADIAR_STATS
      // TODO: stats_replace.jump_down.pq.runs += 1u;
#endif
      return jdown_pq<Policy>(ep, dd, m, pq1_bound, pq2_bound);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace the level of all nodes in a single top-down sweep.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::__dd_type
  replace__jdown(const exec_policy& ep,
                 const typename Policy::dd_type& dd,
                 const replace_func<Policy>& m)
  {
    const size_t pq1_bound = __replace::jdown__ilevel_upper_bound<Policy, get_2level_cut, 2u>(dd);
    const size_t pq2_bound = __replace::jdown__ilevel_upper_bound<Policy, get_1level_cut, 0u>(dd);

    return __replace::jdown<Policy>(ep, dd, m, pq1_bound, pq2_bound);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace the level of all nodes in a single top-down sweep.
  ///
  /// \details Knowing that this is an adjacent swap, we can decrease the upper bound on the size of
  ///          the priority queues compared to the `Jump_Down` case.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  inline typename Policy::__dd_type
  replace__adjswap(const exec_policy& ep,
                   const typename Policy::dd_type& dd,
                   const replace_func<Policy>& m)
  {
    const size_t pq1_bound = __replace::adjswap__ilevel_upper_bound<Policy, get_2level_cut, 2u>(dd);
    const size_t pq2_bound = __replace::adjswap__ilevel_upper_bound<Policy, get_1level_cut, 0u>(dd);

    return __replace::jdown<Policy>(ep, dd, m, pq1_bound, pq2_bound);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Algorithms: `Jump_Up`
  //
  // Bottom-up 2-ary product construction that incorporates `intercut` inside of the bottom-up
  // `reduce` sweep.
  /*
  //
  //             e                     _ e _         | ...
  //            / \                   /     \
  //           /   \                (!)      \       | x
  //          /     \              /   \      \
  //         d      f           (a,g) (b,g)    f     | ...
  //        / \          =>      /
  //       c   \               (a,b)                 | y -> x
  //      / \   \              /   \
  //     a   b   g             a   b                 | ...
  */
  // This procedure *can* create duplicate nodes. Hence, one has to still do one (or two?) sorting
  // steps to remove duplicate nodes.
  //
  // To be able to send a variable to a level that is occupied in the input, we again do the
  // doubling trick above inside of the levelized priority queue.

  // TODO

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Algorithms: `Non-Monotone` (+ `Swap`)
  //
  // Starting from the bottom with *nested sweeping*, we accumulate the results of multiple
  // `Jump_Downs`. This is essentially an *insertion sort* on the levels. Here, we can abuse the
  // invariant, that the nested `Jump_Down` is always moving levels down to an empty one.
  /*
  //   .            .  .            .  .             _._          | x -> y
  //  / \                          / \              /   \
  //  . |  ==(z)=>        ==(y)=>  . .    ==(x)=>   .   .         | y -> z
  // / \/                                          / \ / \
  // . .                                           . . . .        | z -> x
  */
  // The main weakness of this operation is if something has to be moved up, i.e. multiple
  // `Jump_Down` operations are used to effectively create a single `Jump_Up`. To mitigate this, we
  // want to preface the nested sweep with one or more `Jump_Up` and `Jump_Down` operations.
  //
  // Furthermore, we could incorporate the `Jump_Up` logic inside the Outer Reduce. But, that would
  // be too much engineering work.

  // TODO

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Inference of the most precise replacement-type

  namespace __replace
  {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Infer the replace type.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename Policy, typename LevelInfoStream, typename ReplaceFunction>
    replace_type
    infer_replace_type(LevelInfoStream& ls, const ReplaceFunction& m)
    {
      using level_type        = typename Policy::level_type;
      using signed_level_type = typename Policy::signed_level_type;
      using result_type       = typename ReplaceFunction::result_type;

      constexpr bool is_total_map   = is_same<result_type, level_type>;
      constexpr bool is_partial_map = is_same<result_type, optional<level_type>>;

      static_assert(is_total_map || is_partial_map);

      bool identity = true;
      bool shift    = true;
      bool monotone = true;

      level_type prev_before = Policy::max_label + 1;
      level_type prev_after  = Policy::max_label + 1;

      signed_level_type prev_diff = 0;

      while (ls.can_pull()) {
        const level_type next_before     = ls.pull().level();
        const result_type next_after_opt = m(next_before);

        if constexpr (is_partial_map) {
          if (!next_after_opt.has_value()) { continue; }
        }

        level_type next_after;
        if constexpr (is_partial_map) {
          if (!next_after_opt.has_value()) { continue; }
          next_after = *next_after_opt;
        } else {
          next_after = next_after_opt;
        }

        if (shift) {
          const signed_level_type next_diff = static_cast<signed_level_type>(next_before)
            - static_cast<signed_level_type>(next_after);

          shift &= Policy::max_label < prev_before || prev_diff == next_diff;
          prev_diff = next_diff;
        }

        identity &= next_before == next_after;
        monotone &= Policy::max_label < prev_before || prev_after < next_after;

        prev_before = next_before;
        prev_after  = next_after;
      }

      if (!monotone) { return replace_type::Non_Monotone; }
      if (!shift) { return replace_type::Monotone; }
      if (!identity) { return replace_type::Shift; }
      return replace_type::Identity;
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Infer the replace type.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename ReplaceFunction>
  replace_type
  infer_replace_type(const typename Policy::dd_type& dd, const ReplaceFunction& m)
  {
    level_info_ifstream<false> ls(dd);
    return __replace::infer_replace_type<Policy>(ls, m);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Infer the replace type.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename ReplaceFunction>
  replace_type
  infer_replace_type(const typename Policy::__dd_type& __dd, const ReplaceFunction& m)
  {
    level_info_ifstream<true> ls(__dd);
    return __replace::infer_replace_type<Policy>(ls, m);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Public interface

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  replace(const exec_policy& ep,
          const typename Policy::dd_type& dd,
          const replace_func<Policy>& m,
          replace_type m_type)
  {
    // Return if nothing needs to be remapped
    if (dd->is_terminal()) {
#ifdef ADIAR_STATS
      stats_replace.terminal_returns += 1u;
#endif
      return dd;
    }

    const replace_type inferred_type =
      m_type == replace_type::Auto ? infer_replace_type<Policy>(dd, m) : m_type;

    // Map internal nodes
    switch (inferred_type) {
      // LCOV_EXCL_START
    case replace_type::Auto:
      adiar_unreachable();
      // LCOV_EXCL_STOP

    case replace_type::Non_Monotone:
#ifdef ADIAR_STATS
      stats_replace.nested_sweeps += 1u;
#endif
      throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");

    case replace_type::Jump_Down:
#ifdef ADIAR_STATS
      // TODO
#endif
      return replace__jdown<Policy>(ep, dd, m);

    case replace_type::Adjacent_Swap:
#ifdef ADIAR_STATS
      // TODO
#endif
      return replace__adjswap<Policy>(ep, dd, m);

    case replace_type::Monotone:
#ifdef ADIAR_STATS
      stats_replace.monotonic_scans += 1u;
#endif
      return replace__monotone<Policy>(dd, m);

    case replace_type::Shift:
#ifdef ADIAR_STATS
      stats_replace.shift_returns += 1u;
#endif
      return replace__shift<Policy>(dd, m);

    case replace_type::Identity:
#ifdef ADIAR_STATS
      stats_replace.identity_returns += 1u;
#endif
      return dd;
    }
    adiar_unreachable(); // LCOV_EXCL_LINE
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  replace(const exec_policy& ep,
          typename Policy::__dd_type&& __dd,
          const replace_func<Policy>& m,
          replace_type m_type)
  {
    // Is it already reduced?
    if (__dd.template has<typename Policy::shared_node_file_type>()) {
      const typename Policy::dd_type dd(
        __dd.template get<typename Policy::shared_node_file_type>(), __dd._negate, __dd._shift);
      return replace<Policy>(ep, dd, m, m_type);
    }

    const replace_type inferred_type =
      m_type == replace_type::Auto ? infer_replace_type<Policy>(__dd, m) : m_type;

    // Otherwise, map while reducing
    switch (inferred_type) {
      // LCOV_EXCL_START
    case replace_type::Auto:
      adiar_unreachable();
      // LCOV_EXCL_STOP

    case replace_type::Non_Monotone:
#ifdef ADIAR_STATS
      stats_replace.nested_sweeps += 1u;
#endif
      throw invalid_argument("Non-monotonic variable replacement not (yet) supported.");

    case replace_type::Jump_Down:
#ifdef ADIAR_STATS
      // TODO
#endif
      return replace__jdown<Policy>(ep, std::move(__dd), m);

    case replace_type::Adjacent_Swap:
#ifdef ADIAR_STATS
      // TODO
#endif
      return replace__adjswap<Policy>(ep, std::move(__dd), m);

    case replace_type::Monotone:
    case replace_type::Shift:
#ifdef ADIAR_STATS
      stats_replace.monotonic_reduces += 1u;
#endif
      return replace__monotone<Policy>(ep, std::move(__dd), m);

    case replace_type::Identity:
#ifdef ADIAR_STATS
      stats_replace.identity_reduces += 1u;
#endif
      return typename Policy::dd_type(std::move(__dd));
    }
    adiar_unreachable(); // LCOV_EXCL_LINE
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replace variables based on the given (total) map.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  typename Policy::__dd_type
  replace(typename Policy::__dd_type&& __dd, const replace_func<Policy>& m, replace_type m_type)
  {
    const exec_policy ep = __dd._policy;
    return replace<Policy>(ep, std::move(__dd), m, m_type);
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_REPLACE_H
