#include <adiar/bdd.h>
#include <adiar/exception.h>
#include <adiar/functional.h>
#include <tpie/tpie.h>

#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/stack.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/levelized_ifstream.h>
#include <adiar/internal/io/node_ofstream.h>
#include <adiar/internal/io/ofstream.h>
#include <adiar/internal/util.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class bdd_eval_func_visitor
  {
    const predicate<bdd::label_type>& af;
    bool result = false;

  public:
    bdd_eval_func_visitor(const predicate<bdd::label_type>& f)
      : af(f)
    {}

    inline bdd::pointer_type
    visit(const bdd::node_type& n)
    {
      const bool a = af(n.label());
      return a ? n.high() : n.low();
    }

    inline void
    visit(const bool s)
    {
      result = s;
    }

    inline bool
    get_result()
    {
      return result;
    }
  };

  bool
  bdd_eval(const bdd& bdd, const predicate<bdd::label_type>& af)
  {
    bdd_eval_func_visitor v(af);
    traverse(bdd, v);
    return v.get_result();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  class bdd_eval_generator_visitor
  {
    const generator<pair<bdd::label_type, bool>>& _generator;

    // The `_next_pair` value is only queried if there actually are BDD nodes.
    // Hence, we may as well unwrap the `optional<...>`.
    pair<bdd::label_type, bool> _next_pair;

    bool result = false;

  public:
    bdd_eval_generator_visitor(const generator<pair<bdd::label_type, bool>>& g)
      : _generator(g)
    {
      const optional<pair<bdd::label_type, bool>> p = _generator();
      if (p) { _next_pair = p.value(); }
    }

    inline bdd::pointer_type
    visit(const bdd::node_type& n)
    {
      const bdd::label_type level = n.label();

      while (_next_pair.first < level) {
        const optional<pair<bdd::label_type, bool>> p = _generator();

        if (!p) { throw out_of_range("Labels are insufficient to traverse BDD"); }

        if (p.value().first <= _next_pair.first) {
          throw invalid_argument("Labels are not in ascending order");
        }

        _next_pair = p.value();
      }

      if (_next_pair.first != level) {
        throw invalid_argument("Missing assignment for node visited in BDD");
      }

      return n.child(_next_pair.second);
    }

    inline void
    visit(const bool s)
    {
      result = s;
    }

    inline bool
    get_result()
    {
      return result;
    }
  };

  bool
  bdd_eval(const bdd& bdd, const generator<pair<bdd::label_type, bool>>& xs)
  {
    bdd_eval_generator_visitor v(xs);
    internal::traverse(bdd, v);
    return v.get_result();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // TODO: Merge code duplication with Curiously Recurring Template Pattern

  template <typename Visitor>
  class bdd_satX__stack
  {
  private:
    Visitor _visitor;

    const generator<bdd::label_type>& _generator;
    optional<bdd::label_type> _next_domain;

    // Reserve an internal memory vector (of up to 8 MiB) for the result.
    // TODO: Abstract the stack into <adiar/internal/data_structures/stack.h>.
    using value_type = pair<bdd::label_type, bool>;
    internal::stack<internal::memory_mode::Internal, value_type> _stack;

  public:
    bdd_satX__stack(const generator<bdd::label_type>& g, const size_t stack_size)
      : _generator(g)
      , _stack(stack_size)
    {
      this->_next_domain = this->_generator();
    }

    bdd::pointer_type
    visit(const bdd::node_type& n)
    {
      // Add skipped levels
      while (this->_next_domain && this->_next_domain.value() <= n.label()) {
        const bdd::label_type next_domain = this->_next_domain.value();
        if (next_domain != n.label()) {
          this->_stack.push({ next_domain, !Visitor::default_direction });
        }
        this->_next_domain = this->_generator();
      }
      // Update with this level
      const bdd::pointer_type next = this->_visitor.visit(n);
      this->_stack.push({ n.label(), next == n.low() });
      return next;
    }

    void
    visit(const bool t)
    {
      // Add skipped levels
      while (this->_next_domain) {
        this->_stack.push({ this->_next_domain.value(), !Visitor::default_direction });
        this->_next_domain = this->_generator();
      }
      // Finally, visit terminal
      _visitor.visit(t);
    }

  public:
    bdd
    build_bdd()
    {
      // TODO (optimisation): Use 'internal::build_chain' directly to inline
      //                      popping values from the stack.
      return bdd_cube([this]() {
        if (this->_stack.empty()) { return make_optional<value_type>(); }

        const value_type top = this->_stack.top();
        this->_stack.pop();
        return make_optional<value_type>(top);
      });
    }
  };

  template <typename Visitor>
  class bdd_satX__functional
  {
  private:
    Visitor _visitor;

    const consumer<pair<bdd::label_type, bool>>& _consumer;

  public:
    bdd_satX__functional(const consumer<pair<bdd::label_type, bool>>& c)
      : _consumer(c)
    {}

    bdd::pointer_type
    visit(const bdd::node_type& n)
    {
      const bdd::pointer_type next = this->_visitor.visit(n);
      this->_consumer({ n.label(), next == n.high() });
      return next;
    }

    void
    visit(const bool t)
    {
      _visitor.visit(t);
    }
  };

  template <typename Visitor>
  bdd
  __bdd_satX(const bdd& f, const generator<bdd::label_type>& dom, const size_t levels)
  {
    if (bdd_isfalse(f)) { return f; }

    bdd_satX__stack<Visitor> v(dom, levels);
    internal::traverse(f, v);

    return v.build_bdd();
  }

  template <typename Visitor>
  bdd
  __bdd_satX(const bdd& f)
  {
    if (bdd_istrue(f)) { return f; }

    const generator<bdd::label_type> nothing = []() -> optional<bdd::label_type> { return {}; };
    return __bdd_satX<Visitor>(f, nothing, f->levels());
  }

  template <typename Visitor>
  void
  __bdd_satX(const bdd& f, const consumer<pair<bdd::label_type, bool>>& c)
  {
    if (bdd_isconst(f)) { return; }

    bdd_satX__functional<Visitor> v(c);
    internal::traverse(f, v);
  }

  bdd
  bdd_satmin(const bdd& f)
  {
    return __bdd_satX<internal::traverse_satmin_visitor>(f);
  }

  bdd
  bdd_satmin(const bdd& f, const generator<bdd::label_type>& d, const size_t d_levels)
  {
    const size_t total_levels = std::min<size_t>(f->levels() + d_levels, bdd::max_label + 1);
    return __bdd_satX<internal::traverse_satmin_visitor>(f, d, total_levels);
  }

  bdd
  bdd_satmin(const bdd& f, const bdd& d)
  {
    if (!bdd_iscube(d)) { throw domain_error("BDD 'd' is not a cube"); }

    internal::level_info_ifstream d_levels(d);
    const generator<bdd::label_type> d_gen = make_generator__levels(d_levels);

    const size_t total_levels = std::min<size_t>(f->levels() + d->levels(), bdd::max_label + 1);

    return __bdd_satX<internal::traverse_satmin_visitor>(f, d_gen, total_levels);
  }

  void
  bdd_satmin(const bdd& f, const consumer<pair<bdd::label_type, bool>>& c)
  {
    return __bdd_satX<internal::traverse_satmin_visitor>(f, c);
  }

  bdd
  bdd_satmax(const bdd& f)
  {
    return __bdd_satX<internal::traverse_satmax_visitor>(f);
  }

  bdd
  bdd_satmax(const bdd& f, const generator<bdd::label_type>& d, const size_t d_levels)
  {
    const size_t total_levels = std::min<size_t>(f->levels() + d_levels, bdd::max_label + 1);
    return __bdd_satX<internal::traverse_satmax_visitor>(f, d, total_levels);
  }

  bdd
  bdd_satmax(const bdd& f, const bdd& d)
  {
    if (!bdd_iscube(d)) { throw domain_error("BDD 'd' is not a cube"); }

    internal::level_info_ifstream d_levels(d);
    const generator<bdd::label_type> d_gen = make_generator__levels(d_levels);

    const size_t total_levels = std::min<size_t>(f->levels() + d->levels(), bdd::max_label + 1);

    return __bdd_satX<internal::traverse_satmax_visitor>(f, d_gen, total_levels);
  }

  void
  bdd_satmax(const bdd& f, const consumer<pair<bdd::label_type, bool>>& c)
  {
    return __bdd_satX<internal::traverse_satmax_visitor>(f, c);
  }
}
