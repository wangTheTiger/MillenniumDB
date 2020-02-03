#ifndef RELATIONAL_MODEL__PROJECTION_H_
#define RELATIONAL_MODEL__PROJECTION_H_

#include "base/binding/binding_iter.h"
#include "relational_model/binding/binding_id_iter.h"

#include <set>
#include <memory>

class Projection : public BindingIter {

private:
    std::unique_ptr<BindingIter> iter;
    std::unique_ptr<Binding> root_input;
    std::set<std::string> projection_vars;

public:
    Projection(std::unique_ptr<BindingIter> iter, std::set<std::string> projection_vars);
    ~Projection() = default;

    void begin();
    std::unique_ptr<Binding> next();
};

#endif //RELATIONAL_MODEL__PROJECTION_H_
