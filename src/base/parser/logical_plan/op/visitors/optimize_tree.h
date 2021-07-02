#ifndef BASE__OPTIMIZE_TREE_H_
#define BASE__OPTIMIZE_TREE_H_

#include <set>
#include <vector>

#include "base/graph/graph_object.h"
#include "base/parser/logical_plan/op/op_label.h"
#include "base/parser/logical_plan/op/op_unjoint_object.h"
#include "base/parser/logical_plan/op/op.h" // NEW
#include "base/parser/logical_plan/op/visitors/op_visitor.h"

/* Simplifies the logical plan generated based on certain properties of the logical plan.
 * This simplifications only happen if the query has an `OPTIONAL` clause.
 * The simplifications are done by executing a DFS algorithm over the Optional Nodes,
 * going through their children in order and saving the variables involved in each node.
 * Since the query is well defined, once a variable is assigned it must not be assigned once again,
 * so any node that does not assign any new variables is marked as useless and is eliminated.
 */
class OptimizeTree : public OpVisitor {
private:
    // these sets are to avoid having redundant labels/properties inside an OPTIONAL
    std::set<OpLabel> global_label_set;
    std::set<OpProperty> global_properties_set;

    std::set<std::string> global_var_names;
    std::vector<std::unique_ptr<Op>> optionals;

    bool delete_current = false;
    bool move_children_up = false;
    bool optional_to_match = true;

public:
    void visit(OpSelect&) override;
    void visit(OpMatch&) override;
    void visit(OpFilter&) override;
    void visit(OpConnection&) override;
    void visit(OpLabel&) override;
    void visit(OpProperty&) override;
    void visit(OpOrderBy&) override;
    void visit(OpGroupBy&) override;
    void visit(OpOptional&) override;
    void visit(OpTransitiveClosure&) override;
    void visit(OpUnjointObject&) override;
    void visit(OpGraphPatternRoot&) override;
    void visit(OpDistinct&) override;
};

#endif // BASE__OPTIMIZE_TREE_H_
