#ifndef ADIAR_INTERNAL_IO_NODE_OFSTREAM_H
#define ADIAR_INTERNAL_IO_NODE_OFSTREAM_H

#include <adiar/exception.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/levelized_ofstream.h>
#include <adiar/internal/io/node_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Writer for nodes, hiding derivation of all meta information and applying sanity checks
  ///        on the validity of the input.
  ///
  /// \see   shared_levelized_file<node>
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class node_ofstream : public levelized_ofstream<node>
  {
  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Construct a dummy node, that is invalid within a decision diagram and hence easy to
    /// recognise.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node
    dummy()
    {
      // Notice, this goes around any of the consistency checks of 'node'!
      return node(node::uid_type(0, 0), node::pointer_type::nil(), node::pointer_type::nil());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Buffer of latest pushed element, such that one can compare with it.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node _latest_node = dummy();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Number of nodes pushed to the current level.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t _level_size = 0u;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Variables for 1-level cut
    //
    // We will count the following globally
    // - The number of arcs pushed at the very bottom
    // - The number of (long) arcs that cross at least one pushed level
    //
    // While for each level we can safely count
    // - The number of (short) arcs from a single level to the next
    ////////////////////////////////////////////////////////////////////////////////////////////////
    size_t _terminals_at_bottom[2] = { 0u, 0u };

    cut::size_type _max_1level_short_internal  = 0u;
    cut::size_type _curr_1level_short_internal = 0u;

    node::pointer_type _long_internal_ptr        = node::pointer_type::nil();
    cut::size_type _number_of_long_internal_arcs = 0u;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to any levelized node file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_ofstream()
      : levelized_ofstream<node>()
    {}

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized node file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_ofstream(levelized_file<node>& nf)
      : levelized_ofstream<node>(nf)
    {
      _file_ptr->sorted    = !levelized_ofstream::has_pushed();
      _file_ptr->indexable = !levelized_ofstream::has_pushed();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized node file.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_ofstream(adiar::shared_ptr<levelized_file<node>> nf)
      : levelized_ofstream<node>(nf)
    {
      _file_ptr->sorted    = !levelized_ofstream::has_pushed();
      _file_ptr->indexable = !levelized_ofstream::has_pushed();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up from the levelized file (if need be).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ~node_ofstream()
    {
      close();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Open a file
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    open(adiar::shared_ptr<levelized_file<node>>& f)
    {
      levelized_ofstream::open(f);

      // Reset all meta-data
      _latest_node = dummy();

      _level_size = 0u;

      _terminals_at_bottom[0] = 0u;
      _terminals_at_bottom[1] = 0u;

      _max_1level_short_internal  = 0u;
      _curr_1level_short_internal = 0u;

      _long_internal_ptr            = node::pointer_type::nil();
      _number_of_long_internal_arcs = 0u;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    is_open() const
    {
      return levelized_ofstream::is_open();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a file (if need be)
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    close()
    {
      if (!is_open()) { return; }

      // Has '.push' been used?
      if (_latest_node != dummy()) {
        // Output level information of the final level
        if (!_latest_node.is_terminal()) {
          unsafe_push(level_info(_latest_node.level(), _level_size));
        }

        _level_size = 0u; // TODO: remove?

        // 1-level cut
        _max_1level_short_internal =
          std::max(_max_1level_short_internal, _curr_1level_short_internal);

        const cut::size_type max_1level_internal_cut =
          _max_1level_short_internal + _number_of_long_internal_arcs;

        _file_ptr->max_1level_cut[cut::Internal] = max_1level_internal_cut;

        const size_t terminals_above_bottom[2] = {
          _file_ptr->number_of_terminals[false] - _terminals_at_bottom[false],
          _file_ptr->number_of_terminals[true] - _terminals_at_bottom[true]
        };

        _file_ptr->max_1level_cut[cut::Internal_False] =
          std::max(max_1level_internal_cut + terminals_above_bottom[false],
                   _file_ptr->number_of_terminals[false]);

        _file_ptr->max_1level_cut[cut::Internal_True] =
          std::max(max_1level_internal_cut + terminals_above_bottom[true],
                   _file_ptr->number_of_terminals[true]);

        _file_ptr->max_1level_cut[cut::All] = std::max(
          max_1level_internal_cut + terminals_above_bottom[false] + terminals_above_bottom[true],
          _file_ptr->number_of_terminals[false] + _file_ptr->number_of_terminals[true]);
      }

      // Run final i-level cut computations
      fixup_ilevel_cuts();

      levelized_ofstream::close();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Write the next node to the file (and check consistency).
    ///
    /// \details Writes the given node to the end of the file and also writes to the level_info file
    ///          if necessary. The given node must have valid children (not checked), no duplicate
    ///          nodes created (not properly checked), and must be topologically prior to any nodes
    ///          already written to the file (checked).
    ///
    /// \throws domain_error If the writer is yet not attached.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    push(const node& n)
    {
      if (!is_open()) {
        throw domain_error("node_ofstream is not yet attached to any levelized_file<node>");
      }

      const bool first_push = _latest_node == dummy();

      adiar_assert(first_push == !has_pushed(),
                   "'push' function has only been designed for use on empty files");

      adiar_assert(!_latest_node.is_terminal(), "Cannot push after having pushed a terminal");

      const bool new_level = !first_push && _latest_node.level() != n.level();

      // -------------------------------------------------------------------------------------------
      // Terminal edge-case
      if (n.is_terminal()) {
        adiar_assert(first_push, "A terminal can only be pushed once and into an empty file");
        adiar_assert(!has_pushed(), "A terminal can only be pushed into an empty file");
        _file_ptr->number_of_terminals[n.uid().value()]++;
      }

      // -------------------------------------------------------------------------------------------
      // Canonicity checks, i.e. *sorted* and *indexable*.
      if (first_push) {
        _file_ptr->indexable &= n.is_terminal() || n.id() == node::max_id;
      } else {
        // Check it is sorted and indexable
        _file_ptr->sorted &= new_level
          || (n.high() < _latest_node.high()
              || (n.high() == _latest_node.high() && n.low() < _latest_node.low()));

        _file_ptr->indexable &=
          new_level ? n.id() == node::max_id : n.uid().id() == _latest_node.id() - 1u;
      }

      // -------------------------------------------------------------------------------------------
      // 1-level cut

      // Commit prior level when starting to push the next one
      if (new_level) {
        // Update level information with the level just finished
        unsafe_push(level_info(_latest_node.level(), _level_size));
        _level_size = 0u;

        // Update 1-level cut information
        _max_1level_short_internal =
          std::max(_max_1level_short_internal, _curr_1level_short_internal);

        _curr_1level_short_internal = 0u;
        _long_internal_ptr          = node::uid_type(_latest_node.level(), node::max_id);
      }

      // Update cut of the current level
      const bool is_pushing_to_bottom = _long_internal_ptr == node::pointer_type::nil();
      if (is_pushing_to_bottom && !n.is_terminal()) {
        _terminals_at_bottom[n.low().value()]++;
        _terminals_at_bottom[n.high().value()]++;
      }

      if (n.low().is_node()) {
        if (n.low() > _long_internal_ptr) {
          _number_of_long_internal_arcs++;
        } else {
          _curr_1level_short_internal++;
        }
      }

      if (n.high().is_node()) {
        if (n.high() > _long_internal_ptr) {
          _number_of_long_internal_arcs++;
        } else {
          _curr_1level_short_internal++;
        }
      }

      // -------------------------------------------------------------------------------------------
      // Write node to file
      _latest_node = n;
      _level_size++;

      unsafe_push(n);
    }

    node_ofstream&
    operator<<(const node& n)
    {
      this->push(n);
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to level information file without any checks.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_push(const level_info& m)
    {
#ifdef ADIAR_STATS
      stats_node_file.push_level += 1;
#endif
      _file_ptr->width = std::max<size_t>(_file_ptr->width, m.width());
      levelized_ofstream::push(m);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the underlying node file without any checks.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_push(const node& n)
    {
#ifdef ADIAR_STATS
      stats_node_file.push_node += 1;
#endif
      if (n.low().is_terminal()) { _file_ptr->number_of_terminals[n.low().value()]++; }
      if (n.high().is_terminal()) { _file_ptr->number_of_terminals[n.high().value()]++; }

      levelized_ofstream::template push<0>(n);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Change the 1-level cut size to the given one.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_set_1level_cut(const cuts_t& o)
    {
      for (size_t ct = 0u; ct < cut::size; ct++) { _file_ptr->max_1level_cut[ct] = o[ct]; }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Change the 1-level cut size to the maximum of the current or the given cuts.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_max_1level_cut(const cuts_t& o)
    {
      max_cut(_file_ptr->max_1level_cut, o);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Increase the current maximum 1-level cut size by the given cut.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_inc_1level_cut(const cuts_t& o)
    {
      inc_cut(_file_ptr->max_1level_cut, o);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overwrite the number of false and true arcs.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_set_number_of_terminals(const size_t number_of_false, const size_t number_of_true)
    {
      _file_ptr->number_of_terminals[false] = number_of_false;
      _file_ptr->number_of_terminals[true]  = number_of_true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overwrite the sorted flag.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_set_sorted(const bool flag_value)
    {
      _file_ptr->sorted = flag_value;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overwrite the indexable flag.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_set_indexable(const bool flag_value)
    {
      _file_ptr->indexable = flag_value;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overwrite all flags related to canonicity.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    unsafe_set_canonical(const bool flag_value)
    {
      unsafe_set_sorted(flag_value);
      unsafe_set_indexable(flag_value);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether anything has been pushed to any of the underlying files.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    has_pushed()
    {
      return levelized_ofstream::has_pushed();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the underlying file is empty.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool
    empty()
    {
      return levelized_ofstream::empty();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Helper function to bound derived 1-level and 2-level cuts with their trivial upper
    ///        bounds (assuming nothing is pushed later).
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    fixup_ilevel_cuts()
    {
      const size_t number_of_nodes = levelized_ofstream::size();
      const size_t number_of_false = _file_ptr->number_of_terminals[false];
      const size_t number_of_true  = _file_ptr->number_of_terminals[true];

      // -------------------------------------------------------------------------------------------
      // Upper bound for any directed cut based on number of internal nodes.
      const cut::size_type max_cut = number_of_nodes < cut::max // overflow?
        ? number_of_nodes + 1
        : cut::max;

      // -------------------------------------------------------------------------------------------
      // Upper bound on just 'all arcs'. This is better than 'max_cut' above, if
      // there are 'number_of_nodes' or more arcs to terminals.
      const bool noa_overflow_safe = number_of_nodes <= cut::max / 2u;
      const size_t number_of_arcs  = 2u * number_of_nodes;

      const cuts_t all_arcs_cut = {
        { noa_overflow_safe ? number_of_arcs - number_of_false - number_of_true : cut::max,
          noa_overflow_safe ? number_of_arcs - number_of_true : cut::max,
          noa_overflow_safe ? number_of_arcs - number_of_false : cut::max,
          noa_overflow_safe ? number_of_arcs : cut::max }
      };

      // -------------------------------------------------------------------------------------------
      // Maximum 1-level cuts
      const bool is_terminal = number_of_false + number_of_true == 1;

      if (is_terminal) {
        _file_ptr->max_1level_cut[cut::Internal]       = 0u;
        _file_ptr->max_1level_cut[cut::Internal_False] = number_of_false;
        _file_ptr->max_1level_cut[cut::Internal_True]  = number_of_true;
        _file_ptr->max_1level_cut[cut::All]            = 1u;
      } else {
        for (size_t ct = 0u; ct < cut::size; ct++) {
          // Use smallest sound upper bound. Since it is not a terminal, then there
          // must be at least one in-going arc to the root.
          _file_ptr->max_1level_cut[ct] = std::max<cut::size_type>(
            1lu,
            std::min<cut::size_type>({ _file_ptr->max_1level_cut[ct], max_cut, all_arcs_cut[ct] }));
        }
      }

      // -------------------------------------------------------------------------------------------
      // Maximum 2-level cut
      const size_t number_of_levels = levelized_ofstream::levels();

      if (is_terminal || number_of_nodes == number_of_levels) {
        for (size_t ct = 0u; ct < cut::size; ct++) {
          _file_ptr->max_2level_cut[ct] = _file_ptr->max_1level_cut[ct];
        }
      } else { // General case
        for (size_t ct = 0u; ct < cut::size; ct++) {
          // Upper bound based on 1-level cut
          const cut::size_type ub_from_1level_cut = _file_ptr->max_1level_cut[ct] < cut::max / 3u
            ? ((_file_ptr->max_1level_cut[cut::Internal] * 3u) / 2u
               + (_file_ptr->max_1level_cut[ct] - _file_ptr->max_1level_cut[cut::Internal]))
            : cut::max;

          // Use smallest sound upper bound.
          _file_ptr->max_2level_cut[ct] = std::min(
            { _file_ptr->max_2level_cut[ct], ub_from_1level_cut, max_cut, all_arcs_cut[ct] });
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Increase the size of cut `c` to be the maximum of itself and `o`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    max_cut(cuts_t& c, const cuts_t& o)
    {
      for (size_t ct = 0u; ct < cut::size; ct++) { c[ct] = std::max(c[ct], o[ct]); }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Add the arcs in cut `o` to cut `c`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void
    inc_cut(cuts_t& c, const cuts_t& o)
    {
      for (size_t ct = 0u; ct < cut::size; ct++) { c[ct] += o[ct]; }
    }
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_OFSTREAM_H
