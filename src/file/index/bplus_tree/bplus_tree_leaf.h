#ifndef FILE__B_PLUS_TREE_LEAF_H_
#define FILE__B_PLUS_TREE_LEAF_H_

#include <iostream>
#include <memory>
#include <tuple>

class Page;
class Record;
class BPlusTreeParams;

class BPlusTreeLeaf {
friend class BPlusTreeDir;
friend class BPlusTree;
public:
    BPlusTreeLeaf(const BPlusTreeParams& params, Page& page);
    ~BPlusTreeLeaf();

    std::unique_ptr<std::pair<Record, int>> insert(const Record& key, const Record& value);
    std::pair<int, int> search_leaf(const Record& min);

    std::unique_ptr<Record> get(const Record& key);

    void create_new(const Record& key, const Record& value);

    bool is_leaf()  { return true; }
    int get_value_count() { return *value_count; }
    int has_next()  { return *next_leaf != 0; }

    bool check() const;
    void print() const;

    std::unique_ptr<BPlusTreeLeaf> get_next_leaf();
    std::unique_ptr<Record> get_record(int pos);

private:
    const BPlusTreeParams& params;
    Page& page;
    int* value_count;
    int* next_leaf;
    uint64_t* records; // key+value

    int search_index(int from, int to, const Record& record);
    bool equal_record(const Record& record, int index);
    void shift_right_records(int from, int to);
};

#endif //FILE__B_PLUS_TREE_LEAF_H_
