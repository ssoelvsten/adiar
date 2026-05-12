#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/select.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/unreachable.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class zdd_subset_labels
  {
    const generator<zdd::label_type>& gen;

    /// \brief The current level (including the current algorithm level)
    optional<zdd::label_type> l_incl = make_optional<zdd::label_type>();

    /// \brief The next level (definitely excluding the current level)
    optional<zdd::label_type> l_excl = make_optional<zdd::label_type>();

    /// We will rememeber how far the algorithm in substitution.h has got
    zdd::level_type alg_level = 0;

    /// Remember whether the current level is affected
    bool l_matches = false;

    /// We will remember whether any level of the input actually matched.
    bool l_match = false;

  protected:
    zdd_subset_labels(const generator<zdd::label_type>& g)
      : gen(g)
    {
      l_incl = gen();
      if (l_incl) { l_excl = gen(); }
    }

  public:
    /// \brief Forwards through the input to the given level
    inline void
    setup_level(const zdd::label_type new_level)
    {
      adiar_assert(alg_level <= new_level,
                   "The algorithm should ask for the levels in increasing order.");

      alg_level = new_level;

      while (l_incl && l_incl.value() < new_level) {
        l_incl = std::move(l_excl);
        if (l_incl) { l_excl = gen(); };
      }

      l_matches = l_incl && l_incl.value() == new_level;
      l_match |= l_matches;
    }

  protected:
    /// \brief Obtain whether the current level is a match
    bool
    current_matches() const
    {
      return l_matches;
    }

    /// \brief Whether the manager has a next level (including the current)
    bool
    has_level_incl() const
    {
      return l_incl && alg_level <= l_incl.value();
    }

    /// \brief Get the current level (including the current algorithm level)
    zdd::level_type
    level_incl() const
    {
      adiar_assert(has_level_incl());
      return l_incl.value();
    }

    /// \brief Whether the manager has a level ahead of the current
    bool
    has_level_excl() const
    {
      return (l_incl && alg_level < l_incl.value()) || l_excl;
    }

    /// \brief Get the next level (excluding the current one)
    zdd::level_type
    level_excl() const
    {
      adiar_assert(has_level_excl());
      if (alg_level < l_incl.value()) { return l_incl.value(); }
      return l_excl.value();
    }

  public:
    bool
    empty_assignment() const
    {
      return !has_level_incl();
    }

    bool
    no_match() const
    {
      return !l_match;
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename AssignmentPolicy>
  class zdd_offset_policy
    : public zdd_policy
    , public AssignmentPolicy
  {
  public:
    template <typename Arg>
    zdd_offset_policy(const Arg& a)
      : AssignmentPolicy(a)
    {}

  public:
    internal::select_rec
    process(const zdd::node_type& n)
    {
      if (AssignmentPolicy::current_matches()) { return n.low(); }
      return n;
    }

    zdd
    terminal(bool terminal_val) const
    {
      return zdd_terminal(terminal_val);
    }

    static constexpr bool skip_reduce = true;
  };

  __zdd
  zdd_offset(const exec_policy& ep, const zdd& A, const generator<zdd::label_type>& vars)
  {
    // Both { Ø }, and Ø cannot have more variables removed
    if (zdd_isterminal(A)) { return A; }

    zdd_offset_policy<zdd_subset_labels> policy(vars);

    // Empty set of variables in `xs`?
    if (policy.empty_assignment()) { return A; }

    // Run select sweep
    __zdd res = internal::select(ep, A, policy);

    // Skip Reduce if no level of `xs` matched with any in `A`.
    if (policy.no_match()) { return A; }
    return res;
  }

  __zdd
  zdd_offset(const zdd& A, const generator<zdd::label_type>& vars)
  {
    return zdd_offset(exec_policy(), A, vars);
  }

  __zdd
  zdd_offset(const exec_policy& ep, const zdd& A, zdd::label_type var)
  {
    return zdd_offset(ep, A, make_generator(var));
  }

  __zdd
  zdd_offset(const zdd& A, zdd::label_type var)
  {
    return zdd_offset(exec_policy(), A, var);
  }

  __zdd
  zdd_offset(const exec_policy& ep, const zdd& A)
  {
    return zdd_offset(ep, A, zdd_topvar(A));
  }

  __zdd
  zdd_offset(const zdd& A)
  {
    return zdd_offset(exec_policy(), A);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename AssignmentPolicy>
  class zdd_onset_policy
    : public zdd_policy
    , public AssignmentPolicy
  {
  public:
    template <typename Arg>
    zdd_onset_policy(const Arg& a)
      : AssignmentPolicy(a)
    {}

  public:
    internal::select_rec
    process(const zdd::node_type& n) const
    {
      if (AssignmentPolicy::current_matches()) {
        if (AssignmentPolicy::has_level_excl()) {
          if (n.high().is_terminal() || n.high().level() > AssignmentPolicy::level_excl()) {
            return zdd::pointer_type(false);
          }
        }

        return zdd::node_type(n.uid(), zdd::pointer_type(false), n.high());
      }

      if (AssignmentPolicy::has_level_incl()) {
        // If recursion goes past the intended level, then it is replaced with
        // the false terminal.
        const zdd::pointer_type low =
          n.low().is_terminal() || n.low().level() > AssignmentPolicy::level_incl()
          ? zdd::pointer_type(false)
          : n.low();

        // If this applies to high, then the node should be skipped entirely.
        if (n.high().is_terminal() || n.high().level() > AssignmentPolicy::level_incl()) {
          return low;
        }
        return zdd::node_type(n.uid(), low, n.high());
      }
      return n;
    }

    zdd
    terminal(bool terminal_val)
    {
      return zdd_terminal(!AssignmentPolicy::has_level_excl() && terminal_val);
    }

    static constexpr bool skip_reduce = true;
  };

  __zdd
  zdd_onset(const exec_policy& ep, const zdd& A, const generator<zdd::label_type>& xs)
  {
    if (zdd_isfalse(A)) { return A; }

    zdd_onset_policy<zdd_subset_labels> policy(xs);

    // Empty set of variables in `xs`?
    if (policy.empty_assignment()) { return A; }

    // If `A` is { Ø } and `xs` is non-empty, then it trivially collapses to Ø.
    if (zdd_istrue(A)) { return zdd_empty(); }

    // Run select sweep
    __zdd res = internal::select(ep, A, policy);

    // Skip Reduce no levels of `xs` matched with one from `A`.
    if (policy.no_match()) { return zdd_empty(); }
    return res;
  }

  __zdd
  zdd_onset(const zdd& A, const generator<zdd::label_type>& xs)
  {
    return zdd_onset(exec_policy(), A, xs);
  }

  __zdd
  zdd_onset(const exec_policy& ep, const zdd& A, zdd::label_type var)
  {
    return zdd_onset(ep, A, make_generator(var));
  }

  __zdd
  zdd_onset(const zdd& A, zdd::label_type var)
  {
    return zdd_onset(exec_policy(), A, var);
  }

  __zdd
  zdd_onset(const exec_policy& ep, const zdd& A)
  {
    return zdd_onset(ep, A, zdd_topvar(A));
  }

  __zdd
  zdd_onset(const zdd& A)
  {
    return zdd_onset(exec_policy(), A);
  }
}
