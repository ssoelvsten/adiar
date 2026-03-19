#ifndef ADIAR_BVEC_H
#define ADIAR_BVEC_H

#include <vector>
#include <adiar/bdd.h>

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

        
    };

    bvec bvec_false(size_t bitlen) {
        return bvec(bitlen,bdd_false());
    }

    bvec bvec_true(size_t bitlen) {
        return bvec(bitlen,bdd_true());
    }

    bvec bvec_const(size_t bitlen, size_t i) {
        std::vector<bdd> res(bitlen);
        //TODO: Implement actual integer value initialization
        return res;
    }

    template<typename Integer> 
    bvec bvec_const(Integer i) {
        return bvec_const(8 * sizeof(Integer), i);
    }
}



#endif // ADIAR_BVEC_H 
