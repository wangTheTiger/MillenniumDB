#ifndef BASE__PROPERTY_PATH_PARSER_H
#define BASE__PROPERTY_PATH_PARSER_H

#include "base/parser/grammar/query/query_ast.h"
#include "base/parser/logical_plan/op/op_path.h"

class PropertyPathParser {
public:
    PropertyPathParser() = default;
    ~PropertyPathParser() = default;

    std::unique_ptr<OpPath> operator()(query::ast::PropertyPathAlternatives& p , bool inverse = false);
    std::unique_ptr<OpPath> operator()(query::ast::PropertyPathSequence& p, bool inverse = false);
    std::unique_ptr<OpPath> operator()(query::ast::PropertyPathAtom& p, bool inverse = false);
};

#endif // BASE__PROPERTY_PATH_PARSER_H