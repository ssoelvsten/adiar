#include <stdint.h>

#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/domain.h>
#include <adiar/exception.h>

#include <adiar/internal/algorithms/count.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/dd_func.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  // SatCount Policy
  struct sat_data
  {
    static constexpr bool sort_on_tiebreak = true;
    static constexpr bool has_level        = false;

    /// Sum of satisfying assignments up to parent
    uint64_t sum;

    /// Number of levels visited up to parent
    bdd::label_type levels_visited;

    /// Sorting predicate
    inline bool
    operator<(const sat_data& o) const
    {
      return this->levels_visited < o.levels_visited;
    }
  };

  class sat_count_policy : public bdd_policy
  {
  public:
    using data_type = sat_data;

    static constexpr data_type init_data = { 1u, 0u };

    static constexpr uint64_t
    resolve_false(const data_type& /*d*/, const typename bdd::label_type /*varcount*/)
    {
      return 0u;
    }

    static inline uint64_t
    resolve_true(const data_type& d, const typename bdd::label_type varcount)
    {
      adiar_assert(d.levels_visited <= varcount, "Cannot have visited more levels than exist");

      const uint64_t unvisited = varcount - d.levels_visited;
      return d.sum * (1u << unvisited);
    }

    static inline data_type
    merge(const data_type&& acc, const data_type& next)
    {
      adiar_assert(acc.sum > 0u && next.sum > 0u,
                   "No request should have an 'empty' set of assignemnts");
      adiar_assert(acc.levels_visited <= next.levels_visited,
                   "Requests should be merged in ascending order of visited levels");

      const uint64_t visited_diff = next.levels_visited - acc.levels_visited;

      return { acc.sum * (1u << visited_diff) + next.sum, next.levels_visited };
    }

    static inline data_type
    merge_end(const data_type&& acc)
    {
      return { acc.sum, acc.levels_visited + 1u };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  size_t
  bdd_nodecount(const bdd& f)
  {
    return internal::dd_nodecount(f);
  }

  bdd::label_type
  bdd_varcount(const bdd& f)
  {
    return internal::dd_varcount(f);
  }

  uint64_t
  bdd_pathcount(const exec_policy& ep, const bdd& f)
  {
    return bdd_isterminal(f)
      ? 0
      : internal::count<internal::path_count_policy<bdd_policy>>(ep, f, bdd_varcount(f));
  }

  uint64_t
  bdd_pathcount(const bdd& f)
  {
    return bdd_pathcount(exec_policy(), f);
  }

  uint64_t
  bdd_satcount(const exec_policy& ep, const bdd& f, bdd::label_type varcount)
  {
    if (varcount < bdd_varcount(f)) {
      throw invalid_argument("'varcount' ought to be at least the number of levels in the BDD");
    }

    if (bdd_isterminal(f)) { return dd_valueof(f) ? std::min(1u, varcount) << varcount : 0u; }

    return internal::count<sat_count_policy>(ep, f, varcount);
  }

  uint64_t
  bdd_satcount(const bdd& f, bdd::label_type varcount)
  {
    return bdd_satcount(exec_policy(), f, varcount);
  }

  uint64_t
  bdd_satcount(const exec_policy& ep, const bdd& f)
  {
    return bdd_satcount(ep, f, std::max<bdd::label_type>(domain_size(), bdd_varcount(f)));
  };

  uint64_t
  bdd_satcount(const bdd& f)
  {
    return bdd_satcount(exec_policy(), f);
  };
}
