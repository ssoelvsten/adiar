// TPIE Imports
#include "adiar/bdd.h"
#include "adiar/internal/algorithms/replace.h"
#include "adiar/types.h"
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>

using namespace adiar;
using namespace internal;




int
main(int argc, char* argv[])
{
  std::cout << "-------------------------------------------------------------------------------\n"
            << "  Adiar " << adiar::version_string << " : Playground \n"
            << "-------------------------------------------------------------------------------\n"
            << "\n";

  size_t M = 1024;

  try {
    if (argc > 1) { M = std::stoi(argv[1]); }
  } catch (const std::invalid_argument& ex) {
    std::cerr << "Invalid number: " << argv[1] << "\n";
    return -1;
  } catch (const std::out_of_range& ex) {
    std::cerr << "Number out of range: " << argv[1] << "\n";
    return -1;
  }

  adiar::adiar_init(M * 1024 * 1024);
  const bdd::pointer_type terminal_T = bdd::pointer_type(true);
  const bdd::pointer_type terminal_F = bdd::pointer_type(false);

  {
    shared_levelized_file<bdd::node_type> bdd_6_nf;
    /*
    //            1         ---- x0
    //           / \
    //           2 3        ---- x1
    //         _/ X \_
    //        | _/ \_ |
    //         X     X
    //        / \   / \
    //       4  5  6  7     ---- x2
    //      / \/ \/ \/ \
    //      F T  8  T  F    ---- x3
    //          / \
    //          F T
    */

    { // Garbage collect early and free write-lock
      const node n8 = node(3, bdd::max_id, terminal_F, terminal_T);
      const node n7 = node(2, bdd::max_id, terminal_T, terminal_F);
      const node n6 = node(2, bdd::max_id - 1, n8.uid(), terminal_T);
      const node n5 = node(2, bdd::max_id - 2, terminal_T, n8.uid());
      const node n4 = node(2, bdd::max_id - 3, terminal_F, terminal_T);
      const node n3 = node(1, bdd::max_id, n4.uid(), n6.uid());
      const node n2 = node(1, bdd::max_id - 1, n5.uid(), n7.uid());
      const node n1 = node(0, bdd::max_id, n2.uid(), n3.uid());

      node_ofstream nw(bdd_6_nf);
      nw << n8 << n7 << n6 << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_6(bdd_6_nf);


    const replace_func<bdd_policy> m = [](const int x) { if (x == 0) {return 1;}
                                                               if (x == 1) {return 0;}
                                                               if (x == 2) {return 3;}
                                                               if (x == 3) {return 2;}
                                                               else return x; };
    bdd res_n = bdd_replace(bdd_6,m,replace_type::Non_Monotone);
    bdd_printdot(res_n, "adj_swap_ns.dot");
    //bdd res = bdd_replace(bdd_6,m,replace_type::Swap_Adjacent);
    //bdd_printdot(res, "adj_swap.dot");
  }

  //adiar::statistics_print();

  adiar::adiar_deinit();
  return 0;
}
