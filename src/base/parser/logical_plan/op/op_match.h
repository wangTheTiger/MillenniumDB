#ifndef BASE__OP_MATCH_H_
#define BASE__OP_MATCH_H_

#include <iostream>
#include <set>
#include <vector>

#include "base/ids/node_id.h"
#include "base/parser/logical_plan/exceptions.h"
#include "base/parser/logical_plan/op/op_connection.h"
#include "base/parser/logical_plan/op/op_label.h"
#include "base/parser/logical_plan/op/op_property.h"
#include "base/parser/logical_plan/op/op_property_path.h"

#include "base/parser/logical_plan/op/op_unjoint_object.h"
#include "base/parser/logical_plan/op/op.h" // TODO: try to delete
#include "base/parser/logical_plan/op/property_paths/path_constructor.h"

class OpMatch : public Op {
public:
    std::set<OpLabel>          labels;
    std::set<OpProperty>       properties;
    std::set<OpConnection>     connections;
    std::set<OpPropertyPath>   property_paths;
    std::set<OpUnjointObject>  unjoint_objects;

    std::set<Var> vars; // contains declared variables and anonymous (auto-generated in the constructor)

    uint_fast32_t *anon_count;

    OpMatch(const std::vector<query::ast::LinearPattern>& graph_pattern, uint_fast32_t *anon_count) :
        anon_count(anon_count)
    {
        std::vector<boost::variant<std::string, bool, int64_t, float>> pending_unjoint_objects;
        for (auto& linear_pattern : graph_pattern) {
            // UnjointObjects will be processed at the end
            if (   linear_pattern.path.empty()
                && linear_pattern.root.labels.empty()
                && linear_pattern.root.properties.empty())
            {
                // TODO: que hacer con MATCH (5)
                pending_unjoint_objects.push_back(linear_pattern.root.id);
            }
            else {
                auto last_node_id = process_node(linear_pattern.root);
                for (auto& linear_pattern_step : linear_pattern.path) {
                    auto current_node_id = process_node(linear_pattern_step.node);

                    if (linear_pattern_step.path.type() == typeid(query::ast::Edge)) {
                        // EDGE
                        auto edge = boost::get<query::ast::Edge>(linear_pattern_step.path);
                        std::vector<std::string> types;
                        auto edge_name = process_edge(edge, types);

                        connections.insert(
                            edge.direction == query::ast::EdgeDirection::right
                                ? OpConnection(last_node_id, current_node_id, edge_name, std::move(types))
                                : OpConnection(current_node_id, last_node_id, edge_name, std::move(types)));
                    } else {
                        auto property_path = boost::get<query::ast::PropertyPath>(linear_pattern_step.path);

                        Var path_var = property_path.var.empty()
                                ? Var("?_p" + std::to_string((*anon_count)++))
                                : Var(property_path.var);
                        vars.insert(path_var);

                        PathConstructor path_constructor;
                        if (property_path.direction == query::ast::EdgeDirection::right) {
                            property_paths.emplace(path_var,
                                                   last_node_id,
                                                   current_node_id,
                                                   path_constructor(property_path.path_alternatives));
                        } else { // property_path.direction == query::ast::EdgeDirection::left
                            property_paths.emplace(path_var,
                                                   current_node_id,
                                                   last_node_id,
                                                   path_constructor(property_path.path_alternatives));
                        }
                    }
                }
            }
        }
        for (auto& pending_unjoint_object : pending_unjoint_objects) {
            auto node_id = get_node_id(pending_unjoint_object);
            // TODO: filter redundant UnjointObjects
            // ej: MATCH (?x)->(?y), (?x)
            if (true) {
                unjoint_objects.insert(OpUnjointObject(node_id));
            }
        }
    }


    NodeId get_node_id(const boost::variant<std::string, bool, int64_t, float>& id) {
        if (id.type() == typeid(std::string)) {
            auto str = boost::get<std::string>(id);
            // TODO: delete prints
            if (str.empty()) {
                // anonymous variable
                std::cout << "node is anonymous var(empty)\n";
                std::string s = "?_" + std::to_string((*anon_count)++);
                Var v(s);
                vars.insert(v);
                return NodeId(v);
            } else if (str[0] == '?') {
                // explicit variable
                std::cout << "node is explicit var: " << str << "\n";
                Var v(str);
                vars.insert(v);
                return NodeId(v);
            }  else if (str[0] == '"') {
                // string
                // delete first and last characters: ("")
                std::string tmp = str.substr(1, str.size() - 2);
                std::cout << "node is string:" << tmp << "\n";
                return NodeId(std::move(tmp));
            }  else {
                // identifier
                std::cout << "node is name: " << str << "\n";
                return NodeId(NodeName(str));
            }
        } else if (id.type() == typeid(bool)) {
            auto b = boost::get<bool>(id);
            std::cout << "node is bool: " << (b ? "true" : "false") << "\n";
            return NodeId(b);
        } else if (id.type() == typeid(int64_t)) {
            auto i = boost::get<int64_t>(id);
            std::cout << "node is int: " << i << "\n";
            return NodeId(i);
        } else { // if (id.type() == typeid(float)) {
            auto f = boost::get<float>(id);
            std::cout << "node is float: " << f << "\n";
            return NodeId(f);
        }
    }


    NodeId process_node(const query::ast::Node& node) {
        NodeId node_id = get_node_id(node.id);

        for (auto& label : node.labels) {
            labels.insert(OpLabel(node_id, label));
        }

        for (auto& property : node.properties) {
            auto new_property = OpProperty(node_id, property.key, property.value);
            auto property_search = properties.find(new_property);

            if (property_search != properties.end()) {
                auto old_property = *property_search;
                if (old_property.value != property.value) {
                    throw QuerySemanticException(node_id.to_string() + "." + property.key
                                                 + " its declared with different values");
                }
            }
            else {
                properties.insert(new_property);
            }
        }
        return node_id;
    }


    Var process_edge(const query::ast::Edge& edge, std::vector<std::string>& types) {
        Var edge_var =  edge.var.empty()
            ? Var("?_e" + std::to_string((*anon_count)++))
            : Var(edge.var);
        vars.insert(edge_var);

        for (const auto &type : edge.types) {
            if (type[0] == '?') {
                vars.insert(Var(type));
            }
            types.push_back(type);
        }

        if (edge.types.size() == 0) {
            vars.insert(Var("_type_" + edge_var.value));
        }

        for (auto& property : edge.properties) {
            auto new_property = OpProperty(NodeId(edge_var), property.key, property.value);
            auto property_search = properties.find(new_property);

            if (property_search != properties.end()) {
                auto old_property = *property_search;
                if (old_property.value != property.value) {
                    throw QuerySemanticException(edge_var.value + "." + property.key
                                                 + " its declared with different values in MATCH");
                }
            } else {
                properties.insert(new_property);
            }
        }

        return edge_var;
    }


    std::ostream& print_to_ostream(std::ostream& os, int indent=0) const override{
        os << std::string(indent, ' ');
        os << "OpMatch()\n";

        for (auto &label : labels) {
            label.print_to_ostream(os, indent + 2);
        }
        for (auto &property : properties) {
            property.print_to_ostream(os, indent + 2);
        }
        for (auto &connection : connections) {
            connection.print_to_ostream(os, indent + 2);
        }
        for (auto &property_path : property_paths) {
            property_path.print_to_ostream(os, indent + 2);
        }
        for (auto& unjoint_object : unjoint_objects) {
            unjoint_object.print_to_ostream(os, indent + 2);
        }
        return os;
    }


    void accept_visitor(OpVisitor &visitor) override {
        visitor.visit(*this);
    }


    void get_vars(std::set<Var>& set) const override {
        for (auto& v : vars) {
            set.insert(v);
        }
    }
};

#endif // BASE__OP_MATCH_H_
