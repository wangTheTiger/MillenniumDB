#ifndef GRAMMAR__VISITORS__PRINTER_H
#define GRAMMAR__VISITORS__PRINTER_H

#include "grammar/ast.h"
#include "grammar/visitors/formula_tree.h"

#include <boost/variant.hpp>
#include <boost/optional.hpp>

uint_fast32_t const tabsize = 2;

namespace visitors {

    // Prints the AST generated at parsing in a JSON-like
    // format to a given stream.
    class printer
        : public boost::static_visitor<void>
    {

    private:
        std::ostream& out;
        uint_fast32_t indent;

    public:

        // Constructor
        printer(std::ostream& out, uint_fast32_t indent = 0)
            : out(out), indent(indent) {}

        void operator()(ast::root const& r) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Select> = ";
            boost::apply_visitor(printer(out, indent+tabsize), r.selection_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<GraphPattern> = [" << '\n';
            for (auto const& lPattern: r.graphPattern_) {
                tab(indent+2*tabsize);
                out << "<LinearPattern> = ";
                printer(out, indent+2*tabsize)(lPattern);
                if(&lPattern != &r.graphPattern_.back())
                    out << ',';
                out << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent+tabsize);
            out << "<Where> = ";
            printer(out, indent+tabsize)(r.where_);
            out << '\n';
            tab(indent);
            out << '}' << '\n';
        }

        void operator()(ast::condition const& cond) const {
            if(cond.negation_) {
                out << '{' << '\n';
                tab(indent+tabsize);
                out << "NOT = ";
                boost::apply_visitor(printer(out, indent+tabsize), cond.content_);
                out << '\n';
                tab(indent);
                out << '}';
            }
            else {
                boost::apply_visitor(printer(out, indent), cond.content_);
            }
        }

        void operator()(boost::optional<ast::formula> const& formula) const {
            if(formula) {
                ast::formula realFormula = static_cast<ast::formula>(formula.get());
                (*this)(realFormula);
            }
            else {
                out << "[not present]" << '\n';
            }
        }

        void operator()(ast::formula const& formula) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Formula> = ";
            printer(out, indent+tabsize)(formula.root_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Path> = [\n";
            for (auto const& sFormula: formula.path_) {
                tab(indent+2*tabsize);
                out << "<Op> = ";
                boost::apply_visitor(printer(out, indent+2*tabsize), sFormula.op_);
                out << ',' << '\n';
                tab(indent+2*tabsize);
                out << "<Formula> = ";
                printer(out, indent+2*tabsize)(sFormula.cond_);
                if(&sFormula != &formula.path_.back())
                    out << ',';
                out << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent);
            out << '}';

        }

        void operator()(ast::element const& elem) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Function> = ";
            printer(out, indent+tabsize)(elem.function_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = ";
            printer(out, indent+tabsize)(elem.variable_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Key> = ";
            printer(out, indent+tabsize)(elem.key_);
            out << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(ast::statement const& stat) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<LHS> = ";
            printer(out, indent+tabsize)(stat.lhs_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Comparator> = ";
            boost::apply_visitor(printer(out, indent+tabsize), stat.comparator_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<RHS> = ";
            boost::apply_visitor(printer(out, indent+tabsize), stat.rhs_);
            out << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(ast::value const& val) const {
            boost::apply_visitor(printer(out, indent), val);
        }

        void operator()(ast::edge const& edge) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = \"" << edge.variable_ << "\",\n";
            tab(indent+tabsize);
            out << "<Labels> = [" << '\n';
            for (auto const& label: edge.labels_) {
                tab(indent+2*tabsize);
                out << label;
                if(&label != &edge.labels_.back())
                    out << ',';
                out << '\n';
            }
            tab(indent+tabsize);
            out << "]," << '\n';
            tab(indent+tabsize);
            out << "<Direction> = ";
            if(edge.isright_)
                out << "->," << '\n';
            else
                out << "<-," << '\n';
            tab(indent+tabsize);
            out << "<Properties> = {" << '\n';
            for (auto const& prop: edge.properties_) {
                tab(indent+2*tabsize);
                out << '"' <<  prop.key_ << "\" : ";
                boost::apply_visitor(printer(out, indent+2*tabsize), prop.value_);
                if(&prop != &edge.properties_.back())
                    out << ',';
                out << '\n';
            }
            tab(indent+tabsize);
            out << '}' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(ast::all_ const&) const {
            out << "<All>";
        }

        void operator()(ast::node const& node) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Variable> = \"" << node.variable_ << "\",\n";
            tab(indent+tabsize);
            out << "<Labels> = [" << '\n';
            for (auto const& label: node.labels_) {
                tab(indent+2*tabsize);
                out << '"' << label << '"';
                if(&label != &node.labels_.back())
                    out << ',';
                out << '\n';
            }
            tab(indent+tabsize);
            out << "]," << '\n';
            tab(indent+tabsize);
            out << "<Properties> = {" << '\n';
            for (auto const& prop: node.properties_) {
                tab(indent+2*tabsize);
                out << '"' <<  prop.key_ << "\" : ";
                boost::apply_visitor(printer(out, indent+2*tabsize), prop.value_);
                if(&prop != &node.properties_.back())
                    out << ',';
                out << '\n';
            }
            tab(indent+tabsize);
            out << '}' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(ast::linear_pattern const& lPattern) const {
            out << '{' << '\n';
            tab(indent+tabsize);
            out << "<Root> = ";
            printer(out, indent+tabsize)(lPattern.root_);
            out << ',' << '\n';
            tab(indent+tabsize);
            out << "<Path> = [\n";
            for(auto const& stepPath: lPattern.path_) {
                tab(indent+2*tabsize);
                out << "<Edge> = ";
                printer(out, indent+2*tabsize)(stepPath.edge_);
                out << ',' << '\n';
                tab(indent+2*tabsize);
                out << "<Node> = ";
                printer(out, indent+2*tabsize)(stepPath.node_);
                if(&stepPath != &lPattern.path_.back())
                    out << ',';
                out << '\n';
            }
            tab(indent+tabsize);
            out << ']' << '\n';
            tab(indent);
            out << '}';
        }

        void operator()(std::vector<ast::element> const& container) const {
            out << '[' << '\n';
            for(auto const& element: container) {
                tab(indent+tabsize);
                out << "<Element> = ";
                printer(out, indent+tabsize)(element);
                out << ',' << '\n';
            }
            tab(indent);
            out << ']';
        }

        void operator()(std::string const& text) const {
            out << '"' << text << '"';
        }

        void operator() (int const& n)          const {out << n;}
        void operator() (double const& n)       const {out << n;}
        void operator() (ast::and_ const&)      const {out << "AND";}
        void operator() (ast::or_ const&)       const {out << "OR";}
        void operator() (ast::eq_ const&)       const {out << "==";}
        void operator() (ast::neq_ const&)      const {out << "!=";}
        void operator() (ast::gt_ const&)       const {out << ">";}
        void operator() (ast::lt_ const&)       const {out << "<";}
        void operator() (ast::geq_ const&)      const {out << ">=";}
        void operator() (ast::leq_ const&)      const {out << "<=";}

        void operator() (bool const& b) const {
            if(b)
                out << "TRUE";
            else
                out << "FALSE";
        }

        // FORMULA TREE


        void operator() (formtree::formula const& f) const {
            boost::apply_visitor(printer(out, indent), f);
            out << '\n';
        }

        void operator() (formtree::and_op const& f) const {
            out << "<AND> = {\n";
            for(auto & elem: f.content) {
                tab(indent+tabsize);
                boost::apply_visitor(printer(out, indent+tabsize), elem);
                if(&elem != &f.content.back())
                    out << ',';
                out << '\n';
            }
            tab(indent);
            out << "}";
        }

        void operator() (formtree::or_op const& f) const{
            out << "<OR> = {\n";
            for(auto & elem: f.content) {
                tab(indent+tabsize);
                boost::apply_visitor(printer(out, indent+tabsize), elem);
                if(&elem != &f.content.back())
                    out << ',';
                out << '\n';
            }
            tab(indent);
            out << "}";
        }

        void operator() (formtree::not_op const& f) const {
            out << "<NOT> = {\n";
            tab(indent+tabsize);
            boost::apply_visitor(printer(out, indent+tabsize), f.content);
            out << '\n';
            tab(indent);
            out << '}';
        }

        void operator() (formtree::statement const& f) const {
            out << "?" << f.lhs.variable_ << "." << f.lhs.key_
                << " ";
            boost::apply_visitor(printer(out, indent+tabsize), f.comp);
            out << " ";
            if(f.rhs.type() == typeid(ast::element)) {
                ast::element el = boost::get<ast::element>(f.rhs);
                out << "?" << el.variable_ << '.' << el.key_;
            }
            else {
                boost::apply_visitor(printer(out, indent+tabsize), f.rhs);
            }
        }


        void tab(uint_fast32_t spaces) const {
            for(uint_fast32_t i = 0; i < spaces; i++) {
                out << ' ';
            }
        }
    }; // class printer
}

#endif  // GRAMMAR__VISITORS__PRINTER_H