#include <vector>
#include <adiar/bdd.h>
#include <adiar/bvec.h>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace adiar {
    /////////////////////////////////////////////////////////////////////////
    /// \brief Zero-length bvec constructor, effectively a "safe nullpointer"
    /////////////////////////////////////////////////////////////////////////
    bvec::bvec(size_t bitlen)
        : _bits(0), _bitlen(bitlen)
    {}
    /////////////////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    /////////////////////////////////////////////////////////////////////////
    bvec::bvec(const bvec& fs) 
        : _bits(fs._bits), _bitlen(fs._bitlen)
    {}
    /////////////////////////////////////////////////////////////////////////
    /// \brief Move constructor for right-hand values
    /////////////////////////////////////////////////////////////////////////
    bvec::bvec(bvec&& fs) 
    : _bits(std::move(fs._bits)), _bitlen(fs._bitlen)
    {}

    /////////////////////////////////////////////////////////////////////////
    /// \brief Conversion constructor from a raw bit-vector
    /////////////////////////////////////////////////////////////////////////
    bvec::bvec(const std::vector<bdd>& bits, size_t bitlen) 
    : _bits(bits), _bitlen(bitlen)
    {
        //Truncates to bitlen and removes any prefix of false
        while (this->_bits.size() > 0 && (!this->_bits.back() || this->_bitlen < this->_bits.size())) {
            this->_bits.pop_back();
        }
    }
    /////////////////////////////////////////////////////////////////////////
    /// \brief Conversion constructor from a raw bit-vector for right-hand values
    /////////////////////////////////////////////////////////////////////////
    bvec::bvec(std::vector<bdd>&& bits, size_t bitlen)
    : _bits(std::move(bits)), _bitlen(bitlen)
    {}

    /////////////////////////////////////////////////////////////////////////
    /// \brief Parameterized constructor with length `bitlen` and given initial value `f`
    /////////////////////////////////////////////////////////////////////////
    bvec::bvec(const size_t bitlen, const bdd& f) 
        : _bits(bitlen,f), _bitlen(bitlen)
    {}

    const bdd&              // return type
    bvec::at(size_t index) const  //name(...) context
    { 
        if (_bits.size() <= index) { return default_value; } //TODO: This warns that we are returning a local temp. Can we move it?
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
    bvec::to_string() const {
        std::stringstream out;

        out << "0x";
        for (auto i = this->rbegin(); i != this->rend(); i++)
        {
            if (bdd_isfalse(*i)) {
                out << "0";
            } else if(bdd_istrue(*i)) {
                out << "1";
            } else {
                out << "_";
            }
        }

        return out.str();
    }
    
    // Comparators
    
    std::ostream&
    operator<<(std::ostream& os, const bvec& a)
    {
        return os << a.to_string();
    }

    bool
    operator==(const bvec& x, const bvec& y) {
        return bvec_equal(x,y);
    }

    bool
    bvec_equal(const bvec& x, const bvec& y) {
        if(x.size() != y.size()) { return false; }
        
        for (size_t i = 0; i < x.size(); i++) {
            if(!bdd_equal(x.at(i),y.at(i))) { return false; }
        }
        return true;
    }

    // Constructors
    bvec bvec_false(size_t bitlen = bvec::MAX_BITLEN) {
        return bvec(std::vector<bdd>(0,bdd_false()),bitlen);
    }

    bvec
    bvec_true(size_t bitlen = bvec::MAX_BITLEN) {
        return bvec(std::vector<bdd>(bitlen,bdd_true()),bitlen);
    }

    bvec 
    bvec_const(size_t value, size_t bitlen) {
        size_t max = std::ceil(std::log2(value))+1;
        size_t msb = std::min( value != 0 ? max : 0,bitlen);
        std::vector<bdd> res;
        res.reserve(msb);
        // Should be able to compute the most significant bit position and stop after that
        for (size_t i = 0; i < msb; i++) {
            res.push_back(value & 1 ? bdd_true() : bdd_false());

            value >>= 1;
        }

        return bvec(res,bitlen);
    }
    
    // Bitwise operations


    //Helper function for bitwise operations
    template<typename BDD_OP>
    bvec _bvec_bitwise_op(size_t size, size_t bitlen, const BDD_OP& op) {
        std::vector<bdd> res;
        
        for(size_t i = 0; i < size; i++) {
            const bdd bit = op(i);
            if (bit) {
                while(res.size() < i) {
                    res.push_back(bdd_false());
                }
                res.push_back(bit);
            }
        }
        
        return bvec(res, bitlen);
    }
    
    template<typename BDD_OP>
    bvec _bvec_bitwise_op(const bvec& x, const bvec& y, const BDD_OP& op) {
        const size_t size = std::max(x.size(),y.size());
        const size_t bitlen = std::max(x.bitlen(),y.bitlen());

        return _bvec_bitwise_op(size,bitlen,op);
    }

    bvec bvec_and(const bvec& x, const bvec& y) {
        return _bvec_bitwise_op(x,y,[&](size_t i){ return bdd_and(x.at(i),y.at(i)); });
    }

    bvec bvec_or(const bvec& x, const bvec& y) {
        return _bvec_bitwise_op(x,y,[&](size_t i){ return bdd_or(x.at(i),y.at(i)); });
    }
    
    bvec bvec_xor(const bvec& x, const bvec& y) {
        return _bvec_bitwise_op(x,y,[&](size_t i){ return bdd_xor(x.at(i),y.at(i)); });
    }

    bvec bvec_not(const bvec& x) {
        return _bvec_bitwise_op(x.bitlen(),x.bitlen(),[&](size_t i){ return bdd_not(x.at(i)); });
    }

    //Arithmetic operations

    bvec
    bvec_add(const bvec& x, const bvec& y) {
        const size_t bitlen = std::max(x.bitlen(),y.bitlen());
        const size_t size = std::max(x.size(),y.size())+1;

        bdd carry = bdd_false();

        std::vector<bdd> res;
        res.reserve(size);

        for (size_t i = 0; i<size; ++i) {
            const bdd xors = bdd_xor(bdd_xor(x.at(i), y.at(i)), carry);
            res.push_back(xors);
            if (i+1 <= bitlen) {
                adiar_assert(i != size-1 || bitlen >= size, "is not last bit with overflow");
                carry = bdd_or(bdd_and(carry, bdd_or(x.at(i),y.at(i))), bdd_and(x.at(i),y.at(i)));
            }
        }

        // This assumes that the vector constructor truncates size above bitlen and false prefix.
        res.push_back(carry);

        return bvec(res, bitlen);
    }

}
