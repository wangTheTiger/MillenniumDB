#ifndef RELATIONAL_MODEL__LABELED_CONNECTION_PLAN_H_
#define RELATIONAL_MODEL__LABELED_CONNECTION_PLAN_H_

#include "base/ids/graph_id.h"
#include "base/ids/object_id.h"
#include "base/graph/graph_object.h"
#include "relational_model/query_optimizer/join_plan/join_plan.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"

class LabeledConnectionPlan : public JoinPlan {
public:
    LabeledConnectionPlan(const LabeledConnectionPlan& other);
    LabeledConnectionPlan(GraphId graph_id, ObjectId label_id, VarId node_from_var_id,
                          VarId node_to_var_id, VarId edge_var_id);
    ~LabeledConnectionPlan() = default;

    double estimate_cost() override;
    double estimate_output_size() override;

    void set_input_vars(std::vector<VarId>& input_var_order) override;
    std::vector<VarId> get_var_order() override;
    std::unique_ptr<BindingIdIter> get_binding_id_iter() override;
    std::unique_ptr<JoinPlan> duplicate() override;

    void print(int indent) override;
private:
    GraphId graph_id;

    ObjectId label_id;
    VarId node_from_var_id;
    VarId node_to_var_id;
    VarId edge_var_id;

    bool node_from_assigned;
    bool node_to_assigned;
    bool edge_assigned;

    std::unique_ptr<ScanRange> get_node_from_range();
    std::unique_ptr<ScanRange> get_node_to_range();
    std::unique_ptr<ScanRange> get_edge_range();
};

#endif // RELATIONAL_MODEL__LABELED_CONNECTION_PLAN_H_
