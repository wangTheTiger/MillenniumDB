#ifndef BASE__NEGATION_H_
#define BASE__NEGATION_H_

#include "base/graph/condition/condition.h"

#include <memory>

class Negation : public Condition {
public:
    std::unique_ptr<Condition> condition;

    Negation(std::unique_ptr<Condition> condition)
        : condition(std::move(condition)) { }

    bool eval(Binding& binding) {
        return !condition->eval(binding);
    }

    ConditionType type() {
        return ConditionType::negation;
    }
};

#endif //BASE__NEGATION_H_
