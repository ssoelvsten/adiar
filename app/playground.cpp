// TPIE Imports
#include "adiar/bdd.h"
#include "adiar/bdd/bdd_policy.h"
#include "adiar/builder.h"
#include "adiar/exception.h"
#include "adiar/exec_policy.h"
#include "adiar/functional.h"
#include "adiar/internal/algorithms/nested_sweeping.h"
#include "adiar/internal/algorithms/reduce.h"
#include "adiar/internal/algorithms/replace.h"
#include "adiar/internal/assert.h"
#include "adiar/internal/data_structures/levelized_priority_queue.h"
#include "adiar/internal/data_structures/sorter.h"
#include "adiar/internal/data_types/arc.h"
#include "adiar/internal/data_types/level_info.h"
#include "adiar/internal/data_types/ptr.h"
#include "adiar/internal/data_types/request.h"
#include "adiar/internal/data_types/uid.h"
#include "adiar/internal/io/arc_ifstream.h"
#include "adiar/internal/memory.h"
#include "adiar/types.h"
#include <algorithm>
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>

using namespace adiar;
using namespace internal;



//pretty printing lists
void
print_list(std::vector<int> l){
  std::cout << "[";
  for (int i = 0 ; i < l.size() ; i++){
    std::cout  << l[i]  << ", ";
  }
  std::cout << "]\n";
}

bdd
diamond(int N)
{
  bdd_builder builder;
  const auto bot = builder.add_node(false);
  const auto top = builder.add_node(true);

  auto a = top;

  for (int i = N-1; 0 <= i; --i) {
    const int a_var = 2 * i;
    const int b_var = 2 * i + 1;
    auto t1 = builder.add_node(b_var, bot, a);
    auto t2 = builder.add_node(b_var, a, bot);
    a = builder.add_node(a_var, t2, t1);
  }
  return builder.build();
}

//quadratic 
bdd 
quadratic_builder(int N){
  const auto bot = bdd_bot();
  const auto top = bdd_top();

  auto a1 = top;
  auto b1 = top;

  for (int i = N-1; 0 <= i; --i) {
    const int a_var = 2 * i + 1;
    a1 = bdd_ite(bdd_ithvar(a_var), bot, a1);
    const int b_var = 2 * i;
    b1 = bdd_ite(bdd_ithvar(b_var), b1, a1);
  }
  return b1;
}

bdd 
tree_builder(int N){
  bdd a = bdd_ithvar(N*2 + 1);
  bdd b = ~bdd_ithvar(N*2 + 1);
  for (int i = (N * 2-1); 0 <= i; i -= 2) {
      //std::cout << i << '\n';
      a = bdd_ite(bdd_ithvar(i), bdd_ite(bdd_ithvar(i+1), b, true), bdd_ite(bdd_ithvar(i+1), a, false));
      b = bdd_ite(bdd_ithvar(i), bdd_ite(bdd_ithvar(i+1), b, true), bdd_ite(bdd_ithvar(i+1), a, true));
  }
  bdd res = bdd_ite(bdd_ithvar(0), b, a);
  return res;

}

//split map to adj_swap part and rest part
  template<typename Policy>
  tuple<std::vector<int>, 4>
  adj_map_split(const typename Policy::dd_type dd , const replace_func<Policy>& m){
    //build old, new lists
    std::vector<int> old, mapped;
    {
      level_info_ifstream<true> level_info_file(dd);
      while(level_info_file.can_pull()){
        typename Policy::label_type l = level_info_file.pull().level();
        old.push_back(l);
        mapped.push_back(m(l));
      }
    }

    //build out lists!
    std::vector<int> adj_starts, adj_targets, sweep_starts, sweep_targets;
    int min_seen = mapped[0];
    for (int i = 0 ; i < old.size() ; i++){
      if (i < old.size()-1 && old[i] == mapped[i+1] && old[i+1] == mapped[i]){
        //then this is end of an adj swap (cus backwards)
        adj_targets.push_back(old[i]);
      }
      else if (i > 0 && old[i] == mapped[i-1] && old[i-1] == mapped[i]) {
        //then this is start of an adj swap (cus backwards)
        adj_starts.push_back(old[i]);
      }
      else if (old[i] != mapped[i]){
        if (mapped[i] > min_seen) {sweep_starts.push_back(old[i]); sweep_targets.push_back(mapped[i]);}
        else {min_seen = mapped[i];}
      }
    }
    //reversing adj_swap maps to work with top-down sweep
    std::reverse(adj_starts.begin(), adj_starts.end());
    std::reverse(adj_targets.begin(), adj_targets.end());
    const tuple<std::vector<int>, 4> res = {adj_starts, adj_targets, sweep_starts, sweep_targets};
    return res;
  }

int
main(int argc, char* argv[])
{
  std::cout << "-------------------------------------------------------------------------------\n"
            << "  Adiar " << adiar::version_string << " : Playground \n"
            << "-------------------------------------------------------------------------------\n"
            << "\n";

  size_t M = 1;

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
                                                               
    //bdd res_n = bdd_replace(bdd_6,m,replace_type::Non_Monotone);
    //bdd_printdot(res_n, "adj_swap_ns.dot");
    //bdd res = bdd_replace(bdd_6,m,replace_type::Swap_Adjacent);
    //bdd_printdot(res, "adj_swap.dot");

    //example with middle layers
        shared_levelized_file<bdd::node_type> bdd_9_nf;
    //purpose - many levels to facilitate swaps not side by side
    // also exp worst case if bad order
    /*
    //  (x0 /\ x1) \/ (x2 /\ x3) \/ (x4 /\ x5) \/ (x6 /\ x7)
    */

    { // Garbage collect early and free write-lock
      const node n7 = node(7, bdd::max_id, terminal_F, terminal_T);
      const node n6 = node(6, bdd::max_id, terminal_F, n7.uid());
      const node n5 = node(5, bdd::max_id, n6.uid(), terminal_T);
      const node n4 = node(4, bdd::max_id, n6.uid(), n5.uid());
      const node n3 = node(3, bdd::max_id, n4.uid(), terminal_T);
      const node n2 = node(2, bdd::max_id, n4.uid(), n3.uid());
      const node n1 = node(1, bdd::max_id, n2.uid(), terminal_T);
      const node n0 = node(0, bdd::max_id, n2.uid(), n1.uid());

      node_ofstream nw(bdd_9_nf);
      nw << n7 << n6 << n5 << n4 << n3 << n2 << n1 << n0;
    }
    const bdd bdd_9(bdd_9_nf);

    const replace_func<bdd_policy> m1 = [](const int x) { if (x == 1) {return 2;}
                                                               if (x == 2) {return 1;}
                                                               if (x == 5) {return 6;}
                                                               if (x == 6) {return 5;}
                                                               return x; };
            
            //bdd out = bdd_replace(bdd_9, m1);
            //bdd_printdot(out, "middle_layers.dot");
    

    //jump-up testing..
    shared_levelized_file<bdd::node_type> bdd_test_nf;
    { // Garbage collect early and free write-lock
      const node n6 = node(6, bdd::max_id, terminal_F, terminal_T);
      const node n5 = node(5, bdd::max_id, terminal_F, n6.uid());
      const node n4 = node(4, bdd::max_id, n5.uid(), terminal_T);
      const node n2 = node(2, bdd::max_id, n5.uid(), n4.uid());
      const node n1 = node(1, bdd::max_id, n2.uid(), n6.uid());
      const node n0 = node(0, bdd::max_id, n2.uid(), n1.uid());

      node_ofstream nw(bdd_test_nf);
      nw << n6 << n5 << n4 << n2 << n1 << n0;
    }
    const bdd bdd_test(bdd_test_nf);

    const replace_func<bdd_policy> m_ju = [](const int x) { if (x == 5) {return 3;}
                                                                    return x; };
    /*bdd res_ns = bdd_replace(bdd_test, m_ju,replace_type::Non_Monotone);
    bdd_printdot(res_ns, "jump_up_ns.dot");

    bdd res = bdd_replace(bdd_test, m_ju,replace_type::Jump_Up);
    bdd_printdot(res, "jump_up.dot");*/

    //testing ra
    shared_levelized_file<bdd::node_type> bdd_1_ext_nf;
    /*
    //        1        ---- x0
    //       / \
    //       | 2       ---- x2
    //       |/ \
    //       3  |      ---- x4
    //      / \ |
    //     4   5       ---- x5
    //    / \ / \
    //   F   T   F 
    */
    { // Garbage collect early and free write-lock
      const node n5 = node(5, bdd::max_id, terminal_T, terminal_F);
      const node n4 = node(5, bdd::max_id-1, terminal_F, terminal_T);
      const node n3 = node(4, bdd::max_id, n4, n5);
      const node n2 = node(2, bdd::max_id, n3.uid(), n5);
      const node n1 = node(0, bdd::max_id, n3.uid(), n2.uid());

      node_ofstream nw(bdd_1_ext_nf);
      nw << n5 << n4 << n3 << n2 << n1;
    }
    const bdd bdd_1_ext(bdd_1_ext_nf);
    const replace_func<bdd_policy> m_ra = [](const int x) {if (x == 0) return 3;
                                                    if (x == 4) return 6;
                                                    return x; };
    
    //trying to force 4 or more nodes in resulting jump up level
    //make var order horrible in exp worse case bdd_9
    //(x0 /\ x1) \/ (x2 /\ x3) \/ (x4 /\ x5) \/ (x6 /\ x7)
    const replace_func<bdd_policy> reverse_m = [](int x) {
      if (x == 0) return 0;
      if (x == 1) return 8;
      if (x == 2) return 1;
      if (x == 3) return 5;
      if (x == 4) return 2;
      if (x == 5) return 6;
      if (x == 6) return 3;
      if (x == 7) return 7;
      return x;
    };
    
    /*bdd should_be_big = bdd_replace(bdd_9, reverse_m);
    bdd_printdot(should_be_big, "should_be_big.dot");

    const replace_func<bdd_policy> jump_many = [](int x){ if (x == 8) {return 4;} return x;};

    bdd jump_with_many_ns = bdd_replace(should_be_big, jump_many, replace_type::Non_Monotone);
    bdd_printdot(jump_with_many_ns , "jump_with_many_ns.dot");

    bdd jump_with_many = bdd_replace(should_be_big, jump_many, replace_type::Jump_Up);
    bdd_printdot(jump_with_many , "jump_with_many.dot");

    bool for_real_det_samme = bdd_equal(jump_with_many_ns,jump_with_many);
    std::cout << "they are equal : " << for_real_det_samme << "\n";*/

    //testing map splitter
     const replace_func<bdd_policy> m_with_swaps = [](int x) {
      if (x == 0) return 0;
      if (x == 1) return 2;
      if (x == 2) return 1;
      if (x == 3) return 4;
      if (x == 4) return 5;
      if (x == 5) return 3;
      if (x == 6) return 7;
      if (x == 7) return 6;
      return x;
    };

    //tuple<std::vector<int>, 4> test = map_adj_split<bdd_policy>(bdd_9, m_with_swaps);
    /*std::cout << "adj_swap_starts: "; print_list(test[0]);
    std::cout << "adj_swap_ends: "; print_list(test[1]);
    std::cout << "sweep_starts: "; print_list(test[2]);
    std::cout << "sweep_ends: "; print_list(test[3]);*/

    //test policy?
    //nested_sweeping_replace<bdd_policy> test_policy(levs, make_generator(test[2].begin(), test[2].end()),make_generator(test[3].begin(), test[3].end()));
    
    //doing adj swaps then nested sweep
    //bdd swapped_first = bdd_replace(bdd_9, m_with_swaps, replace_type::Non_Monotone_Test);

    //just nested sweep
    //bdd just_nested = bdd_replace(bdd_9, m_with_swaps);

    shared_levelized_file<bdd::node_type> bdd_4_nf;
    /*
    //
    //        1_       ---- x0
    //        | \
    //        2 |      ---- x1
    //       / \|
    //      F   T
    */

    { // Garbage collect early and free write-lock
      const node n2 = node(1, bdd::max_id, terminal_F, terminal_T);
      const node n1 = node(0, bdd::max_id,n2.uid(), terminal_T );

      node_ofstream nw(bdd_4_nf);
      nw << n2 << n1;
    }
    const bdd bdd_4(bdd_4_nf);
    
    const replace_func<bdd_policy> m4 = [](const int x) { if (x == 0) return 1;
                                                     if (x == 1) return 0;
                                                     return x; };

    //bdd res = bdd_replace(bdd_4, m4);




    //quadratic
    //bdd quad_50 = quadratic_builder(50);
    const replace_func<bdd_policy> rev = [](int x) {
      return 100 - x;
    };
    //bdd_printdot(quad_50, "quad_before.dot");
    //bdd bdd_after = bdd_replace(quad_50, rev, adiar::replace_type::Non_Monotone);
    //bdd_printdot(bdd_after, "quad.dot");

    //tree?
    //bdd tree_5 = tree_builder(5); //8 vars
    const replace_func<bdd_policy> tree_map = [](int x) {
      if (x % 2 != 0){
        return x+ (5 *2+1);
      }
      return x;
    };

    //bdd bdd_tree_after = bdd_replace(tree_5, tree_map);
    //bdd_printdot(bdd_tree_after, "tree.dot");

    bdd diamond_15 = diamond(15);
    const replace_func<bdd_policy> odd_split = [](int x) {
      if (x == 0) {return 0;}
      if (x == 1) {return 8;}
      if (x == 2) {return 1;}
      if (x == 3) {return 9;}
      if (x == 4) {return 2;}
      if (x == 5) {return 10;}
      if (x == 6) {return 3;}
      if (x == 7) {return 11;}
      if (x == 8) {return 4;}
      if (x == 9) {return 12;}
      if (x == 10) {return 5;}
      if (x == 11) {return 13;}
      if (x == 12) {return 6;}
      if (x == 13) {return 14;}
      if (x == 14) {return 7;}
      return x;
    };
    bdd big_diamond = bdd_replace(diamond_15, odd_split);
    bdd big_diamond2 = bdd_replace(big_diamond, odd_split);

  }

  adiar::statistics_print();

  adiar::adiar_deinit();
  return 0;
}
