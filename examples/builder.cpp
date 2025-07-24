#include "examples_common.cpp"

int main(int argc, char* argv[])
{
  // ===== Parse argument =====
  {
    bool should_error_exit = init_cl_arguments(argc, argv);
    if (should_error_exit) { return 1; }
  }

  // ===== Builder example =====
  adiar::adiar_init(M*1024*1024);

  {
    /// [example]
    adiar::bdd_builder b;

    const adiar::bdd_ptr p2 = b.add_node(2, false, true);
    const adiar::bdd_ptr p1 = b.add_node(1, p2, true);
    const adiar::bdd_ptr p0 = b.add_node(0, p2, p1);

    adiar::bdd f = b.build();
    /// [example]

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
