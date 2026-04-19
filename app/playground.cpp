// TPIE Imports
#include "adiar/bdd.h"
#include "adiar/exec_policy.h"
#include "adiar/functional.h"
#include "adiar/internal/algorithms/reduce.h"
#include "adiar/internal/algorithms/replace.h"
#include "adiar/internal/assert.h"
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



//types for jump-up 
  // Data structures
  struct jump_mapping
  {
    node::uid_type old_uid;
    node::pointer_type new_uid;
    assignment payload = assignment::None; 
    //assignment = ternary type found in types file (maybe not the most intuitive thing in the world to use it here...)
    //default to none st. standard algo can be used for non-involved levels
  };

  class jump_up_arc : public arc {
    // fields
    private:
      assignment _payload;
      arc::label_type _xi;
    public:
      //constructors... i need like a million?
      jump_up_arc(const arc& a, assignment p , arc::label_type xi) : 
      arc(a), _payload(p), _xi(xi) {}

      jump_up_arc() {_payload = assignment::None; _xi = 0;}
    
    //accessors
    assignment payload() const {return _payload;}
    arc::label_type xi() const {return _xi;}

    //level for pq
    arc::label_type
    level() const
    {
      return std::max(source().label(), _xi);
    }

  };


//sorter stuff:
  struct jump_up_queue_lt //for pq
  {
    bool
    operator()(const jump_up_arc& a, const jump_up_arc& b)
    {
      // We want: sort by source then payload then low/high child?
      // should this take into account the weird xi stuff? no right? thats just for pq
      if (a.source().level() >  b.source().level()) {return true;} //if one source is greater it should be first
      if (a.source().level() <  b.source().level()) {return false;} //if one source is greater it should be first
      if (a.source().id() > b.source().id()) {return true;}
      if (a.source().id() < b.source().id()) {return false;}
      //if we get to here the sources must have same uids..
      //so now- we decide: no payload < false payload < true payload (follows ternary type ints)
      if (a.payload() < b.payload() ) {return true;}
      if (a.payload() > b.payload() ) {return false;}
      //if we get here thay also have same payload..
      //sort on arc type
      return a.out_idx() > b.out_idx();
    }
  };

   struct jump_reduce_uid_lt
  {
    bool
    operator()(const jump_mapping& a, const jump_mapping& b)
    {
      //grouping payloads
      if (a.old_uid == b.old_uid) {return a.payload < b.payload;}
      return a.old_uid > b.old_uid;
    }
  };

//printing
std::string
arc_to_string(jump_up_arc a) {
  std::stringstream stream;
  std::string payload_string;
  switch (a.payload()) {
    case assignment::None : payload_string = "None"; break;
    case assignment::False : payload_string = "⊥"; break;
    case assignment::True : payload_string = "T"; break;
  } 
   const std::string arrow =
        !a.source().is_node() || a.source().out_idx() ? " ---> " : " - -> ";
  stream << "(" << a.source() << arrow << a.target() << ", p: " << payload_string << ")";
  return stream.str();
}

// new node struct and node_of - to include payload
  struct jump_up_node : public node {
    //fields we need i think
    assignment _payload = assignment::None;

    //otherwise just constructor stuff?
    jump_up_node() = default;

    jump_up_node(const jump_up_node&) = default;

    jump_up_node(const node& n, const assignment payload )
      : node(n), _payload(payload)
    {}

    jump_up_node&
    operator=(const jump_up_node& n) = default;
  };

  inline jump_up_node
  j_node_of(const jump_up_arc& low, const jump_up_arc& high)
  {
    adiar_assert(essential(low.source()) == essential(high.source()), "Source are the same origin");

    adiar_assert(low.out_idx() == 0u, "Out-index is correct on low arc");
    adiar_assert(high.out_idx() == 1u, "Out-index is correct on high arc");

    adiar_assert(!low.target().is_node() || low.target().out_idx() == 0u,
                 "Out-index is empty in low target");
    adiar_assert(!high.target().is_node() || high.target().out_idx() == 0u,
                 "Out-index is empty in high target");

    adiar_assert(low.source().is_flagged() == false, "Source is not flagged on low arc");
    adiar_assert(high.source().is_flagged() == false, "Source is not flagged on high arc");

    adiar_assert(essential(low.source()) == low.source()
                 && essential(high.source()) == low.source());
    node res = node(node::uid_type(low.source()), low.target(), high.target());
    return jump_up_node(res, low.payload());
  }

  template <typename pq_t, typename arc_ifstream_t>
  inline jump_up_arc
  _jump_get_next(pq_t& reduce_pq, arc_ifstream_t& arcs)
  {
    if (!reduce_pq.can_pull()
        || (arcs.can_pull_terminal() && arcs.peek_terminal().source() > reduce_pq.top().source())) {
      return jump_up_arc(arcs.pull_terminal(), assignment::None, 0); //maybe dangerous..
    } else {
      return reduce_pq.pull();
    }
  }


//attempting jump-up reduce sweep...

template <typename Policy, typename pq_t, template <typename, typename> typename sorter_t>
  bdd
  jump_up( const shared_levelized_file<arc>& in_file,
           const size_t lpq_memory,
           const size_t sorters_memory,
           const replace_func<Policy>& m)
  {
    //input/output stuff:
    arc_ifstream<> arcs(in_file);
    level_info_ifstream<> levels(in_file);
    level_info_ifstream<> levels_for_map(in_file);

    // Set up output
    shared_levelized_file<typename Policy::node_type> out_file = __reduce_init_output<Policy>();
    node_ofstream out(out_file);

    //finding jump_ups (potentially list should just be passed instead of finding here??)
    std::vector<typename Policy::label_type> jump_starts, jump_targets;
    while(levels_for_map.can_pull()){
      level_info li = levels_for_map.pull();
      if(li.level() > m(li.level())) {
        //then it's a jump up!
        jump_starts.push_back(li.level());
        jump_targets.push_back(m(li.level()));
      }
    }
    //making generators..
    typename std::vector<typename Policy::label_type>::iterator s_begin = jump_starts.begin(), s_end = jump_starts.end();
    typename std::vector<typename Policy::label_type>::iterator t_begin = jump_targets.begin(), t_end = jump_targets.end();
    generator<typename Policy::label_type> level_gen = make_generator(s_begin, s_end);
    generator<typename Policy::label_type> target_gen_for_pq = make_generator(t_begin, t_end);
    generator<typename Policy::label_type> target_gen_for_me = make_generator(t_begin, t_end);

    // Initialize pq - new arc type and new sorting
    statistics::levelized_priority_queue_t stats_dummy;
    pq_t pq({ in_file, target_gen_for_pq }, lpq_memory, in_file->max_1level_cut, stats_dummy); //also has level for xi's -> cus reqs treated like they at that level..?
    //setup jump levels
    optional<typename Policy::label_type> xj = level_gen();
    optional<typename Policy::label_type> xi = target_gen_for_me();

    
    //so we also need to run reduce for the target levels... so cant just use level file like normal reduce
    while (!pq.empty() || arcs.can_pull_terminal()) {
      std::cout << "iter outer loop\n";
      //next level is greatest of tops of leaf file/ pq
      typename Policy::label_type level;
      //if max level is in pq
      if(!arcs.can_pull_terminal() || (!pq.empty() && pq.has_next_level() && pq.next_level() >= arcs.peek_terminal().source().level())){
        std::cout << "level from pq is max?\n";
        level = pq.current_level();
        //pq.setup_next_level(); //maybe dangerouse to have here..?
        //reduce level epilogue takes care of pq level setup, so maybe we just dotn do this here?
      } else {
        std::cout << "level from leaves is max?\n";
        level = arcs.peek_terminal().source().level();
      }
      std::cout << "starting work for level " << level << "\n";

      //temp files for each level
      iofstream<jump_mapping> red1_mapping;
      size_t unreduced_width = (level == xi) ? 0 : levels.pull().width(); //mayybe?
      std::cout << "width of current layer " << unreduced_width << "\n";
      sorter_t<jump_up_node, reduce_node_children_lt> child_grouping(sorters_memory, unreduced_width, 2);
      sorter_t<jump_mapping, jump_reduce_uid_lt> red2_mapping(sorters_memory, unreduced_width, 2);

      if (level > xj) {
      //completely standard reduce stuff - except new request & pq type but hopefully it's fine?  
      std::cout << "found uninvolved level " << level << "\n";
      //hate to see it - we should just do reduce stuff but type issues with arcs -> big copy paste for now..
      while ((arcs.can_pull_terminal() && arcs.peek_terminal().source().label() == level)
           || pq.can_pull() ) {
        std::cout << "pullign more nodes... \n";
        const jump_up_arc e_high = _jump_get_next(pq, arcs);
        const jump_up_arc e_low  = _jump_get_next(pq, arcs);
        std::cout << "found reqs " << arc_to_string(e_high) << " and " << arc_to_string(e_low) <<"\n";
        const jump_up_node n = j_node_of(e_low, e_high);
        std::cout << "built node " << n << "\n";
        //red1
        if(n.low() == n.high()){
           if (!red1_mapping.is_open()) { red1_mapping.open(); }
           red1_mapping.write({ n.uid(), n.low() });
        } else {
          child_grouping.push(n);
        }
      }
      //TODO: cuts

      //red2
      child_grouping.sort();
      typename Policy::id_type out_id = Policy::max_id;
      node out_node = node(node::uid_type(), ptr_uint64::nil(), ptr_uint64::nil());

      while (child_grouping.can_pull()) {
        std::cout << "entered red2 loop \n";
        const node next_node = child_grouping.pull();

        if (out_node.low() != unflag(next_node.low())
            || out_node.high() != unflag(next_node.high())) {
          adiar_assert(0 <= out_id, "Should still have more ids left");
          out_node = node(level, out_id--, unflag(next_node.low()), unflag(next_node.high()));
          out.unsafe_push(out_node);
          //TODO: cuts
        } 
        std::cout << "pushign mapping " << next_node.uid() << " -> " << out_node.uid() << "\n";
        red2_mapping.push({ next_node.uid(), out_node.uid() });
      }
      //update level info
      const size_t reduced_width = Policy::max_id - out_id;
      if (reduced_width > 0) { out.unsafe_push(level_info(level, reduced_width)); }

      //forwarding
      red2_mapping.sort();

      jump_mapping next_red1  = { node::uid_type(), node::uid_type() }; // <-- dummy value
      bool has_next_red1 = red1_mapping.is_open() && red1_mapping.size() > 0;
      if (has_next_red1) {
        red1_mapping.seek_begin();
        next_red1 = red1_mapping.next();
      }
      std::cout << "past red1 map stuff\n";
      jump_mapping next_red2  = { node::uid_type(), node::uid_type() }; // <-- dummy value
      bool has_next_red2 = red2_mapping.can_pull();
      if (has_next_red2) { next_red2 = red2_mapping.pull(); }
      std::cout << "past red2 map stuff\n";

      // Pass all the mappings to Q
      while (has_next_red1 || has_next_red2) {
        std::cout << "entered forwarding loop\n";
        // Find the mapping with largest old_uid
        const bool is_red1_current =
          !has_next_red2 || (has_next_red1 && next_red1.old_uid > next_red2.old_uid);

        const jump_mapping current_map = is_red1_current ? next_red1 : next_red2;
        std::cout << "found map: " << current_map.old_uid  << " -> " << current_map.new_uid << "\n";
        // Find all arcs that have the target that match the current mapping's old_uid
        while (arcs.can_pull_internal() && current_map.old_uid == arcs.peek_internal().target()) {
          const ptr_uint64 s = arcs.pull_internal().source();
          std::cout << "found matching arc: " << s << "\n";
          // If Reduction Rule 1 was used, then tell the parents to add to the global cut.
          const ptr_uint64 t = is_red1_current ? flag(current_map.new_uid)
                                              : static_cast<ptr_uint64>(current_map.new_uid);
          jump_up_arc n_req = {{s, t},assignment::None, 0};
          std::cout << "trying to push request to pq" << arc_to_string(n_req);
          pq.push(jump_up_arc(n_req));
        }

        // Update the mapping that was used
        if (is_red1_current) {
          has_next_red1 = red1_mapping.has_next();
          if (has_next_red1) { next_red1 = red1_mapping.next(); }
        } else {
          has_next_red2 = red2_mapping.can_pull();
          if (has_next_red2) { next_red2 = red2_mapping.pull(); }
        }
      }

      // Move on to the next level
      red1_mapping.close();
      //TODO: cuts
      __reduce_level__epilogue(arcs, pq, out, false); //donno if i need ot do this?

      } else if (level == xj) {
        //replace but
        // (1) we output no nodes
        // (2) instead of making one F2 mapping to new uid we make 2 F2 mappings:
        //      (a) one to the low child with payload \bot 
        //      (b) one to the high child with payload \top
        // (3) when forwardsing: for each incoming edge push 2 requests - one for each of the new mappings, containing their payloads. 
        std::cout << "found jump level " << level << "\n";
        while ((arcs.can_pull_terminal() && arcs.peek_terminal().source().label() == level)
            || pq.can_pull()) {
          //pull arcs and build nodes like normal..
          std::cout << "pulling arcs \n";
          const jump_up_arc e_high = _jump_get_next(pq, arcs);
          const jump_up_arc e_low  = _jump_get_next(pq, arcs);
          const jump_up_node n = j_node_of(e_low, e_high);
          std::cout << "built node " << n << "\n";

          //reduction rule 1
          if(n.low() == n.high()){
            if (!red1_mapping.is_open()) { red1_mapping.open(); }
            red1_mapping.write({ n.uid(), n.low() });
          } else {
            child_grouping.push(n);
          }
        }  
        //TODO update cuts (idk reduce does this..)

        //reduction rule 2
        child_grouping.sort(); //group duplicates 
        node out_node = node(node::uid_type(), ptr_uint64::nil(), ptr_uint64::nil());
        while (child_grouping.can_pull()) {
          std::cout << "entered red2 loop";
          //for each non-dupe pulled we want to push two mappings - one for \bot, one for \top
          const node next_node = child_grouping.pull();
          if (out_node.low() != unflag(next_node.low()) || out_node.high() != unflag(next_node.high())) {
            out_node = next_node;
            std::cout << "pushing red2 mapping: " <<  next_node.uid() << " -> " <<  next_node.low() << " , bot" << "\n";
            red2_mapping.push({ next_node.uid(), next_node.low(), assignment::False });
            std::cout << "pushing red2 mapping: " <<  next_node.uid() << " -> " <<  next_node.high() << " , top" << "\n";
            red2_mapping.push({ next_node.uid(), next_node.high(), assignment::True });
          }
        }

        //forwarding setup
        red2_mapping.sort(); //sort back to decending uid (should also take into account payload!)

        //fiddly cus file might not exist?
        jump_mapping next_red1;
        bool has_next_red1 = red1_mapping.is_open() && red1_mapping.size() > 0;
        if (has_next_red1) {red1_mapping.seek_begin(); next_red1 = red1_mapping.next();}

        //actual forwarding...
        while (has_next_red1 || red2_mapping.can_pull()) {
          std::cout << "entered forwarding loop\n";
          //findign next mappin as the one from eitehr F1 or F2 with largest old uid
          const bool is_red1_current = !(red2_mapping.can_pull()) || (has_next_red1 && next_red1.old_uid > red2_mapping.top().old_uid);
          if (is_red1_current) {
           
            //handle like normal
            const jump_mapping current_map = next_red1;
             std::cout << "found mapping from F1 " << current_map.old_uid << " -> "  << current_map.new_uid << "\n";
            while (arcs.can_pull_internal() && current_map.old_uid == arcs.peek_internal().target()) {
               const ptr_uint64 s = arcs.pull_internal().source();
               const ptr_uint64 t = flag(current_map.new_uid);
               jump_up_arc n_req = {jump_up_arc({s, t}, assignment::None, xi.value())};
               std::cout << "pushing req " << n_req << "\n";
               pq.push(n_req);
            }
          } else {
            // pull 2 mappings and push 2 reqs
            std::cout << "found mappings from F2 : \n ";
            const jump_mapping map_low = red2_mapping.pull();
            const jump_mapping map_high = red2_mapping.pull();
            std::cout << "  " << map_low.old_uid << " -> "  << map_low.new_uid <<"\n";
            std::cout << "  " << map_high.old_uid << " -> "  << map_high.new_uid <<"\n";
            adiar_assert(map_low.payload == assignment::False, "bot req is not first apparently?");
            adiar_assert(map_low.old_uid == map_high.old_uid, "pulled mappings not for same old uid!");
            while (arcs.can_pull_internal() && map_low.old_uid == arcs.peek_internal().target()) {
              const ptr_uint64 s = arcs.pull_internal().source();
              std::cout << "found matching req starting at " << s << "\n";
              const jump_up_arc n_low = {{s,map_low.new_uid}, assignment::False, xi.value()};
              const jump_up_arc n_high = {{s,map_high.new_uid}, assignment::True, xi.value()};
              std::cout << "pushing requests: " << arc_to_string(n_low) << " and " << arc_to_string(n_high) << "\n";
              pq.push(n_low);
              pq.push(n_high);
            }
          }
          //update next_red1 if any..
          if(is_red1_current) {
            has_next_red1 = red1_mapping.has_next(); 
            if (has_next_red1) {next_red1 = red1_mapping.next();}
          }
        }
        // Move on to the next level
        red1_mapping.close();
        //TODO update cut
        //epilogue (updates pq and does a bunch of asserts)
        const bool terminal_value = next_red1.new_uid.is_terminal() && next_red1.new_uid.value();
        __reduce_level__epilogue<>(arcs, pq, out, terminal_value);

      } else if (level > xi) {
        // reduce but group on payloads - potentially use non-payload reqs twice
        std::cout << "found between level " << level << "\n";
        jump_up_arc e_extra, e_high, e_low;  
        jump_up_arc dummy = jump_up_arc({node::pointer_type::nil(), node::pointer_type::nil()},assignment::None, 0);
         e_extra = dummy;
        while ((arcs.can_pull_terminal() && arcs.peek_terminal().source().label() == level)
           || pq.can_pull()) {
          std::cout << "pulling more arcs\n";
          //TODO: horribly written - can probably be much cleaner
          if(e_extra == dummy) {
            std::cout << "no existing e_extra\n";
            e_high = _jump_get_next(pq, arcs);
            e_low  = _jump_get_next(pq, arcs);
            std::cout << "pulled arcs:" << arc_to_string(e_high) << " and " << arc_to_string(e_low) << "\n";
          } else {
            std::cout << "existing e_extra = " << arc_to_string(e_extra) << "\n";
            e_high = (e_extra.out_idx()) ? e_extra : _jump_get_next(pq, arcs);
            e_low = (!e_extra.out_idx()) ? e_extra : _jump_get_next(pq, arcs);
            std::cout << "pulled arcs:" << arc_to_string(e_high) << " and " << arc_to_string(e_low) << "\n";
          }

          //checkign if we need to remeber a req..
          if(e_high.payload() == assignment::None && e_low.payload() != assignment::None){
            if (e_extra == e_high) {std::cout << "extra reset \n" ; e_extra = dummy;}
            else {std::cout << "extra set to high \n" ;e_extra = e_high;}
            }
          if (e_low.payload() == assignment::None && e_high.payload() != assignment::None){
            if (e_extra == e_low) {std::cout << "extra reset \n" ; e_extra = dummy;}
            else {std::cout << "extra set to low \n" ; e_extra = e_low;}}
          
          const jump_up_node n = j_node_of(e_low, e_high);
          std::cout << "made node " << n  << "\n";
          //reduction rule 1
          if(n.low() == n.high()){
            if (!red1_mapping.is_open()) { red1_mapping.open(); }
            red1_mapping.write({ n.uid(), n.low() });
          } else {
            child_grouping.push(n); //so not allowed ot push 2 nodes with same uid i think?
            std::cout << "pushed to F " << n << "\n";
          }
        }

        //TODO cuts stuff
        /*cuts_t local_1level_cut   = { { 0u, 0u, 0u, 0u } };
        cuts_t tainted_1level_cut = { { 0u, 0u, 0u, 0u } };*/

        /*__reduce_cut_add(local_1level_cut,
                    pq.size_without_terminals(),
                    pq.terminals(false) + arcs.unread_terminals(false),
                    pq.terminals(true) + arcs.unread_terminals(true));*/
        
        //reduction rule 2
        child_grouping.sort(); //payload shouldn't be important here i think?
        std::cout << "sorted F successfully? \n";
        typename Policy::id_type out_id = Policy::max_id;
        node out_node = node(node::uid_type(), ptr_uint64::nil(), ptr_uint64::nil());
        
        while (child_grouping.can_pull()) {
          std::cout << "entered red2 loop\n";
          const jump_up_node next_node = child_grouping.pull();
          std::cout << "found node " << next_node << "\n";
          if (out_node.low() != unflag(next_node.low())
              || out_node.high() != unflag(next_node.high())) {
            adiar_assert(0 <= out_id, "Should still have more ids left");
            out_node = node(level, out_id--, unflag(next_node.low()), unflag(next_node.high()));
            std::cout << "pushing node to out " << out_node << "\n";
            out.unsafe_push(out_node); //needs to be unsafe donno why
            //TODO cut things here!
          } 
          std::cout << "new F2 mapping: " << next_node.uid() << " -> " << out_node.uid() << "\n";
          red2_mapping.push({ next_node.uid(), out_node.uid(), next_node._payload });
        }

        //update level info:
        // Add number of nodes to level information, if any nodes were pushed to the output.
        const size_t reduced_width = Policy::max_id - out_id;
        if (reduced_width > 0) { out.unsafe_push(level_info(level, reduced_width)); }
        
        //forwarding
        red2_mapping.sort(); //sort back to decending uid (should also take into account payload!)

        //fiddly cus file might not exist and for some reason we cant peek the next thing?
        jump_mapping next_red1;
        bool has_next_red1 = red1_mapping.is_open() && red1_mapping.size() > 0;
        if (has_next_red1) {red1_mapping.seek_begin(); next_red1 = red1_mapping.next();}

        //actual forwarding...
        while (has_next_red1 || red2_mapping.can_pull()) {
          std::cout << "entered forwarding loop\n";
          //gotta pull 2 reqs - it aint pretty
          bool is_red1_current = !red2_mapping.can_pull() || (has_next_red1 && next_red1.old_uid > red2_mapping.top().old_uid);
          const jump_mapping map_low = is_red1_current ? next_red1 : red2_mapping.pull();
          //hate to see this
          has_next_red1 = red1_mapping.has_next();
          if (has_next_red1) {next_red1 = red1_mapping.next();}
          is_red1_current = !red2_mapping.can_pull() || (has_next_red1 && next_red1.old_uid > red2_mapping.top().old_uid);
          const jump_mapping map_high = is_red1_current ? next_red1 : red2_mapping.pull();
          adiar_assert(map_low.payload == assignment::False, "first pulled is not payload bot");
          adiar_assert(map_low.old_uid == map_high.old_uid, "pulled pair uids dont match!");
          std::cout << "found mappings: \n";
          std::cout << "   " << map_low.old_uid << " -> " << map_low.new_uid << "\n";
          std::cout << "   " << map_high.old_uid << " -> " << map_high.new_uid << "\n";
          while (arcs.can_pull_internal() && map_low.old_uid == arcs.peek_internal().target()) {
            const ptr_uint64 s = arcs.pull_internal().source();
            std::cout << "found matching req starting in :" << s << "\n";
            const jump_up_arc n_low = {{s,map_low.new_uid}, assignment::False, xi.value()};
            const jump_up_arc n_high = {{s,map_high.new_uid}, assignment::True, xi.value()};
            std::cout << "pushing requests: " << arc_to_string(n_low) << " and " << arc_to_string(n_high) << "\n";
            pq.push(jump_up_arc(n_low));
            pq.push(jump_up_arc(n_high));
          }

        }
        // Move on to the next level
        red1_mapping.close();
        //TODO: update cuts?

        const bool terminal_value = false; //i donno what this is for
        __reduce_level__epilogue<>(arcs, pq, out, terminal_value);
        
      } else { // level is xi
        std::cout << "found target level " << level << "\n";
        const typename Policy::label_type cur_xi = xi.value();
        typename Policy::id_type out_id = Policy::max_id;
        xi = level_gen();
        //ok so here -> i dont know if we need to do reduciton stuff?
        //observation: we dont need to handle leaves in this case -> they'll be handled in following layers
        //TODO also cut stuff here -> need to update them when we pushing nodes i think
        while(pq.can_pull()){
          std::cout << "pulling arcs\n";
          jump_up_arc r1, r2, r3, r4;
          r1 = pq.pull();
          if(r1.payload() == assignment::None) {
            //this is an edge to a level below xj -> we just pass it on
            jump_up_arc n_req = {{r1.source(), r1.target()}, r1.payload(), (xi.has_value() ? xi.value() : 0)};
            std::cout << "found green edge " << arc_to_string(r1) << "so we push req" << n_req <<"\n";
            pq.push(n_req);
          } else {
            std::cout << "found blue edge" << arc_to_string(r1) << "pulling another" << "\n";
            adiar_assert(r1.payload() == assignment::False, "somehow pulled a top first (level xi)");
            r2 = pq.pull();
            if (r2.payload() == assignment::True){
              std::cout << "found match" << arc_to_string(r2) << "\n";
              // we're in case where one child is red/green (drawings..)
              //just output one one node
              node::uid_type out_uid(cur_xi, out_id);
              node res_node = {out_uid, r1.target(), r2.target()};
              jump_up_arc n_req = {{r1.source(), out_uid }, assignment::None, (xi.has_value() ? xi.value() : 0)};
              std::cout << "outputting node " << res_node << ", pushing req " << arc_to_string(n_req) << "\n";
              out.unsafe_push(res_node);
              pq.push(n_req);
              out_id --;
            } else {
              std::cout << "found another" << arc_to_string(r2) << " pulling 2 more... \n";
              //were in both children blue case (drawing)
              //push 2 nodes and reqs
              r3 = pq.pull();
              r4 = pq.pull();
              std::cout << " " << arc_to_string(r3) << ", " << arc_to_string(r4) << "\n";
              adiar_assert(r1.payload() != r3.payload(), "r1 and r3 match :c");
              adiar_assert(r2.payload() != r4.payload(), "r2 and r4 match :c");
              node::uid_type out_uid1(cur_xi, out_id); node::uid_type out_uid2(cur_xi, out_id-1);
              const node n1 = {out_uid1, r1.target(), r3.target()};
              const node n2 = {out_uid2, r2.target(), r4.target()};
              const jump_up_arc n_req1 = {{r1.source(), out_uid1},assignment::None, (xi.has_value() ? xi.value() : 0)};
              const jump_up_arc n_req2 = {{r2.source(), out_uid2},assignment::None, (xi.has_value() ? xi.value() : 0)};
              out.unsafe_push(n1); out.unsafe_push(n2);
              pq.push(n_req1); pq.push(n_req2);
              std::cout << "outputting node " << n1 << ", pushing req " << arc_to_string(n_req1) << "\n";
              std::cout << "outputting node " << n2 << ", pushing req " << arc_to_string(n_req2) << "\n";
              out_id = out_id -2;
            }
          }
        }
        //update xj!
        xj = target_gen_for_me();
        //update level info
          const size_t reduced_width = Policy::max_id - out_id;
          if (reduced_width > 0) { out.unsafe_push(level_info(level, reduced_width)); }
        // TODO - cuts
        const bool terminal_value = false; //still donno what this is
        __reduce_level__epilogue<>(arcs, pq, out, terminal_value);
    }
  }
  std::cout << "exited big loop";
  return typename Policy::dd_type(out_file);
}


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
    bdd res = bdd_replace(bdd_test, m_ju);
    bdd_printdot(res, "jump_up_ns.dot");

    arc a(ptr_uint64(2,0), uid_uint64(3,0));
    arc b(ptr_uint64(2,0), uid_uint64(4,0));
    jump_up_arc test_arc1(a, assignment::None, 3);
    jump_up_arc test_arc2(b, assignment::True, 3);

    std::vector<bdd::label_type> levs = {0,1,2,3,4};
    /*auto iter_s = levs.rbegin();
    auto iter_e = levs.rend();*/

    using test_pq_t = levelized_arc_priority_queue<jump_up_arc, jump_up_queue_lt, 1, memory_mode::External, 2>;
    /*statistics::levelized_priority_queue_t stats_dummy;
    test_pq_t arc_pq({make_generator(iter_s,iter_e)},memory_available()/2,memory_available()/8,stats_dummy);
    arc_pq.push(test_arc2);
    arc_pq.push(test_arc1);
    std::cout << "we've pushed 2 arcs to the pq with xi larger than source id.. " << "\n";
    std::cout << "pq has next level? : " << arc_pq.has_next_level() << "\n";
    std::cout << "the next level of pq is: " << arc_pq.next_level() << "\n";
    arc_pq.setup_next_level();
    jump_up_arc a1 = arc_pq.pull();
    jump_up_arc a2 = arc_pq.pull();
    std::cout << "first req pulled from pq is: " << arc_to_string(a1) << "\n";
    std::cout << "second req pulled from pq is: " << arc_to_string(a2) << "\n";*/

    //seemingly this works.. 
    bdd res2 = jump_up<bdd_policy, test_pq_t, external_sorter>(transpose(bdd_test), memory_available()/2, memory_available()/2, m_ju);
    bdd_printdot(res2, "jump_up_actual.dot");
  }

  //adiar::statistics_print();

  adiar::adiar_deinit();
  return 0;
}
