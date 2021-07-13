#include "optimize_tree.h"

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_distinct.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_graph_pattern_root.h"
#include "base/parser/logical_plan/op/op_group_by.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_optional.h"
#include "base/parser/logical_plan/op/op_order_by.h"
#include "base/parser/logical_plan/op/op_select.h"

using namespace std;

void OptimizeTree::visit(OpOptional& op_optional) {
    op_optional.op->accept_visitor(*this);
    bool current_move_children_up = false;
    if (delete_current) {
        current_move_children_up = true;
    }
    delete_current = false;
    move_children_up = false;
    optional_to_match = false;
    for (auto it = op_optional.optionals.begin(); it != op_optional.optionals.end(); ) {
        (*it)->accept_visitor(*this);
        if (move_children_up || optional_to_match) {
            it = op_optional.optionals.erase(it);
            it = op_optional.optionals.insert(it,
                                              std::make_move_iterator(optionals.begin()),
                                              std::make_move_iterator(optionals.end()));
            advance(it, optionals.size());
            optionals.clear();

            // reset
            move_children_up = false;
            optional_to_match = false;

        }
        else if (delete_current) {
            it = op_optional.optionals.erase(it);
            delete_current = false;
        } else {
            ++it;
        }
    }

    if (op_optional.optionals.size() == 0) {
        if (current_move_children_up) {
            move_children_up = false;
            delete_current = true;
        } else {
            move_children_up = false;
            delete_current= false;
            optional_to_match = true;
            optionals.emplace_back(move(op_optional.op));
        }

    } else if(current_move_children_up) {
        move_children_up = true;
        delete_current = false;
        for (auto it=op_optional.optionals.begin(); it!=op_optional.optionals.end(); ) {
            optionals.emplace_back(move(*it));
            it = op_optional.optionals.erase(it);
        }
    }
}


void OptimizeTree::visit(OpMatch& op_match) {
    // delete already assigned properties
    for (auto it = op_match.properties.begin(); it != op_match.properties.end(); ) {
        auto op_property = *it;
        auto property_search = global_properties_set.find(op_property);
        if (property_search != global_properties_set.end()) {
            auto found_property = *property_search;
            if (op_property.value != found_property.value) {
                it = op_match.properties.erase(it);
            } else {
                ++it;
            }
        }
        global_properties_set.insert(op_property);
    }

    // delete already assigned labels
    for (auto it = op_match.labels.begin(); it != op_match.labels.end(); ) {
        auto op_label = *it;
        auto label_search = global_label_set.find(op_label);
        if (label_search != global_label_set.end()) {
            it = op_match.labels.erase(it);
        } else {
            ++it;
        }
        global_label_set.insert(op_label);
    }

    // delete already assigned isolated vars
    for (auto it = op_match.isolated_vars.begin(); it != op_match.isolated_vars.end(); ) {
        auto op_unjoint_object = *it;

        if (global_vars.find(op_unjoint_object.var) != global_vars.end()) {
            it = op_match.isolated_vars.erase(it);
        } else {
            ++it;
        }
    }

    // if nothing to match, will be deleted
    if (   op_match.connections.empty()
        && op_match.labels.empty()
        && op_match.properties.empty()
        && op_match.property_paths.empty()
        && op_match.isolated_vars.empty())
    {
        delete_current = true;
    }

    std::set<Var> match_vars;
    op_match.get_vars(match_vars);
    for (auto& var_name : match_vars) {
        global_vars.insert(var_name);
    }
}


void OptimizeTree::visit(OpGraphPatternRoot& op_graph_pattern_root) {
    optional_to_match = false;
    op_graph_pattern_root.op->accept_visitor(*this);
    if (optional_to_match) {
        op_graph_pattern_root.op = move(optionals[0]);
    }
}


void OptimizeTree::visit(OpSelect& op_select) {
    op_select.op->accept_visitor(*this);
}


void OptimizeTree::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
}


void OptimizeTree::visit(OpGroupBy& op_group_by) {
    op_group_by.op->accept_visitor(*this);
}


void OptimizeTree::visit(OpOrderBy& op_order_by) {
    op_order_by.op->accept_visitor(*this);
}


void OptimizeTree::visit(OpDistinct& op_distinct) {
    op_distinct.op->accept_visitor(*this);
}
