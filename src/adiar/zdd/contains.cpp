#include <adiar/zdd.h>

#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/ifstream.h>

namespace adiar
{
  class zdd_contains_visitor
  {
    const generator<zdd::label_type>& gen;

    optional<zdd::label_type> l;

    // First visit is for the root
    bool is_first_visit = true;

    // Remember what we saw the last time
    zdd::label_type visited_label;

    bool terminal_val = false;

  public:
    zdd_contains_visitor(const generator<zdd::label_type>& a)
      : gen(a)
    {
      l = gen();
    }

    inline zdd::pointer_type
    visit(const zdd::node_type& n)
    {
      visited_label = n.level();

      const zdd::pointer_type next_ptr =
        l.has_value() && l.value() == visited_label ? n.high() : n.low();

      if (l) {
        // Did we miss a label before the root?
        if (is_first_visit && l.value() < visited_label) { return zdd::pointer_type::nil(); }

        // Forward once (if node was in the set) to hold onto the next to-be
        // visited level.
        if (l.value() == visited_label) { l = gen(); }

        // Will we miss the next to-be visited level?
        if (next_ptr.is_node() && l.has_value() && visited_label < l.value()
            && l.value() < next_ptr.level()) {
          return zdd::pointer_type::nil();
        }
      }

      is_first_visit = false;
      return next_ptr;
    }

    inline void
    visit(const bool s)
    {
      terminal_val = s;
    }

    inline bool
    get_result()
    {
      return terminal_val && !l.has_value();
    }
  };

  bool
  zdd_contains(const zdd& zdd, const generator<zdd::label_type>& a)
  {
    zdd_contains_visitor v(a);
    internal::traverse(zdd, v);
    return v.get_result();
  }
}
