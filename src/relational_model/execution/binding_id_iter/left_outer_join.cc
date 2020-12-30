
#include "left_outer_join.h"

#include <algorithm>
#include <iostream>

#include "base/ids/var_id.h"

using namespace std;

LeftOuterJoin::LeftOuterJoin(std::size_t binding_size,
                             unique_ptr<BindingIdIter> lhs,
                             unique_ptr<BindingIdIter> rhs) :
    BindingIdIter(binding_size),
    lhs (move(lhs)),
    rhs (move(rhs)) { }


BindingId& LeftOuterJoin::begin(BindingId& input) {
    has_result = false;
    has_left = true;
    current_left = &lhs->begin(input);
    if (lhs->next()) {
        current_right = &rhs->begin(*current_left);
    }else{
        has_left = false;
    }
    return my_binding;
}


void LeftOuterJoin::reset() {
    has_result = false;
    has_left = true;
    lhs->reset();
    if (lhs->next()) {
        rhs->reset();
    } else {
        has_left = false;
    }
}


bool LeftOuterJoin::next() {
    if(!has_left) {
        return false;
    }
    while (true) {
        if (rhs->next()) {
            has_result = true;
            my_binding.add_all(*current_left);
            my_binding.add_all(*current_right);
            return true;
        } else {
            if (!has_result) {
                my_binding.add_all(*current_left);
                has_result = true;
                return true;
            } else {
                if (lhs->next()) {
                    has_result = false;
                    rhs->reset();
                } else {
                    return false;
                }
            }
        }
    }
}


void LeftOuterJoin::analyze(int indent) const {
    // for (int i = 0; i < indent; ++i) {
    //     cout << ' ';
    // }
    // cout << "LeftOuterJoin(\n";
    lhs->analyze(indent);
    cout << ",\n";
    rhs->analyze(indent);
    // cout << "\n";
    // for (int i = 0; i < indent; ++i) {
    //     cout << ' ';
    // }
    // cout << ")";
}

template class std::unique_ptr<LeftOuterJoin>;
