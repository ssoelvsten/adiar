#ifndef ADIAR_INTERNAL_IO_NARC_IFSTREAM_H
#define ADIAR_INTERNAL_IO_NARC_IFSTREAM_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/convert.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/arc_ifstream.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_ifstream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Stream nodes from an arc file (skipping any of Reduce algorithm).
  ///
  /// \param Reverse Whether the reading direction should be reversed (relatively to the ordering of
  ///                nodes within the file).
  ///
  /// \see shared_levelized_file<arc>
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <bool Reverse = false>
  class narc_ifstream : protected arc_ifstream<!Reverse>
  {
  public:
    using value_type = node;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Arc-based input to-be converted into nodes on-the-fly.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    arc_ifstream<!Reverse> _ifstream;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a converted node has been buffered.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool _has_peeked = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Latest converted node.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node _peeked;

  public:
    static size_t
    memory_usage()
    {
      return arc_ifstream<!Reverse>::memory_usage();
    }

  private:
    static constexpr size_t idx__internal = file_traits<arc>::idx__internal;

    static constexpr size_t idx__terminals__in_order = file_traits<arc>::idx__terminals__in_order;

    static constexpr size_t idx__terminals__out_of_order =
      file_traits<arc>::idx__terminals__out_of_order;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create unattached to any file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    narc_ifstream() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to an arc file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    narc_ifstream(levelized_file<arc>& file,
                  [[maybe_unused]] const bool negate                         = false,
                  [[maybe_unused]] const node::signed_label_type level_shift = 0)
      : _ifstream(/*need to sort before attach*/)
    {
      adiar_assert(negate == false);
      adiar_assert(level_shift == 0);
      open(file);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a shared arc file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    narc_ifstream(const shared_ptr<levelized_file<arc>>& file,
                  [[maybe_unused]] const bool negate                         = false,
                  [[maybe_unused]] const node::signed_label_type level_shift = 0)
      : _ifstream(/*need to sort before attach*/)
    {
      adiar_assert(negate == false);
      adiar_assert(level_shift == 0);
      open(file);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to an Decision Diagram.
    ///
    /// \pre The given diagram should contain an unreduced diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    narc_ifstream(const __dd& diagram)
      : _ifstream(/*need to sort before attach*/)
    {
      adiar_assert(diagram.template has<__dd::shared_arc_file_type>());
      adiar_assert(diagram._negate == false);
      open(diagram.template get<__dd::shared_arc_file_type>());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  Open an arc file.
    ///
    /// \remark This sorts the internal arcs of the file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(levelized_file<arc>& file)
    {
      if (file.semi_transposed) {
        file.sort<arc_source_lt>(idx__internal);
        file.semi_transposed = false;
      }
      _ifstream.open(file);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief  Open a shared arc file.
    ///
    /// \remark This sorts the internal arcs of the file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(const shared_ptr<levelized_file<arc>>& file)
    {
      if (file->semi_transposed) {
        file->sort<arc_source_lt>(idx__internal);
        file->semi_transposed = false;
      }
      _ifstream.open(file);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_pull() const
    {
      return _has_peeked || _ifstream.can_pull_internal() || _ifstream.can_pull_terminal();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain next element (and move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const node
    pull()
    {
      adiar_assert(can_pull());
      if (_has_peeked) {
        _has_peeked = false;
        return _peeked;
      }
      return merge_next();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element (but do not move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const node
    peek()
    {
      adiar_assert(can_pull());
      if (!_has_peeked) {
        _has_peeked = true;
        _peeked     = merge_next();
      }
      return _peeked;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Obtain the first node past the seeked value.
    ///
    /// \param u Unique Identifier to seek for.
    ///
    /// \pre     `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const node
    seek(const node::uid_type& u);
    // TODO

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const node
    merge_next()
    {
      const arc a_first  = take_internal() ? _ifstream.pull_internal() : _ifstream.pull_terminal();
      const arc a_second = take_internal() ? _ifstream.pull_internal() : _ifstream.pull_terminal();

      // Merge into a node (providing low arc first)
      if constexpr (Reverse) {
        return node_of(a_second, a_first);
      } else {
        return node_of(a_first, a_second);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    take_internal()
    {
      if constexpr (Reverse) {
        if (!_ifstream.can_pull_terminal()) { return true; }
        if (!_ifstream.can_pull_internal()) { return false; }
      } else {
        adiar_assert(_ifstream.can_pull_terminal(),
                     "When reading top-down there must be terminal arcs left");

        if (!_ifstream.can_pull_internal()) { return false; }
      }

      const arc::pointer_type internal_source = _ifstream.peek_internal().source();
      const arc::pointer_type terminal_source = _ifstream.peek_terminal().source();

      if constexpr (Reverse) {
        return internal_source > terminal_source;
      } else {
        return internal_source < terminal_source;
      }
    }
  };
}

#endif // ADIAR_INTERNAL_IO_NARC_IFSTREAM
