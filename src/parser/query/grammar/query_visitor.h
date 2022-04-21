#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "antlr4-runtime.h"
#include "parser/query/grammar/autogenerated/MDBParserBaseVisitor.h"
#include "parser/query/paths/path.h"
#include "parser/query/paths/path_alternatives.h"
#include "parser/query/paths/path_atom.h"
#include "parser/query/paths/path_kleene_star.h"
#include "parser/query/paths/path_optional.h"
#include "parser/query/paths/path_sequence.h"
#include "parser/query/op/ops.h"
#include "parser/query/return_item/return_item_agg.h"
#include "parser/query/return_item/return_item_count.h"
#include "parser/query/return_item/return_item_var.h"
#include "parser/query/expr/exprs.h"

class QueryVisitor : public MDBParserBaseVisitor {
private:
    int anon_var_counter = 0;

    std::vector<std::pair<Var, QueryElement>> set_items;

    std::vector<std::unique_ptr<ReturnItem>> return_items;

    std::vector<std::unique_ptr<ReturnItem>> order_by_items;

    std::vector<std::unique_ptr<ReturnItem>> group_by_items;

    std::vector<bool> order_by_ascending_order;

    std::unique_ptr<Expr> current_expr;

    std::unique_ptr<OpBasicGraphPattern> current_basic_graph_pattern;

    std::set<Var> possible_isolated_vars;

    std::unique_ptr<IPath> current_path;

    // to detect possible isolated vars / terms
    // initialized false to avoid calling
    // current_basic_graph_pattern->add_isolated_term (segfault)
    // when seeing a DESCRIBE query
    bool first_element_isolated = false;

    bool current_path_inverse;

    std::string last_node_id;
    std::string saved_node_id;
    std::string saved_edge_id;
    std::string saved_type_id;

public:
    std::unique_ptr<Op> current_op;

    virtual antlrcpp::Any visitRoot(MDBParser::RootContext* ctx) override {
        visitChildren(ctx);
        if (set_items.size() > 0) {
            current_op = std::make_unique<OpSet>(std::move(current_op), std::move(set_items));
        }
        return 0;
    }

    virtual antlrcpp::Any visitDescribeStatement(MDBParser::DescribeStatementContext* ctx) override {
        visitChildren(ctx);
        current_op = std::make_unique<OpDescribe>(last_node_id);
        return 0;
    }

    virtual antlrcpp::Any visitMatchStatement(MDBParser::MatchStatementContext* ctx) override {
        visitChildren(ctx);
        current_op = std::make_unique<OpMatch>(std::move(current_op));
        return 0;
    }

    virtual antlrcpp::Any visitSetItem(MDBParser::SetItemContext* ctx) override {
        set_items.push_back({ Var(ctx->VARIABLE()->getText()),
                              QueryElement::deduce(ctx->fixedNodeInside()->getText())});
        return 0;
    }

    /* Return */
    virtual antlrcpp::Any visitReturnList(MDBParser::ReturnListContext* ctx) override {
        for (auto& return_expr : ctx->returnItem()) {
            return_expr->accept(this);
        }

        uint64_t limit = OpReturn::DEFAULT_LIMIT;
        if (ctx->UNSIGNED_INTEGER() != nullptr) {
            limit = stoll(ctx->UNSIGNED_INTEGER()->getText());
        }

        current_op = std::make_unique<OpReturn>(std::move(current_op),
                                                std::move(return_items),
                                                ctx->K_DISTINCT() != nullptr,
                                                limit);
        return 0;
    }

    virtual antlrcpp::Any visitReturnItemVar(MDBParser::ReturnItemVarContext* ctx) override {
        return_items.push_back(std::make_unique<ReturnItemVar>(Var(ctx->getText())));
        return 0;
    }

    virtual antlrcpp::Any visitReturnItemAgg(MDBParser::ReturnItemAggContext* ctx) override {
        auto var = ctx->VARIABLE()->getText();
        if (ctx->KEY() != nullptr) {
            var += ctx->KEY()->getText();
        }
        return_items.push_back(
            std::make_unique<ReturnItemAgg>(asciistrtolower(ctx->aggregateFunc()->getText()), std::move(var))
        );
        return 0;
    }

    virtual antlrcpp::Any visitReturnItemCount(MDBParser::ReturnItemCountContext* ctx) override {
        std::string inside_var;
        if (ctx->VARIABLE() != nullptr) {
            inside_var = ctx->VARIABLE()->getText();
            if (ctx->KEY() != nullptr) {
                inside_var += ctx->KEY()->getText();
            }
        } else {
            inside_var = "*";
        }
        return_items.push_back(std::make_unique<ReturnItemCount>(ctx->K_DISTINCT() != nullptr,
                                                                 std::move(inside_var)));
        return 0;
    }

    virtual antlrcpp::Any visitReturnAll(MDBParser::ReturnAllContext* ctx) override {
        auto vars = current_op->get_vars();
        for (auto& var : vars) {
            // filter anonymous vars
            if (var.name[1] != '_') {
                return_items.push_back(std::make_unique<ReturnItemVar>(var));
            }
        }
        uint64_t limit = OpReturn::DEFAULT_LIMIT;
        if (ctx->UNSIGNED_INTEGER() != nullptr) {
            limit = stoll(ctx->UNSIGNED_INTEGER()->getText());
        }

        current_op = std::make_unique<OpReturn>(std::move(current_op),
                                                std::move(return_items),
                                                ctx->K_DISTINCT() != nullptr,
                                                limit);
        return visitChildren(ctx);
    }
    /* End Return */

    /* Order By */
    virtual antlrcpp::Any visitOrderByStatement(MDBParser::OrderByStatementContext* ctx) override {
        for (auto order_by_item : ctx->orderByItem()) {
            order_by_item->accept(this);
        }

        current_op = std::make_unique<OpOrderBy>(std::move(current_op),
                                                 std::move(order_by_items),
                                                 std::move(order_by_ascending_order));
        return 0;
    }

    virtual antlrcpp::Any visitOrderByItemVar(MDBParser::OrderByItemVarContext*ctx) override {
        auto var = ctx->VARIABLE()->getText();
        if (ctx->KEY() != nullptr) {
            var += ctx->KEY()->getText();
        }
        order_by_items.push_back(std::make_unique<ReturnItemVar>(Var(var)));
        order_by_ascending_order.push_back(ctx->K_DESC() == nullptr);
        return 0;
    }

    virtual antlrcpp::Any visitOrderByItemAgg(MDBParser::OrderByItemAggContext* ctx) override {
        auto var = ctx->VARIABLE()->getText();
        if (ctx->KEY() != nullptr) {
            var += ctx->KEY()->getText();
        }
        order_by_items.push_back(std::make_unique<ReturnItemAgg>(asciistrtolower(ctx->aggregateFunc()->getText()), std::move(var)));
        order_by_ascending_order.push_back(ctx->K_DESC() == nullptr);
        return 0;
    }

    virtual antlrcpp::Any visitOrderByItemCount(MDBParser::OrderByItemCountContext* ctx) override {
        auto var = ctx->VARIABLE()->getText();
        if (ctx->KEY() != nullptr) {
            var += ctx->KEY()->getText();
        }
        order_by_items.push_back(std::make_unique<ReturnItemCount>(ctx->K_DISTINCT() != nullptr, std::move(var)));
        order_by_ascending_order.push_back(ctx->K_DESC() == nullptr);
        return 0;
    }
    /* End Order By */

    /* Group By */
    virtual antlrcpp::Any visitGroupByStatement(MDBParser::GroupByStatementContext* ctx) override {
        for (auto group_by_item : ctx->groupByItem()) {
            group_by_item->accept(this);
        }

        current_op = std::make_unique<OpGroupBy>(std::move(current_op),
                                                  std::move(group_by_items));
        return 0;
    }

    virtual antlrcpp::Any visitGroupByItem(MDBParser::GroupByItemContext* ctx) override {
        auto var = ctx->VARIABLE()->getText();
        if (ctx->KEY() != nullptr) {
            var += ctx->KEY()->getText();
        }
        group_by_items.push_back(std::make_unique<ReturnItemVar>(Var(var)));
        return 0;
    }
    /* End Group By */

    /* Graph Pattern */
    virtual antlrcpp::Any visitGraphPattern(MDBParser::GraphPatternContext* ctx) override {
        ctx->basicPattern()->accept(this);
        auto parent = std::move(current_basic_graph_pattern);
        if (ctx->optionalPattern().size() > 0) {
            std::vector<std::unique_ptr<Op>> optional_children;
            for (auto& opt : ctx->optionalPattern()) {
                opt->accept(this);
                optional_children.push_back(std::move(current_op));
            }
            current_op = std::make_unique<OpOptional>(std::move(parent), std::move(optional_children));
        } else {
            current_op = std::move(parent);
        }
        return 0;
    }

    virtual antlrcpp::Any visitBasicPattern(MDBParser::BasicPatternContext* ctx) override {
        current_basic_graph_pattern = std::make_unique<OpBasicGraphPattern>();
        visitChildren(ctx);
        for (auto& pending_unjoint_var : possible_isolated_vars) {
            current_basic_graph_pattern->try_add_possible_isolated_var(pending_unjoint_var);
        }
        return 0;
    }

    virtual antlrcpp::Any visitLinearPattern(MDBParser::LinearPatternContext* ctx) override {
        first_element_isolated = ctx->children.size() == 1;
        ctx->children[0]->accept(this);
        saved_node_id = last_node_id;
        for (size_t i = 2; i < ctx->children.size(); i += 2) {
            ctx->children[i]->accept(this);     // accept node
            ctx->children[i - 1]->accept(this); // accept edge or path
            saved_node_id = last_node_id;
        }

        return 0;
    }

    virtual antlrcpp::Any visitFixedNodeInside(MDBParser::FixedNodeInsideContext* ctx) override {
        last_node_id = ctx->getText();
        if (first_element_isolated) {
            current_basic_graph_pattern->add_isolated_term(last_node_id);
        }
        return 0;
    }

    virtual antlrcpp::Any visitVarNode(MDBParser::VarNodeContext* ctx) override {
        std::string id;

        // Get VarName
        auto var = ctx->VARIABLE();
        if (var != nullptr) {
            id = var->getText();
            if (first_element_isolated && ctx->TYPE().empty() && ctx->properties() == nullptr) {
                possible_isolated_vars.insert(Var(id));
            }
        } else {
            id = "?_" + std::to_string(anon_var_counter++);
            if (first_element_isolated && ctx->TYPE().empty() && ctx->properties() == nullptr) {
                // we know for sure is an isolated var
                current_basic_graph_pattern->add_isolated_var(OpIsolatedVar(Var(id)));
            }
        }

        // Process Labels
        for (auto& label : ctx->TYPE()) {
            current_basic_graph_pattern->add_label(OpLabel(id, label->getText()));
        }

        // Process Properties
        auto properties = ctx->properties();
        if (properties != nullptr) {
            for (auto property : properties->property()) {
                std::string key = property->identifier()->getText();
                std::string val;

                if (property->value() == nullptr) {
                    if (property->FALSE_PROP() != nullptr) {
                        val = "false";
                    }
                    if (property->TRUE_PROP() != nullptr) {
                        val = "true";
                    }
                } else {
                    val = property->value()->getText();
                }
                current_basic_graph_pattern->add_property(OpProperty(id, key, val));
            }
        }

        last_node_id = std::move(id);
        return 0;
    }

    virtual antlrcpp::Any visitEdge(MDBParser::EdgeContext* ctx) override {
        if (ctx->edgeInside() == nullptr) {
            saved_edge_id = "?_" + std::to_string(anon_var_counter++);
            saved_type_id = "?_" + std::to_string(anon_var_counter++);
        }
        visitChildren(ctx);
        if (ctx->GT() != nullptr) {
            // right direction
            current_basic_graph_pattern->add_edge(OpEdge(saved_node_id, last_node_id, saved_type_id, saved_edge_id));
        } else {
            // left direction
            current_basic_graph_pattern->add_edge(OpEdge(last_node_id, saved_node_id, saved_type_id, saved_edge_id));
        }
        return 0;
    }

    virtual antlrcpp::Any visitEdgeInside(MDBParser::EdgeInsideContext* ctx) override {
        if (ctx->VARIABLE() != nullptr) {
            saved_edge_id = ctx->VARIABLE()->getText();
        } else if (ctx->EDGE_ID()) {
            saved_edge_id = ctx->EDGE_ID()->getText();
        } else {
            saved_edge_id = "?_" + std::to_string(anon_var_counter++);
        }

        if (ctx->TYPE_VAR() != nullptr) {
            auto tmp      = ctx->TYPE_VAR()->getText();
            saved_type_id = tmp.substr(1, tmp.size() - 1); // remove ':'
        } else if (ctx->TYPE() != nullptr) {
            auto tmp      = ctx->TYPE()->getText();
            saved_type_id = tmp.substr(1, tmp.size() - 1); // remove ':'
        } else {
            saved_type_id = "?_" + std::to_string(anon_var_counter++);
        }

        auto properties = ctx->properties();
        if (properties != nullptr) {
            for (auto property : properties->property()) {
                std::string key = property->identifier()->getText();
                std::string val;

                if (property->value() == nullptr) {
                    if (property->FALSE_PROP() != nullptr) {
                        val = "false";
                    }
                    if (property->TRUE_PROP() != nullptr) {
                        val = "true";
                    }
                } else {
                    val = property->value()->getText();
                }
                current_basic_graph_pattern->add_property(OpProperty(saved_edge_id, key, val));
            }
        }
        return visitChildren(ctx);
    }
    /* End Graph Pattern */

    virtual antlrcpp::Any visitPath(MDBParser::PathContext* ctx) override {
        std::string path_var;
        if (ctx->VARIABLE() == nullptr) {
            path_var = "?_" + std::to_string(anon_var_counter++);
        } else {
            path_var = ctx->VARIABLE()->getText();
        }

        PathSemantic semantic = PathSemantic::ALL;
        if (ctx->pathType() == nullptr) {
            semantic = PathSemantic::ANY;
        } else {
            auto semantic_str = ctx->pathType()->getText();
            std::transform(semantic_str.begin(), semantic_str.end(), semantic_str.begin(), asciitolower);
            if (semantic_str == "any") {
                semantic = PathSemantic::ANY;
            }
        }

        current_path_inverse = false;
        ctx->pathAlternatives()->accept(this);
        if (ctx->GT() != nullptr) {
            // right direction
            current_basic_graph_pattern->add_path(
                OpPath(path_var, saved_node_id, last_node_id, semantic, move(current_path)));
        } else {
            // left direction
            current_basic_graph_pattern->add_path(
                OpPath(path_var, last_node_id, saved_node_id, semantic, move(current_path)));
        }
        return 0;
    }

    virtual antlrcpp::Any visitPathAlternatives(MDBParser::PathAlternativesContext* ctx) override {
        if (ctx->pathSequence().size() > 1) {
            std::vector<std::unique_ptr<IPath>> alternatives;
            for (auto s : ctx->pathSequence()) {
                s->accept(this);
                alternatives.push_back(move(current_path));
            }
            current_path = std::make_unique<PathAlternatives>(move(alternatives));
        } else {
            ctx->pathSequence()[0]->accept(this);
        }
        return 0;
    }

    virtual antlrcpp::Any visitPathSequence(MDBParser::PathSequenceContext* ctx) override {
        if (ctx->pathAtom().size() > 1) {
            std::vector<std::unique_ptr<IPath>> sequence;
            if (current_path_inverse) {
                for (int i = ctx->pathAtom().size() - 1; i >= 0; i--) {
                    ctx->pathAtom()[i]->accept(this);
                    sequence.push_back(move(current_path));
                }
            } else {
                for (auto a : ctx->pathAtom()) {
                    a->accept(this);
                    sequence.push_back(move(current_path));
                }
            }
            current_path = std::make_unique<PathSequence>(move(sequence));
        } else {
            ctx->pathAtom()[0]->accept(this);
        }
        return 0;
    }

    virtual antlrcpp::Any visitPathAtomSimple(MDBParser::PathAtomSimpleContext* ctx) override {
        std::string tmp  = ctx->TYPE()->getText();
        std::string type = tmp.substr(1, tmp.size() - 1); // remove ':'

        bool inverse = (ctx->children[0]->getText() == "^") ^ current_path_inverse;

        current_path = std::make_unique<PathAtom>(type, inverse);

        auto suffix = ctx->pathSuffix();
        if (suffix == nullptr) {
            // no suffix
            return 0;
        } else if (suffix->op == nullptr) {
            // {min, max}
            std::vector<std::unique_ptr<IPath>> seq;
            unsigned min = std::stoul(suffix->min->getText());
            unsigned max = std::stoul(suffix->max->getText());
            unsigned i = 0;
            for (; i < min; i++) {
                seq.push_back(current_path->duplicate());
            }
            for (; i < max; i++) {
                seq.push_back(std::make_unique<PathOptional>(current_path->duplicate()));
            }
            current_path = std::make_unique<PathSequence>(move(seq));
        } else if (suffix->op->getText() == "*") {
            // kleene star
            current_path = std::make_unique<PathKleeneStar>(move(current_path));

        } else if (suffix->op->getText() == "+") {
            // A+ => A / A*
            auto kleene_star = std::make_unique<PathKleeneStar>(current_path->duplicate());
            std::vector<std::unique_ptr<IPath>> seq;
            seq.push_back(move(current_path));
            seq.push_back(move(kleene_star));
            current_path = std::make_unique<PathSequence>(move(seq));

        } else if (suffix->op->getText() == "?") {
            if (!current_path->nullable()) {
                current_path = std::make_unique<PathOptional>(move(current_path));
            }
            // else we avoid a redundant PathOptional, current_path stays the same
        }
        return 0;
    }

    virtual antlrcpp::Any visitPathAtomAlternatives(MDBParser::PathAtomAlternativesContext* ctx) override {
        auto previous_current_path_inverse = current_path_inverse;

        current_path_inverse = (ctx->children[0]->getText() == "^") ^ current_path_inverse;
        ctx->pathAlternatives()->accept(this);
        current_path_inverse = previous_current_path_inverse;

        auto suffix = ctx->pathSuffix();
        if (suffix == nullptr) {
            // no suffix
        } else if (suffix->op == nullptr) {
            // {min, max}
            std::vector<std::unique_ptr<IPath>> seq;
            unsigned min = std::stoul(suffix->min->getText());
            unsigned max = std::stoul(suffix->max->getText());
            unsigned i = 0;
            for (; i < min; i++) {
                seq.push_back(current_path->duplicate());
            }
            for (; i < max; i++) {
                seq.push_back(std::make_unique<PathOptional>(current_path->duplicate()));
            }
            current_path = std::make_unique<PathSequence>(move(seq));
        } else if (suffix->op->getText() == "*") {
            // kleene star
            current_path = std::make_unique<PathKleeneStar>(move(current_path));

        } else if (suffix->op->getText() == "+") {
            // A+ => A / A*
            auto kleene_star = std::make_unique<PathKleeneStar>(current_path->duplicate());
            std::vector<std::unique_ptr<IPath>> seq;
            seq.push_back(move(current_path));
            seq.push_back(move(kleene_star));
            current_path = std::make_unique<PathSequence>(move(seq));

        } else if (suffix->op->getText() == "?") {
            if (!current_path->nullable()) {
                current_path = std::make_unique<PathOptional>(move(current_path));
            }
            // else we avoid a redundant PathOptional, current_path stays the same
        }
        return 0;
    }
    /* End Paths */

    /* Expressions */
    virtual antlrcpp::Any visitWhereStatement(MDBParser::WhereStatementContext* ctx) override {
        ctx->conditionalOrExpr()->accept(this);
        current_op = std::make_unique<OpWhere>(std::move(current_op), std::move(current_expr));
        return 0;
    }

    virtual antlrcpp::Any visitExprVar(MDBParser::ExprVarContext* ctx) override {
        auto var = ctx->VARIABLE()->getText();
        if (ctx->KEY() != nullptr) {
            auto key = ctx->KEY()->getText();
            auto property_var = var + key;
            key.erase(0, 1); // ".key" to "key"
            current_expr = std::make_unique<ExprVarProperty>(var, property_var, key);
        } else {
            current_expr = std::make_unique<ExprVar>(var);
        }
        return 0;
    }

    virtual antlrcpp::Any visitExprValueExpr(MDBParser::ExprValueExprContext* ctx) override {
        current_expr = std::make_unique<ExprConstant>(ctx->getText());
        return 0;
    }

    virtual antlrcpp::Any visitConditionalOrExpr(MDBParser::ConditionalOrExprContext* ctx) override {
        ctx->conditionalAndExpr()[0]->accept(this);

        if (ctx->conditionalAndExpr().size() > 1) {
            std::vector<std::unique_ptr<Expr>> or_list;
            or_list.push_back(std::move(current_expr));
            for (std::size_t i = 1; i < ctx->conditionalAndExpr().size(); i++) {
                ctx->conditionalAndExpr()[i]->accept(this);
                or_list.push_back(std::move(current_expr));
            }
            current_expr = std::make_unique<ExprOr>(std::move(or_list));
        }
        return 0;
    }

    virtual antlrcpp::Any visitConditionalAndExpr(MDBParser::ConditionalAndExprContext* ctx) override {
        ctx->comparisonExpr()[0]->accept(this);

        if (ctx->comparisonExpr().size() > 1) {
            std::vector<std::unique_ptr<Expr>> and_list;
            and_list.push_back(std::move(current_expr));
            for (std::size_t i = 1; i < ctx->comparisonExpr().size(); i++) {
                ctx->comparisonExpr()[i]->accept(this);
                and_list.push_back(std::move(current_expr));
            }
            current_expr = std::make_unique<ExprAnd>(std::move(and_list));
        }
        return 0;
    }

    virtual antlrcpp::Any visitComparisonExprOp(MDBParser::ComparisonExprOpContext* ctx) override {
        ctx->aditiveExpr()[0]->accept(this);
        if (ctx->aditiveExpr().size() > 1) {
            auto saved_lhs = std::move(current_expr);
            ctx->aditiveExpr()[1]->accept(this);
            auto op = ctx->op->getText();
            if (op == "==") {
                current_expr = std::make_unique<ExprEquals>(std::move(saved_lhs), std::move(current_expr));
            } else if (op == "!=") {
                current_expr = std::make_unique<ExprNotEquals>(std::move(saved_lhs), std::move(current_expr));
            }  else if (op == "<") {
                current_expr = std::make_unique<ExprLess>(std::move(saved_lhs), std::move(current_expr));
            } else if (op == "<=") {
                current_expr = std::make_unique<ExprLessOrEquals>(std::move(saved_lhs), std::move(current_expr));
            } else if (op == ">=") {
                current_expr = std::make_unique<ExprGreaterOrEquals>(std::move(saved_lhs), std::move(current_expr));
            } else if (op == ">") {
                current_expr = std::make_unique<ExprGreater>(std::move(saved_lhs), std::move(current_expr));
            } else {
                throw std::invalid_argument(op + " not recognized as a valid ComparisonExpr operator");

            }
        }
        return 0;
    }

    virtual antlrcpp::Any visitComparisonExprIs(MDBParser::ComparisonExprIsContext* ctx) override {
        ctx->aditiveExpr()->accept(this);
        current_expr = std::make_unique<ExprIs>(ctx->K_NOT() != nullptr,
                                                std::move(current_expr),
                                                asciistrtolower(ctx->exprTypename()->getText()));
        return 0;
    }

    virtual antlrcpp::Any visitAditiveExpr(MDBParser::AditiveExprContext* ctx) override {
        auto multiplicativeExprs = ctx->multiplicativeExpr();
        multiplicativeExprs[0]->accept(this);
        for (std::size_t i = 1; i < multiplicativeExprs.size(); i++) {
            auto saved_lhs = std::move(current_expr);
            multiplicativeExprs[i]->accept(this);
            auto op = ctx->op[i - 1]->getText();
            if (op == "+") {
                current_expr = std::make_unique<ExprAddition>(std::move(saved_lhs), std::move(current_expr));
            } else if (op == "-") {
                current_expr = std::make_unique<ExprSubtraction>(std::move(saved_lhs), std::move(current_expr));
            }  else {
                throw std::invalid_argument(op + " not recognized as a valid AditiveExpr operator");
            }
        }
        return 0;
    }

    virtual antlrcpp::Any visitMultiplicativeExpr(MDBParser::MultiplicativeExprContext* ctx) override {
        auto unaryExprs = ctx->unaryExpr();
        unaryExprs[0]->accept(this);
        for (std::size_t i = 1; i < unaryExprs.size(); i++) {
            auto saved_lhs = std::move(current_expr);
            unaryExprs[i]->accept(this);
            auto op = ctx->op[i - 1]->getText();
            if (op == "*") {
                current_expr = std::make_unique<ExprMultiplication>(std::move(saved_lhs), std::move(current_expr));
            } else if (op == "/") {
                current_expr = std::make_unique<ExprDivision>(std::move(saved_lhs), std::move(current_expr));
            } else if (op == "|") {
                current_expr = std::make_unique<ExprModulo>(std::move(saved_lhs), std::move(current_expr));
            } else {
                throw std::invalid_argument(op + " not recognized as a valid MultiplicativeExpr operator");
            }
        }
        return 0;
    }

    virtual antlrcpp::Any visitUnaryExpr(MDBParser::UnaryExprContext* ctx) override {
        if (ctx->unaryExpr() != nullptr) {
            ctx->unaryExpr()->accept(this);
            if (ctx->K_NOT() != nullptr) {
                current_expr = std::make_unique<ExprNot>(std::move(current_expr));
            } else if (ctx->children[0]->getText() == "+") {
                current_expr = std::make_unique<ExprUnaryPlus>(std::move(current_expr));
            } else if (ctx->children[0]->getText() == "-") {
                current_expr = std::make_unique<ExprUnaryMinus>(std::move(current_expr));
            } else {
                throw std::invalid_argument(ctx->children[0]->getText() + " not recognized as a valid UnaryExpr operator");
            }
        } else {
            ctx->atomicExpr()->accept(this);
        }
        return 0;
    }
    /* End Expressions */

    static char asciitolower(char c) {
        if (c <= 'Z' && c >= 'A')
            return c - ('Z' - 'z');
        return c;
    }

    static std::string asciistrtolower(const std::string& str) {
        std::string res;
        for (std::size_t i = 0; i < str.size(); i++) {
            res += asciitolower(str[i]);
        }
        return res;
    }
};
