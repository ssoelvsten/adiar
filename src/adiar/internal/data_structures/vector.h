#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_VECTOR_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_VECTOR_H

#include <tpie/internal_vector.h>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  template <memory_mode mem_mode, typename T>
  class vector;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal memory array to match the interface of `std::vector`.
  ///
  /// \details TPIE also provides the `internal_vector`. Yet, that does not (yet) include a reverse
  ///          iterator. Hence, for now it's easier for us to just wrap the `array` directly
  ///          instead of using the array. This also gives us the freedom to make it more conform to
  ///          the `std::vector<T>`.
  //
  // TODO: explicit or implicit `resize` when full?
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class vector<memory_mode::Internal, T>
  {
  private:
    using array_type = tpie::array<T>;

    /// \brief The underlying (internal memory) array to store the information.
    array_type _array;

    /// \brief The size of `_array` that was allocated.
    size_t _capacity;

    /// \brief The number of elements placed in the vector.
    size_t _size = 0;

  public:
    using value_type             = typename array_type::value_type;
    using iterator               = typename array_type::iterator;
    using reverse_iterator       = typename array_type::reverse_iterator;
    using const_iterator         = typename array_type::const_iterator;
    using const_reverse_iterator = typename array_type::const_reverse_iterator;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    static size_t
    memory_usage(size_t capacity = 0)
    {
      return array_type::memory_usage(capacity);
    }

    static size_t
    memory_fits(size_t memory_bytes)
    {
      const size_t c = array_type::memory_fits(memory_bytes);

      adiar_assert(memory_usage(c) <= memory_bytes, "memory_fits and memory_usage should agree.");
      return c;
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    vector(size_t capacity = 0)
      : _array(capacity)
      , _capacity(capacity)
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    value_type&
    at(size_t i)
    {
      adiar_assert(i < this->size(), "Use of invalid index!");
      return this->_array[i];
    }

    const value_type&
    at(size_t i) const
    {
      adiar_assert(i < this->size(), "Use of invalid index!");
      return this->_array[i];
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
      return this->_array[0];
    }

    const value_type&
    front() const
    {
      return this->_array[0];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
  public:
    value_type&
    back()
    {
      return this->_array[this->_size - 1];
    }

    const value_type&
    back() const
    {
      return this->_array[this->_size - 1];
    }

    value_type&
    push_back(const value_type& x)
    {
      adiar_assert(this->_size < this->_capacity, "Cannot push at full capacity");
      this->_array[this->_size] = x;
      this->_size += 1;
      return this->back();
    }

    void
    pop_back()
    {
      this->_size -= 1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    iterator
    begin()
    {
      return this->_array.begin();
    }

    const_iterator
    begin() const
    {
      return this->_array.begin();
    }

    iterator
    end()
    {
      return this->_array.begin() + this->_size;
    }

    const_iterator
    end() const
    {
      return this->_array.begin() + this->_size;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    reverse_iterator
    rbegin()
    {
      return this->_array.rend() - this->_size;
    }

    const_reverse_iterator
    rbegin() const
    {
      return this->_array.rend() - this->_size;
    }

    reverse_iterator
    rend()
    {
      return this->_array.rend();
    }

    const_reverse_iterator
    rend() const
    {
      return this->_array.rend();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t
    size() const
    {
      return this->_size;
    }

    bool
    empty() const
    {
      return this->_size == 0;
    }

    size_t
    capacity() const
    {
      return this->_capacity;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Alias for a `vector<memory_mode::Internal, T>`.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using internal_vector = vector<memory_mode::Internal, T>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief An external memory vector in TPIE.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO Implement an external memory (temporary) vector (wrapper on `file`, I suppose).
}

#endif // ADIAR_INTERNAL_DATA_STRUCTURES_VECTOR_H
