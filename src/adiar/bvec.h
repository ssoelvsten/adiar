#ifndef ADIAR_BVEC_H
#define ADIAR_BVEC_H

#include <vector>
#include <adiar/bdd.h>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace adiar {
    class bvec
    {
        public: static constexpr size_t MAX_BITLEN = 128;
        private:
        std::vector<bdd> _bits;
        size_t _bitlen;
        const bdd default_value = bdd();

        public:
        /////////////////////////////////////////////////////////////////////////
        /// \brief Zero-length bvec constructor, effectively a "safe nullpointer"
        /////////////////////////////////////////////////////////////////////////
        bvec(size_t bitlen = MAX_BITLEN);
        /////////////////////////////////////////////////////////////////////////
        /// \brief Copy constructor
        /////////////////////////////////////////////////////////////////////////
        bvec(const bvec& fs);
        /////////////////////////////////////////////////////////////////////////
        /// \brief Move constructor for right-hand values
        /////////////////////////////////////////////////////////////////////////
        bvec(bvec&& fs);

        /////////////////////////////////////////////////////////////////////////
        /// \brief Conversion constructor from a raw bit-vector
        /////////////////////////////////////////////////////////////////////////
        bvec(const std::vector<bdd>& bits, size_t bitlen = MAX_BITLEN);
        /////////////////////////////////////////////////////////////////////////
        /// \brief Conversion constructor from a raw bit-vector for right-hand values
        /////////////////////////////////////////////////////////////////////////
        bvec(std::vector<bdd>&& bits, size_t bitlen = MAX_BITLEN);

        /////////////////////////////////////////////////////////////////////////
        /// \brief Parameterized constructor with length `bitlen` and given initial value `f`
        /////////////////////////////////////////////////////////////////////////
        bvec(const size_t bitlen, const bdd& f);
        
        inline size_t bitlen() const { return _bitlen; }
        inline size_t size() const { return _bits.size(); }

        const bdd&
        at(size_t index) const;

        std::vector<bdd>::const_iterator
        begin() const;

        std::vector<bdd>::const_iterator
        end() const;

        std::vector<bdd>::const_reverse_iterator
        rbegin() const;

        std::vector<bdd>::const_reverse_iterator
        rend() const;

        std::string
        to_string() const;
    };

    std::ostream&
    operator<<(std::ostream& os, const bvec& a);

    // Constructors

    bvec 
    bvec_false(size_t bitlen);

    bvec 
    bvec_true(size_t bitlen);

    bvec 
    bvec_const(size_t value, size_t bitlen);

    template<typename Integer> 
    bvec 
    bvec_const(Integer i) {
        return bvec_const(i,8 * sizeof(Integer));
    }
        
    //Comparators

    bool
    bvec_equal(const bvec& x, const bvec& y);

    bool
    operator==(const bvec& x, const bvec& y);

    
    // Bitwise operations

    //Helper function for bitwise operations
    template<typename BDD_OP>
    bvec 
    _bvec_bitwise_op(size_t size, size_t bitlen, const BDD_OP& op);
    
    template<typename BDD_OP>
    bvec 
    _bvec_bitwise_op(const bvec& x, const bvec& y, const BDD_OP& op);
    
    bvec 
    bvec_and(const bvec& x, const bvec& y);
    
    bvec 
    bvec_or(const bvec& x, const bvec& y);
    
    bvec
    bvec_xor(const bvec& x, const bvec& y);

    bvec
    bvec_not(const bvec& x);

    //Arithmetic operations

    bvec
    bvec_add(const bvec& x, const bvec& y);
}

//TODO: bvec_truncate(new_bitlen); use _bvec_bitwise_op(...new_bitlen...) to truncate
//TODO: Implement arithmetic operations
//TODO: Add variables, bvec_var(ITER begin, ITER end, bitlen)

#endif // ADIAR_BVEC_H 
