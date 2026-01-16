#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_VECTOR_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_VECTOR_H

#include <tpie/internal_vector.h>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  template <memory_mode mem_mode, typename T>
  class vector;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal memory vector.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class vector<memory_mode::Internal, T>
  {
  private:
    using vector_type = tpie::internal_vector<T>;

    size_t _capacity;
    vector_type _vector;

  public:
    using value_type     = typename vector_type::value_type;
    using iterator       = typename vector_type::iterator;
    using const_iterator = typename vector_type::const_iterator;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static size_t
    memory_usage(size_t capacity = 0)
    {
      return vector_type::memory_usage(capacity);
    }

    static size_t
    memory_fits(size_t memory_bytes)
    {
      const size_t c = vector_type::memory_fits(memory_bytes);

      adiar_assert(memory_usage(c) <= memory_bytes, "memory_fits and memory_usage should agree.");
      return c;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    vector(size_t capacity = 0)
      : _capacity(capacity)
      , _vector(capacity)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type&
    at(size_t i)
    {
      adiar_assert(i < this->size(), "Use of invalid index!");
      return this->_vector[i];
    }

    const value_type&
    at(size_t i) const
    {
      adiar_assert(i < this->size(), "Use of invalid index!");
      return this->_vector[i];
    }

    value_type&
    operator[](size_t i)
    {
      return this->at(i);
    }

    const value_type&
    operator[](size_t i) const
    {
      return this->at(i);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type&
    front()
    {
      return this->_vector.front();
    }

    const value_type&
    front() const
    {
      return this->_vector.front();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type&
    back()
    {
      return this->_vector.back();
    }

    const value_type&
    back() const
    {
      return this->_vector.back();
    }

    value_type&
    push_back(const value_type& x)
    {
      adiar_assert(this->size() < this->_capacity, "Cannot push at full capacity");
      return this->_vector.push_back(x);
    }

    void
    pop_back()
    {
      return this->_vector.pop_back();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    iterator
    begin()
    {
      return this->_vector.begin();
    }

    const_iterator
    begin() const
    {
      return this->_vector.begin();
    }

    iterator
    end()
    {
      return this->_vector.end();
    }

    const_iterator
    end() const
    {
      return this->_vector.end();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty() const
    {
      return this->_vector.empty();
    }

    size_t
    size() const
    {
      return this->_vector.size();
    }

    size_t
    capacity() const
    {
      return this->_capacity;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    reset()
    {
      this->_vector.clear();
    }

    void
    reset(size_t capacity)
    {
      this->_vector.resize(capacity);
      this->_capacity = capacity;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief An external memory vector in TPIE.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO Implement an external memory (temporary) vector (wrapper on `file`, I suppose).
}

#endif // ADIAR_INTERNAL_DATA_STRUCTURES_VECTOR_H
