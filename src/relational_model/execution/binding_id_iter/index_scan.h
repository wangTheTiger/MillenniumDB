#ifndef RELATIONAL_MODEL__GRAPH_SCAN_H_
#define RELATIONAL_MODEL__GRAPH_SCAN_H_

#include <array>
#include <memory>

#include "storage/index/bplus_tree/bplus_tree.h"
#include "relational_model/execution/binding/binding_id_iter.h"
#include "relational_model/execution/binding_id_iter/scan_ranges/scan_range.h"

template <std::size_t N>
class IndexScan : public BindingIdIter {
private:
    BPlusTree<N>& bpt;
    std::unique_ptr<BptIter<N>> it;

    BindingId* my_input;
    std::array<std::unique_ptr<ScanRange>, N> ranges;

    // statistics
    uint_fast32_t results_found = 0;
    uint_fast32_t bpt_searches = 0;

public:
    IndexScan(std::size_t binding_size, BPlusTree<N>& bpt, std::array<std::unique_ptr<ScanRange>, N> ranges);
    ~IndexScan() = default;

    void analyze(int indent = 0) const override;
    BindingId& begin(BindingId& input) override;
    void reset() override;
    bool next() override;
};

#endif // RELATIONAL_MODEL__GRAPH_SCAN_H_
