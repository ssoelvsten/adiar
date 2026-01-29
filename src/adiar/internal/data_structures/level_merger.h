#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H

#include <array>
#include <variant>

#include <adiar/functional.h>
#include <adiar/type_traits.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/levelized_ifstream.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Merges the levels from several inputs.
  ///
  /// \tparam Comp The comparator with which to merge the levels.
  ///
  /// \tparam InputCount Number of inputs.
  ///
  /// \remark Currently, variadic inputs are supported by use of `virtual` functions, i.e. by use of
  ///         runtime-resolved inheritance. Yet, this is always used in a context where the type of
  ///         the individual arguments are known. Hence, one should be able to replace `InputCount`
  ///         with a compile-time known list `<typename... Types>`. To this end, a deduction guide
  ///         might be useful.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Comp, size_t InputCount>
  class level_merger
  {
  public:
    using value_type = dd::label_type;
    using arg_type   = std::variant<dd, __dd, generator<value_type>>;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Move to <adiar/internal/io/...> ?
    class istream
    {
    public:
      using value_type = level_merger::value_type;

      virtual ~istream() = default;

    public:
      virtual bool
      can_pull() = 0;

      virtual value_type
      peek() = 0;

      virtual value_type
      pull() = 0;
    };

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Wrapper for `generator`s to provide the same interface as an `ifstream`, including
    ///        the ability to `peek()`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Move to <adiar/internal/util.h> or <adiar/internal/io/...> ?
    class generator_istream : public istream
    {
    private:
      const generator<value_type> _gen;
      optional<value_type> _next;

    public:
      generator_istream(const generator<value_type>& gen)
        : _gen(gen)
      {
        this->_next = this->_gen();
      }

      ~generator_istream() = default;

    public:
      bool
      can_pull() override
      {
        return this->_next.has_value();
      }

      value_type
      peek() override
      {
        return this->_next.value();
      }

      value_type
      pull() override
      {
        const value_type ret = this->_next.value();
        this->_next          = this->_gen();
        return ret;
      }
    };

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    class level_info_istream : public istream
    {
    private:
      level_info_ifstream<> _ifstream;

    public:
      template <typename T>
      level_info_istream(const T& t)
        : _ifstream(t)
      {}

      ~level_info_istream() = default;

    public:
      bool
      can_pull() override
      {
        return this->_ifstream.can_pull();
      }

      value_type
      peek() override
      {
        return this->_ifstream.peek().level();
      }

      value_type
      pull() override
      {
        return this->_ifstream.pull().level();
      }
    };

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Manages initialization and access to an input of levels.
    ///
    /// \details Since the `internal::ifstream` is not copy-constructable, then we have to create
    ///          the object on the heap.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    class istream_ptr
    {
    public:
      using value_type = typename istream::value_type;

    private:
      unique_ptr<istream> _ptr;

    public:
      /// \brief Conversion for reduced diagrams (top-down).
      istream_ptr(const dd& diagram)
        : _ptr(adiar::make_unique<level_info_istream>(diagram))
      {}

      /// \brief Conversion for unreduced diagrams (bottom-up).
      istream_ptr(const __dd& diagram)
        : _ptr(adiar::make_unique<level_info_istream>(diagram))
      {}

      /// \brief Conversion for files with levels
      template <typename T>
      istream_ptr(const levelized_file<T>& f)
        : _ptr(adiar::make_unique<level_info_istream>(f))
      {}

      /// \brief Conversion for files with levels
      template <typename T>
      istream_ptr(const shared_ptr<levelized_file<T>>& f)
        : _ptr(adiar::make_unique<level_info_istream>(f))
      {}

      /// \brief Conversion for generators. This requires the addition of an intermediate lambda
      ///        which takes care of converting to `value_type`.
      template <typename Generator,
                typename = enable_if<!is_constructible<level_info_ifstream<>, Generator>>>
      istream_ptr(const Generator& gen)
        : _ptr(adiar::make_unique<generator_istream>([=]() -> optional<value_type> {
          const auto x = gen();
          if (!x.has_value()) { return {}; }
          return x.value();
        }))
      {}

    public:
      istream*
      operator*()
      {
        return this->_ptr.get();
      }

      istream*
      operator->()
      {
        return *(*this);
      }
    };

  public:
    static size_t
    memory_usage()
    {
      // NOTE: We assume that all `ifstream`s will use the same amount of memory.
      return InputCount * level_info_ifstream<>::memory_usage();
    }

  private:
    Comp _comparator = Comp();
    std::array<istream_ptr, InputCount> _istream_ptrs;

  public:
    level_merger(const level_merger&) = delete;
    level_merger(level_merger&&)      = delete;

    level_merger(std::array<istream_ptr, InputCount>&& args)
      : _istream_ptrs(std::move(args))
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are more levels to fetch.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    can_pull()
    {
      for (istream_ptr& p : this->_istream_ptrs) {
        if (p->can_pull()) { return true; }
      }
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next level.
    ///
    /// \pre `can_pull() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    peek()
    {
      adiar_assert(can_pull(), "Cannot peek past end of all streams");

      bool has_min_level   = false;
      value_type min_level = 0u;
      for (istream_ptr& p : this->_istream_ptrs) {
        if (!p->can_pull()) { continue; }

        if (!has_min_level || _comparator(level_of(p->peek()), min_level)) {
          has_min_level = true;
          min_level     = level_of(p->peek());
        }
      }

      return min_level;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next level and go to the next.
    ///
    /// \pre `can_pull() == true`
    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type
    pull()
    {
      adiar_assert(can_pull(), "Cannot pull past end of all streams");

      value_type min_level = peek();

      // pull from all with min_level
      for (istream_ptr& p : this->_istream_ptrs) {
        if (p->can_pull() && level_of(p->peek()) == min_level) { p->pull(); }
      }
      return min_level;
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H
