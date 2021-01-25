#include "index_scan.h"

#include <cassert>
#include <iostream>
#include <vector>

#include "base/ids/var_id.h"
#include "storage/index/record.h"
#include "storage/index/bplus_tree/bplus_tree.h"
#include "storage/index/bplus_tree/bplus_tree_leaf.h"

using namespace std;

template class IndexScan<2>;
template class IndexScan<3>;
template class IndexScan<4>;
template class std::unique_ptr<IndexScan<2>>;
template class std::unique_ptr<IndexScan<3>>;
template class std::unique_ptr<IndexScan<4>>;

template <std::size_t N>
IndexScan<N>::IndexScan(std::size_t /*binding_size*/, BPlusTree<N>& bpt, std::array<std::unique_ptr<ScanRange>, N> ranges) :
    bpt    (bpt),
    ranges (move(ranges)) { }


template <std::size_t N>
void IndexScan<N>::begin(BindingId& parent_binding, bool parent_has_next) {
    assert(ranges.size() == N && "Inconsistent size of ranges and bpt");

    this->parent_binding = &parent_binding;
    std::array<uint64_t, N> min_ids;
    std::array<uint64_t, N> max_ids;
    if (!parent_has_next) {
        // TODO: would be better to not search in the bpt, something like it = bpt.get_range_null()
        for (uint_fast32_t i = 0; i < N; ++i) {
            min_ids[i] = UINT64_MAX;
            max_ids[i] = UINT64_MAX;
        }
    } else {
        for (uint_fast32_t i = 0; i < N; ++i) {
            assert(ranges[i] != nullptr);
            min_ids[i] = ranges[i]->get_min(parent_binding);
            max_ids[i] = ranges[i]->get_max(parent_binding);
        }
    }
    it = bpt.get_range(
        Record<N>(std::move(min_ids)),
        Record<N>(std::move(max_ids))
    );
    ++bpt_searches;
}


template <std::size_t N>
bool IndexScan<N>::next() {
    assert(it != nullptr);
    auto next = it->next();
    if (next != nullptr) {
        for (uint_fast32_t i = 0; i < N; ++i) {
            ranges[i]->try_assign(*parent_binding, ObjectId(next->ids[i]));
        }
        ++results_found;
        return true;
    } else {
        return false;
    }
}


template <std::size_t N>
void IndexScan<N>::reset() {
    std::array<uint64_t, N> min_ids;
    std::array<uint64_t, N> max_ids;

    for (uint_fast32_t i = 0; i < N; ++i) {
        min_ids[i] = ranges[i]->get_min(*parent_binding);
        max_ids[i] = ranges[i]->get_max(*parent_binding);
    }

    it = bpt.get_range(
        Record<N>(std::move(min_ids)),
        Record<N>(std::move(max_ids))
    );
    ++bpt_searches;
}


template <std::size_t N>
void IndexScan<N>::assign_nulls(){
    for (uint_fast32_t i = 0; i < N; ++i) {
        ranges[i]->try_assign(*parent_binding, ObjectId::get_null());
    }
}


template <std::size_t N>
void IndexScan<N>::analyze(int indent) const {
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    auto real_factor = static_cast<double>(results_found) / static_cast<double>(bpt_searches);
    cout << "IndexScan(bpt_searches: " << bpt_searches << ", found: " << results_found << ")\n";
    for (int i = 0; i < indent; ++i) {
        cout << ' ';
    }
    cout << "  ↳ Real factor: " << real_factor;
}
