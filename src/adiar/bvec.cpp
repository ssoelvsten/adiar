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
    {}
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
        size_t max = std::max(std::ceil(std::log2(value)),1.0);
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
            if (!bdd_isfalse(bit)) {
                while(res.size() < i) {
                    res.push_back(bdd_false());
                }
                res.push_back(bit);
            }
        }
        
        return bvec(res, bitlen);
    }
    
    template<typename BDD_OP>
    bvec _bvec_bitwise_op(bvec x, bvec y, const BDD_OP& op) {
        const size_t size = std::max(x.size(),y.size());
        const size_t bitlen = std::max(x.bitlen(),y.bitlen());

        return _bvec_bitwise_op(size,bitlen,op);
    }

    bvec bvec_and(bvec x, bvec y) {
        return _bvec_bitwise_op(x,y,[&](size_t i){ return bdd_and(x.at(i),y.at(i)); });
    }

    bvec bvec_or(bvec x, bvec y) {
        return _bvec_bitwise_op(x,y,[&](size_t i){ return bdd_or(x.at(i),y.at(i)); });
    }
    
    bvec bvec_xor(bvec x, bvec y) {
        return _bvec_bitwise_op(x,y,[&](size_t i){ return bdd_xor(x.at(i),y.at(i)); });
    }

    bvec bvec_not(bvec x) {
        return _bvec_bitwise_op(x.bitlen(),x.bitlen(),[&](size_t i){ return bdd_not(x.at(i)); });
    }

    //Arithmetic operations

}
//TODO: Move implementation to bvec.cpp
//TODO: Use "const bvec&" instead of "bvec" (call-by-reference instead of call-by-value)
//TODO: Rebase from main, then use (if(f) instead of if(!bdd_isfalse(f)))
//TODO: Test overview both existing and missing, test bvec_equal, use bvec_equal instead of bitwise assertions
//TODO: bvec_truncate(new_bitlen); use _bvec_bitwise_op(...new_bitlen...) to truncate
//TODO: Implement arithmetic operations
//TODO: Add variables, bvec_var(ITER begin, ITER end, bitlen)
