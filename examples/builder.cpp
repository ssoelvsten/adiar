#include <iostream>

#include <adiar/adiar.h>

////////////////////////////////////////////////////////////////////////////////
// Example explained in 'docs/tutorial/builder.md'
adiar::bdd example()
{
  /// [example]
  adiar::bdd_builder b;

  const adiar::bdd_ptr p2 = b.add_node(2, false, true);
  const adiar::bdd_ptr p1 = b.add_node(1, p2, true);
  const adiar::bdd_ptr p0 = b.add_node(0, p2, p1);

  adiar::bdd f = b.build();
  /// [example]

  return f;
}

////////////////////////////////////////////////////////////////////////////////
int main()
{
  adiar::adiar_init(adiar::minimum_memory);

  {
    adiar::bdd f = example();

    std::cout << "adiar::bdd_topvar(f)    = " << adiar::bdd_topvar(f) << "\n";
    std::cout << "adiar::bdd_nodecount(f) = " << adiar::bdd_nodecount(f) << "\n";
    std::cout << "adiar::bdd_varcount(f)  = " << adiar::bdd_varcount(f) << "\n";
    std::cout << "\n";
    std::cout << "adiar::bdd_pathcount(f) = " << adiar::bdd_pathcount(f) << "\n";
    std::cout << "adiar::bdd_satcount(f)  = " << adiar::bdd_satcount(f) << "\n";
  }

  adiar::adiar_deinit();
  return 0;
}
