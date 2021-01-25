#ifndef BASE__OP_GROUP_BY_H_
#define BASE__OP_GROUP_BY_H_

#include <string>
#include <vector>

#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/logical_plan/op/op.h"

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

    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpGroupBy()";
        bool first = true;
        for (auto & item : items) {
            if (!first) os << ", ";
            first = false;
            if (item.key) {
                os << item.var << "." << item.key.get();
            } else {
                os << item.var;
            }
        }
        os << ")\n";
        return op->print_to_ostream(os, indent + 2);
    }

    std::set<std::string> get_var_names() const override {
        // TODO: should add properties mentioned in the GROUP BY that are not present in the MATCH?
        return op->get_var_names();
    }
};

#endif // BASE__OP_GROUP_BY_H_