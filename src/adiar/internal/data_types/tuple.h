#ifndef ADIAR_INTERNAL_DATA_TYPES_TUPLE_H
#define ADIAR_INTERNAL_DATA_TYPES_TUPLE_H

#include <algorithm>
#include <array>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/unreachable.h>

namespace adiar::internal
{
  // TODO: when used with something else than just integers: turn into taking
  //       and returning 'const&' arguments.

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to one elements.

  // 'first(t1)' is not needed, it's just the one element...

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to two elements.
  template <typename T>
  inline const T&
  first(const T& t1, const T& t2)
  {
    return std::min(t1, t2);
  }

  template <typename T>
  inline const T&
  second(const T& t1, const T& t2)
  {
    return std::max(t1, t2);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to three elements.
  template <typename T>
  inline const T&
  first(const T& t1, const T& t2, const T& t3)
  {
    return std::min(t1, std::min(t2, t3));
  }

  template <typename T>
  inline const T&
  second(const T& t1, const T& t2, const T& t3)
  {
    return std::max(std::min(t1, t2), std::min(std::max(t1, t2), t3));
  }

  template <typename T>
  inline const T&
  third(const T& t1, const T& t2, const T& t3)
  {
    return std::max(t1, std::max(t2, t3));
  }

  //////////////////////////////////////////////////////////////////////////////
  // Ordered access to four elements.
  template <typename T>
  inline const T&
  first(const T& t1, const T& t2, const T& t3, const T& t4)
  {
    return std::min(std::min(t1, t2), std::min(t3, t4));
  }

  template <typename T>
  inline const T&
  second(const T& t1, const T& t2, const T& t3, const T& t4)
  {
    return std::min(std::min(std::max(t1, t2), std::max(t3, t4)),
                    std::max(std::min(t1, t2), std::min(t3, t4)));
  }

  template <typename T>
  inline const T&
  third(const T& t1, const T& t2, const T& t3, const T& t4)
  {
    return std::max(std::min(std::max(t1, t2), std::max(t3, t4)),
                    std::max(std::min(t1, t2), std::min(t3, t4)));
  }

  template <typename T>
  inline const T&
  fourth(const T& t1, const T& t2, const T& t3, const T& t4)
  {
    return std::max(std::max(t1, t2), std::max(t3, t4));
  }

  //////////////////////////////////////////////////////////////////////////////
  // TODO (Nested Sweeping Clean Up):
  //   - Map function to create a tuple<B> from tupla<A> and f : A -> B

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Tuple holding elements and providing an ordered access.
  ///
  /// \tparam value_t     The type of elements within the tuple. This part
  ///                     should have a partial ordering '<'.
  ///
  /// \tparam cardinality The number of elements in the tuple
  ///                     (default: `2`)
  ///
  /// \tparam is_sorted   Whether elements are given in sorted order
  ///                     (default: `false`).
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, uint8_t Cardinality = 2, bool IsSorted = false>
  class tuple
  {
    /* ============================== CONSTANTS ============================= */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the elements in the tuple.
    ////////////////////////////////////////////////////////////////////////////
    using value_type = T;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements stored in the tuple.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr uint8_t cardinality = Cardinality;

    static_assert(Cardinality > 0, "A tuple cannot be 'unit' type.");
    static_assert(Cardinality <= 4, "No support (yet) for tuples of that cardinality.");

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether elements are supposed to be in sorted order.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr bool is_sorted = cardinality == 1 || IsSorted;

    /* ============================== ELEMENTS ============================== */
  private:
    std::array<value_type, cardinality> _values;

    /* ============================ DIRECT ACCESS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Direct access to tuple elements in the order they were given.
    ////////////////////////////////////////////////////////////////////////////
    inline const value_type&
    at(const size_t idx) const
    {
      adiar_assert(idx < cardinality, "Tuple index must be within its cardinality.");
      return _values[idx];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Direct access to tuple elements in the order they were given.
    ////////////////////////////////////////////////////////////////////////////
    inline const value_type&
    operator[](const size_t idx) const
    {
      return this->at(idx);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief direct (read-only) access to the underlying array.
    ////////////////////////////////////////////////////////////////////////////
    inline const std::array<value_type, cardinality>&
    data() const
    {
      return this->_values;
    }

    /* =========================== ORDERED ACCESS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the first element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline const value_type&
    first() const
    {
      static_assert(1 <= cardinality,
                    "Must at least be a 1-ary tuple to retrieve the first element.");

      if constexpr (is_sorted) {
        return this->at(0);
      } else if constexpr (cardinality == 2) {
        return adiar::internal::first(_values[0], _values[1]);
      } else if constexpr (cardinality == 3) {
        return adiar::internal::first(_values[0], _values[1], _values[2]);
      } else if constexpr (cardinality == 4) {
        return adiar::internal::first(_values[0], _values[1], _values[2], _values[3]);
      }
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the second element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline const value_type&
    second() const
    {
      static_assert(2 <= cardinality,
                    "Must at least be a 2-ary tuple to retrieve the second element.");

      if constexpr (is_sorted) {
        return this->at(1);
      } else if constexpr (cardinality == 2) {
        return adiar::internal::second(_values[0], _values[1]);
      } else if constexpr (cardinality == 3) {
        return adiar::internal::second(_values[0], _values[1], _values[2]);
      } else if constexpr (cardinality == 4) {
        return adiar::internal::second(_values[0], _values[1], _values[2], _values[3]);
      }
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the second element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline const value_type&
    third() const
    {
      static_assert(3 <= cardinality,
                    "Must at least be a 3-ary tuple to retrieve the third element.");

      if constexpr (is_sorted) {
        return this->at(2);
      } else if constexpr (cardinality == 3) {
        return adiar::internal::third(_values[0], _values[1], _values[2]);
      } else if constexpr (cardinality == 4) {
        return adiar::internal::third(_values[0], _values[1], _values[2], _values[3]);
      }
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Access to the second element wrt. the elements ordering.
    ////////////////////////////////////////////////////////////////////////////
    inline const value_type&
    fourth() const
    {
      static_assert(3 <= cardinality,
                    "Must at least be a 4-ary tuple to retrieve the fourth element.");

      if constexpr (is_sorted) {
        return this->at(3);
      } else if constexpr (cardinality == 4) {
        return adiar::internal::fourth(_values[0], _values[1], _values[2], _values[3]);
      }
      adiar_unreachable(); // LCOV_EXCL_LINE
    }

    /* ============================ CONSTRUCTORS ============================ */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Default construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    tuple() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const tuple& p) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move construction (trivial).
    ///
    /// \details The default, copy, and move construction has to be `default` to
    ///          ensure it is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    tuple(tuple&& p) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Destruction (trivial).
    ///
    /// \details The destructor has to be `default` to ensure it is a *POD* and
    ///          hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    ~tuple() = default;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Copy assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    tuple&
    operator=(const tuple& p) = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Move assignment (trivial).
    ///
    /// \details The copy and move assignment has to be `default` to ensure it
    ///          is a *POD* and hence can be used by TPIE's files.
    ////////////////////////////////////////////////////////////////////////////
    tuple&
    operator=(tuple&& p) = default;

  public:
    // Provide other constructors for ease of use

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Initialize a tuple with all elements being the given one.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const value_type& elem)
      : _values{ elem }
    {
      // TODO: replace with a (templated) default value (?)
      if constexpr (2 <= cardinality) _values[1] = elem;
      if constexpr (3 <= cardinality) _values[2] = elem;
      if constexpr (4 <= cardinality) _values[3] = elem;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a 2-ary tuple with the two given elements.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const value_type& elem1, const value_type& elem2)
      : _values{ elem1, elem2 }
    {
      static_assert(cardinality == 2, "Constructor is only designed for 2-ary tuples.");

      if constexpr (is_sorted) { adiar_assert(elem1 <= elem2); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a 3-ary tuple with the three given elements.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const value_type& elem1, const value_type& elem2, const value_type& elem3)
      : _values{ elem1, elem2, elem3 }
    {
      static_assert(cardinality == 3, "Constructor is only designed for 3-ary tuples.");

      if constexpr (is_sorted) {
        adiar_assert(elem1 <= elem2);
        adiar_assert(elem2 <= elem3);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a 4-ary tuple with the four given elements.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const value_type& elem1,
          const value_type& elem2,
          const value_type& elem3,
          const value_type& elem4)
      : _values{ elem1, elem2, elem3, elem4 }
    {
      static_assert(cardinality == 4, "Constructor is only designed for 4-ary tuples.");

      if constexpr (is_sorted) {
        adiar_assert(elem1 <= elem2);
        adiar_assert(elem2 <= elem3);
        adiar_assert(elem3 <= elem4);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a tuple from a `std::array` of the same cardinality.
    ////////////////////////////////////////////////////////////////////////////
    tuple(const std::array<value_type, cardinality>& elems)
      : _values(elems)
    {
      if constexpr (is_sorted) {
        for (size_t i = 0; i < (cardinality - 1); ++i) {
          adiar_assert(this->at(i) <= this->at(i + 1));
        }
      }
    }

    /* ============================== COMPARATORS =========================== */
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical Less-Than.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator<(const tuple& o) const
    {
      // TODO: manual loop unrolling?
      for (size_t i = 0; i < cardinality; i++) {
        if (this->_values[i] < o._values[i]) { return true; };
        if (this->_values[i] > o._values[i]) { return false; };
      }
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Less-Than comparison with a single element.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator<(const value_type& o) const
    {
      static_assert(cardinality == 1);
      return this->_values[0] < o;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical Greater-Than.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator>(const tuple& o) const
    {
      return (o < *this);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Greater-Than comparison with a single element.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator>(const value_type& o) const
    {
      static_assert(cardinality == 1);
      return this->_values[0] > o;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Point-wise Equality.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator==(const tuple& o) const
    {
      // TODO: manual loop unrolling?
      for (size_t i = 0; i < cardinality; i++) {
        if (this->_values[i] != o._values[i]) { return false; };
      }
      return true;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Equality comparison with a single element.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator==(const value_type& o) const
    {
      static_assert(cardinality == 1);
      return this->_values[0] == o;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Point-wise Inequality.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator!=(const tuple& o) const
    {
      return !(*this == o);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Inequality comparison with a single element.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator!=(const value_type& o) const
    {
      static_assert(cardinality == 1);
      return this->_values[0] != o;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical Less-Than and Equality.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator<=(const tuple& o) const
    {
      return (*this < o) || (*this == o);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Lexicographical Greater-Than and Equality.
    ////////////////////////////////////////////////////////////////////////////
    inline bool
    operator>=(const tuple& o) const
    {
      return (*this > o) || (*this == o);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  // Non-lexicographical comparators

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the first element (in sorted order).
  //////////////////////////////////////////////////////////////////////////////
  template <typename Tuple>
  struct tuple_first_lt
  {
    /// \copydoc tuple_first_lt
    inline bool
    operator()(const Tuple& a, const Tuple& b)
    {
      // Sort primarily by the element to be encountered first.
      // If non-singleton, sort secondly lexicographically.
      if constexpr (Tuple::cardinality == 1) {
        return a.first() < b.first();
      } else {
        return a.first() < b.first() || (a.first() == b.first() && a < b);
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the second element (in sorted order).
  //////////////////////////////////////////////////////////////////////////////
  template <typename Tuple>
  struct tuple_second_lt
  {
    /// \copydoc tuple_second_lt
    inline bool
    operator()(const Tuple& a, const Tuple& b)
    {
      // Sort primarily by the element to be encountered second
      return a.second() < b.second() ||
        // Sort secondly lexicographically.
        (a.second() == b.second() && a < b);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the third element (in sorted order).
  //////////////////////////////////////////////////////////////////////////////
  template <typename Tuple>
  struct tuple_third_lt
  {
    /// \copydoc tuple_third_lt
    inline bool
    operator()(const Tuple& a, const Tuple& b)
    {
      // Sort primarily by the element to be encountered third
      return a.third() < b.third() ||
        // Sort secondly lexicographically.
        (a.third() == b.third() && a < b);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Ordering based on the fourth element (in sorted order).
  //////////////////////////////////////////////////////////////////////////////
  template <typename Tuple>
  struct tuple_fourth_lt
  {
    /// \copydoc tuple_fourth_lt
    inline bool
    operator()(const Tuple& a, const Tuple& b)
    {
      // Sort primarily by the element to be encountered fourth
      return a.fourth() < b.fourth() ||
        // Sort secondly lexicographically.
        (a.fourth() == b.fourth() && a < b);
    }
  };
}

#endif // ADIAR_INTERNAL_DATA_TYPES_TUPLE_H
