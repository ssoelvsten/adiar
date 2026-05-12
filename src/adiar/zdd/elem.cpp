#include <adiar/functional.h>
#include <adiar/zdd.h>
#include <tpie/tpie.h>

#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/data_structures/stack.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/ofstream.h>

namespace adiar
{
  /// \brief Whether the next choice needs to be output.
  template <typename Visitor>
  bool
  __zdd_Xelem__output(const zdd::node_type& curr, const zdd::pointer_type& next)
  {
    return /* Ignore everything that definitely did not pick the 'high' arc */
      next == curr.high()
      /* For ties between 'low' and 'high', check whether 'high' arc would have
         been the default direction. */
      && (next != curr.low() || Visitor::default_direction);
  }

  //////////////////////////////////////////////////////////////////////////////
  template <typename Visitor>
  class __zdd_Xelem__functional
  {
  private:
    Visitor _visitor;
    const consumer<zdd::label_type>& _consumer;

  public:
    __zdd_Xelem__functional(const consumer<zdd::label_type>& c)
      : _consumer(c)
    {}

    zdd::pointer_type
    visit(const zdd::node_type& n)
    {
      adiar_assert(!n.high().is_terminal() || n.high().value(), "high terminals are never false");

      const zdd::pointer_type next = _visitor.visit(n);
      if (__zdd_Xelem__output<Visitor>(n, next)) { _consumer(n.level()); }
      return next;
    }

    void
    visit(const bool t)
    {
      _visitor.visit(t);
    }
  };

  template <typename Visitor>
  void
  __zdd_Xelem(const zdd& A, const consumer<zdd::label_type>& c)
  {
    __zdd_Xelem__functional<Visitor> v(c);
    internal::traverse(A, v);
  }

  void
  zdd_minelem(const zdd& A, const consumer<zdd::label_type>& c)
  {
    return __zdd_Xelem<internal::traverse_satmin_visitor>(A, c);
  }

  void
  zdd_maxelem(const zdd& A, const consumer<zdd::label_type>& c)
  {
    return __zdd_Xelem<internal::traverse_satmax_visitor>(A, c);
  }

  //////////////////////////////////////////////////////////////////////////////
  template <typename Visitor>
  class __zdd_Xelem__stack
  {
  private:
    Visitor _visitor;

    // Reserve an internal memory vector (of up to 8 MiB) for the result.
    // TODO: Abstract the stack into <adiar/internal/data_structures/stack.h>.
    using value_type = zdd::label_type;
    internal::stack<internal::memory_mode::Internal, value_type> _stack;

  public:
    __zdd_Xelem__stack(size_t max_size)
      : _stack(max_size)
    {}

    zdd::pointer_type
    visit(const zdd::node_type& n)
    {
      adiar_assert(!n.high().is_terminal() || n.high().value(), "high terminals are never false");

      const zdd::pointer_type next = _visitor.visit(n);
      if (__zdd_Xelem__output<Visitor>(n, next)) { _stack.push(n.level()); }
      return next;
    }

    void
    visit(const bool t)
    {
      _visitor.visit(t);
    }

    zdd
    build_zdd()
    {
      // TODO (optimisation): Use 'internal::build_chain' directly to inline
      //                      popping values from the stack.
      return zdd_vars([this]() {
        if (this->_stack.empty()) { return make_optional<value_type>(); }

        const value_type top = this->_stack.top();
        this->_stack.pop();
        return make_optional<value_type>(top);
      });
    }
  };

  template <typename Visitor>
  zdd
  __zdd_Xelem(const zdd& A)
  {
    __zdd_Xelem__stack<Visitor> v(A->levels());
    internal::traverse(A, v);
    return v.build_zdd();
  }

  zdd
  zdd_minelem(const zdd& A)
  {
    return __zdd_Xelem<internal::traverse_satmin_visitor>(A);
  }

  zdd
  zdd_maxelem(const zdd& A)
  {
    return __zdd_Xelem<internal::traverse_satmax_visitor>(A);
  }
}
