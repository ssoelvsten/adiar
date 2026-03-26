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
    private:
        std::vector<bdd> _bits;
        size_t _max_length;
        const bdd default_value = bdd();

    public:
        /////////////////////////////////////////////////////////////////////////
        /// \brief Zero-length bvec constructor, effectively a "safe nullpointer"
        /////////////////////////////////////////////////////////////////////////
        bvec(size_t max_length = SIZE_T_MAX)
            : _bits(0), _max_length(max_length)
        {}
        /////////////////////////////////////////////////////////////////////////
        /// \brief Copy constructor
        /////////////////////////////////////////////////////////////////////////
        bvec(const bvec& fs) 
            : _bits(fs._bits), _max_length(fs._max_length)
        {}
        /////////////////////////////////////////////////////////////////////////
        /// \brief Move constructor for right-hand values
        /////////////////////////////////////////////////////////////////////////
        bvec(bvec&& fs) 
        : _bits(std::move(fs._bits)), _max_length(fs._max_length)
        {}

        /////////////////////////////////////////////////////////////////////////
        /// \brief Conversion constructor from a raw bit-vector
        /////////////////////////////////////////////////////////////////////////
        bvec(const std::vector<bdd>& bits, size_t max_length = SIZE_T_MAX) 
        : _bits(bits), _max_length(max_length)
        {}
        /////////////////////////////////////////////////////////////////////////
        /// \brief Conversion constructor from a raw bit-vector for right-hand values
        /////////////////////////////////////////////////////////////////////////
        bvec(std::vector<bdd>&& bits, size_t max_length = SIZE_T_MAX)
        : _bits(std::move(bits)), _max_length(max_length)
        {}

        /////////////////////////////////////////////////////////////////////////
        /// \brief Parameterized constructor with length `bitlen` and given initial value `f`
        /////////////////////////////////////////////////////////////////////////
        bvec(const size_t bitlen, const bdd& f) 
            : _bits(bitlen,f), _max_length(bitlen)
        {}
        
        public:
        size_t bitlen() const { return _max_length; }
        size_t size() const { return _bits.size(); }

        const bdd&              // return type
        at(size_t index) const  //name(...) context
        { 
            if (_bits.size() <= index) { return default_value; } //TODO: This warns that we are returning a local temp. Can we move it?
            return _bits.at(index);
        }

        std::vector<bdd>::const_iterator
        begin() const  
        { 
            return _bits.cbegin(); 
        }

        std::vector<bdd>::const_iterator
        end() const  
        { 
            return _bits.cend(); 
        }

        std::vector<bdd>::const_reverse_iterator
        rbegin() const  
        { 
            return _bits.crbegin(); 
        }

        std::vector<bdd>::const_reverse_iterator
        rend() const  
        { 
            return _bits.crend(); 
        }

        std::string
        to_string() const {
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
    };

    inline std::ostream&
    operator<<(std::ostream& os, const bvec& a)
    {
        return os << a.to_string();
    }

    // Constructors

    bvec bvec_false(size_t bitlen = SIZE_T_MAX) {
        return bvec(std::vector<bdd>(0,bdd_false()),bitlen);
    }

    bvec bvec_true(size_t bitlen = SIZE_T_MAX) {
        return bvec(std::vector<bdd>(bitlen,bdd_true()),bitlen);
    }

    bvec bvec_const(size_t value, size_t bitlen) {
        size_t msb = value != 0 ? std::max(std::ceil(std::log2(value)),1.0) : 0;
        std::vector<bdd> res;
        res.reserve(msb);
        // Should be able to compute the most significant bit position and stop after that
        for (size_t i = 0; i < msb; i++) {
            res.push_back(value & 1 ? bdd_true() : bdd_false());

            value >>= 1;
        }

        return bvec(res,bitlen);
    }

    template<typename Integer> 
    bvec bvec_const(Integer i) {
        return bvec_const(i,8 * sizeof(Integer));
    }


    //Helper function for bitwise operations
    //TODO: Figure out how to do higher order function templating with constrains on function signature
    template<typename BDD_OP>
    bvec _bvec_bitwise_op(bvec x, bvec y, BDD_OP op) {
        const size_t size = std::max(x.size(),y.size());
        std::vector<bdd> res;
        
        for(size_t i = 0; i < size; i++) {
            const bdd bit = op(x.at(i),y.at(i));
            if (!bdd_isfalse(bit)) {
                while(res.size() < i) {
                    res.push_back(bdd_false());
                }
                res.push_back(bit);
            }
        }
        
        return bvec(res, std::max(x.bitlen(),y.bitlen()));
    }

    //Comparators

    bool
    bvec_equal(const bvec& x, const bvec& y) {
        if(x.size() != y.size()) { return false; }
        
        for (size_t i = 0; i < x.size(); i++) {
            if(!bdd_equal(x.at(i),y.at(i))) { return false; }
        }
        return true;
    }

    bool
    operator==(const bvec& x, const bvec& y) {
        return bvec_equal(x,y);
    }

    // Boolean operations
    bvec bvec_and(bvec x, bvec y) {
        return _bvec_bitwise_op(x,y,[](const bdd& f, const bdd& g){ return bdd_and(f,g); });
    }

    bvec bvec_or(bvec x, bvec y) {
        return _bvec_bitwise_op(x,y,[](const bdd& f, const bdd& g){ return bdd_or(f,g); });
    }
    
    bvec bvec_xor(bvec x, bvec y) {
        return _bvec_bitwise_op(x,y,[](const bdd& f, const bdd& g){ return bdd_xor(f,g); });
    }

    bvec bvec_not(bvec x) {
        //TODO: Do we require same bitlength? If yes, do we do it at compile-time or run-time?
        std::vector<bdd> res(x.bitlen());
        
        for(size_t i = 0; i < x.bitlen(); i++) {
            res.at(i) = bdd_not(x.at(i));
        }

        return bvec(res);
    }
}



#endif // ADIAR_BVEC_H 
