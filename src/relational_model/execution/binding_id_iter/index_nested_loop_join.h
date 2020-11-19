#ifndef RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
#define RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_

#include <memory>
#include <vector>

#include "base/ids/var_id.h"
#include "base/binding/binding_id_iter.h"

class IndexNestedLoopJoin : public BindingIdIter {
public:
    IndexNestedLoopJoin(std::size_t binding_size,
                        std::unique_ptr<BindingIdIter> lhs,
                        std::unique_ptr<BindingIdIter> rhs);
    ~IndexNestedLoopJoin() = default;

    void analyze(int indent = 0) const override;
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;

private:
    std::unique_ptr<BindingIdIter> lhs;
    std::unique_ptr<BindingIdIter> rhs;

    BindingId* current_left;
    BindingId* current_right;
};

template class std::unique_ptr<IndexNestedLoopJoin>;

#endif // RELATIONAL_MODEL__INDEX_NESTED_LOOP_JOIN_H_
