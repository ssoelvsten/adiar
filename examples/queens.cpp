#include <cstdint>
#include <iterator>
#include <ratio>
#include <vector>

// Timing
#include <chrono>

/* A few chrono wrappers to improve readability of the code below */
typedef std::chrono::high_resolution_clock::time_point timestamp_t;

inline timestamp_t get_timestamp() {
  return std::chrono::high_resolution_clock::now();
}

inline double duration_of(timestamp_t &before, timestamp_t &after) {
  return std::chrono::duration<double, std::ratio<1,1> /* seconds */>(after - before).count();
}

// Command-line arguments
#include <getopt.h>

int    N = -1;
size_t M =  0;

////////////////////////////////////////////////////////////////////////////////
// For comments and explanation, please see 'docs/tutorial/queens.md'.
////////////////////////////////////////////////////////////////////////////////
#include <adiar/adiar.h>

size_t largest_nodes = 0;

/// [label]
inline typename adiar::bdd::label_type label_of_position(uint64_t i, uint64_t j)
{
  return (N * i) + j;
}
/// [label]

/// [queens_S]
adiar::bdd queens_S(int i, int j)
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
        typename adiar::bdd::label_type label = label_of_position(row, column);

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
        next = builder.add_node(label_of_position(row, j + row_diff), next, false);
      }

      // Column
      next = builder.add_node(label_of_position(row, j), next, false);

      if (row_diff <= j) {
        // Diagonal to the left is within bounds
        next = builder.add_node(label_of_position(row, j - row_diff), next, false);
      }
    }
  } while (row-- > 0);

  adiar::bdd res = builder.build();

  largest_nodes = std::max(largest_nodes, bdd_nodecount(res));
  return res;
}
/// [queens_S]

/// [queens_R]
adiar::bdd queens_R(int i)
{
  adiar::bdd out = queens_S(i, 0);

  for (int j = 1; j < N; j++) {
    out |= queens_S(i, j);
    largest_nodes = std::max(largest_nodes, bdd_nodecount(out));
  }
  return out;
}
/// [queens_R]

/// [queens_B]
adiar::bdd queens_B()
{
  if (N == 1) {
    return queens_S(0, 0);
  }

  adiar::bdd out = queens_R(0);

  for (int i = 1; i < N; i++) {
    out &= queens_R(i);
    largest_nodes = std::max(largest_nodes, bdd_nodecount(out));
  }
  return out;
}
/// [queens_B]

/// [queens_count]
uint64_t queens_count(const adiar::bdd &board)
{
  return adiar::bdd_satcount(board);
}
/// [queens_count]

uint64_t deepest_row = 0;

/// [print_assignment]
void print_assignment(std::vector<uint64_t>& assignment)
{
  std::cout << "|  |  | ";
  for (char i = 0; i < N; ++i) {
    const char col = 'A' + i;
    const char row = '1' + (assignment.at(i) % N);
    std::cout << row << col << " ";
  }
  std::cout << "\n";
}
/// [print_assignment]

/// [queens_list_rec]
uint64_t queens_list_rec(uint64_t N, uint64_t row,
                         std::vector<uint64_t>& partial_assignment,
                         const adiar::bdd& constraints)
{
  if (adiar::bdd_isfalse(constraints)) {
    return 0;
  }
  deepest_row = std::max(deepest_row, row);

  uint64_t solutions = 0;

  for (uint64_t col_q = 0; col_q < N; col_q++) {
    partial_assignment.push_back(col_q);

    // Construct the assignment for this entire row
    std::vector<adiar::pair<adiar::bdd::label_type, bool>> column_assignment;

    for (uint64_t col = 0; col < N; col++) {
      column_assignment.push_back({label_of_position(row, col), col == col_q});
    }

    adiar::bdd restricted_constraints = adiar::bdd_restrict(constraints,
                                                            column_assignment.begin(),
                                                            column_assignment.end());

    if (adiar::bdd_pathcount(restricted_constraints) == 1) {
      solutions += 1;

      // Obtain the lexicographically minimal true assignment. Well, only one
      // exists, so we get the only one left.
      adiar::bdd_satmin(restricted_constraints,
                        [&partial_assignment](adiar::pair<adiar::bdd::label_type, bool> xv) {
                          // Skip all empty cells, i.e. assignments to `false`.
                          if (!xv.second) { return; }
                          // Push non-empty cells, i.e. assignments to `true`. Since `bdd_satmin`
                          // provides the assignment (ascendingly) according to the variable
                          // ordering, then we can merely push to `partial_assignment`
                          partial_assignment.push_back(xv.first);
                        });
      print_assignment(partial_assignment);

      // Forget the forced assignment again
      for (uint64_t r = N-1; r > row; r--) {
        partial_assignment.pop_back();
      }
    } else if (adiar::bdd_istrue(restricted_constraints)) {
      print_assignment(partial_assignment);
      solutions += 1;
    } else {
      solutions += queens_list_rec(N, row+1, partial_assignment, restricted_constraints);
    }
    partial_assignment.pop_back();
  }

  return solutions;
}
/// [queens_list_rec]

/// [queens_list]
uint64_t queens_list(uint64_t N, const adiar::bdd& board)
{
  std::cout << "|  | solutions:" << std::endl;

  if (N == 1) {
    // To make the recursive function work for N = 1 we would have to have the
    // adiar::count_paths check above at the beginning. That would in all other
    // cases merely result in an unecessary counting of paths at the very start.
    std::vector<uint64_t> assignment { 0 };
    print_assignment(assignment);

    return 1;
  }

  std::vector<uint64_t> partial_assignment { };
  partial_assignment.reserve(N);

  return queens_list_rec(N, 0, partial_assignment, board);
}
/// [queens_list]

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

int main(int argc, char* argv[])
{
  // ===== Parse argument =====
  {
    int c;

    opterr = 0; // Squelch errors of "weird" command-line arguments

    while ((c = getopt(argc, argv, "N:M:")) != -1) {
      try {
        switch(c) {
        case 'N':
          N = std::stoi(optarg);
          continue;

        case 'M':
          M = std::stoi(optarg);
          if (M == 0) {
            std::cerr << "Must specify positive amount of memory for Adiar (-M)" << std::endl;
          }
          continue;
        }
      } catch (std::invalid_argument const &ex) {
        std::cerr << "Invalid number: " << argv[1] << std::endl;
        return -1;
      } catch (std::out_of_range const &ex) {
        std::cerr << "Number out of range: " << argv[1] << std::endl;
        return -1;
      }
    }

    if (M == 0) {
      std::cerr << "Must specify MiB of memory for Adiar (-M)" << std::endl;
      return -1;
    }

    if (N == -1) {
      N = 8;
    }
  }


  // ===== ADIAR =====
  // Initialize

  adiar::adiar_init(M*1024*1024);
  std::cout << "| Initialized Adiar with " << M << " MiB of memory"  << "\n" << "|" << "\n";

  bool correct_result = true;

  // ===== N Queens =====
  std::cout << "| " << N << "-Queens : Board construction"  << "\n";
  timestamp_t before_board = get_timestamp();
  adiar::bdd board = queens_B();
  timestamp_t after_board = get_timestamp();

  std::cout << "|  | time: " << duration_of(before_board, after_board) << " s" << "\n";
  std::cout << "|  | largest BDD  : " << largest_nodes << " nodes" << "\n";
  std::cout << "|  | final size: " << bdd_nodecount(board) << " nodes"<< "\n";

  // Run counting example
  timestamp_t before_count = get_timestamp();
  uint64_t solutions = queens_count(board);
  timestamp_t after_count = get_timestamp();

  std::cout << "| " << N << "-Queens : Counting assignments"  << "\n";
  std::cout << "|  | number of solutions: " << solutions << "\n";
  std::cout << "|  | time: " << duration_of(before_count, after_count) << " s" << "\n";

  correct_result = solutions == expected_result[N];

  // Run enumeration example (for N small enough for each coordinate to use two characters only)
  if (solutions > 0 && N < 10) {
    std::cout << "| " << N << "-Queens : Pruning search"  << "\n";
    timestamp_t before_list = get_timestamp();
    uint64_t listed_solutions = queens_list(N, board);
    timestamp_t after_list = get_timestamp();

    correct_result = correct_result && listed_solutions == expected_result[N];

    std::cout << "|  | number of solutions: " << listed_solutions << "\n";
    std::cout << "|  | deepest recursion: "
              << deepest_row << (deepest_row == 1 ? " row" : " rows") << "\n";
    std::cout << "|  | time: " << duration_of(before_list, after_list) << " s" << "\n";
  }

  // ===== ADIAR =====
  // Print statistics, if compiled with those flags.
  std::cout << "\n";
  adiar::statistics_print();

  // Close all of Adiar down again
  adiar::adiar_deinit();

  // Return 'all good'
  return !correct_result;
}
