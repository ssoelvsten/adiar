#include <utility>

#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/internal/algorithms/quantify.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/levelized_ifstream.h>
#include <adiar/internal/io/levelized_ofstream.h>

namespace adiar
{
  template <bool ShortcuttingValue>
  class bdd_quantify_policy : public bdd_policy
  {
  private:
    // Inherit from `predicate<bool, bool>` to hide these compile-time simplifications `bool_op`
    // (see #162)
    static constexpr bdd::pointer_type shortcutting_terminal = bdd::pointer_type(ShortcuttingValue);
    static constexpr bdd::pointer_type irrelevant_terminal = bdd::pointer_type(!ShortcuttingValue);

  public:
    static inline bdd::pointer_type
    resolve_root(const bdd::node_type& r)
    {
      // TODO: should all but the last case not have a 'suppression taint'?

      // Return shortcutting terminal (including its tainting flag).
      if (essential(r.low()) == shortcutting_terminal) { return r.low(); }
      if (essential(r.high()) == shortcutting_terminal) { return r.high(); }

      // Return other child (including its tainting flag) for irrelevant terminals.
      if (essential(r.low()) == irrelevant_terminal) { return r.high(); }
      if (essential(r.high()) == irrelevant_terminal) { return r.low(); }

      // Otherwise return 'nothing'
      return r.uid();
    }

  public:
    static inline bool
    keep_terminal(const bdd::pointer_type& p)
    {
      return essential(p) != irrelevant_terminal;
    }

    static inline bool
    collapse_to_terminal(const bdd::pointer_type& p)
    {
      return essential(p) == shortcutting_terminal;
    }

    // LCOV_EXCL_START
    static inline bdd::pointer_type
    resolve_terminals(const bdd::pointer_type& /*a*/, const bdd::pointer_type& /*b*/)
    {
      // Since only a single terminal terminal survives, this piece of code is never executed.
      adiar_unreachable();
    }

    // LCOV_EXCL_STOP

  public:
    static inline internal::cut
    cut_with_terminals()
    {
      const bool incl_false = bdd::pointer_type(false) != shortcutting_terminal;
      const bool incl_true  = bdd::pointer_type(true) != shortcutting_terminal;

      return internal::cut(incl_false, incl_true);
    }

  public:
    static constexpr bool quantify_onset = true;
  };

  //////////////////////////////////////////////////////////////////////////////
  __bdd
  bdd_exists(const exec_policy& ep, const bdd& f, bdd::label_type var)
  {
    return internal::quantify<bdd_quantify_policy<true>>(ep, f, var);
  }

  __bdd
  bdd_exists(const bdd& f, bdd::label_type var)
  {
    return bdd_exists(exec_policy(), f, var);
  }

  // TODO bdd_exists(__bdd&& f, bdd::label_type var)
  //
  // Use nested sweeping to skip some of the Reduce step!

  __bdd
  bdd_exists(const exec_policy& ep, const bdd& f, const predicate<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<true>>(ep, f, vars);
  }

  __bdd
  bdd_exists(const bdd& f, const predicate<bdd::label_type>& vars)
  {
    return bdd_exists(exec_policy(), f, vars);
  }

  __bdd
  bdd_exists(const exec_policy& ep, bdd&& f, const predicate<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<true>>(ep, std::move(f), vars);
  }

  __bdd
  bdd_exists(bdd&& f, const predicate<bdd::label_type>& vars)
  {
    return bdd_exists(exec_policy(), std::move(f), vars);
  }

  __bdd
  bdd_exists(const exec_policy& ep, __bdd&& f, const predicate<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<true>>(ep, std::move(f), vars);
  }

  __bdd
  bdd_exists(__bdd&& f, const predicate<bdd::label_type>& vars)
  {
    return bdd_exists(exec_policy(), std::move(f), vars);
  }

  __bdd
  bdd_exists(const exec_policy& ep, const bdd& f, const generator<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<true>>(ep, f, vars);
  }

  __bdd
  bdd_exists(const bdd& f, const generator<bdd::label_type>& vars)
  {
    return bdd_exists(exec_policy(), f, vars);
  }

  __bdd
  bdd_exists(const exec_policy& ep, bdd&& f, const generator<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<true>>(ep, std::move(f), vars);
  }

  __bdd
  bdd_exists(bdd&& f, const generator<bdd::label_type>& vars)
  {
    return bdd_exists(exec_policy(), std::move(f), vars);
  }

  __bdd
  bdd_exists(const exec_policy& ep, __bdd&& f, const generator<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<true>>(ep, std::move(f), vars);
  }

  __bdd
  bdd_exists(__bdd&& f, const generator<bdd::label_type>& vars)
  {
    return bdd_exists(exec_policy(), std::move(f), vars);
  }

  //////////////////////////////////////////////////////////////////////////////

  __bdd
  bdd_forall(const exec_policy& ep, const bdd& f, bdd::label_type var)
  {
    return internal::quantify<bdd_quantify_policy<false>>(ep, f, var);
  }

  __bdd
  bdd_forall(const bdd& f, bdd::label_type var)
  {
    return bdd_forall(exec_policy(), f, var);
  }

  // TODO bdd_forall(__bdd&& f, bdd::label_type var)
  //
  // Use nested sweeping to skip some of the Reduce step!

  __bdd
  bdd_forall(const exec_policy& ep, const bdd& f, const predicate<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<false>>(ep, f, vars);
  }

  __bdd
  bdd_forall(const bdd& f, const predicate<bdd::label_type>& vars)
  {
    return bdd_forall(exec_policy(), f, vars);
  }

  __bdd
  bdd_forall(const exec_policy& ep, bdd&& f, const predicate<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<false>>(ep, std::move(f), vars);
  }

  __bdd
  bdd_forall(bdd&& f, const predicate<bdd::label_type>& vars)
  {
    return bdd_forall(exec_policy(), std::move(f), vars);
  }

  __bdd
  bdd_forall(const exec_policy& ep, __bdd&& f, const predicate<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<false>>(ep, std::move(f), vars);
  }

  __bdd
  bdd_forall(__bdd&& f, const predicate<bdd::label_type>& vars)
  {
    return bdd_forall(exec_policy(), std::move(f), vars);
  }

  __bdd
  bdd_forall(const exec_policy& ep, const bdd& f, const generator<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<false>>(ep, f, vars);
  }

  __bdd
  bdd_forall(const bdd& f, const generator<bdd::label_type>& vars)
  {
    return bdd_forall(exec_policy(), f, vars);
  }

  __bdd
  bdd_forall(const exec_policy& ep, bdd&& f, const generator<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<false>>(ep, std::move(f), vars);
  }

  __bdd
  bdd_forall(bdd&& f, const generator<bdd::label_type>& vars)
  {
    return bdd_forall(exec_policy(), std::move(f), vars);
  }

  __bdd
  bdd_forall(const exec_policy& ep, __bdd&& f, const generator<bdd::label_type>& vars)
  {
    return internal::quantify<bdd_quantify_policy<false>>(ep, std::move(f), vars);
  }

  __bdd
  bdd_forall(__bdd&& f, const generator<bdd::label_type>& vars)
  {
    return bdd_forall(exec_policy(), std::move(f), vars);
  }
}
