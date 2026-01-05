#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H

#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/levelized_ifstream.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  // TODO (code clarity):
  //   Add to 'File' an enum with 'Ascending'/'Descending' to then derive the comparator in
  //   conjunction with 'Reverse'.

  // TODO: Remove
  template <typename File>
  struct level_ifstream_t
  {
    template <bool Reverse = false>
    using stream_t = level_info_ifstream<Reverse>;
  };

  template <>
  struct level_ifstream_t<file<ptr_uint64::label_type>>
  {
    template <bool Reverse = false>
    using stream_t = ifstream<ptr_uint64::label_type, Reverse>;
  };

  template <>
  struct level_ifstream_t<shared_file<ptr_uint64::label_type>>
  {
    template <bool Reverse = false>
    using stream_t = ifstream<ptr_uint64::label_type, Reverse>;
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Merges the levels from one or more files.
  ///
  /// \tparam File Type of the files to read from.
  ///
  /// \tparam Comp Comparator with which to merge the levels.
  ///
  /// \tparam FileCount  Number of files to read from.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename File, typename Comp, size_t FileCount, bool Reverse = false>
  class level_merger
  {
    static_assert(0 < FileCount, "At least one file should be merged");

    using stream_type = typename level_ifstream_t<File>::template stream_t<Reverse>;

  public:
    static size_t
    memory_usage()
    {
      return FileCount * stream_type::memory_usage();
    }

    using level_type = ptr_uint64::label_type;

  private:
    Comp _comparator = Comp();

    unique_ptr<stream_type> _level_ifstreams[FileCount];

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Attach onto the given list of files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    hook(const File (&fs)[FileCount])
    {
      for (size_t idx = 0u; idx < FileCount; idx++) {
        _level_ifstreams[idx] = adiar::make_unique<stream_type>(fs[idx]);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Attach onto the given list of decision diagrams.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    hook(const dd (&dds)[FileCount])
    {
      for (size_t idx = 0u; idx < FileCount; idx++) {
        _level_ifstreams[idx] =
          adiar::make_unique<stream_type>(dds[idx].file_ptr(), dds[idx].shift());
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Attach onto the given list of (unreduced) decision diagrams.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    hook(const __dd (&dds)[FileCount])
    {
      for (size_t idx = 0u; idx < FileCount; idx++) {
        _level_ifstreams[idx] = adiar::make_unique<stream_type>(dds[idx] /*, dds[idx]._shift*/);
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are more levels to fetch.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_pull()
    {
      for (size_t idx = 0u; idx < FileCount; idx++) {
        if (_level_ifstreams[idx]->can_pull()) { return true; }
      }
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next level.
    ///
    /// \pre `can_pull() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_type
    peek()
    {
      adiar_assert(can_pull(), "Cannot peek past end of all streams");

      bool has_min_level   = false;
      level_type min_level = 0u;
      for (size_t idx = 0u; idx < FileCount; idx++) {
        if (_level_ifstreams[idx]->can_pull()
            && (!has_min_level
                || _comparator(level_of(_level_ifstreams[idx]->peek()), min_level))) {
          has_min_level = true;
          min_level     = level_of(_level_ifstreams[idx]->peek());
        }
      }

      return min_level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next level and go to the next.
    ///
    /// \pre `can_pull() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    level_type
    pull()
    {
      adiar_assert(can_pull(), "Cannot pull past end of all streams");

      level_type min_level = peek();

      // pull from all with min_level
      for (const unique_ptr<stream_type>& level_info_ifstream : _level_ifstreams) {
        if (level_info_ifstream->can_pull() && level_of(level_info_ifstream->peek()) == min_level) {
          level_info_ifstream->pull();
        }
      }
      return min_level;
    }
  };
}

#endif //  ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H
