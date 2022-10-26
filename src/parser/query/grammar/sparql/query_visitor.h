#pragma once

#include <memory>
#include <regex>
#include <vector>

#include "antlr4-runtime.h"
#include "base/exceptions.h"
#include "base/graph_object/datetime.h"
#include "base/query/sparql/decimal.h"
#include "base/query/sparql/sparql_element.h"
#include "base/query/sparql/path.h"
#include "base/query/sparql/path_alternatives.h"
#include "base/query/sparql/path_sequence.h"
#include "base/query/sparql/path_atom.h"
#include "base/query/sparql/path_kleene_star.h"
#include "base/query/sparql/path_optional.h"
#include "parser/query/grammar/sparql/autogenerated/SparqlParserBaseVisitor.h"
#include "parser/query/op/sparql/ops.h"
#include "third_party/robin_hood/robin_hood.h"

namespace SPARQL {

class QueryVisitor : public SparqlParserBaseVisitor {
private:
    std::vector<Var>      select_variables;
    std::vector<OpTriple> current_triples;

    std::vector<Var>  order_by_items;
    std::vector<bool> order_by_ascending;
    std::unique_ptr<Var>  order_by_current_expr;

    SparqlElement current_sparql_element;

    bool current_path_inverse;

    uint64_t limit  = OpSelect::DEFAULT_LIMIT;
    uint64_t offset = OpSelect::DEFAULT_OFFSET;
    
    std::unique_ptr<IPath> current_path;

    std::string base_iri;
    robin_hood::unordered_map<std::string, std::string> prefix_iris_map;

public:
    std::unique_ptr<Op> current_op;

    virtual antlrcpp::Any visitConstructQuery(SparqlParser::ConstructQueryContext* /* ctx */) override {
        throw NotSupportedException("Construct query");
    }

    virtual antlrcpp::Any visitDescribeQuery(SparqlParser::DescribeQueryContext* /* ctx */) override {
        throw NotSupportedException("Describe query");
    }

    virtual antlrcpp::Any visitAskQuery(SparqlParser::AskQueryContext* /* ctx */) override {
        throw NotSupportedException("Ask query");
    }

    virtual antlrcpp::Any visitUpdateCommand(SparqlParser::UpdateCommandContext* /* ctx */) override {
        throw NotSupportedException("Update command");
    }

    virtual antlrcpp::Any visitQuery(SparqlParser::QueryContext* ctx) override {
        // Root parser rule
        visitChildren(ctx);
        if (current_op == nullptr) {
            throw QuerySemanticException("Empty query received");
        }
        return 0;
    }

    virtual antlrcpp::Any visitBaseDecl(SparqlParser::BaseDeclContext* ctx) override {
        std::string base_iri_untrimmed = ctx->IRIREF()->getText();
        base_iri                       = base_iri_untrimmed.substr(1, base_iri_untrimmed.size() - 2);
        return 0;
    }

    virtual antlrcpp::Any visitPrefixDecl(SparqlParser::PrefixDeclContext* ctx) override {
        std::string pname_colon       = ctx->PNAME_NS()->getText();
        std::string iri_ref_untrimmed = ctx->IRIREF()->getText();
        std::string prefix_name       = pname_colon.substr(0, pname_colon.size() - 1);
        std::string iri_ref           = iri_ref_untrimmed.substr(1, iri_ref_untrimmed.size() - 2);
        if (prefix_iris_map.contains(prefix_name)) {
            throw QuerySemanticException("Multiple prefix declarations for prefix: '" + prefix_name + "'");
        }
        prefix_iris_map.insert({ prefix_name, iri_ref });
        return 0;
    }

    virtual antlrcpp::Any visitSelectQuery(SparqlParser::SelectQueryContext* ctx) override {
        // Control the visitor order explicitly
        visit(ctx->whereClause());
        visit(ctx->solutionModifier());
        visit(ctx->selectClause());
        return 0;
    }

     virtual antlrcpp::Any visitSelectClause(SparqlParser::SelectClauseContext* ctx) override {
        visitChildren(ctx);
        if (ctx->ASTERISK()) {
            for (auto& var : current_op->get_vars()) {
                if (var.name.find(':') == std::string::npos) {
                    select_variables.push_back(var);
                }
            }
        }
        // We treat both select modifiers as the same (DISTINCT and REDUCED)
        current_op = std::make_unique<OpSelect>(std::move(current_op),
                                                std::move(select_variables),
                                                ctx->selectModifier() != nullptr,
                                                limit,
                                                offset);
        return 0;
    }

    virtual antlrcpp::Any visitSelectSingleVariable(SparqlParser::SelectSingleVariableContext* ctx) override {
        select_variables.push_back(Var(ctx->getText().substr(1)));
        return 0;
    }

    virtual antlrcpp::Any visitWhereClause(SparqlParser::WhereClauseContext* ctx) override {
        visitChildren(ctx);
        current_op = std::make_unique<OpWhere>(std::move(current_op));
        return 0;
    }

    /*
        In SparqlParser.g4 the following rules are defined:
        
        ########################################################
        groupGraphPatternSub
            : triplesBlock?  groupGraphPatternSubList*
        ;
        groupGraphPatternSubList
            : graphPatternNotTriples '.'? triplesBlock?
        ;
        triplesBlock
        :  triplesSameSubjectPath ('.' triplesSameSubjectPath?)*
        ;
        ########################################################

        In order to visit the non-optional triples, we visit:
            a. TriplesBlock groupGraphPatternSub->triplesBlock()
            b. Each TriplesBlock of groupGraphPatternSub->groupGraphPatternSubList()
        For the optional triples, we visit:
            a. Each graphPatternNotTriples in groupGraphPatternSub->groupGraphPatternSubList()

        Then, the triples are handled individually in the visitTriplesSameSubjectPath() method
    */
    virtual antlrcpp::Any visitGroupGraphPatternSub(SparqlParser::GroupGraphPatternSubContext* ctx) override {
        auto ggpsl = ctx->groupGraphPatternSubList();
        // Visit every triple
        // Leftmost triples
        if (ctx->triplesBlock()) {
            visit(ctx->triplesBlock());
        }
        // Rest triples
        for (auto& ggpsl_item : ggpsl) {
            if (ggpsl_item->triplesBlock()) {
                visit(ggpsl_item->triplesBlock());
            }
        }
        auto parent = std::make_unique<OpTriples>(std::move(current_triples));
        // Visit the optional patterns
        std::vector<std::unique_ptr<Op>> optional_children;
        for (auto& ggpsl_item : ggpsl) {
            if (ggpsl_item->graphPatternNotTriples()) {
                visit(ggpsl_item->graphPatternNotTriples());
                optional_children.push_back(std::move(current_op));
            }
        }
        if (optional_children.size()) {
            current_op = std::make_unique<OpOptional>(std::move(parent), std::move(optional_children));
        } else {
            current_op = std::move(parent);
        }
        return 0;
    }

    /*
        In SparqlParser.g4 the following rules are defined:

        ###############################################################################
        triplesSameSubjectPath
            : varOrTerm propertyListPathNotEmpty | triplesNodePath propertyListPath
        ;
        propertyListPathNotEmpty
            : (verbPath|verbSimple) objectListPath (';' propertyListPathNotEmptyList?)*
        ;
        propertyListPathNotEmptyList
            : (verbPath|verbSimple) objectList
        ;
        ###############################################################################

        In order to visit each subject, predicate and object, we visit:
            a. The first alternative path of triplesSameSubjectPath (subject)
            b. Inside propertyListPathNotEmpty, the first predicate and object is
               appart from the rest
    */
    virtual antlrcpp::Any visitTriplesSameSubjectPath(SparqlParser::TriplesSameSubjectPathContext* ctx) override {
        SparqlElement subject;
        SparqlElement predicate;
        SparqlElement object;
        // Build every triple
        // Subject
        visit(ctx->varOrTerm());
        subject = std::move(current_sparql_element);
        // Leftmost predicate and objects
        auto plpne = ctx->propertyListPathNotEmpty();
        if (plpne->verbPath()) {
            visit(plpne->verbPath());
        } else {
            visit(plpne->verbSimple());
        }
        predicate = std::move(current_sparql_element);
        auto olp = plpne->objectListPath();
        for (auto& olp_item : olp->objectPath()) {
            visit(olp_item);
            object = std::move(current_sparql_element);
            current_triples.emplace_back(subject.duplicate(), predicate.duplicate(), object.duplicate());
        }
        // Rest predicates and objects
        for (auto& plpnel_item : plpne->propertyListPathNotEmptyList()) {
            if (plpnel_item->verbPath()) {
                visit(plpnel_item->verbPath());
            } else {
                visit(plpnel_item->verbSimple());
            }
            predicate = std::move(current_sparql_element);
            for (auto& o_item : plpnel_item->objectList()->object()) {
                visit(o_item);
                object = std::move(current_sparql_element);
                current_triples.emplace_back(subject.duplicate(), predicate.duplicate(), object.duplicate());
            }
        }
        return 0;
    }

    virtual antlrcpp::Any visitVar(SparqlParser::VarContext* ctx) override {
        Var value(ctx->getText().substr(1));
        current_sparql_element = SparqlElement(value);
        return 0;
    }

    virtual antlrcpp::Any visitIri(SparqlParser::IriContext* ctx) override {
        std::string iri_ref = IriContext_to_string(ctx);
        Iri value(iri_ref);
        current_sparql_element = SparqlElement(value);
        return 0;
    }

    virtual antlrcpp::Any visitRdfLiteral(SparqlParser::RdfLiteralContext* ctx) override {
        // There are some special cases that are handled differently (e.g. xsd:dateTime)
        std::string str = StringContext_to_string(ctx->string());
        if (ctx->iri()) {
            std::string iri_ref = IriContext_to_string(ctx->iri());
            // Supported datatypes
            // xsd:dateTime
            if (iri_ref == "http://www.w3.org/2001/XMLSchema#dateTime") {
                uint64_t datetime_id = DateTime::get_datetime_id(str.c_str());
                if (datetime_id == DateTime::INVALID_ID) {
                    throw QueryException("Invalid datetime value: " + str);
                }
                DateTime value(datetime_id);
                current_sparql_element = SparqlElement(value);
            }
            // xsd:decimal
            else if (iri_ref == "http://www.w3.org/2001/XMLSchema#decimal") {
                Decimal value(Decimal::normalize(str));
                current_sparql_element = SparqlElement(value);
            }
            // xsd:boolean
            else if (iri_ref == "http://www.w3.org/2001/XMLSchema#boolean") {
                bool value;
                if (str == "true" || str == "1") {
                    value = true;
                } else if (str == "false" || str == "0") {
                    value = false;
                } else {
                    throw QueryException("Unsupported boolean value: " + str);
                }
                current_sparql_element = SparqlElement(value);
            }
            // Unsupported datatypes are interpreted as literals with datatype
            else {
                LiteralDatatype value(str, iri_ref);
                current_sparql_element = SparqlElement(value);
            }
        } else if (ctx->LANGTAG()) {
            LiteralLanguage value(str, ctx->LANGTAG()->getText().substr(1));
            current_sparql_element = SparqlElement(value);
        } else {
            Literal value(str);
            current_sparql_element = SparqlElement(value);
        }
        return 0;
    }

    virtual antlrcpp::Any visitNumericLiteralUnsigned(SparqlParser::NumericLiteralUnsignedContext* ctx) override {
        Decimal value(Decimal::normalize(ctx->getText()));
        current_sparql_element = SparqlElement(value);
        return 0;
    }

    virtual antlrcpp::Any visitNumericLiteralPositive(SparqlParser::NumericLiteralPositiveContext* ctx) override {
        Decimal value(Decimal::normalize(ctx->getText()));
        current_sparql_element = SparqlElement(value);
        return 0;
    }

    virtual antlrcpp::Any visitNumericLiteralNegative(SparqlParser::NumericLiteralNegativeContext* ctx) override {
        Decimal value(Decimal::normalize(ctx->getText()));
        current_sparql_element = SparqlElement(value);
        return 0;
    }

    virtual antlrcpp::Any visitBooleanLiteral(SparqlParser::BooleanLiteralContext* ctx) override {
        bool value(ctx->TRUE() != nullptr);
        current_sparql_element = SparqlElement(value);
        return 0;
    }

    virtual antlrcpp::Any visitBlankNode(SparqlParser::BlankNodeContext* ctx) override {
        Var value(ctx->getText());
        current_sparql_element = SparqlElement(value);
        return 0;
    }

    virtual antlrcpp::Any visitPath(SparqlParser::PathContext* ctx) override {
        current_path_inverse = false;
        visit(ctx->pathAlternative());
        if (current_path->type() == PathType::PATH_ATOM) {
            // If the path is an Atom
            PathAtom* tmp = dynamic_cast<PathAtom*>(current_path.get());
            if (!tmp->inverse) {
                // And it is not inverted, it can be simplified as an Iri
                current_sparql_element = SparqlElement(tmp->iri);
                return 0;
            }
        }
        current_sparql_element = SparqlElement(std::move(current_path));
        return 0;
    }

    virtual antlrcpp::Any visitPathAlternative(SparqlParser::PathAlternativeContext* ctx) override {
        if (ctx->pathSequence().size() > 1) {
            std::vector<std::unique_ptr<IPath>> alternatives;
            for (auto& ps_item : ctx->pathSequence()) {
                visit(ps_item);
                alternatives.push_back(std::move(current_path));
            }
            current_path = std::make_unique<PathAlternatives>(std::move(alternatives));
        } else {
            visit(ctx->pathSequence(0));
        }
        return 0;
    }
    
    virtual antlrcpp::Any visitPathSequence(SparqlParser::PathSequenceContext* ctx) override {
        if (ctx->pathEltOrInverse().size() > 1) {
            std::vector<std::unique_ptr<IPath>> sequence;
            if (current_path_inverse) {
                for (int i = ctx->pathEltOrInverse().size() - 1; i >= 0; i--) {
                    visit(ctx->pathEltOrInverse(i));
                    sequence.push_back(std::move(current_path));
                }
            } else {
                for (auto& pe_item : ctx->pathEltOrInverse()) {
                    visit(pe_item);
                    sequence.push_back(std::move(current_path));
                }
            }
            current_path = std::make_unique<PathSequence>(std::move(sequence));
        } else {
            visit(ctx->pathEltOrInverse(0));
        }
        assert(current_path != nullptr);
        return 0;
    }

    virtual antlrcpp::Any visitPathEltOrInverse(SparqlParser::PathEltOrInverseContext* ctx) override {
        auto pe = ctx->pathElt();
        auto pp = pe->pathPrimary();
        auto mod = pe->pathMod();

        bool previous_current_path_inverse = current_path_inverse;
        current_path_inverse = (ctx->INVERSE() != nullptr) ^ current_path_inverse;
        if (pp->path()) {
            visit(pp->path()->pathAlternative());
            assert(current_path != nullptr);
        } else if (pp->iri()) {
            std::string iri_ref = IriContext_to_string(pp->iri());
            Iri value(iri_ref);
            current_path = std::make_unique<PathAtom>(iri_ref, current_path_inverse);
        } else if (pp->A()) {
            std::string iri_ref = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
            Iri value(iri_ref);
            current_path = std::make_unique<PathAtom>(iri_ref, current_path_inverse);
        } else {
            throw QuerySemanticException("Unsupported path element: '" + ctx->getText() + "'");
        }

        current_path_inverse = previous_current_path_inverse;

        if (mod) {
            switch(mod->getText()[0]) {
                case '*':
                    current_path = std::make_unique<PathKleeneStar>(std::move(current_path));
                    break;
                case '?':
                    if (!current_path->nullable()) {
                        current_path = std::make_unique<PathOptional>(std::move(current_path));
                    }
                    // else we avoid a redundant PathOptional, current_path stays the same
                    break;
                case '+':
                    // A+ => A / A*
                    auto kleene_star = std::make_unique<PathKleeneStar>(current_path->duplicate());
                    std::vector<std::unique_ptr<IPath>> sequence;
                    sequence.push_back(std::move(current_path));
                    sequence.push_back(std::move(kleene_star));
                    current_path = std::make_unique<PathSequence>(std::move(sequence));
                    break;
            }
        }
        return 0;
    }
    virtual antlrcpp::Any visitSolutionModifier(SparqlParser::SolutionModifierContext* ctx) override {
        // LIMIT and OFFSET
        auto limoffc = ctx->limitOffsetClauses();
        if (limoffc) {
            auto limc = limoffc->limitClause();
            if (limc) {
                limit = std::stoull(limc->INTEGER()->getText());
            }
            auto offc = limoffc->offsetClause();
            if (offc) {
                offset = std::stoull(offc->INTEGER()->getText());
            }
        }
        // ORDER BY
        visit(ctx->orderClause());
        current_op = std::make_unique<OpOrderBy>(std::move(current_op), std::move(order_by_items), std::move(order_by_ascending));
        return 0;
    }

    virtual antlrcpp::Any visitOrderClause(SparqlParser::OrderClauseContext* ctx) override {
        for (auto& oc : ctx->orderCondition()) {
            if (oc->var()) {
                order_by_items.emplace_back(oc->var()->getText().substr(1));
                order_by_ascending.push_back(true);
            }
            else if (oc->expression()) {
                // TODO: implement a handler for unsupported expressions
                visit(oc->expression());
                order_by_items.push_back(*order_by_current_expr);
                order_by_ascending.push_back(oc->ASC() != nullptr);
            }
            else {
                throw QuerySemanticException("Unsupported ORDER BY condition: '" + oc->getText() + "'");
            }
        }
        return 0;
    }

    virtual antlrcpp::Any visitBaseExpression(SparqlParser::BaseExpressionContext* ctx) override {
        auto pe = ctx->primaryExpression();
        if (pe->var()) {
            order_by_current_expr = std::make_unique<Var>(pe->var()->getText().substr(1));
        }
        else {
            throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
        }
        return 0;
    }


    virtual antlrcpp::Any visitUnaryMultiplicativeExpression(SparqlParser::UnaryMultiplicativeExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitUnaryAdditiveExpression(SparqlParser::UnaryAdditiveExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitUnaryNegationExpression(SparqlParser::UnaryNegationExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitMultiplicativeExpression(SparqlParser::MultiplicativeExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitAdditiveExpression(SparqlParser::AdditiveExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitUnarySignedLiteralExpression(SparqlParser::UnarySignedLiteralExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitRelationalSetExpression(SparqlParser::RelationalSetExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitRelationalExpression(SparqlParser::RelationalExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitConditionalAndExpression(SparqlParser::ConditionalAndExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    virtual antlrcpp::Any visitConditionalOrExpression(SparqlParser::ConditionalOrExpressionContext* ctx) override {
        throw QuerySemanticException("Unsupported ORDER BY expression: '" + ctx->getText() + "'");
    }

    std::string IriContext_to_string(SparqlParser::IriContext* ctx) {
        std::string iri_ref;
        if (ctx->IRIREF()) {
            std::string iri_ref_untrimmed = ctx->IRIREF()->getText();
            iri_ref = iri_ref_untrimmed.substr(1, iri_ref_untrimmed.size() - 2);
            // Check if the IRI is absolute or not
            // If it is not absolute, it needs to be expanded with the base IRI
            auto pos = iri_ref.find(':');
            if (pos == std::string::npos) {
                if (base_iri.empty()) {
                    throw QuerySemanticException("The IRI '" + iri_ref + "' is not absolute and the base IRI is not defined");
                }
                iri_ref = base_iri + iri_ref;
            }
        } else {
            auto pname = ctx->prefixedName()->getText();
            auto pos = pname.find(':');
            auto prefix = pname.substr(0, pos);
            auto suffix = pname.substr(pos + 1);
            if (!prefix_iris_map.contains(prefix)) {
                throw QuerySemanticException("Prefix '" + prefix + "' not found");
            }
            iri_ref = prefix_iris_map[prefix] + suffix;
        }
        return iri_ref;
    }

    std::string StringContext_to_string(SparqlParser::StringContext* ctx) {
        std::string str = ctx->getText();
        if (ctx->STRING_LITERAL1() || ctx->STRING_LITERAL2()) {
            return str.substr(1, str.size() - 2);
        } else {
            return str.substr(3, str.size() - 6);
        }
    }
};
}