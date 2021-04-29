#include "property_path_bfs_simple_enum.h"

#include <cassert>
#include <iostream>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

PropertyPathBFSSimpleEnum::PropertyPathBFSSimpleEnum(BPlusTree<4>& type_from_to_edge,
                                     BPlusTree<4>& to_type_from_edge,
                                     Id start,
                                     VarId end,
                                     PathAutomaton automaton) :
    type_from_to_edge (type_from_to_edge),
    to_type_from_edge (to_type_from_edge),
    start             (start),
    end               (end),
    automaton         (automaton)
    { }


void PropertyPathBFSSimpleEnum::begin(BindingId& parent_binding, bool parent_has_next) {

    this->parent_binding = &parent_binding;
    if (parent_has_next) {
        // Add inital state to queue
        if (std::holds_alternative<ObjectId>(start)) {
            auto start_object_id = std::get<ObjectId>(start);
            auto start_state = SearchState(automaton.start, start_object_id);
            open.push(start_state);
            visited.insert(start_state);
        } else {
            auto start_var_id = std::get<VarId>(start);
            auto start_object_id = parent_binding[start_var_id];
            auto start_state = SearchState(automaton.start, start_object_id);
            open.push(start_state);
            visited.insert(start_state);
        }
        min_ids[2] = 0;
        max_ids[2] = 0xFFFFFFFFFFFFFFFF;
        min_ids[3] = 0;
        max_ids[3] = 0xFFFFFFFFFFFFFFFF;
        is_first = true;
        // pos 0 and 1 will be set at next()
    }
}


bool PropertyPathBFSSimpleEnum::next() {
    if (is_first) {
        is_first = false;
        if (automaton.start_is_final) {
            auto& current_state = open.front();
            visited.emplace(automaton.final_state, current_state.object_id);
            parent_binding->add(end, current_state.object_id);
            return true;
        }
    }
    while (open.size() > 0) {
        auto& current_state = open.front();
        std::unique_ptr<BptIter<4>> it;
        for (const auto& transition : automaton.transitions[current_state.state]) {
            if (transition.inverse) {
                min_ids[0] = current_state.object_id.id;
                max_ids[0] = current_state.object_id.id;
                min_ids[1] = transition.label.id;
                max_ids[1] = transition.label.id;
                it = to_type_from_edge.get_range(
                    Record<4>(min_ids),
                    Record<4>(max_ids)
                );
            } else {
                min_ids[0] = transition.label.id;
                max_ids[0] = transition.label.id;
                min_ids[1] = current_state.object_id.id;
                max_ids[1] = current_state.object_id.id;
                it = type_from_to_edge.get_range(
                    Record<4>(min_ids),
                    Record<4>(max_ids)
                );
            }
            bpt_searches++;
            auto child_record = it->next();
            while (child_record != nullptr) {
                auto next_state = SearchState(transition.to, ObjectId(child_record->ids[2]));
                if (visited.find(next_state) == visited.end()) {
                    open.push(next_state);
                    visited.insert(next_state);
                }
                child_record = it->next();
            }
        }
        if (current_state.state == automaton.final_state) {
            results_found++;
            parent_binding->add(end, current_state.object_id);
            open.pop();
            return true;
        }
        open.pop();
    }
    return false;
}


void PropertyPathBFSSimpleEnum::reset() {
    // Empty open and visited
    queue<SearchState> empty;
    open.swap(empty);
    visited.clear();

    if (std::holds_alternative<ObjectId>(start)) {
        auto start_object_id = std::get<ObjectId>(start);
        auto start_state = SearchState(automaton.start, start_object_id);
        open.push(start_state);
        visited.insert(start_state);

    } else {
        auto start_var_id = std::get<VarId>(start);
        auto start_object_id = (*parent_binding)[start_var_id];
        auto start_state = SearchState(automaton.start, start_object_id);
        open.push(start_state);
        visited.insert(start_state);
    }
    is_first = true;
}


void PropertyPathBFSSimpleEnum::assign_nulls() { }


void PropertyPathBFSSimpleEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "PropertyPathBFSSimpleEnum(bpt_searches: " << bpt_searches
         << ", found: " << results_found <<")\n";
}
