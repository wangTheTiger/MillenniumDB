#include "agg_count_var_distinct.h"

#include "third_party/murmur3/murmur3.h"

void AggCountVarDistinct::process() {
    tuple[0] = (*binding_iter)[var_id];
    if (tuple[0] != GraphObject::make_null()) {
        if (!extendable_table->is_in_or_insert(tuple)) {
            count++;
        }
    }
}
