#include "node_table_enum.h"

NodeTableEnum::NodeTableEnum(const VarId var_id, RandomAccessTable<1>& table) :
    var_id (var_id),
    table  (table) { }


void NodeTableEnum::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        std::cout << ' ';
    }
    // TODO: add info
    std::cout << "NodeTableEnum()";
}


void NodeTableEnum::begin(BindingId& input) {
    my_binding = std::make_unique<BindingId>(input.var_count());
    my_input = &input;
    current_pos = 0;
}


void NodeTableEnum::reset(BindingId& input) {
    my_binding = std::make_unique<BindingId>(input.var_count());
    my_input = &input;
    current_pos = 0;
}


BindingId* NodeTableEnum::next() {
    auto record = table[current_pos++];
    if (record != nullptr) {
        my_binding->add_all(*my_input);
        my_binding->add(var_id, ObjectId(record->ids[0]));
        return my_binding.get();
    } else {
        return nullptr;
    }
}
