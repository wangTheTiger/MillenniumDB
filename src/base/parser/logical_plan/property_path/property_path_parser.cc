#include "property_path_parser.h"

#include "base/parser/logical_plan/op/op_path_alternatives.h"
#include "base/parser/logical_plan/op/op_path_atom.h"
#include "base/parser/logical_plan/op/op_path_sequence.h"
#include "base/parser/logical_plan/op/op_path_suffix.h"

#include <cassert>
#include <vector>

using namespace std;

unique_ptr<OpPath> PropertyPathParser::operator()(query::ast::PropertyPathAlternatives& p, bool inverse) {
    assert(p.alternatives.size() > 0);
    if (p.alternatives.size() > 1) {
        vector<unique_ptr<OpPath>> alternatives;

        for (auto& alternative : p.alternatives) {
            alternatives.push_back( (*this)(alternative, inverse) );
        }

        return make_unique<OpPathAlternatives>(move(alternatives));
    } else {
        return (*this)(p.alternatives[0], inverse);
    }
}


unique_ptr<OpPath> PropertyPathParser::operator()(query::ast::PropertyPathSequence& p, bool inverse) {
    assert(p.atoms.size() > 0);
    if (p.atoms.size() > 1) {
        vector<unique_ptr<OpPath>> sequence;

        if (inverse) {
            for (signed int i = p.atoms.size() - 1; i >= 0; i--) {
                sequence.push_back( (*this)(p.atoms[i], inverse) );
            }
        } else {
            for (auto& atom : p.atoms) {
                sequence.push_back( (*this)(atom, inverse) );
            }
        }

        return make_unique<OpPathSequence>(move(sequence));
    } else {
        return (*this)(p.atoms[0], inverse);
    }
}


unique_ptr<OpPath> PropertyPathParser::operator()(query::ast::PropertyPathAtom& p, bool inverse) {
    unique_ptr<OpPath> tmp;

    if (p.atom.type() == typeid(std::string)) {
        string s = boost::get<string>(p.atom);
        tmp = make_unique<OpPathAtom>(s, p.inverse ^ inverse); // ^ = XOR
    } else {
        query::ast::PropertyPathAlternatives a = boost::get<query::ast::PropertyPathAlternatives>(p.atom);
        tmp = (*this)(a, p.inverse ^ inverse); // ^ = XOR
    }

    if (p.suffix.type() == typeid(query::ast::PropertyPathSuffix)) {
        query::ast::PropertyPathSuffix suffix = boost::get<query::ast::PropertyPathSuffix>(p.suffix);
        switch (suffix) {
            case query::ast::PropertyPathSuffix::NONE :
                return tmp;
            case query::ast::PropertyPathSuffix::ONE_OR_MORE :
                return make_unique<OpPathSuffix>(move(tmp), 1, OpPathSuffix::MAX);
            case query::ast::PropertyPathSuffix::ZERO_OR_MORE :
                return make_unique<OpPathSuffix>(move(tmp), 0, OpPathSuffix::MAX);
            case query::ast::PropertyPathSuffix::ZERO_OR_ONE :
                return make_unique<OpPathSuffix>(move(tmp), 0, 1);
        }
    }
    query::ast::PropertyPathBoundSuffix suffix = boost::get<query::ast::PropertyPathBoundSuffix>(p.suffix);
    return make_unique<OpPathSuffix>(move(tmp), suffix.min, suffix.max);
}
