#ifndef ADIAR_INTERNAL_DATA_TYPES_UID_H
#define ADIAR_INTERNAL_DATA_TYPES_UID_H

#include <stdexcept>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>

namespace adiar::internal
{
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
    /// \brief Constructor for a uid of an internal node (label, id).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    __uid(const pointer_type& p)
      : pointer_type(essential(p))
    {
      adiar_assert(!p.is_nil(), "UID must be created from non-nil value");
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
    __uid(const typename pointer_type::label_type label, const typename pointer_type::id_type id)
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
    __uid(typename pointer_type::terminal_type v)
      : pointer_type(v)
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this uid identifies a terminal node.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // This functions is overwritten, such that we can provide a specialization for
    // '__uid<ptr_uint64>'.
    inline bool
    is_terminal() const
    {
      return this->is_terminal();
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

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Specialization for the single-integer pointer `ptr_uint64`.

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the `ptr` for this node uid with the given `out_idx` as auxiliary information.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <>
  inline ptr_uint64
  __uid<ptr_uint64>::as_ptr(const ptr_uint64::out_idx_type out_idx) const
  {
    adiar_assert(pointer_type::is_node());

    // Based on the bit-layout, we can do this much faster than decode and
    // re-encode all three values.
    constexpr pointer_type::raw_type out_idx_mask = ~(max_out_idx << pointer_type::data_shift);

    return pointer_type(
      (this->_raw & out_idx_mask)
      | (static_cast<pointer_type::raw_type>(out_idx) << pointer_type::data_shift));
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
