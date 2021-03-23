#include "distinct_ordered.h"

using namespace std;

DistinctOrdered::DistinctOrdered(GraphModel& model, unique_ptr<BindingIter> _child_iter, std::vector<VarId> projected_vars) :
    model          (model),
    child_iter     (move(_child_iter)),
    child_binding  (child_iter->get_binding()),
    projected_vars (projected_vars)
    { }


void DistinctOrdered::begin() {
    child_iter->begin();
    current_tuple = std::vector<GraphObject>(projected_vars.size());
}


bool DistinctOrdered::next() {
    if (last_tuple.size() == 0) {
        // first tuple
        if (child_iter->next()) {
            for (const auto& var : projected_vars) {
                last_tuple.push_back( child_binding[var] );
            }
            return true;
        } else {
            return false;
        }
    } else {
        // not first tuple
        while (child_iter->next()) {
            for (size_t i = 0; i < projected_vars.size(); i++) {
                current_tuple[i] = child_binding[projected_vars[i]];
            }
            if (current_tuple != last_tuple) {
                last_tuple = current_tuple;
                return true;
            }
        }
        return false;
    }
}


void DistinctOrdered::analyze(int indent) const {
    std::cout << std::string(indent, ' ');
    std::cout << "DistinctOrdered()\n";
    child_iter->analyze(indent+2);
}
