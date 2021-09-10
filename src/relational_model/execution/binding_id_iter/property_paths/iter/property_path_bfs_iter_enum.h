/*
The class PropertyPathBFSIterEnum implements a linear iterator for evaluating
 a single property path which has either a strting or an ending point, but not
 both. I.e. (sub)expressions of the form (?x)=[:label*]=>(?y), or
 (?x)=[:label*]=>(?y), where either ?x or ?y are already instantiated. If both
 ?x and ?y are already instantiated the class PropertyPathBFSCheck is used for
 query evaluation.

The BFS search algorihtm is executed from the assigned starting point of the
query (sub)expression. This class assumes that QuadModel is used (as most
currently implemented evaluation classes do), in order to be able to access
the required B+ trees.

Since the query pattern uses a regular expression to specify the path constraint,
 it is naturally associated to an automaton equivalent to the regular expression.

Due to the search being guided by the automaton, it can be conceptualized as a
search in the graph consisting of pairs (nodeID,automatonState), where a move
from (nodeID1,automatonState1) to (nodeID2,automatonState2) is possible only if
in our graph there is a connection (nodeID1,type,nodeID2,edge), and
(automatonState1,type,automatonState2) is a transition in the automaton
generated by the regular expression specifying the query.

Elements of the class are:
    - nodes:
        the B+ tree containing all nodes
        (used to determine whether a constant node exists in the graph)
    - type_from_to_edge:
        the B+ tree allowing us to search forward using a type and a from node
    - to_type_from_edge:
        the B+ tree used to traverse edges in reverse
        (from to, knowing type towards from)
    - path_var:
        the ID of the path variable that stores the connected nodes
        (if specified)
    - start:
        the ID of the start node
        (it is possible that this was assigned in a previous iterator that
        piped its results to PropertyPathBFSSimpleEnum; in this case the value
        of this variable is transformed into an object ID in begin())
    - end:
        the ID of the variable storing the end node of the path
        (
        Property path is always evaluated start to end.
        If the patter is of the form (Q1)=[:a*]=>(?x) this is natural.
        A query of the form (?x)=[:a*]=>(Q1) uses the inverse automaton;
        that is, an automaton for (^:a)*, which traverses :a-typed edges
        in reverse. The set of query answers is equivalent to the original
        query (?x)=[:a*]=>(Q1), but the actual query gets rewritten to
        (Q1)=[(^:a)*]=>(?x), and then evaluated.
        )

    - automaton:
        the automaton for the regular expression used to specify the query
    - first_next:
        a boolean value signalling whether this is the first time that next was
        called; we need this in the case when the initial state of the automaton
        is also an end state, in which case any starting node is also a query
        result and needs to be returned. For instance in the case of the query
        (Q1)=[a*]=>(?x), Q1 is a result whenever present in the graph due to
        the empty path matching a*.

    - parent_binding:
        as in all piped iterators, contains the binding passed upwards

    - min_ids:
        search range in the connections B+tree (e.g. in type_from_to_edge)
    - max_ids:
        search range in the connections B+tree

    - iter:
        the B+tree iterator for fetching the children of the node currently
        being explored in BFS.

    - current transition:
        the transition of the automaton we are currently expanding from the
        automaton state in the top of the BFS queue; note that this is simply
        the number of the transition. The transition itself is accessed by
        automaton.transitions[state][current_transition]

    - visited:
        the set of visited SearchState elements
        (i.e. pairs (nodeID,automatonState) already used in our search)
    - open:
        the queue of SearchState elements we are currently exploring

    - results_found: for statistics
    - bpt_searches: for statistics

The class methods are the same as for any iterator we define in MillenniumDB:
    - begin
    - next
    - reset
    - analyze
    - assign_nulls

Additionally, the following method is used to create the required B+ tree
iterators for BFS search:
    - set_iter(current_state):
        we want to expand the nodeID in current_state=(nodeID,automatonState)
        and fetch all of its children according to the current_transition
        specified above; depending on the transition's direction, we set the
        appropriate from_type_to_edge iter (for forward looking transitions),
        or to_type_from_edge iter (for backwards transitions).
    - current_state_has_next(current_state):
        iterator over possible SearchState elements in the BFS search that
        follow from the state on the top of the queue; a B+tree iter is used
        to fetch graph nodes, and a transition from the current automaton
        state is iterated over using current_transition; note that more than
        one transition might start in the same automaton state (e.g. for 
        expressions of the form (a|b), or for Kleene stars)
*/

#ifndef RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_
#define RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_

#include <array>
#include <memory>
#include <unordered_set>
#include <queue>
#include <variant>

#include "base/binding/binding_id_iter.h"
#include "base/parser/logical_plan/op/property_paths/path_automaton.h"
#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/execution/binding_id_iter/property_paths/search_state.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"
#include "storage/index/bplus_tree/bplus_tree.h"

/*
PropertyPathBFSIterEnum enumerates paths from or to a specifc node,
using BFS algorithm, thus paths returned are optimal.

Precaution with the exponencial memory usage of open due to
ramification of BFS
*/


class PropertyPathBFSIterEnum : public BindingIdIter {
    using Id = std::variant<VarId, ObjectId>;

private:
    // Attributes determined in the constuctor
    ThreadInfo*   thread_info;
    BPlusTree<1>& nodes;
    BPlusTree<4>& type_from_to_edge;  // Used to search foward
    BPlusTree<4>& to_type_from_edge;  // Used to search backward
    VarId         path_var;
    Id            start;
    VarId         end;
    PathAutomaton automaton;

    // Attributes determined in begin
    BindingId* parent_binding;
    bool first_next = true;

    // Ranges to search in BPT. They are not local variables because some positions are reused.
     std::array<uint64_t, 4> min_ids;
     std::array<uint64_t, 4> max_ids;

    // Structs for BFS
    std::unordered_set<SearchState, SearchStateHasher> visited;
    // open stores a pointer to a SearchState stored in visited
    // that allows to avoid use visited.find to get a pointer and
    // use the state extracted of the open directly.
    std::queue<const SearchState*> open;

    // Stores the children of state in expansion
    std::unique_ptr<BptIter<4>> iter;
    // The index of the transition that set_iter method uses to
    // construct iter attribute.
    uint32_t current_transition = 0;

    // Statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

    std::unordered_set<SearchState, SearchStateHasher>::iterator
        current_state_has_next(const SearchState* current_state);

    // Set iter attribute that give all states that connects with
    // current_state with label of a specific transition
    void set_iter(const SearchState* current_state);

public:
    PropertyPathBFSIterEnum(ThreadInfo*   thread_info,
                            BPlusTree<1>& nodes,
                            BPlusTree<4>& type_from_to_edge,
                            BPlusTree<4>& to_type_from_edge,
                            VarId path_var,
                            Id start,
                            VarId end,
                            PathAutomaton automaton);
    ~PropertyPathBFSIterEnum() = default;

    void analyze(std::ostream& os, int indent = 0) const override;
    void begin(BindingId& parent_binding) override;
    void reset() override;
    void assign_nulls() override;
    bool next() override;

};


#endif // RELATIONAL_MODEL__PROPERTY_PATH_BFS_ITER_ENUM_H_
