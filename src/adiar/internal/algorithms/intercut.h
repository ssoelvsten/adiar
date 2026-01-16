#ifndef ADIAR_INTERNAL_ALGORITHMS_INTERCUT_H
#define ADIAR_INTERNAL_ALGORITHMS_INTERCUT_H

#include <adiar/exec_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cnl.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_ofstream.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/node_ifstream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  //  Intercut Algorithm
  // ====================
  //
  // Given a Decision Diagram, a subset of the levels are have their arcs "cut"
  // in two with a new diagram node inserted (of any desired shape).
  // Furthermore, existing nodes on said level are changed as desired. Nodes on
  // *offset* levels are changed differently.
  /*
  //          ( )     ---- xi                ( )        ---- xi
  //         /   \                          /   \
  //        ( )  |    ---- xj     =>       ( )  (?)     ---- xj
  //        / \  |                         / \  ||
  //        a b  c                         b a  c
  */
  // Examples of uses are `zdd_extend`.
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics
  extern statistics::intercut_t stats_intercut;

  //////////////////////////////////////////////////////////////////////////////
  // Priority queue

  class intercut_req : public arc
  { // TODO: replace with request class
  private:
    ptr_uint64::label_type _level = ptr_uint64::max_label + 1u;

  public:
    intercut_req()                    = default;
    intercut_req(const intercut_req&) = default;
    ~intercut_req()                   = default;

  public:
    intercut_req(ptr_uint64 source, ptr_uint64 target, ptr_uint64::label_type lvl)
      : arc(source, target)
      , _level(lvl)
    {}

  public:
    ptr_uint64::label_type
    level() const
    {
      return _level;
    }
  };

  struct intercut_req_lt
  {
    bool
    operator()(const intercut_req& a, const intercut_req& b)
    {
      return a.level() < b.level() || (a.level() == b.level() && a.target() < b.target())
#ifndef NDEBUG
        || (a.level() == b.level() && a.target() == b.target() && a.source() < b.source())
#endif
        ;
    }
  };

  template <size_t look_ahead, memory_mode mem_mode>
  using intercut_priority_queue_t =
    levelized_node_priority_queue<intercut_req, intercut_req_lt, look_ahead, mem_mode, 2u, 0u>;

  //////////////////////////////////////////////////////////////////////////////

  struct intercut_rec_output
  {
    ptr_uint64 low;
    ptr_uint64 high;
  };

  struct intercut_rec_skipto
  {
    ptr_uint64 tgt;
  };

  using intercut_rec = std::variant<intercut_rec_output, intercut_rec_skipto>;

  //////////////////////////////////////////////////////////////////////////////
  // Helper functions
  template <typename intercut_policy>
  bool
  cut_terminal(const typename intercut_policy::label_type curr_level,
               const typename intercut_policy::label_type cut_level,
               const bool terminal_value)
  {
    return curr_level < cut_level && cut_level <= intercut_policy::max_label
      && (!terminal_value || intercut_policy::cut_true_terminal)
      && (terminal_value || intercut_policy::cut_false_terminal);
  }

  template <typename intercut_policy>
  class intercut_out__pq
  {
  public:
    static constexpr bool ignore_nil = false;

    template <typename pq_t>
    static inline void
    forward(arc_ofstream& aw,
            pq_t& pq,
            const typename intercut_policy::pointer_type source,
            const typename intercut_policy::pointer_type target,
            const typename intercut_policy::label_type curr_level,
            const typename intercut_policy::label_type next_cut)
    {
      const typename intercut_policy::label_type target_level = target.level();

      if (target.is_terminal()
          && !cut_terminal<intercut_policy>(curr_level, next_cut, target.value())) {
        aw.push_terminal(arc(source, target));
        return;
      }
      pq.push(intercut_req(source, target, std::min(target_level, next_cut)));
    }
  };

  class intercut_out__ofstream
  {
  public:
    static constexpr bool ignore_nil = true;

    template <typename pq_t>
    static inline void
    forward(arc_ofstream& aw,
            pq_t& /*pq*/,
            const ptr_uint64 source,
            const ptr_uint64 target,
            const ptr_uint64::label_type /*curr_level*/,
            const ptr_uint64::label_type /*next_cut*/)
    {
      aw.push_internal(arc(source, target));
    }
  };

  template <typename intercut_policy, typename in_policy, typename pq_t>
  inline void
  intercut_in__pq(arc_ofstream& aw,
                  pq_t& pq,
                  const typename intercut_policy::label_type out_label,
                  const typename intercut_policy::pointer_type pq_target,
                  const typename intercut_policy::pointer_type out_target,
                  const typename intercut_policy::label_type l)
  {
    adiar_assert(out_label <= out_target.level(),
                 "should forward/output a node on this level or ahead.");

    while (pq.can_pull() && pq.top().level() == out_label && pq.top().target() == pq_target) {
      const intercut_req parent = pq.pull();

      if (in_policy::ignore_nil && parent.source().is_nil()) { continue; }
      in_policy::forward(aw, pq, parent.source(), out_target, out_label, l);
    }
  }

  template <typename intercut_policy, typename pq_t>
  typename intercut_policy::__dd_type
  __intercut(const exec_policy& ep,
             const typename intercut_policy::dd_type& dd,
             const generator<typename intercut_policy::label_type>& xs,
             const size_t pq_memory,
             const size_t max_pq_size)
  {
    node_ifstream<> in_nodes(dd);
    node n = in_nodes.pull();

    // TODO: Only copy `xs` into a `shared_file<label_type>`, in the degenerate
    //       case, that we have to reverse it.
    //
    // In the general case, we have to use it both in the priority queue
    // `intercut_pq` below, and for a lookahead of the algorithm. The main issue
    // is how to design the priority queue such that it can retrieve, merge with
    // `dd_levels`, and expose `xs`.
    shared_file<typename intercut_policy::label_type> hit_levels;
    {
      ofstream<typename intercut_policy::label_type> lw(hit_levels);
      for (auto x = xs(); x; x = xs()) { lw << x.value(); }

      if (lw.size() == 0) { return intercut_policy::on_empty_labels(dd); }
    }

    if (n.is_terminal()) { return intercut_policy::on_terminal_input(n.value(), dd, hit_levels); }

    ifstream<typename intercut_policy::label_type> ls(hit_levels);
    typename intercut_policy::label_type l = ls.pull();

    shared_levelized_file<arc> out_arcs;
    arc_ofstream aw(out_arcs);

    out_arcs->max_1level_cut = 0;

    shared_file<typename intercut_policy::label_type> dd_levels;
    {
      ofstream<typename intercut_policy::label_type> lw(dd_levels);
      dd_support(dd, [&lw](const typename intercut_policy::label_type x) { lw << x; });
    }

    // Add request for root in the queue
    pq_t intercut_pq({ dd_levels, hit_levels }, pq_memory, max_pq_size, stats_intercut.lpq);
    intercut_pq.push(intercut_req(ptr_uint64::nil(), n.uid(), std::min(l, n.label())));

    // Process nodes of the decision diagram in topological order
    while (!intercut_pq.empty()) {
      // Set up next level
      intercut_pq.setup_next_level();

      const typename intercut_policy::label_type out_label = intercut_pq.current_level();
      typename intercut_policy::id_type out_id             = 0;

      const bool hit_level = out_label == l;

      // Forward to next label to cut on after this level
      while (ls.can_pull() && l <= out_label) { l = ls.pull(); }

      if (!ls.can_pull() && l <= out_label) { l = intercut_policy::max_label + 1; }

      // Update max 1-level cut
      out_arcs->max_1level_cut = std::max(out_arcs->max_1level_cut, intercut_pq.size());

      // Resolve requests that end at the cut for this level
      while (intercut_pq.can_pull()
             && intercut_pq.peek().target().level() == intercut_pq.peek().level()) {
        while (n.uid() < intercut_pq.top().target()) { n = in_nodes.pull(); }

        adiar_assert(n.uid() == intercut_pq.top().target(), "should always find desired node");

        const intercut_rec r =
          hit_level ? intercut_policy::hit_existing(n) : intercut_policy::miss_existing(n);

        if (intercut_policy::may_skip && std::holds_alternative<intercut_rec_skipto>(r)) {
          const intercut_rec_skipto rs = std::get<intercut_rec_skipto>(r);

          if (rs.tgt.is_terminal() && intercut_pq.top().source().is_nil()
              && !cut_terminal<intercut_policy>(out_label, l, rs.tgt.value())) {
            return intercut_policy::terminal(rs.tgt.value());
          }
          // TODO: The 'rs.tgt.is_terminal() && cut_terminal(...)' case can be handled even
          //       better with 'intercut_policy::on_terminal_input' but where the
          //       label file are only of the remaining labels.

          intercut_in__pq<intercut_policy, intercut_out__pq<intercut_policy>>(
            aw, intercut_pq, out_label, n.uid(), rs.tgt, l);
        } else {
          const intercut_rec_output ro = std::get<intercut_rec_output>(r);
          const node::uid_type out_uid(out_label, out_id++);

          intercut_out__pq<intercut_policy>::forward(
            aw, intercut_pq, out_uid.as_ptr(false), ro.low, out_label, l);

          intercut_out__pq<intercut_policy>::forward(
            aw, intercut_pq, out_uid.as_ptr(true), ro.high, out_label, l);

          intercut_in__pq<intercut_policy, intercut_out__ofstream>(
            aw, intercut_pq, out_label, n.uid(), out_uid, l);
        }
      }

      // Resolve requests that end after the cut for this level
      while (intercut_pq.can_pull()) {
        adiar_assert(out_label <= l,
                     "the last iteration in this case is for the very last label to cut on");

        const intercut_req request   = intercut_pq.top();
        const intercut_rec_output ro = intercut_policy::hit_cut(request.target());
        const node::uid_type out_uid(out_label, out_id++);

        intercut_out__pq<intercut_policy>::forward(
          aw, intercut_pq, out_uid.as_ptr(false), ro.low, out_label, l);

        intercut_out__pq<intercut_policy>::forward(
          aw, intercut_pq, out_uid.as_ptr(true), ro.high, out_label, l);

        intercut_in__pq<intercut_policy, intercut_out__ofstream>(
          aw, intercut_pq, out_label, request.target(), out_uid, l);
      }

      // Update meta data
      if (out_id > 0) { aw.push(level_info(out_label, out_id)); }
    }

    return typename intercut_policy::__dd_type(out_arcs, ep);
  }

  template <typename intercut_policy>
  size_t
  __intercut_2level_upper_bound(const typename intercut_policy::dd_type& dd)
  {
    const cut ct = cut(intercut_policy::cut_false_terminal, intercut_policy::cut_true_terminal);
    const safe_size_t max_1level_cut = dd.max_1level_cut(ct);

    return to_size((3 * intercut_policy::mult_factor * max_1level_cut) / 2 + 2);
  }

  template <typename intercut_policy>
  typename intercut_policy::__dd_type
  intercut(const exec_policy& ep,
           const typename intercut_policy::dd_type& dd,
           const generator<typename intercut_policy::label_type>& xs)
  {
    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures and check whether we can run them with a faster internal
    // memory variant.
    const size_t aux_available_memory = memory_available()
      // Input stream
      - node_ifstream<>::memory_usage()
      // Output stream
      - arc_ofstream::memory_usage();

    const size_t pq_memory = aux_available_memory;

    const size_t pq_memory_fits =
      intercut_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>::memory_fits(pq_memory);

    const bool internal_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::Internal;
    const bool external_only =
      ep.template get<exec_policy::memory>() == exec_policy::memory::External;

    const size_t pq_bound = __intercut_2level_upper_bound<intercut_policy>(dd);

    const size_t max_pq_size = internal_only ? std::min(pq_memory_fits, pq_bound) : pq_bound;

    if (!external_only && max_pq_size <= no_lookahead_bound()) {
#ifdef ADIAR_STATS
      stats_intercut.lpq.unbucketed += 1u;
#endif
      return __intercut<intercut_policy, intercut_priority_queue_t<0, memory_mode::Internal>>(
        ep, dd, xs, pq_memory, max_pq_size);
    } else if (!external_only && max_pq_size <= pq_memory_fits) {
#ifdef ADIAR_STATS
      stats_intercut.lpq.internal += 1u;
#endif
      return __intercut<intercut_policy,
                        intercut_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::Internal>>(
        ep, dd, xs, pq_memory, max_pq_size);
    } else {
#ifdef ADIAR_STATS
      stats_intercut.lpq.external += 1u;
#endif
      return __intercut<intercut_policy,
                        intercut_priority_queue_t<ADIAR_LPQ_LOOKAHEAD, memory_mode::External>>(
        ep, dd, xs, pq_memory, max_pq_size);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_INTERCUT_H
