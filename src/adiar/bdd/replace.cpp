#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/functional.h>
#include <adiar/types.h>

#include <adiar/internal/algorithms/replace.h>

namespace adiar
{
  __bdd
  bdd_replace(const exec_policy& ep,
              const bdd& f,
              const function<bdd::label_type(bdd::label_type)>& m,
              replace_type m_type)
  {
    return internal::replace<bdd_policy>(ep, f, m, m_type);
  }

  __bdd
  bdd_replace(const bdd& f,
              const function<bdd::label_type(bdd::label_type)>& m,
              replace_type m_type)
  {
    return bdd_replace(exec_policy(), f, m, m_type);
  }

  __bdd
  bdd_replace(const exec_policy& ep,
              __bdd&& f,
              const function<bdd::label_type(bdd::label_type)>& m,
              replace_type m_type)
  {
    return internal::replace<bdd_policy>(ep, std::move(f), m, m_type);
  }

  __bdd
  bdd_replace(__bdd&& f, const function<bdd::label_type(bdd::label_type)>& m, replace_type m_type)
  {
    return internal::replace<bdd_policy>(std::move(f), m, m_type);
  }
}
