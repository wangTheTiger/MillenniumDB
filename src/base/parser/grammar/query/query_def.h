#ifndef BASE__GRAMMAR_H_
#define BASE__GRAMMAR_H_

#include <boost/spirit/home/x3.hpp>

#include "query_ast.h"
#include "query_ast_adapted.h"
#include "query.h"
#include "base/parser/grammar/common/common_def.h"

namespace query {
    namespace parser {
        namespace x3 = boost::spirit::x3;
        using namespace common::parser;

        using x3::uint32;

        // Declare rules
        x3::rule<class root, ast::Root>
            root = "root";

        x3::rule<class select_item, ast::SelectItem>
            select_item = "select_item";
        x3::rule<class select_items, std::vector<ast::SelectItem>>
            select_items = "select_items";
        x3::rule<class select_statement, ast::SelectStatement>
            select_statement = "select_statement";

        x3::rule<class graph_pattern, ast::GraphPattern>
            graph_pattern = "graph_pattern";
        x3::rule<class optional_pattern, ast::GraphPattern>
            optional_pattern = "optional_pattern";
        x3::rule<class linear_pattern, ast::LinearPattern>
            linear_pattern = "linear_pattern";

        x3::rule<class node, ast::Node>
            node = "node";
        x3::rule<class edge, ast::Edge>
            edge = "edge";

        // PROPERTY PATHS
        x3::rule<class property_path, ast::PropertyPath>
            property_path = "property_path";
        x3::rule<class property_path_atom, ast::PropertyPathAtom>
            property_path_atom = "property_path_atom";
        x3::rule<class property_path_alternatives, ast::PropertyPathAlternatives>
            property_path_alternatives = "property_path_alternatives";
        x3::rule<class property_path_sequence, ast::PropertyPathSequence>
            property_path_sequence = "property_path_sequence";
        x3::rule<class property_path_bound_suffix, ast::PropertyPathBoundSuffix>
            property_path_bound_suffix = "property_path_bound_suffix";

        // WHERE CONDITIONS
        x3::rule<class condition, ast::Condition>
            condition = "condition";
        x3::rule<class statement, ast::Statement>
            statement = "statement";
        x3::rule<class formula, ast::Formula>
            formula = "formula";
        x3::rule<class step_formula, ast::StepFormula>
            step_formula = "step_formula";

        x3::rule<class order, ast::Order>
            order = "order";
        x3::rule<class ordered_select_item, ast::OrderedSelectItem>
            ordered_select_item = "ordered_select_item";
        x3::rule<class ordered_select_items, std::vector<ast::OrderedSelectItem>>
            ordered_select_items = "ordered_select_items";


        auto const comparator =
            lit("==") >> attr(ast::Comparator::EQ) |
            lit("<=") >> attr(ast::Comparator::LE) |
            lit(">=") >> attr(ast::Comparator::GE) |
            lit("!=") >> attr(ast::Comparator::NE) |
            lit('<')  >> attr(ast::Comparator::LT) |
            lit('>')  >> attr(ast::Comparator::GT);

        auto const connector =
            lexeme[no_case["or"]]  >> attr(ast::BinaryOp::Or) |
            lexeme[no_case["and"]] >> attr(ast::BinaryOp::And);

        auto const node_inside =
            -(var | node_name) >> *label >> -("{" >> -(property % ',') >> "}");

        auto const type =
            lexeme[no_case["=TYPE"]] >> '(' >> (var | node_name) >> ')';

        auto const edge_inside =
            -(var | node_name) >> *(type | label) >> -("{" >> -(property % ',') >> "}");

        auto const node_def =
            '(' >> node_inside >> ")";

        auto const edge_def =
            (-("-[" >> edge_inside >> ']') >> "->" >> attr(ast::EdgeDirection::right)) |
            ("<-" >> -('[' >> edge_inside >> "]-") >> attr(ast::EdgeDirection::left));

        auto const property_path_alternatives_def =
            property_path_sequence % "|";

        auto const property_path_sequence_def =
            property_path_atom % "/";

        auto const property_path_def =
            "=["  >> property_path_alternatives >> "]=>" >> attr(ast::EdgeDirection::right) |
            "<=[" >> property_path_alternatives >> "]="  >> attr(ast::EdgeDirection::left);

        auto const property_path_bound_suffix_def =
            "{" >> uint32 >> "," >> uint32 >> "}";

        auto const property_path_suffix =
            property_path_bound_suffix                              |
            lit("*") >> attr(ast::PropertyPathSuffix::ZERO_OR_MORE) |
            lit('+') >> attr(ast::PropertyPathSuffix::ONE_OR_MORE)  |
            lit('?') >> attr(ast::PropertyPathSuffix::ZERO_OR_ONE)  |
            attr(ast::PropertyPathSuffix::NONE) ;

        auto const property_path_atom_def =
            ( ("^" >> attr(true)) | attr(false) )
            >> ( label | ('(' >> property_path_alternatives >> ')') )
            >> property_path_suffix;

        auto const linear_pattern_def =
            node >> *((edge | property_path) >> node);

        auto const statement_def =
            select_item >> comparator >> (select_item | value);

        auto const condition_def =
            -(no_case["NOT"] >> attr(true)) >>
            (
                statement |
                ('(' >> formula >> ')')
            );

        auto const step_formula_def =
            connector >> condition;

        auto const formula_def =
            condition >> *step_formula;

        auto explain_statement =
            no_case["explain"] >> attr(true) |
            attr(false);

        auto distinct_statement =
            no_case["distinct"] >> attr(true) |
            attr(false);

        auto const select_item_def =
            var >> -("." >> key);

        auto const select_items_def =
            ( lit('*') >> attr(std::vector<ast::SelectItem>()) ) |
            select_item % ',';

        auto const select_statement_def =
            no_case["select"] >> distinct_statement >> select_items;

        auto const match_statement =
            no_case["match"] >> graph_pattern;

        auto const linear_pattern_list =
            linear_pattern % ',';

        auto const optional_pattern_def =
            no_case["optional"] >> '{' >> graph_pattern >> '}';

        auto const graph_pattern_def =
            linear_pattern_list >> *optional_pattern;

        auto const where_statement =
            no_case["where"] >> formula;

        auto const order_def =
            no_case["asc"]  >> -no_case["ending"] >> attr(ast::Order::Ascending)  |
            no_case["desc"] >> -no_case["ending"] >> attr(ast::Order::Descending) |
            attr(ast::Order::Ascending) ;

        auto const ordered_select_item_def =
            select_item >> order;

        auto const ordered_select_items_def =
            ordered_select_item % ',';

        auto const group_by_statement =
            no_case["group by"] >> ordered_select_items;

        auto const order_by_statement =
            no_case["order by"] >> ordered_select_items;

        auto const limit_statement =
            no_case["limit"] >> uint32;

        auto const root_def =
            explain_statement
            >> select_statement
            >> match_statement
            >> -(where_statement)
            >> -(group_by_statement)
            >> -(order_by_statement)
            >> -(limit_statement);

        BOOST_SPIRIT_DEFINE(
            root,
            select_item,
            select_items,
            // selection,
            select_statement,
            node,
            edge,

            property_path,
            property_path_atom,
            property_path_alternatives,
            property_path_sequence,
            property_path_bound_suffix,
            // property_path_atoms,

            graph_pattern,
            optional_pattern,
            linear_pattern,

            statement,
            formula,
            step_formula,
            condition,
            order,
            ordered_select_item,
            ordered_select_items
        );
    }

    parser::query_type query() {
        return parser::root;
    }
}

#endif // BASE__GRAMMAR_H_
