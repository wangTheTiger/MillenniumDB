#ifndef RELATIONAL_MODEL__ASSIGNED_VAR_H_
#define RELATIONAL_MODEL__ASSIGNED_VAR_H_

#include <cassert>

#include <stdio.h> // TODO: DELETE
#include <inttypes.h> // TODO: DELETE

#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"

class AssignedVar : public ScanRange {
private:
    VarId var_id;

public:
    AssignedVar(VarId var_id) :
        var_id(var_id) { }

    uint64_t get_min(BindingId& binding_id) override {
        // printf("GET MIN VAR ID: %" PRIuFAST32 "\n", var_id.id);
        auto obj_id = binding_id[var_id];
        assert(!obj_id.is_null() && "var should be assigned in binding");
        return obj_id.id;
    }

    uint64_t get_max(BindingId& binding_id) override {
        auto obj_id = binding_id[var_id];
        assert(!obj_id.is_null() && "var should be assigned in binding");
        return obj_id.id;
    }

    void try_assign(BindingId&, ObjectId) override { }
};

#endif // RELATIONAL_MODEL__ASSIGNED_VAR_H_
