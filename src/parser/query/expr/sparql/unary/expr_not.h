#pragma once

#include <set>
#include <memory>

#include "parser/query/expr/expr.h"

namespace SPARQL {
class ExprNot : public Expr {
public:
    std::unique_ptr<Expr> expr;

    ExprNot(std::unique_ptr<Expr> expr) :
        expr (std::move(expr)) { }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::set<Var> get_vars() const override {
        return expr->get_vars();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "NOT(" << *expr << ')';
    }
};
} // namespace SPARQL
