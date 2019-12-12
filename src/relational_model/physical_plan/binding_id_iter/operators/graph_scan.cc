#include "relational_model/physical_plan/binding_id_iter/operators/graph_scan.h"

#include "base/var/var_id.h"
#include "file/index/record.h"
#include "file/index/bplus_tree/bplus_tree.h"
#include "file/index/bplus_tree/bplus_tree_leaf.h"
#include "file/index/bplus_tree/bplus_tree_params.h"

#include <vector>

using namespace std;

GraphScan::GraphScan(int graph_id, BPlusTree& bpt, vector<VarId> vars)
    : graph_id(graph_id), bpt(bpt), vars(vars)
{
    record_size = bpt.params.total_size;
}

void GraphScan::init(shared_ptr<BindingIdRange const> input)
{
    this->input = input;
    vector<uint64_t> min_ids(record_size);
    vector<uint64_t> max_ids(record_size);

    if (input == nullptr) {
        for (int i = 0; i < record_size; i++) {
            min_ids[i] = 0;
            max_ids[i] = UINT64_MAX;
        }
    }
    else {
        for (int i = 0; i < record_size; i++) {
            auto min_max = input->search_id(vars[i]);
            if (min_max != nullptr) {
                min_ids[i] = min_max->first.id;
                max_ids[i] = min_max->second.id;
            }
            else {
                while (i < record_size) {
                    min_ids[i] = 0;
                    max_ids[i] = UINT64_MAX;
                    i++;
                }
            }
        }
    }

    it = bpt.get_range(
        Record(min_ids),
        Record(max_ids)
    );
}

unique_ptr<BindingId const> GraphScan::next()
{
    auto next = it->next();
    if (next != nullptr) {
        auto res = make_unique<BindingId>();

        res->add(input->get_values());
        for (int i = 0; i < record_size; i++) {
            ObjectId element_id = ObjectId(graph_id, next->ids[i]);
            res->add(vars[i], element_id);
        }
        return res;
    }
    else return nullptr;
}

void GraphScan::reset(shared_ptr<BindingIdRange const> input) {
    init(input);
}
