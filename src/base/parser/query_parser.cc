#include "query_parser.h"

#include <iostream>

#include "base/parser/grammar/query/query_grammar.h"
#include "base/parser/grammar/query/query_ast_printer.h"
#include "base/parser/grammar/manual_plan/manual_plan_grammar.h"
#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_filter.h"
#include "base/parser/logical_plan/op/op_match.h"
#include "base/parser/logical_plan/op/op_select.h"
#include "base/parser/logical_plan/op/visitors/check_var_names.h"

using namespace std;

unique_ptr<OpSelect> QueryParser::get_query_plan(query::ast::Root& ast) {
    unique_ptr<Op> op_match = make_unique<OpMatch>(ast.graph_pattern);
    unique_ptr<Op> op_filter = make_unique<OpFilter>(ast.where, move(op_match));
    int_fast32_t limit = 0;
    if (ast.limit) {
        limit = ast.limit.get();
        if (limit <= 0) {
            throw QuerySemanticException("LIMIT must be a positive number");
        }
    }
    if (ast.selection.type() == typeid(query::ast::All)) {
        return make_unique<OpSelect>(move(op_filter), limit);
    } else {
        return make_unique<OpSelect>(boost::get<std::vector<query::ast::Element>>(ast.selection),
                                     move(op_filter), limit);
    }
}


unique_ptr<OpSelect> QueryParser::get_query_plan(string query) {
    auto iter = query.begin();
    auto end = query.end();

    query::ast::Root ast;
    bool r = phrase_parse(iter, end, query::parser::root, query::parser::skipper, ast);
    if (r && iter == end) { // parsing succeeded
        if (ast.explain) {
            QueryAstPrinter printer(cout);
            printer(ast);
        }
        auto res = QueryParser::get_query_plan(ast);
        check_query_plan(*res);
        return res;
    } else {
        throw QueryParsingException();
    }
}


void QueryParser::check_query_plan(OpSelect& op_select) {
    auto check_var_names = CheckVarNames();
    check_var_names.visit(op_select);
}


manual_plan_ast::Root QueryParser::get_manual_plan(std::string query) {
    cout << "ManualPlan:\n" << query << "\n";
    auto iter = query.begin();
    auto end = query.end();

    manual_plan_ast::Root manual_plan;
    bool r = phrase_parse(iter, end, manual_plan::parser::root, manual_plan::parser::skipper, manual_plan);
    if (r && iter == end) { // parsing succeeded
        return manual_plan;
    } else {
        cout << "ManualPlan failed\n";
        throw QueryParsingException();
    }
}
