#ifndef ADIAR_INTERNAL_DATA_TYPES_ARC_H
#define ADIAR_INTERNAL_DATA_TYPES_ARC_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/data_types/uid.h>

namespace adiar::internal
{
  // TODO (LDD):
  //   Make the source a weighted pointer.

  // TODO (QMDD):
  //   Same as for LDD but with the weight specifically being complex values.

  //////////////////////////////////////////////////////////////////////////////
  /// \brief An arc from some source to a target.
  ///
  /// \details An arc contains a value for the source and one for the target.
  ///          Notice, that we don't specifically have a member for is_high. The
  ///          reason for this choice is that the C++ compiler makes everything
  ///          word-aligned. That means with an explicit is_high member it would
  ///          take up 3 x 64 bits rather than only 2 x 64 bits.
  ///
  /// \remark If `source` is flagged, then this is a high arc rather than a low
  ///         arc.
  ///
  /// \remark  `source` may be nil
  //////////////////////////////////////////////////////////////////////////////
  class arc
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the pointer of this arc's source and target.
    ////////////////////////////////////////////////////////////////////////////
    using pointer_type = ptr_uint64;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the a node's unique identifier.
    ////////////////////////////////////////////////////////////////////////////
    using uid_type = __uid<pointer_type>;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of variable label.
    ////////////////////////////////////////////////////////////////////////////
    using label_type = pointer_type::label_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type able to hold the label of a variable.
    ////////////////////////////////////////////////////////////////////////////
    using signed_label_type = pointer_type::signed_label_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of terminal values.
    ////////////////////////////////////////////////////////////////////////////
    using terminal_type = pointer_type::terminal_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of this node's level identifier.
    ////////////////////////////////////////////////////////////////////////////
    using id_type = pointer_type::id_type;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The maximal possible value for this nodes level identifier.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr id_type max_id = pointer_type::max_id;

  private:
    pointer_type _source;

    // TODO (Attributed Edges):
    //   Add logic related to flag on 'target'.
    //   At that point, 'target()' should always return the unflagged value?

    pointer_type _target;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move assignment has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    arc() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move assignment has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    arc(const arc& a) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move assignment has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    arc(arc&& a) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and
    ///          hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ~arc() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    arc&
    operator=(const arc& a) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    arc&
    operator=(arc&& a) = default;

  public:
    // Provide 'non-default' constructors to make it easy to use outside of TPIE.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct an arc with the given source and target.
    ///
    /// \pre The flags on both `source` and `target` are already set correctly
    ///      and the out-index on `source` is too.
    ////////////////////////////////////////////////////////////////////////////
    arc(const pointer_type& source, const pointer_type& target)
      : _source(source)
      , _target(target)
    {
      adiar_assert(!target.is_node() || target.out_idx() == 0u);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct an arc with the given source and target.
    ///
    /// \pre The flags on `target` is already set correctly.
    ////////////////////////////////////////////////////////////////////////////
    arc(const uid_type& source,
        const pointer_type::out_idx_type& out_idx,
        const pointer_type& target)
      : _source(source.as_ptr(out_idx))
      , _target(target)
    {
      adiar_assert(!target.is_node() || target.out_idx() == 0u);
    }

    /* ================================ NODES =============================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain 'source' value (including flag and out-index).
    ////////////////////////////////////////////////////////////////////////////
    // TODO Always return the essential pointer?
    pointer_type
    source() const
    {
      return _source;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain 'target' value (including flag).
    ////////////////////////////////////////////////////////////////////////////
    // TODO Always return the essential pointer?
    pointer_type
    target() const
    {
      return _target;
    }

    /* ================================= FLAGS ============================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the outdegree index, i.e. the child number this arc is of
    ///        the `source` node.
    ////////////////////////////////////////////////////////////////////////////
    pointer_type::out_idx_type
    out_idx() const
    {
      return _source.out_idx();
    }

    /* ============================== COMPARATORS =========================== */

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check that both `source` and `target` match.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator==(const arc& o) const
    {
      return this->_source == o._source && this->_target == o._target;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Check either `source` or `target` mismatch.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator!=(const arc& o) const
    {
      return !(*this == o);
    }

    /* =============================== OPERATORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain an arc where the target's value (if any) is negated.
    ////////////////////////////////////////////////////////////////////////////
    arc
    operator!() const
    {
      return arc(this->_source, !this->_target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate: First on unique identifier of the source, and
  ///        secondly on whether it is the high arc.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_source_lt
  {
    /// \copydoc arc_target_lt
    bool
    operator()(const arc& a, const arc& b) const
    {
      return a.source() < b.source();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sorting predicate on the target.
  //////////////////////////////////////////////////////////////////////////////
  struct arc_target_lt
  {
    /// \copydoc arc_target_lt
    bool
    operator()(const arc& a, const arc& b) const
    {
      return a.target() < b.target()
#ifndef NDEBUG
        || (a.target() == b.target() && a.source() < b.source())
#endif
        ;
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_ARC_H
