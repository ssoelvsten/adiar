// TPIE Imports
#include <tpie/tpie.h>

// ADIAR Imports
#include <adiar/adiar.h>

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

  {
    // ===== Your code starts here =====

    // =====  Your code ends here  =====
  }

  adiar::statistics_print();

  adiar::adiar_deinit();
  return 0;
}
