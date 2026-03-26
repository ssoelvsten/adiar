#ifndef ADIAR_BVEC_H
#define ADIAR_BVEC_H

#include <vector>
#include <adiar/bdd.h>
#include <sstream>

namespace adiar {
    class bvec
    {
    private:
        const std::vector<bdd> _bits;

    public:
        /////////////////////////////////////////////////////////////////////////
        /// \brief Zero-length bvec constructor, effectively a "safe nullpointer"
        /////////////////////////////////////////////////////////////////////////
        bvec()
            : _bits(0)
        {}
        /////////////////////////////////////////////////////////////////////////
        /// \brief Copy constructor
        /////////////////////////////////////////////////////////////////////////
        bvec(const bvec& fs) 
            : _bits(fs._bits)
        {}
        /////////////////////////////////////////////////////////////////////////
        /// \brief Move constructor for right-hand values
        /////////////////////////////////////////////////////////////////////////
        bvec(bvec&& fs) 
        : _bits(std::move(fs._bits))
        {}

        /////////////////////////////////////////////////////////////////////////
        /// \brief Conversion constructor from a raw bit-vector
        /////////////////////////////////////////////////////////////////////////
        bvec(const std::vector<bdd>& bits) 
        : _bits(bits)
        {}
        /////////////////////////////////////////////////////////////////////////
        /// \brief Conversion constructor from a raw bit-vector for right-hand values
        /////////////////////////////////////////////////////////////////////////
        bvec(std::vector<bdd>&& bits) 
        : _bits(std::move(bits))
        {}

        /////////////////////////////////////////////////////////////////////////
        /// \brief Parameterized constructor with length `bitlen` and given initial value `f` (default value is `bdd()`)
        /////////////////////////////////////////////////////////////////////////
        bvec(const size_t bitlen, const bdd& f = bdd()) 
            : _bits(bitlen,f)
        {}
        
        public:
        size_t bitlen() const { return _bits.size(); }

        const bdd&              // return type
        at(size_t index) const  //name(...) context
        { 
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

    bvec bvec_false(size_t bitlen) {
        return bvec(bitlen,bdd_false());
    }

    bvec bvec_true(size_t bitlen) {
        return bvec(bitlen,bdd_true());
    }

    bvec bvec_const(size_t bitlen, size_t value) {
        std::vector<bdd> res(bitlen);
        for (size_t i = 0; i< bitlen; i++) {
            res.at(i) = value & 1 ? bdd_true() : bdd_false();
            value >>= 1;
        }
        return res;
    }

    template<typename Integer> 
    bvec bvec_const(Integer i) {
        return bvec_const(8 * sizeof(Integer), i);
    }


    //Helper function for bitwise operations
    //TODO: Figure out how to do higher order function templating with constrains on function signature
    template<typename BDD_OP = function<bdd (const bdd&, const bdd&)>>
    bvec _bvec_bitwise_op(bvec x, bvec y, BDD_OP op) {
        std::vector<bdd> res(x.bitlen());
        
        for(size_t i = 0; i < x.bitlen(); i++) {
            res.at(i) = op(x.at(i),y.at(i));
        }

        return bvec(res);
    }


    // Boolean operations
    bvec bvec_and(bvec x, bvec y) {
        //TODO: Do we require same bitlength? If yes, do we do it at compile-time or run-time?
        std::vector<bdd> res(x.bitlen());
        
        for(size_t i = 0; i < x.bitlen(); i++) {
            res.at(i) = bdd_and(x.at(i),y.at(i));
        }
        
        return bvec(res);
    }

    bvec bvec_or(bvec x, bvec y) {
        //TODO: Do we require same bitlength? If yes, do we do it at compile-time or run-time?
        std::vector<bdd> res(x.bitlen());
        
        for(size_t i = 0; i < x.bitlen(); i++) {
            res.at(i) = bdd_or(x.at(i),y.at(i));
        }

        return bvec(res);
    }
    
    bvec bvec_xor(bvec x, bvec y) {
        //TODO: Do we require same bitlength? If yes, do we do it at compile-time or run-time?
        std::vector<bdd> res(x.bitlen());
        
        for(size_t i = 0; i < x.bitlen(); i++) {
            res.at(i) = bdd_xor(x.at(i),y.at(i));
        }

        return bvec(res);
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
