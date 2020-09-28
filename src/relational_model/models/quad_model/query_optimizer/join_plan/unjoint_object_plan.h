#ifndef RELATIONAL_MODEL__LONELY_NODE_PLAN_H_
#define RELATIONAL_MODEL__LONELY_NODE_PLAN_H_

#include "relational_model/models/quad_model/quad_model.h"
#include "relational_model/models/quad_model/query_optimizer/join_plan/join_plan.h"

class UnjointObjectPlan : public JoinPlan {
public:
    UnjointObjectPlan(const UnjointObjectPlan& other);
    UnjointObjectPlan(QuadModel& model, const VarId object_var_id);
    ~UnjointObjectPlan() = default;

    double estimate_cost() override;
    double estimate_output_size() override;

    uint64_t get_vars() override;
    void set_input_vars(const uint64_t input_vars) override;

    std::unique_ptr<BindingIdIter> get_binding_id_iter() override;
    std::unique_ptr<JoinPlan> duplicate() override;

    void print(int indent, bool estimated_cost, std::vector<std::string>& var_names) override;

private:
    QuadModel& model;
    const VarId object_var_id;
};

#endif // RELATIONAL_MODEL__LONELY_NODE_PLAN_H_
