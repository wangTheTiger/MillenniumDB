#ifndef RELATIONAL_MODEL__DISTINCT_HASH_H_
#define RELATIONAL_MODEL__DISTINCT_HASH_H_

#include <map>
#include <memory>

#include "base/binding/binding_iter.h"
#include "base/ids/var_id.h"
#include "storage/index/hash_tuple/extendable_table.h"

class DistinctHash : public BindingIter {
public:
    DistinctHash(std::unique_ptr<BindingIter> child_iter, std::vector<VarId> projected_vars);
    ~DistinctHash() = default;

    inline Binding& get_binding() noexcept override { return child_binding; }

    void begin() override;
    bool next() override;
    void analyze(int indent = 0) const override;    

    bool current_tuple_distinct();

private:
    std::unique_ptr<BindingIter> child_iter;
    Binding& child_binding;
    std::vector<VarId> projected_vars;
    ExtendableTable<GraphObject> extendable_table;

    std::vector<GraphObject> current_tuple;
};

#endif // RELATIONAL_MODEL__DISTINCT_HASH_H_