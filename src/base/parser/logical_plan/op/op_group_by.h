#ifndef BASE__OP_GROUP_BY_H_
#define BASE__OP_GROUP_BY_H_

#include <string>
#include <vector>

#include "base/parser/logical_plan/op/op.h"
#include "base/parser/grammar/query/query_ast.h"

class OpGroupBy : public Op {
public:
    const std::unique_ptr<Op> op;
    std::vector<query::ast::SelectItem> items;
    std::vector<bool> ascending_order;

    OpGroupBy(std::unique_ptr<Op> op, const std::vector<query::ast::OrderedSelectItem>& ordered_items) :
        op (std::move(op))
    {
        for (auto& order_item : ordered_items) {
            items.push_back(order_item.item);
            ascending_order.push_back(order_item.order == query::ast::Order::Ascending);
        }
    }

    ~OpGroupBy() = default;

    void accept_visitor(OpVisitor& visitor) const override {
        visitor.visit(*this);
    }
};

#endif // BASE__OP_GROUP_BY_H_