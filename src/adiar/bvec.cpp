#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

#include <adiar/bdd.h>
#include <adiar/bvec.h>

namespace adiar
{
  /// \brief Find the most significant bit for an integer.
  size_t
  msb(size_t value)
  {
    // TODO: optimize based on operations available from the compiler.
    return value > 0 ? std::floor(std::log2(value)) + 1 : 0;
  }

  /// \brief Derive the `bitlen` to be used when combining two `bvec`s.
  size_t
  join_bitlen(const bvec& x, const bvec& y)
  {
    const size_t upcasted = std::max(x.bitlen(), y.bitlen());

    const size_t size     = std::max(x.size(), y.size());
    const bool variadic_x = x.bitlen() == bvec::variadic_bitlen;
    const bool variadic_y = y.bitlen() == bvec::variadic_bitlen;

    if ((variadic_x || variadic_y) && upcasted < size) { return bvec::variadic_bitlen; }
    return upcasted;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // `bvec` class

  bvec::bvec()
    : _bits(0)
    , _bitlen(bvec::variadic_bitlen)
  {}

  bvec::bvec(const bvec& fs)
    : _bits(fs._bits)
    , _bitlen(fs._bitlen)
  {}

  bvec::bvec(bvec&& fs)
    : _bits(std::move(fs._bits))
    , _bitlen(fs._bitlen)
  {}

  bvec::bvec(size_t value)
    : bvec(bvec_const(bvec::variadic_bitlen, value))
  {}

  bvec::bvec(size_t bitlen, size_t value)
    : bvec(bvec_const(bitlen, value))
  {}

  bvec::bvec(const std::vector<bdd>& bits)
    : bvec(bvec::variadic_bitlen, bits)
  {}

  bvec::bvec(size_t bitlen, const std::vector<bdd>& bits)
    : _bits(bits)
    , _bitlen(bitlen)
  {
    this->truncate(bitlen);
  }

  bvec::bvec(std::vector<bdd>&& bits)
    : bvec(bvec::variadic_bitlen, std::move(bits))
  {}

  bvec::bvec(size_t bitlen, std::vector<bdd>&& bits)
    : _bits(std::move(bits))
    , _bitlen(bitlen)
  {
    this->truncate(bitlen);
  }

  const bdd&
  bvec::at(size_t index) const
  {
    if (_bits.size() <= index) { return this->default_value; }
    return _bits.at(index);
  }

  std::vector<bdd>::const_iterator
  bvec::begin() const
  {
    return _bits.cbegin();
  }

  std::vector<bdd>::const_iterator
  bvec::end() const
  {
    return _bits.cend();
  }

  std::vector<bdd>::const_reverse_iterator
  bvec::rbegin() const
  {
    return _bits.crbegin();
  }

  std::vector<bdd>::const_reverse_iterator
  bvec::rend() const
  {
    return _bits.crend();
  }

  std::string
  bvec::to_string() const
  {
    std::stringstream out;

    out << "0x";
    for (auto i = this->rbegin(); i != this->rend(); i++) {
      if (bdd_isfalse(*i)) {
        out << "0";
      } else if (bdd_istrue(*i)) {
        out << "1";
      } else {
        out << "_";
      }
    }

    return out.str();
  }

  void
  bvec::truncate(size_t bitlen)
  {
    this->_bitlen = bitlen;

    if (bitlen != bvec::variadic_bitlen) {
      // Truncate to bitlen
      while (bitlen < this->_bits.size()) { this->_bits.pop_back(); }
    }

    // Truncate only-false suffix
    while (this->_bits.size() > 0 && !this->_bits.back()) { this->_bits.pop_back(); }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Comparators

  std::ostream&
  operator<<(std::ostream& os, const bvec& a)
  {
    return os << a.to_string();
  }

  bool
  operator==(const bvec& x, const bvec& y)
  {
    return bvec_equal(x, y);
  }

  bool
  bvec_equal(const bvec& x, const bvec& y)
  {
    if (x.size() != y.size()) { return false; }

    for (size_t i = 0; i < x.size(); i++) {
      if (!bdd_equal(x.at(i), y.at(i))) { return false; }
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Constructors

  bvec
  bvec_false(size_t bitlen)
  {
    return bvec(bitlen, std::vector<bdd>(0, bdd_false()));
  }

  bvec
  bvec_true(size_t bits)
  {
    return bvec_true(bvec::variadic_bitlen, bits);
  }

  bvec
  bvec_true(size_t bitlen, size_t bits)
  {
    return bvec(bitlen,
                std::vector<bdd>(bitlen == bvec::variadic_bitlen ? bits : std::min(bitlen, bits),
                                 bdd_true()));
  }

  bvec
  bvec_const(size_t bitlen, size_t value)
  {
    std::vector<bdd> res;
    res.reserve(msb(value));

    for (; value != 0; value >>= 1) { res.push_back(value & 1 ? bdd_true() : bdd_false()); }

    return bvec(bitlen, res);
  }

  bvec
  bvec_const(size_t value)
  {
    return bvec_const(bvec::variadic_bitlen, value);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Bitwise operations

  // Helper function for bitwise operations
  template <typename BDD_OP>
  bvec
  _bvec_bitwise_op(size_t size, size_t bitlen, const BDD_OP& op)
  {
    std::vector<bdd> res;
    res.reserve(size);

    for (size_t i = 0; i < size; i++) {
      const bdd bit = op(i);
      if (bit) {
        while (res.size() < i) { res.push_back(bdd_false()); }
        res.push_back(bit);
      }
    }

    return bvec(bitlen, res);
  }

  template <typename BDD_OP>
  bvec
  _bvec_bitwise_op(const bvec& x, const bvec& y, const BDD_OP& op)
  {
    const size_t size   = std::max(x.size(), y.size());
    const size_t bitlen = join_bitlen(x, y);

    return _bvec_bitwise_op(size, bitlen, op);
  }

  bvec
  bvec_and(const bvec& x, const bvec& y)
  {
    return _bvec_bitwise_op(x, y, [&](size_t i) { return bdd_and(x.at(i), y.at(i)); });
  }

  bvec
  bvec_or(const bvec& x, const bvec& y)
  {
    return _bvec_bitwise_op(x, y, [&](size_t i) { return bdd_or(x.at(i), y.at(i)); });
  }

  bvec
  bvec_xor(const bvec& x, const bvec& y)
  {
    return _bvec_bitwise_op(x, y, [&](size_t i) { return bdd_xor(x.at(i), y.at(i)); });
  }

  bvec
  bvec_not(const bvec& x)
  {
    return _bvec_bitwise_op(
      std::max(x.bitlen(), x.size()), x.bitlen(), [&](size_t i) { return ~x.at(i); });
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Arithmetic operations

  bvec
  bvec_add(const bvec& x, const bvec& y)
  {
    const size_t size   = std::max(x.size(), y.size()) + 1;
    const size_t bitlen = join_bitlen(x, y);

    bdd carry = bdd_false();

    std::vector<bdd> res;
    res.reserve(size);

    for (size_t i = 0; i < size; ++i) {
      const bdd xors = bdd_xor(bdd_xor(x.at(i), y.at(i)), carry);
      res.push_back(xors);
      if (i + 1 <= bitlen) {
        adiar_assert(i != size - 1 || bitlen >= size, "is not last bit with overflow");
        carry = bdd_or(bdd_and(carry, bdd_or(x.at(i), y.at(i))), bdd_and(x.at(i), y.at(i)));
      }
    }

    // This assumes that the vector constructor truncates size above bitlen and false prefix.
    res.push_back(carry);

    return bvec(bitlen, res);
  }

  bvec
  bvec_sub(const bvec& x, const bvec& y)
  {
    const size_t bitlen = std::max(x.bitlen(), y.bitlen());
    const size_t size   = bitlen;

    bdd carry = bdd_true();

    std::vector<bdd> res;
    res.reserve(size);

    for (size_t i = 0; i < size; ++i) {
      const bdd xors = x.at(i) ^ ~y.at(i) ^ carry;
      res.push_back(xors);
      if (i + 1 <= bitlen) {
        adiar_assert(i != size - 1 || bitlen >= size, "is not last bit with overflow");
        carry = (carry & (x.at(i) | ~y.at(i))) | (x.at(i) & ~y.at(i));
      }
    }

    // This assumes that the vector constructor truncates size above bitlen and false prefix.
    res.push_back(carry);

    return bvec(bitlen, res);
  }

  // Helper
  bvec
  bvec_truncate(size_t bitlen, const bvec& x)
  {
    bvec y = x;
    y.truncate(bitlen);
    return y;
  }
}
