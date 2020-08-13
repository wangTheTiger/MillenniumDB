#ifndef BASE__COMMON_GRAMMAR_H_
#define BASE__COMMON_GRAMMAR_H_

#include <string>
#include <boost/spirit/home/x3.hpp>

#include "base/parser/grammar/common/common_ast.h"
#include "base/parser/grammar/common/common_ast_adapted.h"

namespace common_parser {
    namespace x3 = boost::spirit::x3;

    using x3::attr;
    using x3::alpha;
    using x3::alnum;
    using x3::char_;
    using x3::eol;
    using x3::eoi;
    using x3::int64;
    using x3::lexeme;
    using x3::lit;
    using x3::no_case;
    using x3::space;

    x3::real_parser<float, x3::strict_real_policies<float>> const float_ = { };

    auto const line_skipper = "//" >> *(char_ - eol) >> (eol | eoi);
    auto const skipper = space | line_skipper;

    // Rules
    x3::rule<class var, ast::Var>
        var = "var";
    x3::rule<class value, ast::Value>
        value = "value";
    x3::rule<class property, ast::Property>
        property = "property";
    x3::rule<class var_name, std::string>
        var_name = "var_name";


    // Grammar
    auto const var_name_def =
        alpha >> *(alnum | char_('_'));

    auto const var_def =
        lexeme['?' >> var_name];

    auto const key =
        lexeme[+char_("A-Za-zÁÉÍÓÚáéíóúÑñèç0-9#'_")];

    auto const label =
        lexeme[':' >> +char_("A-Za-zÁÉÍÓÚáéíóúÑñèç0-9#'_")];

    auto const boolean =
        lexeme[no_case["true"]]  >> attr(true) |
        lexeme[no_case["false"]] >> attr(false);

    auto const escaped =
        '\\' >> (
            "n"  >> attr('\n') |
            "t"  >> attr('\t') |
            "b"  >> attr('\b') |
            "f"  >> attr('\f') |
            "r"  >> attr('\r') |
            "/"  >> attr('/')  |
            "\\" >> attr('\\') |
            "\"" >> attr('\"')
        );

    auto const string =
        lexeme['"' >> *(escaped | ~char_('"')) >> '"'];

    auto const value_def =
        string | float_ | int64 | boolean;

    auto const property_def =
        key >> ':' >> value;

    BOOST_SPIRIT_DEFINE(
        var,
        value,
        property,
        var_name
    );
}

#endif // BASE__COMMON_GRAMMAR_H_
