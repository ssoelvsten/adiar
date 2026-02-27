#ifndef ADIAR_TYPES_H
#define ADIAR_TYPES_H

#include <optional>
#include <utility>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Possible values to assign a variable.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  enum class assignment : signed char
  {
    /** Assigning to `false`. */
    False = 0,
    /** Assigning to `true`. */
    True = 1,
    /** Assigning to no value (i.e. both `true` and `false` are valid). */
    None = -1
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   Possible guarantees of a variable permutation/remapping `m` of type `int -> int`.
  ///
  /// \details The ordering on the values of `replace_type` reflect the strength of the guarantee.
  ///          For example, `replace_type::Affine < replace_type::Monotone` since any affine
  ///          function is also monotone.
  ///
  /// \see bdd_replace
  //////////////////////////////////////////////////////////////////////////////////////////////////
  enum class replace_type : signed char
  {
    /** Any variable remapping without any guarantees on `m`. */
    Non_Monotone = 8, // TODO

    /* (Combination of 'Jump_Up' and 'Jump_Down'): Variable remapping which only swaps one pair of
                                                   variables in separate blocks.
    */
    // Swap = 7, // TODO

    /* (Single-sweep version of 'Non_Monotone'): Variable remapping, only moving some variables up
                                                 without crossing other variables that are moving.
    */
    // Jump_Up = 6, // TODO

    /* (Single-sweep version of 'Non_Monotone'): Variable remapping, only moving some variables down
                                                 without crossing other variables that are moving.
    */
    // Jump_Down = 5, // TODO

    /* Variable remapping, only swapping variables that are adjacent to each other.
     */
    // Swap_Adjacent = 4, // TODO

    /** For any `x < y` then the mapped values preserve that order, i.e. `m(x) < m(y)`. */
    Monotone = 3,

    /* (faster version of 'Monotone'): `m(x) = ax + b` for some integers `a` and `b`. */
    // Affine = 2, // TODO: Is there a use-case where this type of on-the-fly remapping is needed?

    /** (faster version of 'Monotone'): `m(x) = x + b` for an integer `b`. */
    Shift = 1,

    /** Nothing needs to be done, as `m(x) = x`. */
    Identity = 0
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief A pair of values.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T1, typename T2>
  using pair = std::pair<T1, T2>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an `adiar::pair`, deducing the target type based on the types of the arguments.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T1, typename T2>
  constexpr pair<T1, T2>
  make_pair(const T1& t1, const T2& t2)
  {
    return std::make_pair(t1, t2);
  }

  /*
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an `adiar::pair`, deducing the target type based on the types of the arguments.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template<typename T1, typename T2>
  constexpr pair<T1, T2>
  make_pair(T1 &&t1, T2 &&t2)
  { return std::make_pair(std::move(t1), std::move(t2)); }
  */

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief An optional value, i.e. a possibly existent value.
  ///
  /// \details Not having a value is for example used to indicate the end of streams and generators.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using optional = std::optional<T>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an empty `adiar::optional`, i.e. *None*.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  constexpr optional<T>
  make_optional()
  {
    return optional<T>();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an `adiar::optional` with *Some* value.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  constexpr optional<T>
  make_optional(const T& t)
  {
    return std::make_optional(t);
  }

  /*
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Create an `adiar::optional` with *Some* value.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template<typename T>
  constexpr optional<T>
  make_optional(T &&t)
  { return std::make_optional(std::move(t)); }
  */
}

#endif // ADIAR_TYPES_H
