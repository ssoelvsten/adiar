#ifndef ADIAR_INTERNAL_DATA_TYPES_UID_H
#define ADIAR_INTERNAL_DATA_TYPES_UID_H

#include <stdexcept>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>

namespace adiar::internal
{
  // TODO (ADD, EVBDD, QMDD, ...):
  //   The templated overloads below, `cnot`, `replace`, ..., will clash with the templated
  //   `ptr_...` functions. In this case, add `static const bool is_uid = true` here and `false` on
  //   the pointer implementation. With this, the overload resoultion can be done using SFINAE.

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   A unique identifier a decision diagram node.
  ///
  /// \details This essentially is a *ptr* guaranteed to point to a node, i.e. it is \em never nil,
  ///          and without any associated information, e.g. \em without a flag.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Ptr>
  class __uid : public Ptr
  {
  public:
    using pointer_type = Ptr;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move constructor has to be `default` to ensure it is a *POD*
    ///          and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __uid() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move constructor has to be `default` to ensure it is a *POD*
    ///          and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __uid(const __uid<pointer_type>& p) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move constructor has to be `default` to ensure it is a *POD*
    ///          and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __uid(__uid<pointer_type>&& p) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and hence can be used by
    ///          TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~__uid() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment
    ///
    /// \details The default, copy, and move assignment has to be `default` to ensure it is a *POD*
    ///          and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __uid&
    operator=(const __uid<pointer_type>& p) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment
    ///
    /// \details The default, copy, and move assignment has to be `default` to ensure it is a *POD*
    ///          and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __uid&
    operator=(__uid<pointer_type>&& p) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid from a (non-NIL) pointer.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit __uid(const pointer_type& p)
      : pointer_type(essential(p))
    {
      adiar_assert(!p.is_nil(), "UID must be created from non-nil value");
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsafe construction of a uid from a (non-NIL) pointer.
    ///
    /// \details This is to be used in the cases one is very, very sure that the given pointer
    ///          already satisfies all requirements to be a UID.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static inline __uid
    unsafe(const pointer_type& p)
    {
      adiar_assert(!p.is_nil(), "UID cannot be nil");
      adiar_assert(!p.is_flagged(), "UID cannot be flagged");
      adiar_assert(p.is_terminal() || p.out_idx() == 0, "UID has no out-index");

      // Evil type hack to reinterpret `p` as a `__uid<...>` (Thanks, Quake III)
      return *static_cast<const __uid*>(&p);
    }

    /* ======================================= ATTRIBUTES ======================================= */
    // Remove anything related to the flag.

    bool
    is_flagged() = delete;

    /* =========================================== nil ========================================== */
    // Remove anything related to nil

    static inline constexpr pointer_type
    nil() = delete;

    bool
    is_nil() = delete;

    /* ========================================== NODES ========================================= */
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a pointer to an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit __uid(const typename pointer_type::label_type label,
                   const typename pointer_type::id_type id)
      : pointer_type(label, id)
    {}

    // Remove anything related to out-index

    typename pointer_type::out_idx_type
    out_idx() = delete;

    /* ======================================== TERMINALS ======================================= */
  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for a uid of a terminal node (v).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    explicit __uid(typename pointer_type::terminal_type v)
      : pointer_type(v)
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this uid identifies a terminal node.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // cppcheck-suppress-begin [duplInheritedMember]
    //   This functions is overwritten, such that we can provide a specialization for
    //   '__uid<ptr_uint64>'.
    inline bool
    is_terminal() const
    {
      return this->as_ptr().is_terminal();
    }

    // cppcheck-suppress-end

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain a pointer where terminal values (if any) are negated.
    //////////////////////////////////////////////////////////////////////////////////////////////////
    // cppcheck-suppress-begin [duplInheritedMember]
    __uid<pointer_type>
    operator!() const
    {
      return unsafe(!this->as_ptr());
    }

    // cppcheck-suppress-end

    /* ==================================== POINTER CONVERSION ================================== */

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the `ptr` for this node uid with no auxiliary information.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline pointer_type
    as_ptr() const
    {
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the `ptr` for this node uid with the given `out_idx` as auxiliary information.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    inline pointer_type
    as_ptr(const typename pointer_type::out_idx_type out_idx) const
    {
      adiar_assert(this->is_node());
      return pointer_type(this->label(), this->id(), out_idx);
    }
  };

  /* ========================================= TERMINAL ========================================= */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Negates the content of `p` if it is a terminal and the `negate` flag is set to true.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Uid>
  inline Uid
  cnot(const Uid& u, const bool negate)
  {
    return Uid::unsafe(cnot(static_cast<typename Uid::pointer_type>(u), negate));
  }

  /* =========================================== LEVEL ========================================== */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replaces the level with the one given.
  ///
  /// \pre `u.is_node()`
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Uid>
  inline Uid
  replace(const Uid& u, const typename Uid::level_type new_level)
  {
    return Uid::unsafe(replace(static_cast<typename Uid::pointer_type>(u), new_level));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Replaces the level with the one given.
  ///
  /// \pre `u.is_node()`
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Uid>
  inline Uid
  essential_replace(const Uid& u, const typename Uid::level_type new_level)
  {
    return replace(u, new_level);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Shift the level by given amount.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Uid>
  inline Uid
  shift_replace(const Uid& u, const typename Uid::signed_level_type levels)
  {
    return Uid::unsafe(shift_replace(static_cast<typename Uid::pointer_type>(u), levels));
  }

  /* ============================= SPECIALIZATION FOR `ptr_uint64` ============================== */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the `ptr` for this node uid with the given `out_idx` as auxiliary information.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <>
  inline ptr_uint64
  __uid<ptr_uint64>::as_ptr(const ptr_uint64::out_idx_type out_idx) const
  {
    adiar_assert(pointer_type::is_node());

    // Based on the bit-layout, we can do this much faster than decode and
    // re-encode all three values. We especially can abuse the fact, that
    // the uid already has 0's where the `out_idx` has to go.
    return pointer_type(
      this->_raw | (static_cast<pointer_type::raw_type>(out_idx) << pointer_type::data_shift));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Whether this uid identifies a terminal node.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <>
  inline bool
  __uid<ptr_uint64>::is_terminal() const
  {
    // Since uid never is nil, then this is a slightly a faster logic than the one in 'ptr_uint64'
    // itself. Here, we exploit the fact that if it cannot be nil, then terminals are the largest
    // values. This skips the right-shift instruction.
    return pointer_type::min_terminal <= this->_raw;
  }

  using uid_uint64 = __uid<ptr_uint64>;
}

#endif // ADIAR_INTERNAL_DATA_TYPES_UID_H
