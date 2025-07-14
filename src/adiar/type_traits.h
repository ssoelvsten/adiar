#ifndef ADIAR_TYPE_TRAITS_H
#define ADIAR_TYPE_TRAITS_H

#include <type_traits>

namespace adiar
{
  template <bool Condition, typename Type = void>
  using enable_if = typename std::enable_if<Condition, Type>::type;

  template <typename A, typename B>
  inline constexpr bool is_convertible = std::is_convertible<A, B>::value;

  template <typename A, typename B>
  inline constexpr bool is_same = std::is_same<A, B>::value;

  template <typename A, typename B>
  inline constexpr bool needs_conversion = !is_same<A, B> && is_convertible<A, B>;

  template <typename A>
  inline constexpr bool is_const = std::is_const<std::remove_reference_t<A>>::value;

  template <typename A>
  inline constexpr bool is_mutable = !is_const<A>;

  template <typename A>
  inline constexpr bool is_integral = std::is_integral<A>::value;

  template <typename A>
  inline constexpr bool is_void = std::is_void<A>::value;
}

#endif // ADIAR_TYPE_TRAITS_H
