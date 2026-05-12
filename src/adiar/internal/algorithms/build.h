#ifndef ADIAR_INTERNAL_ALGORITHMS_BUILD_H
#define ADIAR_INTERNAL_ALGORITHMS_BUILD_H

#include <adiar/exception.h>
#include <adiar/types.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/node_ofstream.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Builder for a terminal value
  //////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy>
  inline shared_levelized_file<typename DdPolicy::node_type>
  build_terminal(bool value)
  {
    using node_type = typename DdPolicy::node_type;
    shared_levelized_file<node_type> nf;
    {
      node_ofstream nw(nf);
      nw.unsafe_push(node_type(value));
      nw.unsafe_set_number_of_terminals(!value, value);
      nw.unsafe_set_canonical(true);
    }

    return nf;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Builder for a single-node with children (`false`, `true`).
  //////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy>
  inline shared_levelized_file<typename DdPolicy::node_type>
  build_ithvar(typename DdPolicy::level_type level)
  {
    using node_type    = typename DdPolicy::node_type;
    using pointer_type = typename node_type::pointer_type;

    if (node_type::max_label < level) {
      throw invalid_argument("Cannot represent that large a level");
    }

    shared_levelized_file<node_type> nf;
    {
      node_ofstream nw(nf);
      nw.unsafe_push(node(level, pointer_type::max_id, pointer_type(false), pointer_type(true)));
      nw.unsafe_push(level_info(level, 1u));
      nw.unsafe_set_canonical(true);
    }

    return nf;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Builds a chain of nodes bottom-up.
  //////////////////////////////////////////////////////////////////////////////
  template <typename Policy, typename Generator>
  inline typename Policy::dd_type
  build_chain(const Policy& policy, const Generator& vars)
  {
    using level_type = typename Policy::level_type;

    optional<pair<level_type, bool>> next = vars();

    if (!next) { return build_terminal<Policy>(Policy::init_terminal); }

    shared_levelized_file<typename Policy::node_type> nf;
    node_ofstream nw(nf);

    size_t max_internal_cut = 1;

    size_t terminals_internal[2] = { 0u, 0u };
    bool terminal_at_bottom[2]   = { false, false };
    size_t terminals[2]          = { 0u, 0u };

    typename Policy::pointer_type root(Policy::init_terminal);
    adiar_assert(root.is_terminal());

    do {
      const level_type next_var     = next.value().first;
      const level_type next_negated = next.value().second;

      // Fail if generator is increasing.
      if (!root.is_terminal() && root.level() < next_var) {
        throw invalid_argument("Levels not given in decreasing order");
      }

      // Skip value if generator provides the same (legal) value twice.
      if (!root.is_terminal() && root.level() == next_var) {
        next = vars();
        continue;
      }

      // Skip value, if policy calls for it.
      if (policy.skip(next_var)) {
        next = vars();
        continue;
      }

      // TODO: throw exception for too large levels

      // Create node on chain.
      using node_type = typename Policy::node_type;

      const node_type n = policy.make_node(next_var, root, next_negated);

      adiar_assert(n.level() == next_var, "Policy ought to make a node for this level node");
      adiar_assert(n.id() == node_type::max_id, "Policy ought to make a canonical node");

      max_internal_cut = std::max<size_t>(max_internal_cut, n.low().is_node() + n.high().is_node());

      if (n.low().is_terminal()) {
        terminals_internal[n.low().value()] =
          std::max<size_t>(terminals_internal[n.low().value()], n.high().is_node());

        terminals[n.low().value()] += 1u;
      }

      if (n.high().is_terminal()) {
        terminals_internal[n.high().value()] =
          std::max<size_t>(terminals_internal[n.high().value()], n.low().is_node());

        terminals[n.high().value()] += 1u;
      }

      if (root.is_terminal()) {
        terminal_at_bottom[n.low().value()]  = true;
        terminal_at_bottom[n.high().value()] = true;
      }

      nw.unsafe_push(n);
      nw.unsafe_push(level_info(next_var, 1u));

      root = n.uid();

      // Get next level
      next = vars();
    } while (next);

    // If all values have been skipped by the policy, then collapse to a terminal
    if (nw.size() == 0u) { return build_terminal<Policy>(Policy::init_terminal); }

    // Canonicity (assuming the policies are correct)
    nw.unsafe_set_canonical(true);

    // 1-level cuts
    nf->max_1level_cut[cut::Internal] = max_internal_cut;

    nf->max_1level_cut[cut::Internal_False] =
      std::max<size_t>({ nf->max_1level_cut[cut::Internal],
                         terminals_internal[false] + terminals[false] - terminal_at_bottom[false],
                         terminals[false] });

    nf->max_1level_cut[cut::Internal_True] =
      std::max<size_t>({ nf->max_1level_cut[cut::Internal],
                         terminals_internal[true] + terminals[true] - terminal_at_bottom[true],
                         terminals[true] });

    nf->max_1level_cut[cut::All] = std::max<size_t>({ nf->max_1level_cut[cut::Internal],
                                                      nf->max_1level_cut[cut::Internal_False],
                                                      nf->max_1level_cut[cut::Internal_True],
                                                      terminals[false] + terminals[true] });

    return nf;
  }

  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `build_chain` where the chain goes up the *low* arcs.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy, bool InitTerminal = false, bool HighValue = true>
  class chain_low : public DdPolicy
  {
  public:
    static constexpr bool init_terminal = InitTerminal;

    constexpr bool
    skip(const typename DdPolicy::level_type&) const
    {
      return false;
    }

    inline typename DdPolicy::node_type
    make_node(const typename DdPolicy::level_type& l,
              const typename DdPolicy::pointer_type& r,
              const bool /*negated*/) const
    {
      return typename DdPolicy::node_type(
        l, DdPolicy::max_id, r, typename DdPolicy::pointer_type(HighValue));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `build_chain` where the chain goes up the *high* arcs.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy, bool InitTerminal = true, bool LowValue = false>
  class chain_high : public DdPolicy
  {
  public:
    static constexpr bool init_terminal = InitTerminal;

    constexpr bool
    skip(const typename DdPolicy::level_type&) const
    {
      return false;
    }

    inline typename DdPolicy::node_type
    make_node(const typename DdPolicy::level_type& l,
              const typename DdPolicy::pointer_type& r,
              const bool /*negated*/) const
    {
      return typename DdPolicy::node_type(
        l, DdPolicy::max_id, typename DdPolicy::pointer_type(LowValue), r);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Policy for `build_chain` where the chain goes up both the *low* and
  ///        the *high* arcs.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy, bool InitTerminal = true>
  class chain_both : public DdPolicy
  {
  public:
    static constexpr bool init_terminal = InitTerminal;

    constexpr bool
    skip(const typename DdPolicy::level_type&) const
    {
      return false;
    }

    inline typename DdPolicy::node_type
    make_node(const typename DdPolicy::level_type& l,
              const typename DdPolicy::pointer_type& r,
              const bool /*negated*/) const
    {
      return typename DdPolicy::node_type(l, DdPolicy::max_id, r, r);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for a generator to map its output to fit the `build_chain`
  ///        algorithm.
  //////////////////////////////////////////////////////////////////////////////
  template <typename DdPolicy, typename Generator, bool negate = false>
  class chain_converter
  {
  private:
    const Generator& _gen;

  public:
    using value_type  = pair<typename DdPolicy::level_type, bool>;
    using result_type = optional<value_type>;

  private:
    inline value_type
    map(const value_type& x) const
    {
      return make_pair(x.first, negate ^ x.second);
    }

    inline value_type
    map(const typename DdPolicy::level_type& x) const
    {
      return make_pair(x, negate);
    }

    inline value_type
    map(const int& x) const
    {
      return make_pair(std::abs(x), negate ? 0 < x : x < 0);
    }

  public:
    chain_converter(const Generator& gen)
      : _gen(gen)
    {}

    inline result_type
    operator()() const
    {
      // NOTE: This is similar to a monadic 'bind'/'map'.

      // NOTE: The type of '_gen()' should be 'typename Generator::result_type',
      //       but declaring it to be exactly this results in an error. Hence,
      //       we break the style guide with an 'auto'.
      const auto next_opt = _gen();
      if (next_opt) {
        const value_type next_val = this->map(next_opt.value());
        return make_optional<value_type>(next_val);
      }
      return make_optional<value_type>();
    }
  };
}

#endif // ADIAR_INTERNAL_ALGORITHMS_BUILD_H
