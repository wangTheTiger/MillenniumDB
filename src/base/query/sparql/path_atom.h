#pragma once

#include "base/query/sparql/path.h"

namespace SPARQL {

class PathAtom : public IPath {
public:
    Iri  iri;
    bool inverse;

    PathAtom(Iri iri, bool inverse) : iri(iri), inverse(inverse) { }

    PathAtom(const PathAtom& other) : iri(other.iri), inverse(other.inverse) { }

    PathType type() const override {
        return PathType::PATH_ATOM;
    }

    std::unique_ptr<IPath> duplicate() const override {
        return std::make_unique<PathAtom>(*this);
    }

    std::string to_string() const override {
        std::string ret;
        if (inverse) {
            ret += "^";
        }
        ret += "<";
        ret += iri.name;
        ret += ">";
        return ret;
    }

    bool nullable() const override {
        return false;
    }

    RPQAutomaton get_rpq_base_automaton() const override {
        // Create a simple automaton
        auto automaton = RPQAutomaton();
        automaton.end_states.insert(1);
        // Connect states with atom as label
        std::string iri_str = iri.name;
        automaton.add_transition(Transition(0, 1, iri_str, inverse));
        return automaton;
    }
};
} // namespace SPARQL