#include "query_ast_printer.h"

/*
 * Each operator() overload should not begin nor end with whitespaces
 * the caller has the responsability to indent before calling and insert
 * new line (if necesary) after the call.
 */

using namespace query::ast;

QueryAstPrinter::QueryAstPrinter(std::ostream& out)
    : out(out), base_indent(0) {}

QueryAstPrinter::QueryAstPrinter(std::ostream& out, int_fast32_t base_indent)
    : out(out), base_indent(base_indent) {}


void QueryAstPrinter::indent(std::string str) const {
    int_fast32_t spaces = base_indent * tab_size;
    for (int_fast32_t i = 0; i < spaces; i++) {
        out << " ";
    }
    out << str;
}


void QueryAstPrinter::indent() const {
    int_fast32_t spaces = base_indent * tab_size;
    for (int_fast32_t i = 0; i < spaces; i++) {
        out << " ";
    }
}


void QueryAstPrinter::indent(std::string str, int_fast32_t extra_indent) const {
    int_fast32_t spaces = (base_indent + extra_indent) * tab_size;
    for (int_fast32_t i = 0; i < spaces; i++) {
        out << " ";
    }
    out << str;
}


void QueryAstPrinter::operator()(Root const& r) const {
    indent("{\n");
    auto printer = QueryAstPrinter(out, base_indent+1);
    printer.indent();
    boost::apply_visitor(printer, r.selection);
    out << ",\n";
    printer.indent();
    printer(r.graph_pattern);
    out << ",\n";
    printer.indent();
    printer(r.where);
    out << ",\n";
    printer.indent("\"LIMIT\":");
    if (r.limit) {
        out << r.limit.get();
    } else {
        out << 0;
    }
    indent("\n}\n");
}


void QueryAstPrinter::operator()(std::vector<Element> const& select) const {
    out << "\"SELECT\": [\n";
    auto printer = QueryAstPrinter(out, base_indent+1);
    auto it = select.begin();
    while (it != select.end()) {
        printer.indent();
        printer(*it);
        ++it;
        if (it != select.end()) {
            out << ",\n";
        }
        else {
            out << "\n";
        }
    }
    indent("]");
}


void QueryAstPrinter::operator() (All const&) const {
    out << "\"SELECT\": \"<ALL>\"";
}


void QueryAstPrinter::operator() (std::vector<LinearPattern> const& graph_pattern) const {
    out << "\"MATCH\": [\n";
    auto printer = QueryAstPrinter(out, base_indent+1);
    auto it = graph_pattern.begin();
    while (it != graph_pattern.end()) {
        printer.indent();
        printer(*it);
        ++it;
        if (it != graph_pattern.end()) {
            out << ",\n";
        }
        else {
            out << "\n";
        }
    }
    indent("]");
}


void QueryAstPrinter::operator() (LinearPattern const& linear_pattern) const {
    out << "{\n";
    auto printer = QueryAstPrinter(out, base_indent+1);
    printer.indent("\"GRAPH\": ");
    out << '"' << linear_pattern.graph_name << '"'<< ",\n";
    printer.indent();
    printer(linear_pattern.root);
    for (auto const& step_path : linear_pattern.path) {
        out << ",\n";
        printer.indent();
        printer(step_path);
    }
    out << "\n";
    indent("}");
}


void QueryAstPrinter::operator() (StepPath step_path) const {
    (*this)(step_path.edge);
    out << ",\n";
    indent();
    (*this)(step_path.node);
}


void QueryAstPrinter::operator() (Node node) const {
    out << "\"NODE\": {\n";
    indent("\"VAR\": ", 1);
    out << "\"" << node.var.name << "\",\n";
    indent("\"LABELS\": [", 1);
    auto label_iter = node.labels.begin();
    while (label_iter != node.labels.end()) {
        out << "\"" << *label_iter << "\"";
        ++label_iter;
        if (label_iter != node.labels.end()) {
            out << ", ";
        }
    }
    out << "],\n";
    if (node.properties.size() > 0) {
        indent("\"PROPERTIES\": [\n", 1);
        auto prop_iter = node.properties.begin();
        while (prop_iter != node.properties.end()) {
            indent("{ \"KEY\": ", 2);
            out << "\"" << (*prop_iter).key << "\"";
            out << ", \"VALUE\": ";
            boost::apply_visitor(*this, (*prop_iter).value);
            ++prop_iter;
            if (prop_iter != node.properties.end()) {
                out << " },\n";
            }
            else {
                out << " }\n";
            }
        }
        indent("]\n", 1);
    }
    else {
        indent("\"PROPERTIES\": []\n", 1);
    }
    indent("}");
}


void QueryAstPrinter::operator() (Edge edge) const {
    out << "\"EDGE\": {\n";
    indent("\"DIRECTION\": ", 1);
    if (edge.direction == EdgeDirection::right) {
        out << "\"RIGHT\",\n";
    }
    else {
        out << "\"LEFT\",\n";
    }
    indent("\"VAR\": ", 1);
    out << "\"" << edge.var.name << "\",\n";
    indent("\"LABELS\": [", 1);
    auto label_iter = edge.labels.begin();
    while (label_iter != edge.labels.end()) {
        out << "\"" << *label_iter << "\"";
        ++label_iter;
        if (label_iter != edge.labels.end()) {
            out << ", ";
        }
    }
    out << "],\n";
    if (edge.properties.size() > 0) {
        indent("\"PROPERTIES\": [\n", 1);
        auto prop_iter = edge.properties.begin();
        while (prop_iter != edge.properties.end()) {
            indent("{ \"KEY\": ", 2);
            out << "\"" << (*prop_iter).key << "\"";
            out << ", \"VALUE\": ";
            boost::apply_visitor(*this, (*prop_iter).value);
            ++prop_iter;
            if (prop_iter != edge.properties.end()) {
                out << " },\n";
            }
            else {
                out << " }\n";
            }
        }
        indent("]\n", 1);
    }
    else {
        indent("\"PROPERTIES\": []\n", 1);
    }
    indent("}");
}


void QueryAstPrinter::operator()(Element const& element) const {
    out << "{\n";
    // if (!element.function.empty()) {
    //     indent("\"FUNCTION\": \"", 1);
    //     out << element.function << "\",\n";
    // }
    indent("\"KEY\": \"", 1);
    out << element.key << "\",\n";
    indent("\"VAR\": \"", 1);
    out << element.var.name << "\"\n";
    indent("}");
}


void QueryAstPrinter::operator()(boost::optional<Formula> const& where) const {
    out << "\"FORMULA\": {\n";
    if (where) {
        Formula formula = static_cast<Formula>(where.get());
        auto printer = QueryAstPrinter(out, base_indent+1);
        printer.indent();
        printer(formula.root);
        for (const auto& step_formula : formula.path) {
            out << ",\n";
            printer.indent();
            printer(step_formula);
        }
        out << "\n";
    }
    indent("}");
}


void QueryAstPrinter::operator()(Condition const& condition) const {
    if (condition.negation) {
        out << "\"NOT CONDITION\": {\n";
    }

    else {
        out << "\"CONDITION\": {\n";
    }
    auto printer = QueryAstPrinter(out, base_indent+1);
    printer.indent();
    boost::apply_visitor(printer, condition.content);
    out << "\n";
    indent("}");
}


void QueryAstPrinter::operator()(Statement const& statement) const {
    out << "\"LEFT\": ";
    boost::apply_visitor(*this, statement.lhs);
    out << ",\n";
    indent("\"COMPARATOR\": ");
    boost::apply_visitor(*this, statement.comparator);
    out << ",\n";
    indent("\"RIGHT\": ");
    boost::apply_visitor(*this, statement.rhs);
}


void QueryAstPrinter::operator()(StepFormula const& step_formula) const {
    out << "\"CONNECTOR\": ";
    boost::apply_visitor(*this, step_formula.op);
    out << ",\n";
    indent();
    (*this)(step_formula.condition);
}


void QueryAstPrinter::operator()(Value const& v) const {
    boost::apply_visitor(*this, v);
}


void QueryAstPrinter::operator()(std::string const& text) const {
    out << "\"" << text << "\"";
}


void QueryAstPrinter::operator()(Var const& var) const {
    out << "\"" << var.name << "\"";
}


void QueryAstPrinter::operator() (VarId const& var_id) const {out << "VarId(" << var_id.id << ")"; }
void QueryAstPrinter::operator() (int64_t const& n)    const {out << "(int)" << n; }
void QueryAstPrinter::operator() (float const& n)      const {out << "(float)" << n; }
void QueryAstPrinter::operator() (And const&)          const {out << "\"AND\""; }
void QueryAstPrinter::operator() (Or const&)           const {out << "\"OR\""; }
void QueryAstPrinter::operator() (EQ const&)           const {out << "\"==\""; }
void QueryAstPrinter::operator() (NE const&)           const {out << "\"!=\""; }
void QueryAstPrinter::operator() (GT const&)           const {out << "\">\""; }
void QueryAstPrinter::operator() (LT const&)           const {out << "\"<\""; }
void QueryAstPrinter::operator() (GE const&)           const {out << "\">=\""; }
void QueryAstPrinter::operator() (LE const&)           const {out << "\"<=\""; }


void QueryAstPrinter::operator() (bool const& b) const {
    if (b)
        out << "TRUE";
    else
        out << "FALSE";
}