#include "examples_common.cpp"

#include <optional>

/*******************************************************************************
 * This entire example is based off of the encoding by Randal Bryant for the
 * same problem that you can find here:
 *
 * https://github.com/rebryant/Cloud-BDD/blob/conjunction_streamlined/hamiltonian/hpath.py
 */

/*******************************************************************************
 * We'd find it interesting to output the size of the largest ZDD and the final
 * ZDD.
 */
size_t largest_nodes = 1;

/*******************************************************************************
 *                             Variable ordering
 *
 *                          (N^2 * t) + (N * r) + c.
 *
 * That is, we encode the entire information about position and time-step into
 * the very variable label. This ordering is first by column, second by row, and
 * finally by time. Most importantly, this means that any "snapshot" of the
 * board at a specific time-step are grouped together.
 */
inline typename adiar::zdd::label_type int_of_position(int N, int r, int c, int t = 0)
{
  return (N * N * t) + (N * r) + c;
}

/*******************************************************************************
 *                            Closed Tour Constraint
 *
 * To only look at the closed tours, i.e. hamiltonian cycles and not hamiltonian
 * paths, we merely fix it to start in the top-left corner. Since we are
 * counting unoriented paths, then we can fix one of the two possible next
 * squares to be the one used at t = 1.
 *
 * The following array includes these very squares. The predicate below tests
 * whether the argument is in one of the three squares.
 */
const int closed_squares [3][2] = {{0,0}, {1,2}, {2,1}};

bool is_closed_square(int r, int c)
{
  return (r == closed_squares[0][0] && c == closed_squares[0][1])
      || (r == closed_squares[1][0] && c == closed_squares[1][1])
      || (r == closed_squares[2][0] && c == closed_squares[2][1]);
}

/*******************************************************************************
 * Combining the above, we can construct the ZDD that fixes the initial
 * position, the second position, and the final position while allowing any
 * possibilities for anything in between.
 *
 *                              *          ---- position (0,0) at time 0
 *                             / \
 *                             F *         ---- position (1,2) at time 1
 *                              / \
 *                              F *
 *                                ||
 *                                         ---- positions at time 2 ... N*N-2
 *                                ||
 *                                *        ---- position (2,1) at time N*N-1
 *                               / \
 *                               F T
 */
adiar::zdd constraint_closed(int N)
{
  adiar::zdd_builder builder;

  // Fix t = max_time to be (1,2)
  const int max_time = N*N-1;

  const int stepMax_position = int_of_position(N,
                                               closed_squares[2][0],
                                               closed_squares[2][1],
                                               max_time);

  typename adiar::zdd_ptr root = builder.add_node(stepMax_position, false, true);

  // All in between is as-is but take the hamiltonian constraint into account.
  for (int t = max_time - 1; t > 1; t--) {
    for (int r = N-1; r >= 0; r--) {
      for (int c = N-1; c >= 0; c--) {
        if (is_closed_square(r,c)) { continue; }

        root = builder.add_node(int_of_position(N,r,c,t), root, root);
      }
    }
  }

  // Fix t = 1 to be (2,1)
  const int step1_position = int_of_position(N,
                                             closed_squares[1][0],
                                             closed_squares[1][1],
                                             1);
  root = builder.add_node(step1_position, false, root);

  // Fix t = 0 to be (0,0)
  const int step0_position = int_of_position(N,
                                             closed_squares[0][0],
                                             closed_squares[0][1],
                                             0);
  root = builder.add_node(step0_position, false, root);

  // Finalize
  adiar::zdd out = builder.build();

  const size_t nodecount = zdd_nodecount(out);
  largest_nodes = std::max(largest_nodes, nodecount);

  return out;
}

/*******************************************************************************
 *                              Legal Moves
 *
 * We need a little bit of logic to compute the different parts of a move. This
 * is not the bottleneck, so we will do something somewhat easily comprehensible
 * rather than fast.
 *
 * The following two arrays encode (starting from 'north west' and going
 * clockwise) the i and j coordinates of all the moves.
 */
constexpr int row_moves[8]    = { -2, -2, -1, -1,  1,  1,  2,  2 };
constexpr int column_moves[8] = { -1,  1, -2,  2, -2,  2, -1,  1 };

bool is_legal_move(int /*N*/, int r_from, int c_from, int r_to, int c_to)
{
  for (int idx = 0; idx < 8; idx++) {
    if (r_from + row_moves[idx] == r_to &&
        c_from + column_moves[idx] == c_to) {
      return true;
    }
  }
  return false;
}

bool is_legal_position(int N, int r, int c, int t = 0)
{
  if (r < 0 || N-1 < r)  { return false; }
  if (c < 0 || N-1 < c)  { return false; }
  if (t < 0 || N*N-1 < t) { return false; }

  return true;
}

bool is_reachable(int /*N*/, int r, int c)
{
  for (int idx = 0; idx < 8; idx++) {
    if (is_legal_position(N, r + row_moves[idx], c + column_moves[idx])) {
      return true;
    }
  }
  return false;
}

/*******************************************************************************
 * With the above, we can encode what the legal transitions from one state of
 * one time step to the next. To this end, we have a "don't care" chain for time
 * steps prior and past the two time steps of interest. At time step t we may
 * ask exactly where the Knight is placed to then check whether it is placed in
 * any legal position at the next time step.
 *
 *                       *
 *                       ||
 *                                 ---- positions at time t-1 and earlier
 *                       ||
 *                       *         ---- pos (0,0) at time t
 *                      / \
 *                      *  \       ---- pos (1,0) at time t
 *                     / \  \
 *                    .  .  |
 *                   .   .  |
 *                  .    .  |
 *                          *      ---- pos (1,2) at time t+1
 *                         / \
 *                        *   \    ---- pos (2,1) at time t+1
 *                       / \  |
 *                       F  \ /
 *                           *
 *                           ||
 *                                 ---- positions at time t+2 and later
 *                           ||
 *                           T
 */
adiar::zdd constraint_transition(int N, int t)
{
  adiar::zdd_builder builder;

  const int max_cell = N-1;

  // Time steps t' > t+1:
  adiar::zdd_ptr post_chain_root = builder.add_node(true);

  const int max_time = N*N-1;

  for (int time = max_time; time > t+1; time--) {
    for (int row = max_cell; row >= 0; row--) {
      for (int col = max_cell; col >= 0; col--) {
        if (!is_reachable(N, row, col)) { continue; }

        const int this_label = int_of_position(N, row, col, time);
        post_chain_root = builder.add_node(this_label, post_chain_root, post_chain_root);
      }
    }
  }

  // Time step t+1:
  //   Chain with each possible position reachable from some position at time 't'.
  std::vector<adiar::zdd_ptr> to_chains(N*N, builder.add_node(false));

  for (int row = max_cell; row >= 0; row--) {
    for (int col = max_cell; col >= 0; col--) {
      const int this_label = int_of_position(N, row, col, t+1);

      for (int row_t = max_cell; row_t >= 0; row_t--) {
        for (int col_t = max_cell; col_t >= 0; col_t--) {
          if (!is_reachable(N, row_t, col_t)) { continue; }
          if (!is_legal_move(N, row_t, col_t, row, col)) { continue; }

          const int vector_idx = int_of_position(N, row_t, col_t);

          to_chains.at(vector_idx) = builder.add_node(this_label,
                                                      to_chains.at(vector_idx),
                                                      post_chain_root);
        }
      }
    }
  }

  // Time step t:
  //   For each position at time step 't', check whether we are "here" and go to
  //   the chain checking "where we go to" at 't+1'.
  typename adiar::zdd_ptr root = builder.add_node(false);

  for (int row = max_cell; row >= 0; row--) {
    for (int col = max_cell; col >= 0; col--) {
      if (!is_reachable(N, row, col)) { continue; }

      const int this_label = int_of_position(N, row, col, t);
      const int to_chain_idx = int_of_position(N, row, col);
      root = builder.add_node(this_label, root, to_chains.at(to_chain_idx));
    }
  }

  // Time-step t' < t:
  //   Just allow everything, i.e. add no constraints
  for (int pos = int_of_position(N, max_cell, max_cell, t-1); pos >= 0; pos--) {
    root = builder.add_node(pos, root, root);
  }

  // Finalize
  adiar::zdd out = builder.build();

  const size_t nodecount = zdd_nodecount(out);
  largest_nodes = std::max(largest_nodes, nodecount);

  return out;
}


/*******************************************************************************
 *                           Hamiltonian Constraint
 *
 * Finally, for each position on the board, we want to add the constraint, that
 * you can only be there once.
 *
 * This essentially is an automaton with two states 'not seen' and 'seen'. If
 * the position is 'seen' as taken then one goes to a second chain of nodes,
 * where it may not be seen again.
 */
adiar::zdd constraint_exactly_once(int N, int r, int c)
{
  adiar::zdd_builder builder;

  adiar::zdd_ptr out_never = builder.add_node(false);
  adiar::zdd_ptr out_once = builder.add_node(true);

  const int max_time = N*N-1;
  const int max_cell = N-1;

  for (int this_t = max_time; this_t >= 0; this_t--) {
    for (int this_r = max_cell; this_r >= 0; this_r--) {
      for (int this_c = max_cell; this_c >= 0; this_c--) {
        const int this_label = int_of_position(N, this_r, this_c, this_t);
        const bool is_rc = r == this_r && c == this_c;

        if (!is_rc && (this_t > 0 || this_r > r)) {
          out_once = builder.add_node(this_label, out_once, out_once);
        }

        out_never = builder.add_node(this_label,
                                     out_never,
                                     is_rc ? out_once : out_never);
      }
    }
  }

  // Finalize
  adiar::zdd out = builder.build();

  const size_t nodecount = zdd_nodecount(out);
  largest_nodes = std::max(largest_nodes, nodecount);

  return out;
}

// expected number of directed tours taken from:
//  https://en.wikipedia.org/wiki/Knight%27s_tour#Number_of_tours
uint64_t expected_all[9] = {
  0,
  1,
  0,
  0,
  0,
  1728,
  6637920,
  165575218320,
  19591828170979904
};

uint64_t expected_closed[10] = {
  0,
  1,
  0,
  0,
  0,
  0,
  9862,
  0,
  13267364410532,
  0
};

int main(int argc, char* argv[])
{
  // ===== Parse argument =====
  bool only_closed = false;

  {
    int c;
    bool should_error_exit = init_cl_arguments(argc, argv);

    while ((c = getopt(argc, argv, "c")) != -1) {
      switch(c) {
      case 'c':
        // HACK: 'constraint_closed' only works for 3x3 boards and bigger.
        only_closed = 3 <= N;
        continue;

      case 'h':
        std::cout << "        -c       [ ]          Enumerate only closed tours (rather open and closed)" << std::endl;
        exit(0);
      }
    }

    if (!should_error_exit && N == 0) {
      std::cout << "  Must specify positive number for N" << std::endl;
      should_error_exit = true;
    }

    if (should_error_exit) { exit(1); }
  }

  // ===== ADIAR =====
  // Initialize
  adiar::adiar_init(M * 1024 * 1024);
  std::cout << "| Initialized Adiar with " << M << " MiB of memory"  << std::endl
            << "|" << std::endl;

  // ===== Knight's Tour =====

  std::cout << "| " << N << "x" << N << " Knight's Tour "
            << "(" << (only_closed ? "Closed" : "Open and Closed") << ")" << std::endl
            << "|" << std::endl;;

  adiar::zdd paths;
  std::cout << "| Time:" << std::endl;

  if (N == 1) {
    paths = adiar::zdd_singleton(0);
  } else {
    timestamp_t before_paths = get_timestamp();
    paths = only_closed ? constraint_closed(N) : constraint_transition(N,0);
    for (int t = 1; t < N*N-1; t++) {
      paths &= constraint_transition(N,t);
      largest_nodes = std::max(largest_nodes, zdd_nodecount(paths));
    }
    timestamp_t after_paths = get_timestamp();

    std::cout << "|  | Transition intersection: " << std::endl
              << "|  |  " << duration_of(before_paths, after_paths) << " s" << std::endl;

    timestamp_t before_only_once = get_timestamp();
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        if (only_closed && is_closed_square(i,j)) { continue; }

        paths &= constraint_exactly_once(N,i,j);
        largest_nodes = std::max(largest_nodes, zdd_nodecount(paths));
      }
    }
    timestamp_t after_only_once = get_timestamp();

    std::cout << "|  | Exactly once per constraint: " << std::endl
              << "|  |  " << duration_of(before_only_once, after_only_once) << " s" << std::endl;
  }

  timestamp_t before_count = get_timestamp();
  uint64_t solutions = zdd_size(paths);
  timestamp_t after_count = get_timestamp();

  std::cout << "|  | Counting elements: " << std::endl
            << "|  |  " << duration_of(before_count, after_count) << " s" << std::endl
            << "|" << std::endl
            << "| Solutions: " << solutions << std::endl
            << "|" << std::endl
            << "| Memory use:" << std::endl
            << "|  | Largest ZDD: " << largest_nodes << " nodes" << std::endl
            << "|  | Final ZDD: " << zdd_nodecount(paths) << " nodes" << std::endl;

  // ===== ADIAR =====
  // Print statistics, if compiled with those flags.
  adiar::statistics_print();

  // Close all of Adiar down again
  adiar::adiar_deinit();

  // Return 'all good'
  bool correct_result = only_closed
    ? (N < 10 ? solutions == expected_closed[N] : true)
    : (N < 9 ? solutions == expected_all[N] : true);

  exit(correct_result ? 0 : 1);
}
