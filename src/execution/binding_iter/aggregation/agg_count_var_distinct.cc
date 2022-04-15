#include "agg_count_var_distinct.h"

void AggCountVarDistinct::process() {
    // tuple[0] = (*binding_iter)[var_id];
    // if (tuple[0] != GraphObject::make_null() ) {
    //     if (!extendable_table->is_in_or_insert(tuple)) {
    //         count++;
    //     }
    // }
    auto value = (*binding_iter)[var_id];
    if (value != GraphObject::make_null() ) {
        if (distinct_set.find(value) == distinct_set.end()) {
            distinct_set.insert(value);
            count++;
        }
    }
}
