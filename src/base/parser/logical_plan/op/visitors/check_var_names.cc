#include "check_var_names.h"

#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_connection.h"

void CheckVarNames::visit(OpSelect& op_select) {
    op_select.op->accept_visitor(*this);

    for (auto& select_item : op_select.select_items) {
        if (declared_object_names.find(select_item.var) == declared_object_names.end()) {
            throw QuerySemanticException("Variable \"" + select_item.var +
                "\" used in SELECT is not declared in MATCH");
        }
    }
}


void CheckVarNames::visit(OpMatch& op_match) {
    declared_object_names = op_match.var_names;
}


void CheckVarNames::visit(OpFilter& op_filter) {
    op_filter.op->accept_visitor(*this);
    op_filter.check_var_names(declared_object_names);
}


void CheckVarNames::visit(OpConnection&) { };
void CheckVarNames::visit(OpConnectionType&) { };
void CheckVarNames::visit(OpLabel&) { };
void CheckVarNames::visit(OpProperty&) { };
void CheckVarNames::visit(OpUnjointObject&) { };
