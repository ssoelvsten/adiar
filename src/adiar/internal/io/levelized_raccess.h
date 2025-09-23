#ifndef ADIAR_INTERNAL_IO_LEVELIZED_RACCESS_H
#define ADIAR_INTERNAL_IO_LEVELIZED_RACCESS_H

#include <tpie/array.h>

#include <adiar/internal/dd.h>
#include <adiar/internal/io/levelized_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Random-access to the contents of a levelized file.
  ///
  /// \tparam StreamType Stream to wrap with a *levelized random access* buffer.
  //
  // TODO: Generalize parts of 'node_raccess' to reuse it with levelized files with other
  //       types of content. Yet, what use-case do we have for this?
  //
  // TODO: Support 'StreamType<Reverse>'
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename StreamType>
  class levelized_raccess
  {
  private:
    using stream_type = StreamType /*<Default (top-down) direction>*/;

  public:
    using value_type = typename stream_type::value_type;

    using pointer_type      = typename value_type::pointer_type;
    using uid_type          = typename value_type::uid_type;
    using signed_label_type = typename value_type::signed_label_type;
    using idx_type          = typename value_type::id_type;

  public:
    static size_t
    memory_usage(size_t max_width)
    {
      return stream_type::memory_usage() + tpie::array<value_type>::memory_usage(max_width);
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief File stream to obtain the contents of each level
    ////////////////////////////////////////////////////////////////////////////////////////////////
    stream_type _ifstream;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Maximum width of the contents of 'lfs'. This is the maximum number of elements needed
    ///        to be placed within.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const idx_type _max_width;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer with all elements of the current level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    tpie::array<value_type> _level_buffer;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer with all elements of the current level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool _has_curr_level = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer with all elements of the current level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    signed_label_type _curr_level = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Width of the current level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    idx_type _curr_width = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Root of the diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const uid_type _root;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // levelized_raccess()
    // { }
    //
    // TODO: Add 'open(...)', 'is_open()' 'close()' working multi-usage.

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a mutable levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    levelized_raccess(levelized_file<T>& f,
                      const bool negate                          = false,
                      const typename T::signed_label_type& shift = 0)
      : _ifstream(f, negate, shift)
      , _max_width(f.width)
      , _level_buffer(f.width)
      , _root(_ifstream.peek().uid())
    {
      init();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to an immutable levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    levelized_raccess(const levelized_file<T>& f,
                      const bool negate                         = false,
                      const typename T::signed_label_type shift = 0)
      : _ifstream(f, negate, shift)
      , _max_width(f.width)
      , _level_buffer(f.width)
      , _root(_ifstream.peek().uid())
    {
      init();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to an immutable shared levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    levelized_raccess(const shared_ptr<levelized_file<T>>& f,
                      const bool negate                         = false,
                      const typename T::signed_label_type shift = 0)
      : _ifstream(f, negate, shift)
      , _max_width(f->width)
      , _level_buffer(f->width)
      , _root(_ifstream.peek().uid())
    {
      init();
    }

  private:
    void
    init()
    {
      adiar_assert(_ifstream.can_pull(), "given file should be non-empty");

      // Skip the terminal node for terminal only BDDs. This way, 'has_next_level' is a mere
      // 'can_pull' on the underlying stream.
      if (_root.is_terminal()) { _ifstream.pull(); }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Return root of the diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    uid_type
    root() const
    {
      return _root;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are any more levels.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_next_level() const
    {
      return _ifstream.can_pull();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The immediate next available non-empty level.
    ///
    /// \pre `has_next_level() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    signed_label_type
    next_level()
    {
      return _ifstream.peek().uid().label();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sets up the random access buffer for the specified level.
    ///
    /// \param level The next level to provide random access to. If the requested level does not
    ///              exist, then the buffer will be empty.
    ///
    /// \pre `has_current_level() == false` or `current_level() < level`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    setup_next_level(const signed_label_type level)
    {
      adiar_assert(!has_current_level() || current_level() < level);

      // Set to new level and mark the entire buffer's content garbage.
      _has_curr_level = true;
      _curr_level     = level;
      _curr_width     = 0;

      // Stop early when going "beyond" the available levels
      if (!has_next_level()) { return; }

      // Skip all levels not of interest
      while (_ifstream.can_pull()
             && static_cast<signed_label_type>(_ifstream.peek().uid().label()) < level) {
        _ifstream.pull();
      }

      // Copy over all elements from the requested level
      while (_ifstream.can_pull()
             && static_cast<signed_label_type>(_ifstream.peek().uid().label()) == level) {
        _level_buffer[_curr_width++] = _ifstream.pull();
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sets up the random access buffer for the immediate next available non-empty level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    setup_next_level()
    {
      setup_next_level(next_level());
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there is any current level to access elements from.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_current_level() const
    {
      return _has_curr_level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the current level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    signed_label_type
    current_level() const
    {
      return _curr_level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The width of the current level, i.e. the number of elements one can access to.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    current_width() const
    {
      return _curr_width;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the current level is empty.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty_level() const
    {
      return _curr_width == 0u;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the element of the current level at the given index.
    ///
    /// \pre `idx < current_width()`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const value_type&
    at(idx_type idx) const
    {
      adiar_assert(idx < current_width());
      return _level_buffer[idx];
    }
  };
}

#endif // ADIAR_INTERNAL_IO_LEVELIZED_RACCESS_H
