#ifndef ADIAR_BDD_BDD_POLICY_H
#define ADIAR_BDD_BDD_POLICY_H

#include <adiar/bdd/bdd.h>

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/data_types/uid.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logic related to being a 'Binary' Decision Diagram.
  //////////////////////////////////////////////////////////////////////////////
  using bdd_policy = internal::dd_policy<bdd, __bdd>;

  template <>
  inline bdd::pointer_type
  bdd_policy::reduction_rule(const bdd::node_type& n)
  {
    // If adding attributed edges, i.e. complement edges:
    //    remove the 'essential' below. Currently, it removes any forwarding of
    //    applying Reduction Rule.
    if (essential(n.low()) == essential(n.high())) { return n.low(); }
    return n.uid();
  }

  template <>
  inline bdd_policy::children_type
  bdd_policy::reduction_rule_inv(const bdd::pointer_type& child)
  {
    return { child, child };
  }
}

#endif // ADIAR_BDD_BDD_POLICY_H
