#include <adiar/functional.h>
#include <adiar/types.h>
#include <adiar/zdd.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/data_structures/vector.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/ifstream.h>

namespace adiar
{
  class zdd_change_policy : public zdd_policy
  {
  public:
    static constexpr bool may_skip = true;

    static constexpr bool cut_true_terminal  = true;
    static constexpr bool cut_false_terminal = false;

    static constexpr size_t mult_factor = 1u;

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
      // TODO: simplify with generator function as input
      if (terminal_value) {
        return zdd_vars(make_generator(vars.rbegin(), vars.rend()));
      } else {
        return dd;
      }
    }

    static zdd
    terminal(const bool terminal_value)
    {
      return zdd_terminal(terminal_value);
    }

    static internal::intercut_rec
    hit_existing(const zdd::node_type& n)
    {
      if (n.low().is_false()) { return internal::intercut_rec_skipto{ n.high() }; }

      return internal::intercut_rec_output{ n.high(), n.low() };
    }

    static internal::intercut_rec_output
    hit_cut(const zdd::pointer_type& target)
    {
      return internal::intercut_rec_output{ zdd::pointer_type(false), target };
    }

    static internal::intercut_rec_output
    miss_existing(const zdd::node_type& n)
    {
      return internal::intercut_rec_output{ n.low(), n.high() };
    }
  };

  __zdd
  zdd_change(const exec_policy& ep, const zdd& A, const generator<zdd::label_type>& vars)
  {
    return internal::intercut<zdd_change_policy>(ep, A, vars);
  }

  __zdd
  zdd_change(const zdd& A, const generator<zdd::label_type>& vars)
  {
    return zdd_change(exec_policy(), A, vars);
  }
}
