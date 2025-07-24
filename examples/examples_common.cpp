#include <vector>
#include <cstdint>
#include <getopt.h>
#include <chrono>
#include <ratio>

/* A few chrono wrappers to improve readability of the code below */
typedef std::chrono::high_resolution_clock::time_point timestamp_t;

inline timestamp_t get_timestamp() {
  return std::chrono::high_resolution_clock::now();
}

inline double duration_of(timestamp_t &before, timestamp_t &after) {
  return std::chrono::duration<double, std::ratio<1,1> /* seconds */>(after - before).count();
}

// Adiar Imports
#include <adiar/adiar.h>

// Command-line arguments
int N = -1;
size_t M = 0;

bool init_cl_arguments(int argc, char* argv[])
{
  bool exit = false;
  int c;

  opterr = 0; // Squelch errors of "weird" command-line arguments

  while ((c = getopt(argc, argv, "N:M:h")) != -1) {
    try {
      switch(c) {
      case 'N':
        N = std::stoi(optarg);
        continue;

      case 'M':
        M = std::stoi(optarg);
        if (M == 0) {
          std::cout << "  Must specify positive amount of memory for Adiar (-M)" << std::endl;
        }

        continue;

      case 'h':
        std::cout << "Usage:  -flag    [mandatory]  Description" << std::endl
                  << std::endl
                  << "        -h       [ ]          Print this information" << std::endl
                  << "        -N SIZE  [x]          Specify the size of problem" << std::endl
                  << "        -M MiB   [x]          Specify the amount of memory (MiB) to be dedicated to Adiar" << std::endl;
        return true;
      }
    } catch (std::invalid_argument const &ex) {
      std::cout << "Invalid number: " << argv[1] << std::endl;
      exit = true;
    } catch (std::out_of_range const &ex) {
      std::cout << "Number out of range: " << argv[1] << std::endl;
      exit = true;
    }
  }

  if (N == -1) {
    std::cout << "  Must specify instance size (-N)" << std::endl;
    exit = true;
  }

  if (M == 0) {
    std::cout << "  Must specify MiB of memory for Adiar (-M)" << std::endl;
    exit = true;
  }

  optind = 0; // Reset getopt, such that it can be used again outside
  return exit;
}



