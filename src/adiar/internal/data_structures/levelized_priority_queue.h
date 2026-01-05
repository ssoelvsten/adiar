#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_LEVELIZED_PRIORITY_QUEUE_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_LEVELIZED_PRIORITY_QUEUE_H

#include <limits>

#include <adiar/statistics.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_structures/level_merger.h>
#include <adiar/internal/data_structures/priority_queue.h>
#include <adiar/internal/data_structures/sorter.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/memory.h>

namespace adiar::internal
{
  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Number of items for which the unbucketed queue can be used
  ////////////////////////////////////////////////////////////////////////////////////////////////
  constexpr size_t
  no_lookahead_bound(const size_t degree = 1)
  {
    return 8 * (1 << degree);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Strictly less ' < ' between two levels.
  ////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename LevelFileComp>
  inline bool
  level_cmp_lt(const ptr_uint64::label_type l1,
               const ptr_uint64::label_type l2,
               const LevelFileComp& level_comp)
  {
    return level_comp(l1, l2);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Less or equal ' <= ' between two levels.
  ////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename LevelFileComp>
  inline bool
  level_cmp_le(const ptr_uint64::label_type l1,
               const ptr_uint64::label_type l2,
               const LevelFileComp& level_comp)
  {
    return level_comp(l1, l2) || l1 == l2;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// The preprocessor variable ADIAR_LPQ_LOOKAHEAD can be used to change the number of buckets used
  /// by the levelized priority queue (if the number of elements are larger than the
  /// `no_lookahead_bound`).
  //////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ADIAR_LPQ_LOOKAHEAD
#define ADIAR_LPQ_LOOKAHEAD 1u
#endif

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Struct holding statistics on the levelized priority queue
  //////////////////////////////////////////////////////////////////////////////////////////////////
  extern statistics::levelized_priority_queue_t stats_levelized_priority_queue;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Levelized Priority Queue with a finite circular array of sorters and an overflow
  ///          priority queue.
  ///
  /// \details Improves performance of a regular priority queue by exploiting the performance
  ///          difference between sorting a list and populating a priority queue. This improving
  ///          performance by placing all pushed queue elements into a bucket for its specific level
  ///          and then sorting it when one finally arrives at said level. If no bucket exists for
  ///          said element, then this overflow will be placed in an priority queue to then be later
  ///          be merged with its bucket.
  ///
  /// \remark  Elements may \e only be pushed to a \e later level than the currently read level.
  ///
  /// \tparam T              Type of items to be placed in the priority queue.
  ///
  /// \tparam Comp           Sorting comparator to use in the inner queue buckets and for merging.
  ///
  /// \tparam LookAhead      The number of levels (ahead of the current) to explicitly handle with a
  ///                        sorting algorithm.
  ///
  /// \tparam MemoryMode     Whether to use 'Internal' or 'External' memory data structures.
  ///
  /// \tparam LevelFile      Type of the files to obtain the relevant levels from.
  ///
  /// \tparam LevelFileCount Number of files to obtain the levels from.
  ///
  /// \tparam LevelFileComp  Comparator to be used for merging multiple levels from the files
  ///                        together (`std::less` is top-down while `std::greater` is bottom-up).
  ///
  /// \tparam LevelSkip      The index for the first level one can push to. In other words, the
  ///                        number of levels to 'skip'.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T,
            typename Comp          = std::less<>,
            size_t LookAhead       = ADIAR_LPQ_LOOKAHEAD,
            memory_mode MemoryMode = memory_mode::External,
            typename LevelFile     = shared_file_ptr<levelized_file<T>>,
            size_t LevelFileCount  = 1u,
            typename LevelFileComp = std::less<>,
            size_t LevelSkip       = 1u>
  class levelized_priority_queue
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using value_type = T;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the element comparator.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using value_comp_type = Comp;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of buckets.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr memory_mode mem_mode = MemoryMode;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // using level_type = TODO?;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the sorter for each bucket.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using sorter_t = sorter<mem_mode, value_type, value_comp_type>;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the overflow priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using priority_queue_t = priority_queue<MemoryMode, value_type, value_comp_type>;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the level merger.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using level_merger_t = level_merger<LevelFile, LevelFileComp, LevelFileCount, false>;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of buckets.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr size_t buckets = LookAhead + 1;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Index for no bucket.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr ptr_uint64::label_type out_of_buckets_idx =
      static_cast<ptr_uint64::label_type>(-1);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of data structures in Levelized Priority Queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr size_t data_structures =
      buckets * sorter_t::data_structures + priority_queue_t::data_structures;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Value to reflect 'out of levels'.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr ptr_uint64::label_type no_label = ptr_uint64::max_label + 1;

  private:
    static_assert(0 < LookAhead, "LookAhead must at least be of one level");

    static_assert(0 < ptr_uint64::max_label, "A larger LookAhead than max_label is wasteful");

    static_assert(buckets < out_of_buckets_idx,
                  "LookAhead must not be so large to also include '-1'");

    static_assert(out_of_buckets_idx + 1 == 0,
                  "Overflow to '0' is necessary for internal logic to work");

    static_assert(
      ptr_uint64::max_label + 1 > ptr_uint64::max_label,
      "'ptr_uint64::label_type' should leave a window of at least one above 'max_label'");

  private:
    static size_t
    const_memory_usage()
    {
      return level_merger_t::memory_usage();
    }

  public:
    static size_t
    memory_usage(size_t no_elements)
    {
      return priority_queue<memory_mode::Internal, value_type, value_comp_type>::memory_usage(
               no_elements)
        + buckets
        * sorter<memory_mode::Internal, value_type, value_comp_type>::memory_usage(no_elements)
        + const_memory_usage();
    }

    static size_t
    memory_fits(size_t memory_bytes)
    {
      const size_t const_memory_bytes = const_memory_usage();

      if (memory_bytes < const_memory_bytes) { return 0u; }

      // HACK: just provide the minimum of what either of the data structures
      //       can hold when given an equal share of the memory.
      const size_t memory_per_data_structure =
        (memory_bytes - const_memory_bytes) / data_structures;

      const size_t sorter_fits =
        sorter<memory_mode::Internal, value_type, value_comp_type>::memory_fits(
          memory_per_data_structure);

      const size_t priority_queue_fits =
        priority_queue<memory_mode::Internal, value_type, value_comp_type>::memory_fits(
          memory_per_data_structure);

      const size_t res = std::min(sorter_fits, priority_queue_fits);
      adiar_assert(memory_usage(res) <= memory_bytes, "memory_fits and memory_usage should agree.");
      return res;
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements in the levelized priority queue (excluding the one element
    ///        possibly in `_top_elem`).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t _size = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Cache for the top element.
    ///
    /// \see     levelized_priority_queue::_has_top_elem
    ///
    /// \details Since the priority queue is used in a levelized fashion, then the current level is
    ///          completely frozen. Hence, the top element cannot change on a push.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type _top_elem;
    bool _has_top_elem = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between levels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type _current_level = no_label;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between levels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    LevelFileComp _level_comparator = LevelFileComp();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_comp_type _v_comparator = value_comp_type();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Maximum size of levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t _max_size;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Total memory given in constructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t _memory_given;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Memory used by the label merger.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t _memory_occupied_by_merger = level_merger_t::memory_usage();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Memory to be used for the buckets.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t _memory_for_buckets;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Memory to be used for the overflow priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t _memory_occupied_by_overflow;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Provides the levels to distribute all elements across.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_merger_t _level_merger;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Level of each bucket.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type _buckets_level[buckets];

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sorter for each bucket.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    unique_ptr<sorter_t> _buckets_sorter[buckets];

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Index of the currently read-from bucket (if any).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type _front_bucket_idx = out_of_buckets_idx;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Index of the last available bucket (if any).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type _back_bucket_idx = out_of_buckets_idx;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Next element to take out of the bucket (if any)
    ///
    /// \see    levelized_priority_queue::_has_next_from_bucket
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO:  Currently the implementation of the sorter does not allow one to peek the next value.
    //        We can decrease code complexity by moving this into the sorter's logic.
    value_type _next_from_bucket;
    bool _has_next_from_bucket = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overflow priority queue used for elements pushed to a level where a bucket is (yet)
    ///        not created.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    priority_queue_t _overflow_queue;

#ifdef ADIAR_STATS
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The actual maximum size of the levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t _actual_max_size = 0u;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reference to struct to store non-global stats into.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    statistics::levelized_priority_queue_t& _stats;
#endif

  private:
    static size_t
    mem_overflow_queue(size_t memory_given)
    {
      const size_t const_memory = const_memory_usage();

      adiar_assert(const_memory < memory_given, "There should be enough memory for the merger");

      // subtract memory of the merger to not take any of its memory.
      memory_given -= const_memory;

      // GCC bug 85282: One cannot do a member class specialization of each of
      // the two following cases. So, we will have to resort to a constexpr
      // if-statement instead.
      if constexpr (mem_mode == memory_mode::Internal) {
        // -----------------------------------------------------------------------------------------
        // Internal MEMORY MODE:
        //   Divide memory in equal parts

        return memory_given / data_structures;
      } else if constexpr (mem_mode == memory_mode::External) {
        // -----------------------------------------------------------------------------------------
        // EXTERNAL MEMORY MODE:
        //   Use 1/(4Buckets + 1)th of the memory and at least 8 MiB.

        // TODO: Unit test external memory variants?
        const size_t eight_MiB      = 8 * 1024 * 1024;
        const size_t weighted_share = memory_given / (4 * buckets + 1);

        return std::max(eight_MiB, weighted_share);
      } else {
        // -----------------------------------------------------------------------------------------
        static_assert(mem_mode == memory_mode::Internal && mem_mode == memory_mode::External,
                      "Memory mode must be 'Internal' or 'EXTERNAL' at compile-time");
      }
    }

    levelized_priority_queue(size_t memory_given,
                             size_t max_size,
                             [[maybe_unused]] statistics::levelized_priority_queue_t& stats)
      : _max_size(max_size)
      , _memory_given(memory_given)
      , _memory_for_buckets(memory_given - _memory_occupied_by_merger
                            - mem_overflow_queue(memory_given))
      , _memory_occupied_by_overflow(mem_overflow_queue(memory_given))
      , _overflow_queue(mem_overflow_queue(memory_given), max_size)
#ifdef ADIAR_STATS
      , _stats(stats)
#endif
    {
      adiar_assert(_memory_occupied_by_merger + _memory_for_buckets + _memory_occupied_by_overflow
                     <= _memory_given,
                   "the amount of memory used should be within the given bounds");
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param files        Files to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const LevelFile (&files)[LevelFileCount],
                             size_t memory_given,
                             size_t max_size,
                             statistics::levelized_priority_queue_t& stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    {
      _level_merger.hook(files);
      init_buckets();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param dds          Decision Diagrams to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const dd (&dds)[LevelFileCount],
                             size_t memory_given,
                             size_t max_size,
                             statistics::levelized_priority_queue_t& stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    {
      _level_merger.hook(dds);
      init_buckets();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param dds          Unreduced Decision Diagrams to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const __dd (&dds)[LevelFileCount],
                             size_t memory_given,
                             size_t max_size,
                             statistics::levelized_priority_queue_t& stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    {
      _level_merger.hook(dds);
      init_buckets();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  Computes final memory usage of internal data structures to then distribute the
    ///         remaining memory when initialising the buckets.
    ///
    /// \remark Call this function at the end of the constructor after the level_merger has hooked
    ///         into the input.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    init_buckets()
    {
      // Initially skip the number of levels
      for (size_t idx = 0; _level_merger.can_pull() && idx < LevelSkip; idx++) {
        _level_merger.pull();
      }

      // Set up buckets until no levels are left or all buckets have been
      // instantiated. Notice, that _back_bucket_idx was initialised to -1.
      while (_back_bucket_idx + 1 < buckets && _level_merger.can_pull()) {
        const ptr_uint64::label_type level = _level_merger.pull();

        adiar_assert(_front_bucket_idx == out_of_buckets_idx, "Front bucket not moved");

        _back_bucket_idx++;

        _buckets_level[_back_bucket_idx] = level;
        _buckets_sorter[_back_bucket_idx] =
          sorter_t::make_unique(_memory_for_buckets, _max_size, buckets);
      }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~levelized_priority_queue()
    {
#ifdef ADIAR_STATS
      stats_levelized_priority_queue.sum_predicted_max_size += _max_size;
      _stats.sum_predicted_max_size += _max_size;

      stats_levelized_priority_queue.sum_actual_max_size += _actual_max_size;
      _stats.sum_actual_max_size += _actual_max_size;

      stats_levelized_priority_queue.sum_max_size_ratio += frac(_actual_max_size, _max_size);
      _stats.sum_max_size_ratio += frac(_actual_max_size, _max_size);

      stats_levelized_priority_queue.sum_destructors += 1u;
      _stats.sum_destructors += 1u;
#endif
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there is any current level to pull elements from.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_current_level() const
    {
      return _current_level != no_label;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the current level.
    ///
    /// \pre `has_current_level() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type
    current_level() const
    {
      adiar_assert(has_current_level(), "Needs to have a 'current' level to read the level from");

      return _current_level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are any more (possibly all empty) levels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_next_level() const
    {
      return has_next_bucket();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the next (possibly empty) level.
    ///
    /// \pre `has_next_level() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type
    next_level() const
    {
      return next_bucket_level();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether one can push elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_push() const
    {
      return has_next_level();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element into the priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push(const value_type& e)
    {
      adiar_assert(can_push(), "Should only push when there is a yet unvisited level.");

      const ptr_uint64::label_type level = e.level();

      adiar_assert(level_cmp_le<LevelFileComp>(next_bucket_level(), level, _level_comparator),
                   "Can only push element to next bucket or later.");

      const size_t pushable_buckets = active_buckets() - has_front_bucket();

      adiar_assert(pushable_buckets > 0, "There is at least one pushable bucket (i.e. level)");

      _size++;
#ifdef ADIAR_STATS
      _actual_max_size = std::max(_actual_max_size, _size);
#endif

      ptr_uint64::label_type bucket_offset = 1u;
      do {
        const ptr_uint64::label_type bucket_idx = (_front_bucket_idx + bucket_offset++) % buckets;

        if (_buckets_level[bucket_idx] == level) {
          _buckets_sorter[bucket_idx]->push(e);
#ifdef ADIAR_STATS
          stats_levelized_priority_queue.push_bucket += 1u;
          _stats.push_bucket += 1u;
#endif
          return;
        }
      } while (bucket_offset <= pushable_buckets);

#ifdef ADIAR_STATS
      stats_levelized_priority_queue.push_overflow += 1u;
      _stats.push_overflow += 1u;
#endif
      _overflow_queue.push(e);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief            Set up the next nonempty level to which some requests have been pushed
    ///                   before the given stop_level.
    ///
    /// \param stop_level The level that should be furthest forwarded to. If no stop level is given
    ///                   then the next level is based on the next existing element in the priority
    ///                   queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    setup_next_level(ptr_uint64::label_type stop_level = no_label)
    {
      adiar_assert(stop_level <= ptr_uint64::max_label || stop_level == no_label,
                   "The stop level should be a legal value (or not given)");

      adiar_assert(!has_current_level() || empty_level(),
                   "Level is empty before moving on to the next");

      adiar_assert(stop_level != no_label || !empty(),
                   "Either a stop level is given or we have some non-empty level to forward to");

      const ptr_uint64::label_type overflow_level =
        !_overflow_queue.empty() ? _overflow_queue.top().level() : stop_level;

      stop_level = stop_level == no_label
          || level_cmp_lt<LevelFileComp>(overflow_level, stop_level, _level_comparator)
        ? overflow_level
        : stop_level;

      const bool has_stop_level = stop_level != no_label;

      adiar_assert(has_next_level(), "There should be a next level to go to");

      adiar_assert(
        !has_stop_level || !has_front_bucket()
          || level_cmp_lt<LevelFileComp>(front_bucket_level(), stop_level, _level_comparator),
        "'stop_level' should be past the current front bucket (if it exists)");

      adiar_assert(!has_front_bucket()
                     || level_cmp_lt<LevelFileComp>(
                       front_bucket_level(), back_bucket_level(), _level_comparator),
                   "Back bucket should be (strictly) ahead of the back bucket");

      // TODO: Add statistics on what case is hit.

      // Edge Case: ------------------------------------------------------------------------------ :
      //   The given stop_level is prior to the next bucket
      if (has_stop_level
          && level_cmp_lt<LevelFileComp>(stop_level, next_bucket_level(), _level_comparator)) {
        return;
      }

      // Edge Case: ------------------------------------------------------------------------------ :
      //   All buckets are empty, so we can merely relabel them and not pay for
      //   any (re)initialisation. Furthermore, we can have one more bucket
      //   ready to 'catch' the next elements.
      if (size() == _overflow_queue.size()) {
        adiar_assert(has_stop_level, "Must have a 'stop_level' to go to");

        relabel_buckets(stop_level);
        return;
      }

      // Primary Case: --------------------------------------------------------------------------- :
      //   At least one bucket contains an element. Let us go through each
      //   bucket one-by-one until we find the one or hit the stop_level.
      forward_to_nonempty_bucket(stop_level, has_stop_level);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the current level is empty. Is false, if there is no 'current' level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty_level()
    {
      // TODO: change semantics to require 'has_current_level'
      return !has_current_level()
        || ( // Do we not have any cached element from top()?
          !_has_top_elem
          // and the current bucket is empty?
          && !_has_next_from_bucket
          // and the priority queue has nothing for this level?
          && (_overflow_queue.empty() || current_level() != _overflow_queue.top().level()));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether any elements can be pulled (from the current level).
    ///
    /// \see  levelized_priority_queue::empty_level
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_top()
    {
      return !empty_level();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element of the current level.
    ///
    /// \pre `has_top() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    top()
    {
      adiar_assert(has_top());

      if (!_has_top_elem) {
        _top_elem     = pull();
        _has_top_elem = true;
      }

      return _top_elem;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element of the current level.
    ///
    /// \pre `has_top() == true`
    ///
    /// \see levelized_priority_queue::top
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    peek()
    {
      return top();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether any elements can be pulled (from the current level).
    ///
    /// \see   levelized_priority_queue::has_top,
    ///        levelized_priority_queue::empty_level
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_pull()
    {
      return has_top();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element on the current level and remove it.
    ///
    /// \pre `can_pull() == true` (or `has_top() == true`)
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    pull()
    {
      adiar_assert(can_pull());

      if (_has_top_elem) {
        _has_top_elem = false;
        return _top_elem;
      }

      adiar_assert(_size > 0, "pull on non-top element requires content");
      _size--;

      // Merge bucket with overflow queue
      if (_overflow_queue.empty()
          || (_has_next_from_bucket && _v_comparator(_next_from_bucket, _overflow_queue.top()))) {
        const value_type ret = _next_from_bucket;
        if (_buckets_sorter[_front_bucket_idx]->can_pull()) {
          _next_from_bucket = _buckets_sorter[_front_bucket_idx]->pull();
        } else {
          _has_next_from_bucket = false;
        }
        return ret;
      } else {
        const value_type ret = _overflow_queue.top();
        _overflow_queue.pop();
        return ret;
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Remove the top element of the current level.
    ///
    /// \pre `can_pull() == true` (or `has_top() == true)`
    ///
    /// \see levelized_priority_queue::pull
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    pop()
    {
      pull();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The total number of elements (across all levels).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    size() const
    {
      // TODO: also separate '_size' away from the overflow queue?
      return _size + _has_top_elem;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  Whether the entire priority queue (across all levels) is empty.
    ///
    /// \remark If you only want to know if it is empty for the current level, then use
    ///         `empty_level` (or `can_pull`) instead.
    ///
    /// \see     levelized_priority_queue::empty_level
    ///          levelized_priority_queue::has_top
    ///          levelized_priority_queue::can_pull
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty() const
    {
      return size() == 0;
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  The number of active buckets, incl. the (possibly existing) read-only bucket.
    ///
    /// \remark To compute the number of pushable buckets, please write
    ///         `active_buckets() - has_front_bucket()`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    active_buckets() const
    {
      if (_front_bucket_idx == out_of_buckets_idx) { return _back_bucket_idx + 1; }

      return _front_bucket_idx <= _back_bucket_idx
        ? (_back_bucket_idx - _front_bucket_idx) + 1
        : (buckets - _front_bucket_idx) + _back_bucket_idx + 1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether more buckets are available.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_next_bucket() const
    {
      return _front_bucket_idx != _back_bucket_idx;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Label of the next bucket.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type
    next_bucket_level() const
    {
      adiar_assert(has_next_bucket(), "Cannot obtain level of non-existing next bucket");

      const ptr_uint64::label_type next_idx   = (_front_bucket_idx + 1) % buckets;
      const ptr_uint64::label_type next_level = _buckets_level[next_idx];
      return next_level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there is an 'active' bucket from which can be pulled (though it may need to
    ///        be sorted).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_front_bucket() const
    {
      return _front_bucket_idx != out_of_buckets_idx;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Level of the front bucket.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type
    front_bucket_level() const
    {
      return _buckets_level[_front_bucket_idx];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Level of the back bucket.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type
    back_bucket_level() const
    {
      return _buckets_level[_back_bucket_idx];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Forwards to the first non-empty bucket or at the given `stop_level`.
    ///
    /// \param stop_level     The forwarding will stop early if reaching this
    ///                       level.
    ///
    /// \param has_stop_level Whether to take the stop_level parameter into
    ///                       account.
    ///
    /// \see    levelized_priority_queue::setup_next_level
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline void
    forward_to_nonempty_bucket(const ptr_uint64::label_type stop_level, const bool has_stop_level)
    {
      do {
        adiar_assert(has_next_bucket(), "At least one more bucket can be forwarded to");

        // Is the next bucket past the 'stop_level'?
        if (has_stop_level
            && level_cmp_lt<LevelFileComp>(stop_level, next_bucket_level(), _level_comparator)) {
          break;
        }

        adiar_assert(!has_front_bucket()
                       || level_cmp_lt<LevelFileComp>(
                         front_bucket_level(), back_bucket_level(), _level_comparator),
                     "Inconsistency in has_next_bucket predicate");

        // Replace the current read-only bucket, if there is one
        if (_level_merger.can_pull() && has_front_bucket()) {
          const ptr_uint64::label_type next_level = _level_merger.pull();

          _buckets_level[_front_bucket_idx] = next_level;
          sorter_t::reset_unique(
            _buckets_sorter[_front_bucket_idx], _memory_for_buckets, _max_size, buckets);

          _back_bucket_idx = _front_bucket_idx;
        }
        _front_bucket_idx = (_front_bucket_idx + 1) % buckets;

        adiar_assert(!has_next_bucket() || !has_front_bucket()
                       || level_cmp_lt<LevelFileComp>(
                         front_bucket_level(), back_bucket_level(), _level_comparator),
                     "Inconsistency in has_next_bucket predicate");

        adiar_assert(has_next_bucket() || !has_front_bucket()
                       || front_bucket_level() == back_bucket_level(),
                     "Inconsistency in has_next_bucket predicate");

        // Sort front bucket
        _buckets_sorter[_front_bucket_idx]->sort();

        _has_next_from_bucket = _buckets_sorter[_front_bucket_idx]->can_pull();
        if (_has_next_from_bucket) {
          _next_from_bucket = _buckets_sorter[_front_bucket_idx]->pull();
        }
      } while (!_has_next_from_bucket && has_next_bucket());

      _current_level = front_bucket_level();

      adiar_assert(has_front_bucket(), "Ends with a front bucket");

      adiar_assert(
        (has_stop_level
         && (level_cmp_le<LevelFileComp>(stop_level, front_bucket_level(), _level_comparator)
             || (!has_next_bucket()
                 || level_cmp_lt<LevelFileComp>(
                   stop_level, next_bucket_level(), _level_comparator))))
          || _has_next_from_bucket,
        "Either we stopped early or we found a non-bucket");

      adiar_assert(
        level_cmp_le<LevelFileComp>(front_bucket_level(), back_bucket_level(), _level_comparator),
        "Consistent bucket levels");
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Relabel all buckets to be a level at or past the stop_level. If the current read-only
    ///        bucket is relabelled, then it is also reset to be pushable.
    ///
    /// \see   levelized_priority_queue::setup_next_level
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline void
    relabel_buckets(const ptr_uint64::label_type stop_level)
    {
      adiar_assert(stop_level != no_label, "Relabelling of buckets require a valid 'stop_level'");

      // Backup of start and end of circular array
      const size_t old_front_bucket_idx = _front_bucket_idx;
      const size_t old_back_bucket_idx  = _back_bucket_idx;

      // Create a list of the new levels
      ptr_uint64::label_type new_levels[buckets];
      _back_bucket_idx = out_of_buckets_idx;

      // Copy over still relevant levels from current buckets
      do {
        _front_bucket_idx = (_front_bucket_idx + 1) % buckets;

        adiar_assert(has_front_bucket(), "After increment the front bucket will 'exist'");

        if (level_cmp_le<LevelFileComp>(front_bucket_level(), stop_level, _level_comparator)) {
          _current_level = front_bucket_level();
        } else { // level_cmp_lt<LevelFileComp>(stop_level, front_bucket_level(), _level_comparator)
          new_levels[++_back_bucket_idx] = front_bucket_level();
        }
      } while (_front_bucket_idx != old_back_bucket_idx);

      _front_bucket_idx = out_of_buckets_idx;

      // Add as many levels from the level_merger as we can fit in
      while (_level_merger.can_pull()
             && level_cmp_le<LevelFileComp>(_level_merger.peek(), stop_level, _level_comparator)) {
        _current_level = _level_merger.pull();
      }

      while (_back_bucket_idx + 1 < buckets && _level_merger.can_pull()) {
        new_levels[++_back_bucket_idx] = _level_merger.pull();
      }

      adiar_assert(_back_bucket_idx == out_of_buckets_idx || _back_bucket_idx < buckets,
                   "_back_bucket_idx is a valid index");

      // Relabel all buckets
      if (_back_bucket_idx != out_of_buckets_idx) {
        for (size_t idx = 0; idx <= _back_bucket_idx; idx++) {
          _buckets_level[idx] = new_levels[idx];
        }

        // Reset the prior read-only bucket, if relevant
        if (old_front_bucket_idx <= _back_bucket_idx) {
          sorter_t::reset_unique(
            _buckets_sorter[old_front_bucket_idx], _memory_for_buckets, _max_size, buckets);
        }

        // We can clean up all the dead buckets with a '.reset()' on the
        // unique_ptr, but we will leave that for the destructor to do.
      }
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Specialization of the levelized priority queue with a LookAhead of '0', i.e.
  ///        **without** any buckets.
  ////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T,
            typename Comp,
            memory_mode MemoryMode,
            typename LevelFile,
            size_t LevelFileCount,
            typename LevelFileComp,
            size_t LevelSkip>
  class levelized_priority_queue<T,
                                 Comp,
                                 0u, // <--
                                 MemoryMode,
                                 LevelFile,
                                 LevelFileCount,
                                 LevelFileComp,
                                 LevelSkip>
  {
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using value_type = T;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the element comparator.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using value_comp_type = Comp;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of buckets.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr memory_mode mem_mode = MemoryMode;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the (overflow) priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    using priority_queue_t = priority_queue<mem_mode, value_type, value_comp_type>;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of buckets.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr size_t buckets = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of data structures in Levelized Priority Queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr size_t data_structures = priority_queue_t::data_structures;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Value to reflect 'out of levels'.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr ptr_uint64::label_type no_label = ptr_uint64::max_label + 1;

  public:
    static size_t
    memory_usage(size_t no_elements)
    {
      return priority_queue<memory_mode::Internal, value_type, value_comp_type>::memory_usage(
        no_elements);
    }

    static size_t
    memory_fits(size_t memory_bytes)
    {
      return priority_queue<memory_mode::Internal, value_type, value_comp_type>::memory_fits(
        memory_bytes);
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between levels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type _current_level = no_label;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between levels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    LevelFileComp _level_comparator = LevelFileComp();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_comp_type _v_comparator = value_comp_type();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Maximum size of levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t _max_size;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Total memory given in constructor.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t _memory_given;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overflow priority queue used for elements pushed to a level where a bucket is (yet)
    ///        not created.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    priority_queue_t _priority_queue;

#ifdef ADIAR_STATS
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The actual maximum size of the levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t _actual_max_size = 0u;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reference to struct to store non-global stats into.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    statistics::levelized_priority_queue_t& _stats;
#endif

  private:
    levelized_priority_queue(size_t memory_given,
                             size_t max_size,
                             [[maybe_unused]] statistics::levelized_priority_queue_t& stats)
      : _max_size(max_size)
      , _memory_given(memory_given)
      , _priority_queue(memory_given, max_size)
#ifdef ADIAR_STATS
      , _stats(stats)
#endif
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param files        Files to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const LevelFile (& /*files*/)[LevelFileCount],
                             size_t memory_given,
                             size_t max_size,
                             statistics::levelized_priority_queue_t& stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param dds          Decision Diagrams to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const dd (& /*dds*/)[LevelFileCount],
                             size_t memory_given,
                             size_t max_size,
                             statistics::levelized_priority_queue_t& stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param dds          Decision Diagrams to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const __dd (& /*dds*/)[LevelFileCount],
                             size_t memory_given,
                             size_t max_size,
                             statistics::levelized_priority_queue_t& stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    {}

  public:
    ~levelized_priority_queue()
    {
#ifdef ADIAR_STATS
      stats_levelized_priority_queue.sum_predicted_max_size += _max_size;
      _stats.sum_predicted_max_size += _max_size;

      stats_levelized_priority_queue.sum_actual_max_size += _actual_max_size;
      _stats.sum_actual_max_size += _actual_max_size;

      stats_levelized_priority_queue.sum_max_size_ratio += frac(_actual_max_size, _max_size);
      _stats.sum_max_size_ratio += frac(_actual_max_size, _max_size);

      stats_levelized_priority_queue.sum_destructors += 1u;
      _stats.sum_destructors += 1u;
#endif
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there is any current level to pull elements from.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_current_level() const
    {
      return _current_level != no_label;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the current level.
    ///
    /// \pre `has_current_level() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type
    current_level() const
    {
      adiar_assert(has_current_level(), "Needs to have a 'current' level to read the level from");

      return _current_level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are any more (possibly all empty) levels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_next_level() /*const*/
    {
      if (_priority_queue.empty()) { return false; }
      ptr_uint64::label_type next_label_from_queue = _priority_queue.top().level();
      return (has_current_level()
              && level_cmp_lt<LevelFileComp>(
                _current_level, next_label_from_queue, _level_comparator))
        || (!has_current_level() && !_priority_queue.empty());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the next (possibly empty) level.
    ///
    /// \pre `has_next_level() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_type
    next_level() /*const*/
    {
      return _priority_queue.top().level();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether one can push elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_push() /*const*/
    {
      return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element into the priority queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push(const value_type& v)
    {
#ifdef ADIAR_STATS
      _actual_max_size = std::max(_actual_max_size, _priority_queue.size());
      stats_levelized_priority_queue.push_overflow += 1u;
      _stats.push_overflow += 1u;
#endif
      _priority_queue.push(v);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief            Set up the next nonempty level to which some requests have been pushed
    ///                   before the given `stop_level`.
    ///
    /// \param stop_level The level that should be furthest forwarded to. If no stop level is given
    ///                   then the next level is based on the next existing element in the priority
    ///                   queue.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    setup_next_level(ptr_uint64::label_type stop_level = no_label)
    {
      adiar_assert(stop_level <= ptr_uint64::max_label || stop_level == no_label,
                   "The stop level should be a legal value (or not given)");

      adiar_assert(!has_current_level() || empty_level(),
                   "Level is empty before moving on to the next");

      const bool has_stop_level = stop_level != no_label;

      adiar_assert(has_stop_level || !empty(),
                   "Either a stop level is given or we have some non-empty level to forward to");

      // Edge Case: ------------------------------------------------------------------------------ :
      //   The given stop_level is prior to the next level or there is nothing in the queue
      if ((has_stop_level
           && level_cmp_lt<LevelFileComp>(stop_level, next_level(), _level_comparator))
          || _priority_queue.empty()) {
        _current_level = stop_level;
        return;
      }

      // Edge Case: ------------------------------------------------------------------------------ :
      //   The stop level is before the next level of the queue
      adiar_assert(has_next_level(), "There should be a next level to go to");
      ptr_uint64::label_type next_level_from_queue = next_level();
      if (has_stop_level
          && level_cmp_le<LevelFileComp>(stop_level, next_level_from_queue, _level_comparator)) {
        _current_level = stop_level;
        return;
      }

      // Primary Case: --------------------------------------------------------------------------- :
      //   Set the level to be the next from the queue
      _current_level = next_level_from_queue;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the current level is empty. Is false, if there is no 'current' level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty_level() const
    {
      // TODO: change semantics to require 'has_current_level'
      return !has_current_level() || _priority_queue.empty()
        || current_level() != _priority_queue.top().level();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether any elements can be pulled (from the current level).
    ///
    /// \see   levelized_priority_queue::empty_level
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_top() const
    {
      return !empty_level();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element of the current level.
    ///
    /// \pre   `has_top() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    top()
    {
      adiar_assert(has_top(), "Can only obtain top element on non-empty level");
      return _priority_queue.top();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element of the current level.
    ///
    /// \pre   `has_top() == true`
    ///
    /// \see   levelized_priority_queue::top
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    peek()
    {
      return top();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether any elements can be pulled (from the current level).
    ///
    /// \see   levelized_priority_queue::has_top
    ///        levelized_priority_queue::empty_level
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_pull() const
    {
      return has_top();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element on the current level and remove it.
    ///
    /// \pre   `can_pull() == true` (or `has_top() == true`)
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    pull()
    {
      adiar_assert(!empty_level(), "Can only pull on non-empty level");

      const value_type ret = _priority_queue.top();
      _priority_queue.pop();
      return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Remove the top element of the current level.
    ///
    /// \pre   `can_pull() == true` (or `has_top() == true`)
    ///
    /// \see   levelized_priority_queue::pull
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    pop()
    {
      pull();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The total number of elements (across all levels).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    size() const
    {
      return _priority_queue.size();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  Whether the entire priority queue (across all levels) is empty.
    ///
    /// \remark If you only want to know if it is empty for the current level, then use
    ///         `empty_level` (or `can_pull`) instead.
    ///
    /// \see    levelized_priority_queue::empty_level
    ///         levelized_priority_queue::has_top
    ///         levelized_priority_queue::can_pull
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty() const
    {
      return _priority_queue.empty();
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Levelized Priority Queue to be used with `levelized_file<node>` and a `node_ifstream`
  ///        or `node_raccess`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T,
            typename Comp         = std::less<T>,
            size_t LookAhead      = ADIAR_LPQ_LOOKAHEAD,
            memory_mode mem_mode  = memory_mode::External,
            size_t LevelFileCount = 1u,
            size_t LevelSkip      = 1u>
  using levelized_node_priority_queue = levelized_priority_queue<T,
                                                                 Comp,
                                                                 LookAhead,
                                                                 mem_mode,
                                                                 shared_levelized_file<node>,
                                                                 LevelFileCount,
                                                                 std::less<node::label_type>,
                                                                 LevelSkip>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Levelized Priority Queue to be used with `levelized_file<arc>` and an `arc_ifstream`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T,
            typename Comp         = std::less<T>,
            size_t LookAhead      = ADIAR_LPQ_LOOKAHEAD,
            memory_mode mem_mode  = memory_mode::External,
            size_t LevelFileCount = 1u,
            size_t LevelSkip      = 1u>
  using levelized_arc_priority_queue = levelized_priority_queue<T,
                                                                Comp,
                                                                LookAhead,
                                                                mem_mode,
                                                                shared_levelized_file<arc>,
                                                                LevelFileCount,
                                                                std::greater<arc::label_type>,
                                                                LevelSkip>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Levelized Priority Queue to be used with `shared_file<label_type>`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T,
            typename Comp         = std::less<T>,
            size_t LookAhead      = ADIAR_LPQ_LOOKAHEAD,
            memory_mode mem_mode  = memory_mode::External,
            size_t LevelFileCount = 1u,
            size_t LevelSkip      = 1u>
  using levelized_label_priority_queue =
    levelized_priority_queue<T,
                             Comp,
                             LookAhead,
                             mem_mode,
                             shared_file<ptr_uint64::label_type>,
                             LevelFileCount,
                             std::less<ptr_uint64::label_type>,
                             LevelSkip>;
}

#endif // ADIAR_INTERNAL_DATA_STRUCTURES_LEVELIZED_PRIORITY_QUEUE_H
