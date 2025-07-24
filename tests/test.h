#ifndef ADIAR_TEST_H
#define ADIAR_TEST_H

#include <bandit/bandit.h>

#include <tpie/tpie.h>

using namespace snowhouse;
using namespace bandit;

#include <adiar/adiar.h>

////////////////////////////////////////////////////////////////////////////////
// To improve the relationship between the tests and the algorithms, we will not
// reverse the order in which we read the output.
//
// We also do a few little hacks, to read unreduced output from the file_union
// class.
#include <adiar/internal/io/arc_ifstream.h>
#include <adiar/internal/io/arc_ofstream.h>
#include <adiar/internal/io/node_ifstream.h>
#include <adiar/internal/io/node_ofstream.h>

using namespace adiar;
using namespace adiar::internal;

using level_info_test_ifstream = level_info_ifstream<true>;

class node_test_ifstream : public node_ifstream<true>
{
public:
  node_test_ifstream(const shared_levelized_file<dd::node_type>& f)
    : node_ifstream<true>(f)
  {}

  node_test_ifstream(const bdd& f)
    : node_ifstream<true>(f)
  {}

  node_test_ifstream(const __bdd& f)
    : node_ifstream<true>(f.get<__bdd::shared_node_file_type>(), f._negate)
  {}

  node_test_ifstream(const zdd& f)
    : node_ifstream<true>(f)
  {}

  node_test_ifstream(const __zdd& f)
    : node_ifstream<true>(f.get<__zdd::shared_node_file_type>(), f._negate)
  {}
};

class arc_test_ifstream : public arc_ifstream<true>
{
public:
  arc_test_ifstream(const shared_levelized_file<arc>& f)
    : arc_ifstream<true>(f)
  {}

  arc_test_ifstream(const __bdd& bdd)
    : arc_ifstream<true>(bdd.get<__bdd::shared_arc_file_type>())
  {}

  arc_test_ifstream(const __zdd& zdd)
    : arc_ifstream<true>(zdd.get<__zdd::shared_arc_file_type>())
  {}
};

////////////////////////////////////////////////////////////////////////////////
// To improve the error messages
//
// TODO: move into '<<'/'toString' methods in each respective data type.
namespace snowhouse
{
  std::string
  string_of_adiar_ptr(adiar::internal::ptr_uint64 p)
  {
    std::stringstream stream;
    if (p.is_nil()) {
      stream << "nil";
    } else if (p.is_terminal()) {
      stream << p.value();
    } else { // p.is_node()
      stream << "(x" << p.label() << ", " << p.id() << ")";
    }

    if (p.is_flagged()) { stream << "'"; }

    return stream.str();
  }

  std::string
  string_of_adiar_uid(adiar::internal::uid_uint64 u)
  {
    std::stringstream stream;
    if (u.is_terminal()) {
      stream << u.value();
    } else { // u.is_node()
      stream << "(x" << u.label() << ", " << u.id() << ")";
    }
    return stream.str();
  }

  template <>
  struct Stringizer<arc>
  {
    static std::string
    ToString(const adiar::internal::arc& a)
    {
      std::stringstream stream;
      stream << "arc: " << string_of_adiar_ptr(a.source()) << " " << (a.out_idx() ? "--->" : "- ->")
             << " " << string_of_adiar_ptr(a.target());
      return stream.str();
    }
  };

  template <>
  struct Stringizer<node>
  {
    static std::string
    ToString(const adiar::internal::node& n)
    {
      std::stringstream stream;
      if (n.is_terminal()) {
        stream << "node: " << n.value();
      } else {
        stream << "node: (" << string_of_adiar_uid(n.uid()) << ", " << string_of_adiar_ptr(n.low())
               << ", " << string_of_adiar_ptr(n.high()) << ")";
      }
      return stream.str();
    }
  };

  template <>
  struct Stringizer<level_info>
  {
    static std::string
    ToString(const adiar::internal::level_info& m)
    {
      std::stringstream stream;
      stream << "level_info: ("
             << "x" << m.label() << " (" << m.level() << ")"
             << ", #nodes = " << m.width() << ")";
      return stream.str();
    }
  };
}

////////////////////////////////////////////////////////////////////////////////
// For unit testing levelized_file<int> (local stack and shared heap variants).
namespace adiar::internal
{
  template <>
  struct file_traits<int>
  {
    static constexpr size_t files = 2u;

    struct stats
    {
      // Value for testing meta information
      int meta_value = 0;
    };
  };
}

////////////////////////////////////////////////////////////////////////////////
// Main
int
main(int argc, char* argv[])
{
#ifdef NDEBUG
  std::cerr << "Warning: Internal assertions are not enabled!" << std::endl << std::endl;
  ;

  std::cerr << "Warning: Some tests may fail due to ties in ordering" << std::endl << std::endl;
#endif

#ifndef ADIAR_TEST_NO_INIT
  // Initialize Adiar (and TPIE)
  adiar_init(1024 * 1024 * 1024 /* 1 GiB */);
#endif

  // Run tests
  auto bandit_ret = bandit::run(argc, argv);

  // Output statistics
#ifdef ADIAR_TEST_PRINT_STATS
  adiar::statistics_print();
#endif

#ifndef ADIAR_TEST_NO_INIT
  // Close all of Adiar (and TPIE) down again
  adiar_deinit();
#endif

  if (bandit_ret != 0) exit(bandit_ret);
  exit(0);
}

#endif // ADIAR_TEST_H
