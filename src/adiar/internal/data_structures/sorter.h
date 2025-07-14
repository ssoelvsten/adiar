#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_SORTER_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_SORTER_H

#include <algorithm>
#include <limits>
#include <math.h>
#include <memory>
#include <string>

#include <tpie/sort.h>
#include <tpie/tpie.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>

namespace adiar::internal
{
  template <memory_mode mem_mode, typename T, typename Comp = std::less<T>>
  class sorter;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal vector with TPIE's parallel quick-sort.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Comp>
  class sorter<memory_mode::Internal, T, Comp>
  {
  public:
    using value_type = T;

  private:
    using array_type = tpie::array<T>;
    array_type _array;

    Comp _pred;
    bool _sorted = false;

    size_t _size      = 0;
    size_t _front_idx = 0;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static size_t
    memory_usage(size_t no_elements)
    {
      return array_type::memory_usage(no_elements);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    static size_t
    memory_fits(size_t memory_bytes)
    {
      const size_t ret = array_type::memory_fits(memory_bytes);

      adiar_assert(memory_usage(ret) <= memory_bytes, "memory_fits and memory_usage should agree.");
      return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    static constexpr size_t data_structures = 1u;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    static unique_ptr<sorter<memory_mode::Internal, value_type, Comp>>
    make_unique(size_t memory_bytes, size_t no_elements, size_t no_sorters = 1, Comp comp = Comp())
    {
      return adiar::make_unique<sorter<memory_mode::Internal, value_type, Comp>>(
        memory_bytes, no_elements, no_sorters, comp);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    static void
    reset_unique(unique_ptr<sorter<memory_mode::Internal, value_type, Comp>>& u_ptr,
                 size_t /*memory_bytes*/,
                 size_t /*no_elements*/,
                 size_t /*no_sorters*/ = 1,
                 Comp /*comp*/         = Comp())
    {
      u_ptr->reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
  public:
    sorter([[maybe_unused]] size_t memory_bytes,
           size_t no_elements,
           [[maybe_unused]] size_t no_sorters = 1,
           Comp comp                          = Comp())
      : _array(no_elements)
      , _pred(comp)
    {
      adiar_assert(no_elements <= memory_fits(memory_bytes / no_sorters),
                   "Must be instantiated with enough memory.");
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_push() const
    {
      return !this->_sorted && this->_array.size() > this->_size;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push(const value_type& v)
    {
      adiar_assert(this->can_push());
      this->_array[_size++] = v;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    sort()
    {
      adiar_assert(this->_sorted == false);
      tpie::parallel_sort(this->_array.begin(), this->_array.begin() + this->_size, this->_pred);
      this->_sorted = true;
      adiar_assert(this->_front_idx == 0);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_pull() const
    {
      adiar_assert(this->_sorted, "Only retrieve content when done pushing and sorting it.");
      return this->_front_idx < this->_size;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    top() const
    {
      adiar_assert(this->can_pull());
      return this->_array[this->_front_idx];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    pull()
    {
      adiar_assert(this->can_pull());
      return this->_array[this->_front_idx++];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    reset()
    {
      this->_sorted    = false;
      this->_size      = 0;
      this->_front_idx = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    size() const
    {
      return this->_size - this->_front_idx;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty() const
    {
      return this->size() == 0;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Type alias for sorter for partial type application of the 'internal' memory type.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Comp = std::less<T>>
  using internal_sorter = sorter<memory_mode::Internal, T, Comp>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's external memory sorter, tpie::merge_sorter.
  ///
  /// A wrapper for the tpie::merge_sorter that takes care of all the memory computations involved
  /// in deriving how much memory should be used and how it should be.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Comp>
  class sorter<memory_mode::External, T, Comp>
  {
  public:
    using value_type = T;

  private:
    tpie::merge_sorter<value_type, false, Comp> _sorter;

    bool _has_peeked = false;
    value_type _peeked;

    size_t _pulls = 0u;

  public:
    static constexpr size_t data_structures = 1u;

    static unique_ptr<sorter<memory_mode::External, value_type, Comp>>
    make_unique(size_t memory_bytes, size_t no_elements, size_t no_sorters = 1, Comp comp = Comp())
    {
      return adiar::make_unique<sorter<memory_mode::External, value_type, Comp>>(
        memory_bytes, no_elements, no_sorters, comp);
    }

    static void
    reset_unique(unique_ptr<sorter<memory_mode::External, value_type, Comp>>& u_ptr,
                 size_t memory_bytes,
                 size_t no_elements,
                 size_t no_sorters = 1,
                 Comp comp         = Comp())
    {
      u_ptr = make_unique(memory_bytes, no_elements, no_sorters, comp);
    }

  public:
    sorter(size_t memory_bytes,
           size_t no_elements,
           size_t number_of_sorters = 1,
           Comp comp                = Comp())
      : _sorter(comp)
    {
      // Quickfix: Issue https://github.com/thomasmoelhave/tpie/issues/250
      constexpr size_t minimum_phase1 = sizeof(value_type) * 128 * 1024 + 5 * 1024 * 1024;

      // ===========================================================================================
      // Case 0: No sorters - why are we then instantiating one?
      adiar_assert(number_of_sorters > 0, "Number of sorters should be positive");

      // Consult the internal sorter to get a bound of how much memory is
      // necessary to sort these elements in internal memory. We don't need to
      // allocate more than a constant of this for the external memory case.
      const size_t no_elements_memory =
        2 * sorter<memory_mode::Internal, value_type, Comp>::memory_usage(no_elements);

      // ===========================================================================================
      // Case 1: A single sorter.
      if (number_of_sorters == 1u) {
        const size_t sorter_memory =
          std::min(std::max(no_elements_memory, 2 * minimum_phase1), memory_bytes);

        adiar_assert(sorter_memory <= memory_bytes,
                     "Memory of a single sorter does not exceed given amount.");

        // -----------------------------------------------------------------------------------------
        // Use TPIE's default settings for the three phases.
        _sorter.set_available_memory(sorter_memory);
        _sorter.begin();
        return;
      }

      // ==========================================================================================
      // Case: 2+: Distribute a common area of memory between all sorters.
      adiar_assert(number_of_sorters > 1, "Edge case for a single sorter taken care of above");

      // -------------------------------------------------------------------------------------------
      // We intend to have the memory divided between all the sorters, such that one can be in phase
      // 2 while everyone else is in phase 1 or 3.
      //
      // +----+----+----+------------------------------+
      // | p1 | p1 | p3 |       . . . p2 . . .         |
      // +----+----+----+------------------------------+
      //
      // Phase two is the one that makes the most out of a lot of memory. So, we want to have phase
      // 2 have the largest share. For simplicity, we currently have all the p1 sorters have 1/16 of
      // the entire memory.
      //
      // TODO: It would be better to make p2 'exponentially' larger than p1 for some notion of
      //       'exponentiality'.

      // -------------------------------------------------------------------------------------------
      // Phase 1 : Push Mergesort base cases

      // Take up at most 1/(Sorter-1)'th of 1/16th of the total memory. The last sorter is either in
      // the same phase or another phase.
      const size_t maximum_phase1 = (memory_bytes >> 4) / (number_of_sorters - 1);

      const size_t phase1 = std::max(minimum_phase1, std::min(maximum_phase1, no_elements_memory));

      // -------------------------------------------------------------------------------------------
      // Phase 3 : Top-most and final merging of partial lists.
      //
      // Based on internal workings of `tpie::merge_sorter` this should be at least twice the size
      // of the phase 1 memory.
      constexpr size_t minimum_phase3 = /*2 **/ minimum_phase1;

      const size_t phase3 = std::max(minimum_phase3, phase1);

      // -------------------------------------------------------------------------------------------
      // Phase 2 : Merge sorted lists until there are few enough for phase 3.
      //
      // Use the remaining sorter memory for this very step. If one is in this phase, then all other
      // sorters are in phase 1.
      const size_t phase2 = memory_bytes - phase1 * (number_of_sorters - 1);

      // -----------------------------------------------------------------------
      // Sanity tests...
      adiar_assert(number_of_sorters * phase1 <= memory_bytes,
                   "Memory is enough to have 'N' pushable sorters.");

      adiar_assert((number_of_sorters - 1) * phase1 + phase2 <= memory_bytes,
                   "Memory is enough to have 'N-1' pushable sorters and '1' sorting one.");

      adiar_assert((number_of_sorters - 1) * phase1 + phase3 <= memory_bytes,
                   "Memory is enough to have 'N-1' pushable sorters and '1' pullable one.");

      adiar_assert((number_of_sorters - 1) * phase1 + phase3 + phase1 <= memory_bytes,
                   "Memory is enough to replace the pullable sorter with an 'Nth' pushable one.");

      // -------------------------------------------------------------------------------------------
      // Set the available memory and start the sorter
      _sorter.set_available_memory(phase1, phase2, phase3);
      _sorter.begin();
    }

    // bool
    // can_push();
    //
    // TODO: Update TPIE merge sorter to access the current phase Enum.

    void
    push(const value_type& v)
    {
      _sorter.push(v);
    }

    void
    sort()
    {
      _sorter.end();

      tpie::dummy_progress_indicator dpi{};
      _sorter.calc(dpi);
    }

    bool
    can_pull() /*const*/
    {
      return _has_peeked || _sorter.can_pull();
    }

    value_type
    top()
    {
      adiar_assert(can_pull());
      if (!_has_peeked) {
        _has_peeked = true;
        _peeked     = _sorter.pull();
      }
      return _peeked;
    }

    value_type
    pull()
    {
      adiar_assert(can_pull());
      _pulls++;
      if (_has_peeked) {
        _has_peeked = false;
        return _peeked;
      }
      return _sorter.pull();
    }

    size_t
    size() /*const*/
    {
      // '_sorter.item_count()' from TPIE only provides the number of .push(...)
      // in phase 1, not the number of elements left in phase 3.
      return _sorter.item_count() - _pulls;
    }

    bool
    empty() /*const*/
    {
      return size() == 0;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Type alias for sorter for partial type application of the 'external' memory type.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Comp = std::less<T>>
  using external_sorter = sorter<memory_mode::External, T, Comp>;
}

#endif // ADIAR_INTERNAL_DATA_STRUCTURES_SORTER_H
