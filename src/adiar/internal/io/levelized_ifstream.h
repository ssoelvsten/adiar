#ifndef ADIAR_INTERNAL_IO_LEVELIZED_IFSTREAM_H
#define ADIAR_INTERNAL_IO_LEVELIZED_IFSTREAM_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/unreachable.h>

// TODO: move?
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/node_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief         File stream of levelized files.
  ///
  /// \param T       The type of the file(s)'s elements
  ///
  /// \param Reverse Whether the reading direction should be reversed
  ///
  /// \remark Since the content of all levelized files are generated in Reverse of the desired
  ///         reading order, then 'reversing' the reversed input is equivalent to not reversing the
  ///         underlying stream. Hence, we do hide a negation of the \em Reverse parameter.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T, bool Reverse = false>
  class levelized_ifstream
  {
  public:
    using value_type = T;

    static constexpr size_t streams = file_traits<value_type>::files;
    static_assert(0 < streams, "There must be at least a single file to attach to.");

    static size_t
    memory_usage()
    {
      return streams * ifstream<value_type, Reverse>::memory_usage();
    }

  protected:
    ifstream<value_type, Reverse> _ifstreams[streams];

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create unattached to any file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_ifstream()
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_ifstream(const levelized_file<value_type>& lf)
    {
      open(lf);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a shared levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    levelized_ifstream(const shared_ptr<levelized_file<value_type>>& lf)
    {
      open(lf);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~levelized_ifstream() = default; // <-- detach is within 'ifstream'.

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open a levelized file.
    ///
    /// \pre No `levelized_ofstream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(const levelized_file<value_type>& f)
    {
      if (!f.exists()) f.__touch();

      for (size_t s_idx = 0; s_idx < streams; s_idx++)
        _ifstreams[s_idx].open(f._files[s_idx], nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open a shared levelized file.
    ///
    /// \pre No `levelized_ofstream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(const shared_ptr<levelized_file<value_type>>& f)
    {
      if (!f->exists()) f->touch();

      for (size_t s_idx = 0; s_idx < streams; s_idx++) _ifstreams[s_idx].open(f->_files[s_idx], f);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this stream is attached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_open() const
    {
      const bool res = _ifstreams[0].is_open();
#ifndef NDEBUG
      // TODO: trust the compiler to notice this is an empty for-loop?
      for (size_t s_idx = 1; s_idx < streams; s_idx++) {
        adiar_assert(_ifstreams[s_idx].is_open() == res, "Attachment ought to be synchronised.");
      }
#endif
      return res;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches the stream from a levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    close()
    {
      for (size_t s_idx = 0; s_idx < streams; s_idx++) _ifstreams[s_idx].close();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reset the read heads back to the beginning (relatively to the reading direction).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    reset()
    {
      for (size_t s_idx = 0; s_idx < streams; s_idx++) _ifstreams[s_idx].reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the sub-stream contains more elements.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <size_t s_idx>
    bool
    can_pull() const
    {
      static_assert(s_idx < streams, "Sub-stream index must be within [0; streams).");
      return _ifstreams[s_idx].can_pull();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain next element from a sub-stream (and move its read head).
    ///
    /// \pre `can_pull<s_idx>() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <size_t s_idx>
    value_type
    pull()
    {
      static_assert(s_idx < streams, "Sub-stream index must be within [0; streams)");
      return _ifstreams[s_idx].pull();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element from the specified sub-stream (but do not move the read
    ///        head).
    ///
    /// \pre `can_pull<s_idx>() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <size_t s_idx>
    value_type
    peek()
    {
      static_assert(s_idx < streams, "Sub-stream index must be within [0; streams)");
      return _ifstreams[s_idx].peek();
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Stream to access per-level meta information.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <bool Reverse = false>
  class level_info_ifstream : public ifstream<level_info, !Reverse>
  {
    using parent_type = ifstream<level_info, !Reverse>;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of levels with which an element ought to be shifted.
    //
    // TODO: Specialize `level_info_ifstream` for node files?
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info::signed_level_type _shift = 0;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info_ifstream() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a file<level_info>.
    ///
    /// \param shift_levels
    ///    Number of variable labels (and levels) to shift by.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info_ifstream(const file<level_info>& f, level_info::signed_level_type shift = 0)
      : _shift(shift)
    {
      parent_type::open(f);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a file<level_info>.
    ///
    /// \param shift_levels
    ///    Number of variable labels (and levels) to shift by.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info_ifstream(const adiar::shared_ptr<file<level_info>>& f,
                        level_info::signed_level_type shift = 0)
      : _shift(shift)
    {
      parent_type::open(f);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename value_type>
    level_info_ifstream(const levelized_file<value_type, false>& lf,
                        level_info::signed_level_type shift = 0)
    {
      open(lf, shift);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename value_type>
    level_info_ifstream(const adiar::shared_ptr<levelized_file<value_type, false>>& lf,
                        level_info::signed_level_type shift = 0)
    {
      open(lf, shift);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a (reduced) decision diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info_ifstream(const dd& diagram)
    {
      open(diagram);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a (possibly unreduced) decision diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_info_ifstream(const __dd& diagram)
    {
      open(diagram);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~level_info_ifstream() = default; // <-- detach in '~file<level_info>()'

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open to a levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename value_type>
    void
    open(const levelized_file<value_type, false>& lf, level_info::signed_level_type shift = 0)
    {
      if (!lf.exists()) lf.__touch();
      parent_type::open(lf._level_info_file, nullptr);
      this->_shift = shift;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open to a shared levelized file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename value_type>
    void
    open(const adiar::shared_ptr<levelized_file<value_type, false>>& lf,
         level_info::signed_level_type shift = 0)
    {
      if (!lf->exists()) lf->touch();
      parent_type::open(lf->_level_info_file, lf);
      this->_shift = shift;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open to a (reduced) decision diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(const dd& diagram)
    {
      open<node>(diagram.file_ptr());
      this->_shift = diagram.shift();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open to a (possibly unreduced) decision diagram.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(const __dd& diagram)
    {
      if (diagram.has<__dd::shared_arc_file_type>()) {
        open<arc>(diagram.get<__dd::shared_arc_file_type>());
        this->_shift = 0;
      } else if (diagram.has<__dd::shared_node_file_type>()) {
        open<node>(diagram.get<__dd::shared_node_file_type>());
        this->_shift = diagram._shift;
      } else {
        // We should never be in the case of hooking into a 'no_file'. That type should only be used
        // internally within an algorithm and never escape into its output.
        adiar_unreachable(); // LCOV_EXCL_LINE
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain next element (and move the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const level_info
    pull()
    {
      return shift_replace(parent_type::pull(), this->_shift);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain next element (without moving the read head).
    ///
    /// \pre `can_pull() == true`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const level_info
    peek()
    {
      return shift_replace(parent_type::peek(), this->_shift);
    }
  };
}

#endif // ADIAR_INTERNAL_IO_LEVELIZED_IFSTREAM_H
