#include "zdd.h"

#include <memory>

#include <adiar/bdd/bdd_policy.h>
#include <adiar/domain.h>
#include <adiar/exception.h>
#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/convert.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/ifstream.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // '__zdd' Constructors
  __zdd::__zdd()
    : internal::__dd()
  {}

  __zdd::__zdd(const shared_node_file_type& f)
    : internal::__dd(f)
  {}

  __zdd::__zdd(const shared_arc_file_type& f, const exec_policy& ep)
    : internal::__dd(f, ep)
  {}

  __zdd::__zdd(const zdd& dd)
    : internal::__dd(dd)
  {}

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // 'zdd' Constructors
  zdd::zdd()
    : zdd(zdd_empty())
  {}

  zdd::zdd(terminal_type t)
    : zdd(zdd_terminal(t))
  {}

  zdd::zdd(const shared_node_file_type& A, bool negate, signed_label_type shift)
    : internal::dd(A, negate, shift)
  {
    adiar_assert(A->is_terminal() || shift == 0,
                 "TODO: Support for shifted ZDD inputs in operations");
  }

  zdd::zdd(const zdd& A)
    : internal::dd(A)
  {}

  zdd::zdd(zdd&& A)
    : internal::dd(A)
  {}

  zdd::zdd(__zdd&& A)
    : internal::dd(internal::reduce<zdd_policy>(std::move(A)))
  {}

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Operators
#define __ZDD_OPER(out_t, op)                          \
  out_t operator op(__zdd&& lhs, __zdd&& rhs)          \
  {                                                    \
    return zdd(std::move(lhs)) op zdd(std::move(rhs)); \
  }                                                    \
                                                       \
  out_t operator op(const zdd& lhs, __zdd&& rhs)       \
  {                                                    \
    return lhs op zdd(std::move(rhs));                 \
  }                                                    \
                                                       \
  out_t operator op(__zdd&& lhs, const zdd& rhs)       \
  {                                                    \
    return zdd(std::move(lhs)) op rhs;                 \
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Truthy)

  zdd::operator bool() const
  {
    return !zdd_isempty(*this);
  }

  __zdd::operator bool() &&
  {
    return (zdd(std::move(*this))).operator bool();
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Assignment)

  zdd&
  zdd::operator=(const zdd& other)
  {
    this->_negate = other._negate;
    this->_file   = other._file;
    this->_shift  = other._shift;
    return *this;
  }

  zdd&
  zdd::operator=(__zdd&& other)
  {
    deref();
    return (*this = internal::reduce<zdd_policy>(std::move(other)));
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Relational)

  __ZDD_OPER(bool, ==);

  bool
  operator==(const zdd& lhs, const zdd& rhs)
  {
    return zdd_equal(lhs, rhs);
  }

  __ZDD_OPER(bool, !=);

  bool
  operator!=(const zdd& lhs, const zdd& rhs)
  {
    return zdd_unequal(lhs, rhs);
  }

  __ZDD_OPER(bool, <=);

  bool
  operator<=(const zdd& lhs, const zdd& rhs)
  {
    return zdd_subseteq(lhs, rhs);
  }

  __ZDD_OPER(bool, >=);

  bool
  operator>=(const zdd& lhs, const zdd& rhs)
  {
    return zdd_subseteq(rhs, lhs);
  }

  __ZDD_OPER(bool, <);

  bool
  operator<(const zdd& lhs, const zdd& rhs)
  {
    return zdd_subset(lhs, rhs);
  }

  __ZDD_OPER(bool, >);

  bool
  operator>(const zdd& lhs, const zdd& rhs)
  {
    return zdd_subset(rhs, lhs);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Bit)

  __zdd
  operator~(const zdd& A)
  {
    return zdd_complement(A);
  }

  __zdd
  operator~(__zdd&& A)
  {
    return ~zdd(std::move(A));
  }

  __ZDD_OPER(__zdd, &);

  __zdd
  operator&(const zdd& lhs, const zdd& rhs)
  {
    return zdd_intsec(lhs, rhs);
  }

  zdd&
  zdd::operator&=(const zdd& other)
  {
    return (*this = zdd_intsec(*this, other));
  }

  zdd&
  zdd::operator&=(zdd&& other)
  {
    __zdd temp = zdd_intsec(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __ZDD_OPER(__zdd, |);

  __zdd
  operator|(const zdd& lhs, const zdd& rhs)
  {
    return zdd_union(lhs, rhs);
  }

  zdd&
  zdd::operator|=(const zdd& other)
  {
    return (*this = zdd_union(*this, other));
  }

  zdd&
  zdd::operator|=(zdd&& other)
  {
    __zdd temp = zdd_union(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Set/Arithmetic)

  __ZDD_OPER(__zdd, -);

  __zdd
  operator-(const zdd& A)
  {
    return zdd_complement(A);
  }

  __zdd
  operator-(__zdd&& A)
  {
    return -zdd(std::move(A));
  }

  __zdd
  operator-(const zdd& lhs, const zdd& rhs)
  {
    return zdd_diff(lhs, rhs);
  }

  zdd&
  zdd::operator-=(const zdd& other)
  {
    return (*this = zdd_diff(*this, other));
  }

  zdd&
  zdd::operator-=(zdd&& other)
  {
    __zdd temp = zdd_diff(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __ZDD_OPER(__zdd, +);

  zdd
  operator+(const zdd& A)
  {
    return A;
  }

  __zdd
  operator+(__zdd&& A)
  {
    return A;
  }

  __zdd
  operator+(const zdd& lhs, const zdd& rhs)
  {
    return zdd_union(lhs, rhs);
  }

  zdd&
  zdd::operator+=(const zdd& other)
  {
    return (*this = zdd_union(*this, other));
  }

  zdd&
  zdd::operator+=(zdd&& other)
  {
    __zdd temp = zdd_union(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __ZDD_OPER(__zdd, *);

  __zdd
  operator*(const zdd& lhs, const zdd& rhs)
  {
    return zdd_intsec(lhs, rhs);
  }

  zdd&
  zdd::operator*=(const zdd& other)
  {
    return (*this = zdd_intsec(*this, other));
  }

  zdd&
  zdd::operator*=(zdd&& other)
  {
    __zdd temp = zdd_intsec(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Input variables
  zdd::label_type
  zdd_topvar(const zdd& A)
  {
    return internal::dd_topvar(A);
  }

  zdd::label_type
  zdd_minvar(const zdd& A)
  {
    return internal::dd_minvar(A);
  }

  zdd::label_type
  zdd_maxvar(const zdd& A)
  {
    return internal::dd_maxvar(A);
  }

  void
  zdd_support(const zdd& A, const consumer<zdd::label_type>& cb)
  {
    return internal::dd_support(A, cb);
  }
}
