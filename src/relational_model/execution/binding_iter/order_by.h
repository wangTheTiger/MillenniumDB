#ifndef RELATIONAL_MODEL__ORDER_BY_H_
#define RELATIONAL_MODEL__ORDER_BY_H_

#include <map>
#include <memory>
#include <vector>

#include "base/binding/binding_id.h"
#include "base/graph/graph_model.h"
#include "base/ids/var_id.h"
#include "relational_model/execution/binding/binding_order_by.h"
#include "storage/file_id.h"
#include "storage/tuple_collection/tuple_collection.h"

class OrderBy : public BindingIter {
public:
    OrderBy(GraphModel& model,
            std::unique_ptr<BindingIter> child,
            std::size_t binding_size,
            std::vector<std::pair<std::string, VarId>> order_vars,
            std::vector<bool> ascending);
    ~OrderBy() = default;

    Binding& get_binding() override;
    bool next() override;
    void analyze(int indent = 0) const override;

private:
    std::unique_ptr<BindingIter> child;
    std::size_t binding_size;
    BindingOrderBy my_binding;
    FileId first_file_id;
    FileId second_file_id;

    std::unique_ptr<TupleCollection> run;
    std::unique_ptr<MergeOrderedTupleCollection> merger;
    FileId* output_file_id;
    uint_fast32_t total_pages = 0;
    uint_fast32_t current_page = 0;
    uint64_t page_position = 0;

    void merge_sort();
};

template class std::unique_ptr<OrderBy>;

#endif // RELATIONAL_MODEL__ORDER_BY_H_
