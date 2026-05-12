#ifndef ADIAR_INTERNAL_ALGORITHMS_DOT_H
#define ADIAR_INTERNAL_ALGORITHMS_DOT_H

#include <fstream>
#include <string.h>

#include <adiar/internal/dd.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_ifstream.h>
#include <adiar/internal/io/node_ifstream.h>

namespace adiar::internal
{
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief            Output a container of nodes to a given output stream.
  ///
  /// \tparam File      Type of the node container.
  ///
  /// \param nodes      Container with nodes which can be read with a
  ///                   `node_ifstream<>`.
  ///
  /// \param out        Output stream.
  ///
  /// \param include_id Whether the DOT output ought to include the id. This is only relevant for
  ///                   debugging purposes.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename File>
  inline void
  __print_dot(const File& nodes, std::ostream& out, bool include_id)
  {
    out << "digraph DD {"
        << "\n";

    node_ifstream<> ns(nodes);

    if (nodes->is_terminal()) {
      const bool value = ns.pull().value();

      out << "\tn" << ptr_uint64(value)._raw << " [shape=box, label=<"
          << (value ? File::true_print : File::false_print) << ">];"
          << "\n";
    } else {
      out << "\t// Nodes"
          << "\n";
      out << "\tnode [shape=" << (include_id ? "box" : "circle") << "];"
          << "\n";

      while (ns.can_pull()) {
        const node n = ns.pull();

        out << "\tn" << n.uid()._raw << " [label=<x<SUB>" << n.level();
        if (include_id) { out << ", " << n.id(); }
        out << "</SUB>>, style=rounded];\n";
      }

      out << "\tnode [shape=box];"
          << "\n";
      out << "\tn" << ptr_uint64(false)._raw << " [label=<" << File::false_print << ">];"
          << "\n";
      out << "\tn" << ptr_uint64(true)._raw << " [label=<" << File::true_print << ">];"
          << "\n";

      out << std::endl
          << "\t// Arcs"
          << "\n";

      ns.reset();
      while (ns.can_pull()) {
        const node node = ns.pull();

        out << "\tn" << node.uid()._raw << " -> "
            << "n" << node.low()._raw << " [style=dashed];"
            << "\n";
        out << "\tn" << node.uid()._raw << " -> "
            << "n" << node.high()._raw << " [style=solid];"
            << "\n";
      }

      out << "\n"
          << "\t// Ranks"
          << "\n";

      ns.reset();
      out << "\t{ rank=same; "
          << "n" << ns.pull().uid()._raw << " }"
          << "\n";

      while (ns.can_pull()) {
        const node current_node = ns.pull();

        out << "\t{ rank=same; "
            << "n" << current_node.uid()._raw << " ";

        while (ns.can_pull() && current_node.level() == ns.peek().level()) {
          out << "n" << ns.pull().uid()._raw << " ";
        }
        out << "}"
            << "\n";
      }
    }
    out << "}" << std::endl;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a container of nodes to a given file name
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  inline void
  __print_dot(const dd_t& dd, const std::string& filename, bool include_id)
  {
    // Create output stream for file
    std::ofstream out;
    out.open(filename);

    // Print to output
    __print_dot(dd, out, include_id);

    // Close
    out.close();
  }

  // TODO: print_dot(const shared_levelized_file<node>& nodes, ...)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a decision diagram to a given output stream.
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  void
  print_dot(const dd_t& dd, std::ostream& out, bool include_id)
  {
    __print_dot(dd, out, include_id);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Output a decision diagram to a given file
  //////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  void
  print_dot(const dd_t& dd, const std::string& filename, bool include_id)
  {
    __print_dot(dd, filename, include_id);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Print dot file of an intermediate output of arcs.
  //////////////////////////////////////////////////////////////////////////////
  inline void
  print_dot(const shared_levelized_file<arc>& arcs, const std::string& filename)
  {
    std::ofstream out;
    out.open(filename);

    out << "digraph BDD {" << std::endl;

    out << "\t// Node Arcs" << std::endl;

    arc_ifstream<> as(arcs);
    while (as.can_pull_internal()) {
      const arc a = as.pull_internal();
      out << "\t"
          << "n" << a.target().level() << "_" << a.target().id() << " -> "
          << "n" << a.source().level() << "_" << a.source().id()
          << " [style=" << (a.out_idx() ? "solid" : "dashed") << ", color=blue];" << std::endl;
    }

    out << std::endl << "\t// Sink Arcs" << std::endl;

    out << "\ts0 [shape=box, label=\"0\"];" << std::endl;
    out << "\ts1 [shape=box, label=\"1\"];" << std::endl;

    while (as.can_pull_terminal()) {
      const arc a = as.pull_terminal();
      out << "\t"
          << "n" << a.source().level() << "_" << a.source().id() << " -> "
          << "s" << a.target().value() << " [style=" << (a.out_idx() ? "solid" : "dashed")
          << ", color=red];" << std::endl;
    }

    out << "\t{ rank=min; s0 s1 }" << std::endl << "}" << std::endl;
    out.close();
  }

  // TODO: generalize for non-sharing arc files and std::ostream
}

#endif // ADIAR_INTERNAL_ALGORITHMS_DOT_H
