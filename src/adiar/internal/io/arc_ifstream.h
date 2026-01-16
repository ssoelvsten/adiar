#ifndef ADIAR_INTERNAL_IO_ARC_IFSTREAM_H
#define ADIAR_INTERNAL_IO_ARC_IFSTREAM_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_ifstream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Levelized file stream with additional logic for arc files.
  ///
  /// \see shared_levelized_file<arc>
  //////////////////////////////////////////////////////////////////////////////
  template <bool Reverse = false>
  class arc_ifstream : public levelized_ifstream<arc, !Reverse>
  {
  private:
    using parent_t = levelized_ifstream<arc, !Reverse>;

  public:
    static size_t
    memory_usage()
    {
      return parent_t::memory_usage();
    }

  private:
    static constexpr size_t idx__internal = file_traits<arc>::idx__internal;

    static constexpr size_t idx__terminals__in_order = file_traits<arc>::idx__terminals__in_order;

    static constexpr size_t idx__terminals__out_of_order =
      file_traits<arc>::idx__terminals__out_of_order;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Keep track of the number of unread terminals
    ////////////////////////////////////////////////////////////////////////////
    size_t _unread_terminals[2] = { 0u, 0u };

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a stream unattached to any file.
    ////////////////////////////////////////////////////////////////////////////
    arc_ifstream() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a stream attached to an arc file.
    ////////////////////////////////////////////////////////////////////////////
    arc_ifstream(const levelized_file<arc>& file)
    {
      open(file);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct a stream unattached to a shared arc file.
    ////////////////////////////////////////////////////////////////////////////
    arc_ifstream(const shared_ptr<levelized_file<arc>>& file)
    {
      open(file);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~arc_ifstream() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Open a levelized arc file.
    ///
    /// \pre No `levelized_ofstream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void
    open(const levelized_file<arc>& f)
    {
      // adiar_assert(f.semi_transposed);
      parent_t::open(f);
      _unread_terminals[false] = f.number_of_terminals[false];
      _unread_terminals[true]  = f.number_of_terminals[true];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Open a shared levelized arc file.
    ///
    /// \pre No `levelized_ofstream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void
    open(const shared_ptr<levelized_file<arc>>& f)
    {
      // adiar_assert(f->semi_transposed);
      parent_t::open(f);
      _unread_terminals[false] = f->number_of_terminals[false];
      _unread_terminals[true]  = f->number_of_terminals[true];
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more internal arcs.
    ////////////////////////////////////////////////////////////////////////////
    bool
    can_pull_internal() const
    {
      return parent_t::template can_pull<idx__internal>();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next internal arc (and move the read head).
    ////////////////////////////////////////////////////////////////////////////
    const arc
    pull_internal()
    {
      return parent_t::template pull<idx__internal>();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next internal arc (but do not move the read head).
    ////////////////////////////////////////////////////////////////////////////
    const arc
    peek_internal()
    {
      return parent_t::template peek<idx__internal>();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the next element ought to be pulled from the \em in-order
    ///        index.
    ////////////////////////////////////////////////////////////////////////////
    bool
    take_in_order_terminal()
    {
      const bool in_order_pullable = parent_t::template can_pull<idx__terminals__in_order>();

      const bool out_of_order_pullable =
        parent_t::template can_pull<idx__terminals__out_of_order>();

      if (in_order_pullable != out_of_order_pullable) { return in_order_pullable; }

      const arc::pointer_type in_order_source =
        parent_t::template peek<idx__terminals__in_order>().source();

      const arc::pointer_type out_of_order_source =
        parent_t::template peek<idx__terminals__out_of_order>().source();

      if constexpr (Reverse) {
        return in_order_source < out_of_order_source;
      } else {
        return in_order_source > out_of_order_source;
      }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the number of unread terminals.
    ////////////////////////////////////////////////////////////////////////////
    size_t
    unread_terminals() const
    {
      return _unread_terminals[false] + _unread_terminals[true];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the number of unread terminals of a specific value.
    ////////////////////////////////////////////////////////////////////////////
    const size_t&
    unread_terminals(const bool terminal_value) const
    {
      return _unread_terminals[terminal_value];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the stream contains more terminal arcs.
    ////////////////////////////////////////////////////////////////////////////
    bool
    can_pull_terminal() const
    {
      return unread_terminals() > 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next arc (and move the read head).
    ////////////////////////////////////////////////////////////////////////////
    const arc
    pull_terminal()
    {
      const arc a = take_in_order_terminal()
        ? parent_t::template pull<idx__terminals__in_order>()
        : parent_t::template pull<idx__terminals__out_of_order>();

      adiar_assert(_unread_terminals[a.target().value()] > 0,
                   "Terminal counter should not be zero");
      _unread_terminals[a.target().value()]--;

      return a;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next arc (but do not move the read head).
    ////////////////////////////////////////////////////////////////////////////
    const arc
    peek_terminal()
    {
      return take_in_order_terminal() ? parent_t::template peek<idx__terminals__in_order>()
                                      : parent_t::template peek<idx__terminals__out_of_order>();
    }
  };
}

#endif // ADIAR_INTERNAL_IO_ARC_IFSTREAM_H
