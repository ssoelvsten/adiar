#include <adiar/domain.h>
#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/unreachable.h>

namespace adiar
{
  class zdd_complement_policy : public zdd_policy
  {
  public:
    static constexpr bool may_skip = false;

    static constexpr bool cut_true_terminal  = true;
    static constexpr bool cut_false_terminal = true;

    static constexpr size_t mult_factor = 2u;

  private:
    class on_true_terminal_chain_policy : public zdd_policy
    {
    public:
      static constexpr bool init_terminal = false;

      constexpr bool
      skip(const typename dd_policy::label_type&) const
      {
        return false;
      }

      inline bdd::node_type
      make_node(const zdd::label_type& l, const zdd::pointer_type& r, const bool /*negated*/) const
      {
        if (r.is_terminal()) {
          adiar_assert(r.value() == false, "Root should be Ø");
          return zdd::node_type(l, zdd::max_id, r, zdd_policy::pointer_type(true));
        } else {
          return zdd::node_type(l, zdd::max_id, r, r);
        }
      }

      static constexpr bool link[2]           = { true, true };
      static constexpr bool terminal_value[2] = { true, true };
    };

  public:
    static zdd
    on_empty_labels(const zdd& dd)
    {
      return dd;
    }

    static zdd
    on_terminal_input(const bool terminal_value,
                      const zdd& /*dd*/,
                      const internal::internal_vector<zdd::label_type>& universe)
    {
      // TODO: remove
      const generator<zdd::label_type> universe_generator =
        make_generator(universe.rbegin(), universe.rend());

      using complement_chain_converter =
        internal::chain_converter<zdd_policy, generator<zdd::label_type>>;

      const complement_chain_converter wrapped_universe(universe_generator);

      if (terminal_value) { // Include everything but Ø
        on_true_terminal_chain_policy p;

        return internal::build_chain<>(p, wrapped_universe);
      } else { // The complement of nothing is everything
        internal::chain_both<zdd_policy> p;
        return internal::build_chain<>(p, wrapped_universe);
      }
    }

    // LCOV_EXCL_START
    static zdd
    terminal(const bool /*terminal_value*/)
    {
      adiar_unreachable();
    }

    // LCOV_EXCL_STOP

    // We can improve this by knowing whether we are at the very last label. If
    // that is the case, and the high edge becomes F, then it can be skipped.
    //
    // Yet, this is only 2 nodes that we can kill; that is 4 arcs. This is 32
    // bytes of data and very few computation cycles. For very large cases the
    // shortcutting in branch-prediction probably offsets this?
    static internal::intercut_rec_output
    hit_existing(const zdd::node_type& n)
    {
      const zdd::pointer_type low  = n.low().is_terminal() ? !n.low() : n.low();
      const zdd::pointer_type high = n.high().is_terminal() ? !n.high() : n.high();

      return internal::intercut_rec_output{ low, high };
    }

    static internal::intercut_rec_output
    hit_cut(const zdd::pointer_type& target)
    {
      // T chain: We are definitely outside of the given set
      if (target.is_true()) { return internal::intercut_rec_output{ target, target }; }

      // Otherwise, check whether this variable is true and so we move to the T chain
      return internal::intercut_rec_output{ target, zdd::pointer_type(true) };
    }

    // LCOV_EXCL_START
    static internal::intercut_rec_output
    miss_existing(const zdd::node_type& /*n*/)
    {
      adiar_unreachable();
    }

    // LCOV_EXCL_STOP
  };

  __zdd
  zdd_complement(const exec_policy& ep, const zdd& A, const generator<zdd::label_type>& dom)
  {
    return internal::intercut<zdd_complement_policy>(ep, A, dom);
  }

  __zdd
  zdd_complement(const zdd& A, const generator<zdd::label_type>& dom)
  {
    return zdd_complement(exec_policy(), A, dom);
  }

  __zdd
  zdd_complement(const exec_policy& ep, const zdd& A)
  {
    const internal::shared_file<zdd::label_type> dom = domain_get();
    internal::ifstream<domain_var> ds(dom);

    return zdd_complement(ep, A, make_generator(ds));
  }

  __zdd
  zdd_complement(const zdd& A)
  {
    return zdd_complement(exec_policy(), A);
  }
}
