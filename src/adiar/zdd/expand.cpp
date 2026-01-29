#include <adiar/functional.h>
#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/data_structures/vector.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/unreachable.h>

namespace adiar
{
  class zdd_expand_policy : public zdd_policy
  {
  public:
    static constexpr bool may_skip = false;

    static constexpr bool cut_true_terminal  = true;
    static constexpr bool cut_false_terminal = false;

    static constexpr size_t mult_factor = 2u;

  public:
    static zdd
    on_empty_labels(const zdd& dd)
    {
      return dd;
    }

    static zdd
    on_terminal_input(const bool terminal_value,
                      const zdd& dd,
                      const internal::internal_vector<zdd::label_type>& vars)
    {
      if (terminal_value) {
        return zdd_powerset(make_generator(vars.rbegin(), vars.rend()));
      } else {
        return dd;
      }
    }

    // LCOV_EXCL_START
    static zdd
    terminal(const bool /*terminal_value*/)
    {
      adiar_unreachable();
    }

    // LCOV_EXCL_STOP

    // LCOV_EXCL_START
    static inline internal::intercut_rec
    hit_existing(const zdd::node_type& /*n*/)
    {
      // The user should NOT have supplied a label that hits any existing nodes.
      adiar_unreachable();
    }

    // LCOV_EXCL_STOP

    static inline internal::intercut_rec_output
    hit_cut(const zdd::pointer_type& target)
    {
      return internal::intercut_rec_output{ target, target };
    }

    static inline internal::intercut_rec_output
    miss_existing(const zdd::node_type& n)
    {
      return internal::intercut_rec_output{ n.low(), n.high() };
    }
  };

  __zdd
  zdd_expand(const exec_policy& ep, const zdd& A, const generator<zdd::label_type>& vars)
  {
    return internal::intercut<zdd_expand_policy>(ep, A, vars);
  }

  __zdd
  zdd_expand(const zdd& A, const generator<zdd::label_type>& vars)
  {
    return zdd_expand(exec_policy(), A, vars);
  }
}
