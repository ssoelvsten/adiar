#include "bdd.h"

#include <memory>

#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/dd_func.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // '__bdd' Constructors
  __bdd::__bdd() = default;

  __bdd::__bdd(const shared_node_file_type& f)
    : internal::__dd(f)
  {}

  __bdd::__bdd(const shared_arc_file_type& f, const exec_policy& ep)
    : internal::__dd(f, ep)
  {}

  __bdd::__bdd(const bdd& dd)
    : internal::__dd(dd)
  {}

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // 'bdd' Constructors
  bdd::bdd(bool t)
    : bdd(bdd_terminal(t))
  {}

  bdd::bdd()
    : bdd(false)
  {}

  bdd::bdd(const shared_node_file_type& f, bool negate, signed_label_type shift)
    : internal::dd(f, negate, shift)
  {}

  bdd::bdd(const bdd& f)
    : internal::dd(f)
  {}

  bdd::bdd(bdd&& f)
    : internal::dd(f)
  {}

  bdd::bdd(__bdd&& f)
    : internal::dd(internal::reduce<bdd_policy>(std::move(f)))
  {}

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Operators
#define __BDD_OPER(out_t, op)                          \
  out_t operator op(__bdd&& lhs, __bdd&& rhs)          \
  {                                                    \
    return bdd(std::move(lhs)) op bdd(std::move(rhs)); \
  }                                                    \
                                                       \
  out_t operator op(const bdd& lhs, __bdd&& rhs)       \
  {                                                    \
    return lhs op bdd(std::move(rhs));                 \
  }                                                    \
                                                       \
  out_t operator op(__bdd&& lhs, const bdd& rhs)       \
  {                                                    \
    return bdd(std::move(lhs)) op rhs;                 \
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Assignment)

  bdd&
  bdd::operator=(const bdd& other)
  {
    this->_negate = other._negate;
    this->_file   = other._file;
    this->_shift  = other._shift;
    return *this;
  }

  bdd&
  bdd::operator=(__bdd&& other)
  {
    deref();
    return (*this = internal::reduce<bdd_policy>(std::move(other)));
  }


  //////////////////////////////////////////////////////////////////////////////
  // Operators (Truthy)

  bdd::operator bool() const
  {
    return !bdd_isfalse(*this);
  }

  __bdd::operator bool() &&
  {
    return (bdd(std::move(*this))).operator bool();
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Relational)

  __BDD_OPER(bool, ==);

  bool
  operator==(const bdd& lhs, const bdd& rhs)
  {
    return bdd_equal(lhs, rhs);
  }

  __BDD_OPER(bool, !=);

  bool
  operator!=(const bdd& lhs, const bdd& rhs)
  {
    return bdd_unequal(lhs, rhs);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Bit)

  bdd
  operator~(const bdd& f)
  {
    return bdd_not(f);
  }

  bdd
  operator~(bdd&& f)
  {
    return bdd_not(std::move(f));
  }

  bdd
  operator~(__bdd&& in)
  {
    return ~bdd(std::move(in));
  }

  __BDD_OPER(__bdd, &);

  __bdd
  operator&(const bdd& lhs, const bdd& rhs)
  {
    return bdd_and(lhs, rhs);
  }

  bdd&
  bdd::operator&=(const bdd& other)
  {
    return (*this = bdd_and(*this, other));
  }

  bdd&
  bdd::operator&=(bdd&& other)
  {
    __bdd temp = bdd_and(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __BDD_OPER(__bdd, |);

  __bdd
  operator|(const bdd& lhs, const bdd& rhs)
  {
    return bdd_or(lhs, rhs);
  }

  bdd&
  bdd::operator|=(const bdd& other)
  {
    return (*this = bdd_or(*this, other));
  }

  bdd&
  bdd::operator|=(bdd&& other)
  {
    __bdd temp = bdd_or(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __BDD_OPER(__bdd, ^);

  __bdd
  operator^(const bdd& lhs, const bdd& rhs)
  {
    return bdd_xor(lhs, rhs);
  }

  bdd&
  bdd::operator^=(const bdd& other)
  {
    return (*this = bdd_xor(*this, other));
  }

  bdd&
  bdd::operator^=(bdd&& other)
  {
    __bdd temp = bdd_xor(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  //////////////////////////////////////////////////////////////////////////////
  // Operators (Set/Arithmetic)

  __BDD_OPER(__bdd, +);

  bdd
  operator+(const bdd& f)
  {
    return f;
  }

  __bdd
  operator+(__bdd&& f)
  {
    return f;
  }

  __bdd
  operator+(const bdd& lhs, const bdd& rhs)
  {
    return bdd_or(lhs, rhs);
  }

  bdd&
  bdd::operator+=(const bdd& other)
  {
    return (*this = bdd_or(*this, other));
  }

  bdd&
  bdd::operator+=(bdd&& other)
  {
    __bdd temp = bdd_or(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __BDD_OPER(__bdd, -);

  bdd
  operator-(const bdd& f)
  {
    return bdd_not(f);
  }

  __bdd
  operator-(__bdd&& f)
  {
    return bdd_not(std::move(f));
  }

  __bdd
  operator-(const bdd& lhs, const bdd& rhs)
  {
    return bdd_diff(lhs, rhs);
  }

  bdd&
  bdd::operator-=(const bdd& other)
  {
    return (*this = bdd_diff(*this, other));
  }

  bdd&
  bdd::operator-=(bdd&& other)
  {
    __bdd temp = bdd_diff(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  __BDD_OPER(__bdd, *);

  __bdd
  operator*(const bdd& lhs, const bdd& rhs)
  {
    return bdd_and(lhs, rhs);
  }

  bdd&
  bdd::operator*=(const bdd& other)
  {
    return (*this = bdd_and(*this, other));
  }

  bdd&
  bdd::operator*=(bdd&& other)
  {
    __bdd temp = bdd_and(*this, other);
    other.deref();
    return (*this = std::move(temp));
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Input variables
  //
  // TODO: find a better home
  bdd::label_type
  bdd_topvar(const bdd& f)
  {
    return internal::dd_topvar(f);
  }

  bdd::label_type
  bdd_minvar(const bdd& f)
  {
    return internal::dd_minvar(f);
  }

  bdd::label_type
  bdd_maxvar(const bdd& f)
  {
    return internal::dd_maxvar(f);
  }

  void
  bdd_support(const bdd& f, const consumer<bdd::label_type>& cb)
  {
    return internal::dd_support(f, cb);
  }
}
