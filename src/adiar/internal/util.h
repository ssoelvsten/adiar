#ifndef ADIAR_INTERNAL_UTIL_H
#define ADIAR_INTERNAL_UTIL_H

#include <adiar/functional.h>
#include <adiar/type_traits.h>
#include <adiar/types.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/convert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_ofstream.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_ifstream.h>
#include <adiar/internal/io/node_ifstream.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Template to hide how to obtain the level from a data type.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  inline ptr_uint64::label_type
  level_of(const T& t)
  {
    if constexpr (is_integral<T>) {
      return t;
    } else {
      return t.level();
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief  Turn a `level_ifstream<...>` into a level-only generator function.
  ///
  /// \remark The direction (*ascending* vs. *descending*) of the generator is dictated by the given
  ///         stream's direction.
  ///
  /// \see level_ifstream_t, generator
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename LevelStream>
  generator<ptr_uint64::label_type>
  make_generator__levels(LevelStream& ls)
  {
    return [&ls]() mutable -> optional<ptr_uint64::label_type> {
      if (!ls.can_pull()) { return {}; }
      return level_of(ls.pull());
    };
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain whether the levels in two files are disjoint.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: Move to dd_func?
  template <typename DD>
  bool
  disjoint_levels(const DD& a, const DD& b)
  {
    level_info_ifstream sa(a);
    level_info_ifstream sb(b);

    while (sa.can_pull() && sb.can_pull()) {
      if (level_of(sa.peek()) == level_of(sb.peek())) {
        return false;
      } else if (level_of(sa.peek()) < level_of(sb.peek())) {
        sa.pull();
      } else {
        sb.pull();
      }
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a certain level exists in a file.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: Move to dd_func?
  template <typename DD>
  bool
  has_level(const DD& d, const typename DD::label_type x)
  {
    level_info_ifstream<> in_meta(d);
    while (in_meta.can_pull()) {
      level_info m = in_meta.pull();

      // Are we already past where it should be?
      if (x < m.label()) { return false; }

      // Did we find it?
      if (m.label() == x) { return true; }
    }
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Handles all requests with the desired `target` with the given `Handler(...)`.
  ///
  /// \param pq      Priority queue to take requests from.
  /// \param handler Handler function object to call for each request
  /// \param target  Target to pull for
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: Move into a Request Manager.
  template <typename PriorityQueue, typename Handler>
  void
  request_foreach(PriorityQueue& pq,
                  const typename PriorityQueue::value_type::target_t& target,
                  const Handler& handler)
  {
    while (pq.has_top() && pq.top().target == target) {
      handler(pq.top());
      pq.pop();
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Handles all requests with the desired `target` with the given `Handler(...)`.
  ///
  /// \param pq_1    Priority queue to take requests from.
  /// \param pq_2    Priority queue to take requests from.
  /// \param handler Handler function object to call for each request
  /// \param target  Target to pull for
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: Move into a Request Manager.
  template <typename PriorityQueue_1, typename PriorityQueue_2, typename Handler>
  void
  request_foreach(PriorityQueue_1& pq_1,
                  PriorityQueue_2& pq_2,
                  const typename PriorityQueue_1::value_type::target_t& target,
                  const Handler& handler)
  {
    request_foreach(pq_1, target, handler);
    request_foreach(pq_2, target, handler);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the semi-transposition of a decision diagram.
  ///
  /// \details Creates an `arc_file` of the given diagram. This file is on a valid form to be an
  ///          input to the bottom-up `reduce` algorithm: internal arcs are transposed, i.e. sorted
  ///          on their target in ascending order, while arcs to terminals are left untransposed,
  ///          i.e. sorted on their source).
  ///
  /// \pre `!is_terminal(dd)`
  ///
  /// \see reduce
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename DD>
  shared_levelized_file<arc>
  transpose(const DD& d)
  {
    adiar_assert(!d->is_terminal());

    shared_levelized_file<arc> af;

    // Create the contents of 'af'
    {
      arc_ofstream aw(af);
      { // Split every node into their arcs.
        node_ifstream ns(d);
        while (ns.can_pull()) {
          const typename DD::node_type n = ns.pull();

          // TODO (non-binary nodes):
          aw << low_arc_of(n);
          aw << high_arc_of(n);
        }
      }
      { // Copy over meta information
        af->max_1level_cut = d->max_1level_cut[cut::Internal];

        level_info_ifstream<> lis(d);
        while (lis.can_pull()) { aw << lis.pull(); }
      }
    }

    // Sort internal arcs by their target
    af->sort<arc_target_lt>(file_traits<arc>::idx__internal);
    return af;
  }
}

#endif // ADIAR_INTERNAL_UTIL_H
