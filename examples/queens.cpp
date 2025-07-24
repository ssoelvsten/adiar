#include "examples_common.cpp"

/*******************************************************************************
 * We base our example for N-Queens on the procedure as described in the paper
 * "Parallel Disk-Based Computation for Large, Monolithic Binary Decision
 * Diagrams" by Daniel Kunkle, Vlad Slavici, and Gene Cooperman.
 *
 * We'd find it interesting to output the size of the largest BDD and the final
 * BDD.
 */
size_t largest_nodes = 0;

/*******************************************************************************
 *                             Variable ordering
 *
 * To solve the N-Queens problem, we have to first choose some encoding of the
 * problem. Well stay with the simple row-by-row ordering of variables for now.
 * That is, we represent the position (i,j) on the N x N board board as the
 * variable with label:
 *
 *                                 N*i + j.
 */
inline typename adiar::bdd::label_type label_of_position(uint64_t N, uint64_t i, uint64_t j)
{
  return (N * i) + j;
}

/*******************************************************************************
 *
 * Let us first restrict our attention to the base case of expressing the state
 * of a single field (i,j). We need to express that a single queen is placed
 * here, and that it is in no conflict with any other queens on the board;
 * queens on the same row, column or diagonals.
 *        __________
 *       | \ | /    |
 *       |  \|/     |
 *       |---X------|
 *       |  /|\     |
 *       |_/_|_\____|
 *
 * This essentially is the formula x_ij /\ !is_threatened(i,j), where
 * is_threatened(i,j) is true if one or more queens are placed on conflicting
 * positions.
 *
 * We could construct this with the builders of Adiar, but we can do even better
 * than that! Since the resulting (reduced) BDD is very well structured, we can
 * explicitly construct it. All BDDs are stored on disk bottom-up, so we'll have
 * to start at the bottom rightmost and work ourselves backwards. We can skip
 * all labels for positions that are not in conflict or are not (i,j). All the
 * others, we have to check that they are not true, and for (i,j) that it is.
 *
 *            (k1,l1)
 *             /   \
 *       (k2,l2)   F
 *        /   \
 *        |   F
 *        .
 *        |
 *      (i,j)
 *       / \
 *      T  |
 *         .
 *         |
 *     (k3n,l3n)
 *       /   \
 *       T   F
 *
 * Since we construct it explicitly, then the work we do in the base case goes
 * down to only O(N) time and O(N/B) I/Os rather than O(sort(N)) in both time
 * and I/Os. One pretty much cannot do this base case faster.
 */
adiar::bdd n_queens_S(int i, int j)
{
  adiar::bdd_builder builder;

  int row = N - 1;
  adiar::bdd_ptr next = builder.add_node(true);

  do {
    int row_diff = std::max(row,i) - std::min(row,i);

    if (row_diff == 0) {
      // On row of the queen in question
      int column = N - 1;
      do {
        typename adiar::bdd::label_type label = label_of_position(N, row, column);

        // If (row, column) == (i,j), then the chain goes through high because
        // then we need to check the queen actually is placed here.
        next = column == j
          ? builder.add_node(label, false, next)
          : builder.add_node(label, next, false);
      } while (column-- > 0);
    } else {
      // On another row
      if (j + row_diff < N) {
        // Diagonal to the right is within bounds
        next = builder.add_node(label_of_position(N, row, j + row_diff), next, false);
      }

      // Column
      next = builder.add_node(label_of_position(N, row, j), next, false);

      if (row_diff <= j) {
        // Diagonal to the left is within bounds
        next = builder.add_node(label_of_position(N, row, j - row_diff), next, false);
      }
    }
  } while (row-- > 0);

  adiar::bdd res = builder.build();

  largest_nodes = std::max(largest_nodes, bdd_nodecount(res));
  return res;
}

/*******************************************************************************
 * Now that we have a formula n_queens_S that is true only when the queen is
 * set, then we can combine them with an OR to ensure there is at-least-one
 * queen on the row. Since n_queens_S is also only true when said queen has no
 * conflicts, this also immediately contains all the at-most-one constraints on
 * these queens.
 *
 * We could do the below in two ways:
 *
 * - Recursively split the row in half until we reach the base case of
 *   n_queens_S. This will minimise the number of Apply's that we will make.
 *
 * - Iteratively combine them similar to a list.fold in functional programming
 *   languages. This will minimise the number of BDDs that are "active" at any
 *   given time, since we only need to persist the input and output of each
 *   iteration.
 *
 * For Adiar to be able to achieve optimality on disk, it sacrifices the
 * possibility of a hash-table to instantiate the entire forest of all currently
 * active BDDs. In other words, each BDD is completely separate and no memory is
 * saved, if there is a major overlap. So, we will choose to do it iteratively.
 */
adiar::bdd n_queens_R(int i)
{
  adiar::bdd out = n_queens_S(i, 0);

  for (int j = 1; j < N; j++) {
    out |= n_queens_S(i, j);
    largest_nodes = std::max(largest_nodes, bdd_nodecount(out));
  }
  return out;
}

/*******************************************************************************
 * Now that we have each entire row done, we only need to combine them. Here we
 * again iterate over all rows to combine them one-by-one. One can probably
 * remove the code duplication that we now introduce.
 */
adiar::bdd n_queens_B()
{
  if (N == 1) {
    return n_queens_S(0, 0);
  }

  adiar::bdd out = n_queens_R(0);

  for (int i = 1; i < N; i++) {
    out &= n_queens_R(i);
    largest_nodes = std::max(largest_nodes, bdd_nodecount(out));
  }
  return out;
}

/*******************************************************************************
 * So now that we have a single BDD that is only true, when we have placed a
 * queen on each row and none in conflict, it is only true, when the assignment
 * is a solution to the N-Queens problem. So, now we can merely count the number
 * of assignments that reach a true sink.
 */
uint64_t n_queens_count(const adiar::bdd &board)
{
  return adiar::bdd_satcount(board);
}

/*******************************************************************************
 *
 * Based on: github.com/MartinFaartoft/n-queens-bdd/blob/master/report.tex
 *
 * If we want to list all the assignments, we have to do something more than
 * merely count the number of satisfying assignments, as we did above. We could
 * then iterate over all possible assignments and use the BDD to prune our
 * search tree.
 *
 * We again construct the combined BDD of the at-most-one restrictions with the
 * at-least-one restriction for the rows. Then, starting at the left-most column
 * we may attempt an assignment in each spot, and recurse. Recursion can be
 * stopped early in two cases:
 *
 *   - If the given BDD already is trivially false. We have placed a queen, such
 *     that it conflicts with another.
 *
 *   - If the number of unique paths in the restricted BDD is exactly one. Then
 *     we are forced to place the remaining queens.
 *
 * Since we want to back track in our choices, we may keep BDDs for each column.
 * This is easily achieved by writing it as a recursive procedure. One should
 * notice though, that this will result in multiple BDDs concurrently being
 * instantiated in memory or on disk at the same time.
 *
 * In fact, let us keep track of that!
 */
uint64_t deepest_column = 0;

/* Let us isolate/hide the printing function, to keep the rest a little more
 * clean. */
void n_queens_print_solution(std::vector<uint64_t>& assignment)
{
  std::cout << "|  |  | ";
  for (uint64_t r : assignment) {
    std::cout << r << " ";
  }
  std::cout << std::endl;
}

/* At this point, we now also need to convert an assignment back into a position
 * on the board. So, we'll also need the following two small functions. */
inline uint64_t i_of_label(uint64_t N, typename adiar::bdd::label_type label)
{
  return label / N;
}

inline uint64_t j_of_label(uint64_t N, typename adiar::bdd::label_type label)
{
  return label % N;
}

/* At which point we can then implement the recursive procedure that takes care
 * of a row and possibly recurses. For a sanity check, we also count the number
 * of solutions we list. */
uint64_t n_queens_list(uint64_t N, uint64_t column,
                       std::vector<uint64_t>& partial_assignment,
                       const adiar::bdd& constraints)
{
  if (adiar::bdd_isfalse(constraints)) {
    return 0;
  }
  deepest_column = std::max(deepest_column, column);

  uint64_t solutions = 0;

  for (uint64_t row_q = 0; row_q < N; row_q++) {
    partial_assignment.push_back(row_q);

    // Construct the assignment for this entire column
    std::vector<adiar::pair<adiar::bdd::label_type, bool>> column_assignment;

    for (uint64_t row = 0; row < N; row++) {
      column_assignment.push_back({label_of_position(N, row, column), row == row_q});
    }

    adiar::bdd restricted_constraints = adiar::bdd_restrict(constraints,
                                                            column_assignment.begin(),
                                                            column_assignment.end());

    if (adiar::bdd_pathcount(restricted_constraints) == 1) {
      solutions += 1;

      // Add dummys in 'partial_assignment'
      while (partial_assignment.size() < N) {
        partial_assignment.push_back(-1);
      }

      // Obtain the lexicographically minimal true assignment. Well, only one
      // exists, so we get the only one left.
      adiar::bdd_satmin(restricted_constraints, [&N, &partial_assignment](adiar::pair<adiar::bdd::label_type, bool> xv) {
        // Skip all empty (false) locations
        if (!xv.second) { return; }

        partial_assignment.at(j_of_label(N, xv.first)) = i_of_label(N, xv.first);
      });

      n_queens_print_solution(partial_assignment);

      for (uint64_t c = N-1; c > column; c--) {
        partial_assignment.pop_back();
      }
    } else if (adiar::bdd_istrue(restricted_constraints)) {
      n_queens_print_solution(partial_assignment);
      solutions += 1;
    } else {
      solutions += n_queens_list(N, column+1, partial_assignment, restricted_constraints);
    }
    partial_assignment.pop_back();
  }

  return solutions;
}

uint64_t n_queens_list(uint64_t N, const adiar::bdd& board)
{
  std::cout << "|  | solutions:" << std::endl;

  if (N == 1) {
    /* To make the recursive function work for N = 1 we would have to have the
     * adiar::count_paths check above at the beginning. That would in all other
     * cases merely result in an unecessary counting of paths at the very
     * start. */
    std::vector<uint64_t> assignment { 0 };
    n_queens_print_solution(assignment);

    return 1;
  }

  std::vector<uint64_t> partial_assignment { };
  partial_assignment.reserve(N);

  return n_queens_list(N, 0, partial_assignment, board);
}

// expected number taken from:
//  https://en.wikipedia.org/wiki/Eight_queens_puzzle#Counting_solutions
uint64_t expected_result[28] = {
  0,
  1,
  0,
  0,
  2,
  10,
  4,
  40,
  92,
  352,
  724,
  2680,
  14200,
  73712,
  365596,
  2279184,
  14772512,
  95815104,
  666090624,
  4968057848,
  39029188884,
  314666222712,
  2691008701644,
  24233937684440,
  227514171973736,
  2207893435808352,
  22317699616364044,
  234907967154122528
};

/* Finally, to run the above we need to initialize the underlying TPIE library
 * first, from which point all the file_streams, priority queues and sorters
 * used are at our disposal.
 */
int main(int argc, char* argv[])
{
  // ===== Parse argument =====
  {
    bool should_error_exit = init_cl_arguments(argc, argv);
    if (!should_error_exit && N == 0) {
      std::cout << "  Must specify positive number for N" << std::endl;
      should_error_exit = true;
    }

    if (should_error_exit) { exit(1); }
  }

  // ===== ADIAR =====
  // Initialize

  adiar::adiar_init(M*1024*1024);
  std::cout << "| Initialized Adiar with " << M << " MiB of memory"  << std::endl << "|" << std::endl;

  bool correct_result = true;

  // ===== N Queens =====
  std::cout << "| " << N << "-Queens : Board construction"  << std::endl;
  timestamp_t before_board = get_timestamp();
  adiar::bdd board = n_queens_B();
  timestamp_t after_board = get_timestamp();

  std::cout << "|  | time: " << duration_of(before_board, after_board) << " s" << std::endl;
  std::cout << "|  | largest BDD  : " << largest_nodes << " nodes" << std::endl;
  std::cout << "|  | final size: " << bdd_nodecount(board) << " nodes"<< std::endl;

  // Run counting example
  timestamp_t before_count = get_timestamp();
  uint64_t solutions = n_queens_count(board);
  timestamp_t after_count = get_timestamp();

  std::cout << "| " << N << "-Queens : Counting assignments"  << std::endl;
  std::cout << "|  | number of solutions: " << solutions << std::endl;
  std::cout << "|  | time: " << duration_of(before_count, after_count) << " s" << std::endl;

  correct_result = solutions == expected_result[N];

  // Run enumeration example (for reasonably small N)
  if (N <= 8) {
    std::cout << "| " << N << "-Queens (Pruning search)"  << std::endl;
    timestamp_t before_list = get_timestamp();
    uint64_t listed_solutions = n_queens_list(N, board);
    timestamp_t after_list = get_timestamp();

    correct_result = correct_result && listed_solutions == expected_result[N];

    std::cout << "|  | number of solutions: " << listed_solutions << std::endl;
    std::cout << "|  | deepest recursion: " << deepest_column << " columns" << std::endl;
    std::cout << "|  | time: " << duration_of(before_list, after_list) << " s" << std::endl;
  }

  // ===== ADIAR =====
  // Print statistics, if compiled with those flags.
  adiar::statistics_print();

  // Close all of Adiar down again
  adiar::adiar_deinit();

  // Return 'all good'
  exit(correct_result ? 0 : 1);
}
