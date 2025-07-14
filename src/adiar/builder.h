#ifndef ADIAR_BUILDER_H
#define ADIAR_BUILDER_H

#include <adiar/bdd/bdd_policy.h>
#include <adiar/exception.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_ofstream.h>
#include <adiar/internal/memory.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__builder Builder
///
/// \brief Manual bottom-up construction of decision diagrams.
///
/// In some cases, one may already know the shape of a \ref bdd and/or as a \ref
/// zdd for a more complex function. In those cases, it is much cheaper to
/// construct them by hand than to manipulate logic formulas.
///
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__builder
  ///
  /// \{

  /// \cond
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Shared information between the builder and its pointers.
  ///
  /// \see builder builder_ptr
  //////////////////////////////////////////////////////////////////////////////////////////////////
  struct builder_shared
  {};
  /// \endcond

  template <typename dd_policy>
  class builder;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer type that builders use to identify the nodes they have
  ///        constructed in a decision diagram.
  ///
  /// \see builder
  //////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class builder_ptr
  {
  public:
    friend class builder<Policy>;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The unique identifier of a prior node.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: rename to 'ptr ptr' when using complement edges
    /*const*/ internal::node::uid_type uid;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unique shared reference for the parent builder object.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /*const*/ shared_ptr<const builder_shared> builder_ref;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reference for this specific 'uid' recording whether it is a child of a node created
    ///        later than itself.
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /*const*/ shared_ptr<bool> unreferenced = make_shared<bool>(true);

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Default construction for a pointer to `null`.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Copy construction.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr(const builder_ptr& bp) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move construction.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr(builder_ptr&& bp) = default;

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Copy construction.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr&
    operator=(const builder_ptr& bp) = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move construction.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr&
    operator=(builder_ptr&& bp) = default;

  private:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construction from an Adiar DD node made by a specific builder.
    ///
    /// \param p
    ///    Unique identifier of one of Adiar's nodes.
    ///
    /// \param sp
    ///    Reference (read-only) to the builder's shared information.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr(const internal::node::uid_type& p, const shared_ptr<const builder_shared>& sp)
      : uid(p)
      , builder_ref(sp)
    {}
  };

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Pointer for a BDD node created by a BDD builder.
  ///
  /// \see bdd_builder
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  using bdd_ptr = builder_ptr<bdd_policy>;

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Pointer for a ZDD node created by a ZDD builder.
  ///
  /// \see zdd_builder
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  using zdd_ptr = builder_ptr<zdd_policy>;

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief   A builder for decision diagrams
  ///
  /// \details Nodes should be added bottom-up. Each added node returns a
  ///          builder_ptr so that it can be used as a child for other nodes.
  ///
  /// \tparam Policy
  ///    Logic related to the specific type of decision diagram to construct.
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename Policy>
  class builder
  {
  private:
    // See 'attach_if_needed()' for default values of most values below.

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Type of nodes created within the file.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    using node_type = typename Policy::node_type;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief File containing all prior pushed nodes.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    shared_ptr<internal::levelized_file<node_type>> nf;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Node writer to push new nodes into 'nf'.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    internal::node_ofstream nw;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a terminal value has been returned in 'add_node'.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    bool created_terminal;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief The value of the latest terminal value returned in 'add_node'.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    bool terminal_val;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Label of the current level.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    typename Policy::label_type current_label;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Next available level identifier for the current level.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    typename Policy::id_type current_id;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Number of yet unreferenced nodes.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    size_t unref_nodes;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unique struct for this builder's current phase.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    shared_ptr<builder_shared> builder_ref;

  public:
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    builder() noexcept
    {}

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    ~builder() noexcept
    {}

  public:
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label and its two children.
    ///
    /// \param label
    ///    The variable label for the node to create.
    ///
    /// \param low
    ///    Pointer for when the variable with the given label evaluates to `false`.
    ///
    /// \param high
    ///    Pointer for when the variable with the given label evaluates to `true`.
    ///
    /// \returns Pointer to the (possibly new) node for the desired function.
    ///
    /// \remark This will apply the first reduction rule associated with the specific type of
    ///         decision diagram, e.g. not construct a "don’t care" node for \ref bdd but instead
    ///         just return its child. The builder cannot apply the second reduction rule, i.e.
    ///         merging of duplicate nodes, so that is still left up to you to do.
    ///
    /// \throws invalid_argument
    ///    If a node is malformed or not added in bottom-up order.
    ///
    /// \throws invalid_argument
    ///    If pointers stem from another builder.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr<Policy>
    add_node(typename Policy::label_type label,
             const builder_ptr<Policy>& low,
             const builder_ptr<Policy>& high)
    {
      attach_if_needed();

      // Check validity of input
      if (low.builder_ref != builder_ref || high.builder_ref != builder_ref) {
        throw invalid_argument("Cannot use pointers from a different builder");
      }
      if (label > Policy::max_label) { throw invalid_argument("Nodes must have a valid label"); }
      if (label > current_label) { throw invalid_argument("Nodes must be added bottom-up"); }
      if (low.uid.is_node() && low.uid.label() <= label) {
        throw invalid_argument("Low child must point to a node with higher label");
      }
      if (high.uid.is_node() && high.uid.label() <= label) {
        throw invalid_argument("High child must point to a node with higher label");
      }

      // Update label and ID if necessary
      if (label < current_label) {
        current_label = label;
        current_id    = Policy::max_id;
      }

      // Create potential node
      const node_type n(label, current_id, low.uid, high.uid);

      // Check whether this node is 'redundant'
      const typename node_type::uid_type res_uid = Policy::reduction_rule(n);

      if (res_uid.is_terminal()) {
        created_terminal = true;
        terminal_val     = res_uid.value();
      }

      if (res_uid == low.uid) { return low; }
      if (res_uid == high.uid) { return high; }

      // Push node to file
      nw.push(n);
      unref_nodes++;
      current_id--;

      // Update count of unreferenced nodes
      bool& low_unref = *low.unreferenced;
      if (low_unref && !low.uid.is_terminal()) {
        low_unref = false;
        unref_nodes--;
      }

      bool& high_unref = *high.unreferenced;
      if (high_unref && !high.uid.is_terminal()) {
        high_unref = false;
        unref_nodes--;
      }

      return make_ptr(res_uid);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label and its two children.
    ///
    /// \param label
    ///    The variable label for the node to create.
    ///
    /// \param low
    ///    Terminal value for when the variable with the given label evaluates to `false`.
    ///
    /// \param high
    ///    Pointer for when the variable with the given label evaluates to `true`.
    ///
    /// \returns Pointer to the (possibly new) node for the desired function.
    ///
    /// \throws invalid_argument
    ///    If `label` and `high` are illegal (see \ref add_node).
    /////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr<Policy>
    add_node(const typename Policy::label_type label,
             const bool low,
             const builder_ptr<Policy>& high)
    {
      attach_if_needed();

      builder_ptr<Policy> low_ptr = make_ptr(typename node_type::pointer_type(low));
      return add_node(label, low_ptr, high);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label and its two children.
    ///
    /// \param label
    ///   The variable label for the node to create.
    ///
    /// \param low
    ///   Pointer for when when the variable with the given label evaluates to `false`.
    ///
    /// \param high
    ///    Terminal value for when the variable with the given label evaluates to `true`.
    ///
    /// \returns Pointer to the (possibly new) node for the desired function.
    ///
    /// \throws invalid_argument
    ///    If `label` and `low` are illegal (see \ref add_node).
    /////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr<Policy>
    add_node(const typename Policy::label_type label,
             const builder_ptr<Policy>& low,
             const bool high)
    {
      attach_if_needed();

      builder_ptr<Policy> high_ptr = make_ptr(typename node_type::pointer_type(high));
      return add_node(label, low, high_ptr);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label and its two children.
    ///
    /// \param label The variable label for the node to create.
    ///
    /// \param low   Terminal value for when when the variable with the given
    ///              label evaluates to `false`.
    ///
    /// \param high  Terminal value for when the variable with the given label
    ///              evaluates to `true`.
    ///
    /// \returns     Pointer to the node for the desired function.
    ///
    /// \throws invalid_argument If `label` violates the bottom-up ordering (see
    ///                          \ref add_node).
    /////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr<Policy>
    add_node(const typename Policy::label_type label, const bool low, const bool high)
    {
      attach_if_needed();

      builder_ptr<Policy> low_ptr  = make_ptr(typename node_type::pointer_type(low));
      builder_ptr<Policy> high_ptr = make_ptr(typename node_type::pointer_type(high));
      return add_node(label, low_ptr, high_ptr);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Add a terminal node with a given value.
    ///
    /// \param terminal_value Value of the constant function to create.
    ///
    /// \remark One may call this function at any time during the construction
    ///         process since terminals are not part of the *bottom-up*
    ///         requirement.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr<Policy>
    add_node(bool terminal_value)
    {
      attach_if_needed();

      created_terminal = true;
      terminal_val     = terminal_value;

      return make_ptr(typename node_type::pointer_type(terminal_value));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Builds the decision diagram with the added nodes. This also clears the builder.
    ///
    /// \see clear
    ///
    /// \throws domain_error
    ///    If add_node has not been called
    ///
    /// \throws domain_error
    ///    If there are more than one root in the diagram.
    ///
    /// \pre One has used `add_node` to create a non-empty and singly-rooted decision diagram.
    ///
    /// \warning After calling this function all previously created `builder_ptr` created by this
    ///          object will be invalidated and cannot be used anymore.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    typename Policy::dd_type
    build()
    {
      attach_if_needed();

      if (!nw.has_pushed()) {
        if (created_terminal) {
          nw.push(node_type(terminal_val));
        } else {
          throw domain_error("There must be at least one node or terminal in the decision diagram");
        }
      }
      nw.close();

      if (unref_nodes > 1) { throw domain_error("Decision diagram has more than one root"); }

      const typename Policy::dd_type res(nf);
      close();
      return res;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief   Clear builder of all its current content, discarding all nodes and invalidating any
    ///          pointers to them.
    ///
    /// \warning This will invalidate any pointers created up to this point. You will not be able to
    ///          use them after this.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    void
    clear() noexcept
    {
      close();
    }

  private:
    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Initializes all values and the node file, if necessary.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    inline void
    attach_if_needed() noexcept
    {
      if (nf) { return; }

      adiar_assert(!nw.is_open(),
                   "`nw`'s attachment should be consistent with existence of `nf`");

      // Initialise file
      nf = internal::make_shared_levelized_file<node_type>();
      nw.open(nf);

      // Initialise state variables
      current_label    = Policy::max_label;
      current_id       = Policy::max_id;
      created_terminal = false;
      terminal_val     = false;
      unref_nodes      = 0;

      // Initialise shared builder reference.
      builder_ref = make_shared<builder_shared>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches the node writer and releases the pointer to the file.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    inline void
    close() noexcept
    {
      nw.close();
      nf.reset();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Create a builder_ptr with 'this' builder as its parent.
    /////////////////////////////////////////////////////////////////////////////////////////////////
    builder_ptr<Policy>
    make_ptr(const typename node_type::pointer_type& p) noexcept
    {
      return builder_ptr<Policy>(p, builder_ref);
    }
  };

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Builder for BDDs
  ///
  /// \see builder
  //////////////////////////////////////////////////////////////////////////////////////////////////
  using bdd_builder = builder<bdd_policy>;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Builder for ZDDs
  ///
  /// \see builder
  //////////////////////////////////////////////////////////////////////////////////////////////////
  using zdd_builder = builder<zdd_policy>;

  /// \}
  //////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_BUILDER_H
