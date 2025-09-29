#ifndef ADIAR_INTERNAL_DATA_TYPES_CONVERT_H
#define ADIAR_INTERNAL_DATA_TYPES_CONVERT_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the *low* arc from a node
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline arc
  low_arc_of(const node& n)
  {
    return arc(n.uid() /*.as_ptr(false)*/, n.low());
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Extract the *high* arc from a node
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline arc
  high_arc_of(const node& n)
  {
    return arc(n.uid().as_ptr(true), n.high());
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief      Combine two arcs with the same source into a node.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline node
  node_of(const arc& low, const arc& high)
  {
    adiar_assert(essential(low.source()) == essential(high.source()), "Source are the same origin");

    adiar_assert(low.out_idx() == 0u, "Out-index is correct on low arc");
    adiar_assert(high.out_idx() == 1u, "Out-index is correct on high arc");

    adiar_assert(!low.target().is_node() || low.target().out_idx() == 0u,
                 "Out-index is empty in low target");
    adiar_assert(!high.target().is_node() || high.target().out_idx() == 0u,
                 "Out-index is empty in high target");

    adiar_assert(low.source().is_flagged() == false, "Source is not flagged on low arc");
    adiar_assert(high.source().is_flagged() == false, "Source is not flagged on high arc");

    adiar_assert(essential(low.source()) == low.source()
                 && essential(high.source()) == low.source());

    return node(node::uid_type(low.source()), low.target(), high.target());
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Combine two arcs into a single node with a custom label
  //////////////////////////////////////////////////////////////////////////////////////////////////
  inline node
  node_of(const node::label_type label, const arc& low, const arc& high)
  {
    adiar_assert(essential(low.source()) == essential(high.source()), "Source are the same origin");

    adiar_assert(low.out_idx() == 0u, "Out-index is correct on low arc");
    adiar_assert(high.out_idx() == 1u, "Out-index is correct on high arc");

    adiar_assert(!low.target().is_node() || low.target().out_idx() == 0u,
                 "Out-index is empty in low target");
    adiar_assert(!high.target().is_node() || high.target().out_idx() == 0u,
                 "Out-index is empty in high target");

    adiar_assert(low.source().is_flagged() == false, "Source is not flagged on low arc");
    adiar_assert(high.source().is_flagged() == false, "Source is not flagged on high arc");

    adiar_assert(essential(low.source()) == low.source()
                 && essential(high.source()) == low.source());

    return node(node::uid_type(replace(low.source(), label)), low.target(), high.target());
  }
}

#endif // ADIAR_INTERNAL_DATA_TYPES_CONVERT_H
