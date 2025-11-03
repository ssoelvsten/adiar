#include <adiar/bdd.h>

#include <adiar/internal/algorithms/pred.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/node_ifstream.h>

namespace adiar
{
  bool
  bdd_iscanonical(const bdd& f)
  {
    return internal::dd_iscanonical(f);
  }

  bool
  bdd_isconst(const bdd& f)
  {
    return internal::dd_isterminal(f);
  }

  bool
  bdd_isterminal(const bdd& f)
  {
    return bdd_isconst(f);
  }

  bool
  bdd_isfalse(const bdd& f)
  {
    return internal::dd_isfalse(f);
  }

  bool
  bdd_istrue(const bdd& f)
  {
    return internal::dd_istrue(f);
  }

  bool
  bdd_isvar(const bdd& f)
  {
    // Assuming the BDD is fully reduced (which it should be), then it can only
    // be a single variable, if there are exactly two arcs to terminals.
    return f.number_of_terminals() == 2u;
  }

  bool
  bdd_isithvar(const bdd& f)
  {
    if (!bdd_isvar(f)) { return false; }

    internal::node_ifstream<> ns(f);
    const bdd::node_type root = ns.pull();

    adiar_assert(root.low() != root.high(), "A reduced BDD cannot have identical children");

    return root.high() == bdd::node_type::pointer_type(true);
  }

  bool
  bdd_isnithvar(const bdd& f)
  {
    if (!bdd_isvar(f)) { return false; }

    internal::node_ifstream<> ns(f);
    const bdd::node_type root = ns.pull();

    adiar_assert(root.low() != root.high(), "A reduced BDD cannot have identical children");

    return root.low() == bdd::node_type::pointer_type(true);
  }

  bool
  bdd_iscube(const bdd& f)
  {
    // Assuming the BDD is fully reduced (which it should be), then it can only
    // be a cube if:
    //
    // 1. Its width is exactly 1 (but also allow 0 for the terminal cases)
    //
    // 2. It has only one arc to `true` (this accepts `true` terminal)
    //
    // 3. The number of arcs to `false` is the same as the number of levels
    //    (this rejects `false` terminal).
    return f.width() <= 1u && f.number_of_terminals(true) == 1u
      && f.number_of_terminals(false) == bdd_varcount(f);
  }

  bool
  bdd_equal(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return internal::is_isomorphic(ep, f, g);
  }

  bool
  bdd_equal(const bdd& f, const bdd& g)
  {
    return bdd_equal(exec_policy(), f, g);
  }

  bool
  bdd_unequal(const exec_policy& ep, const bdd& f, const bdd& g)
  {
    return !bdd_equal(ep, f, g);
  }

  bool
  bdd_unequal(const bdd& f, const bdd& g)
  {
    return bdd_unequal(exec_policy(), f, g);
  }
}
