#ifndef STORAGE__EXTENDIBLE_HASH_H_
#define STORAGE__EXTENDIBLE_HASH_H_

#include <cstdint>
#include <string>
#include <map>

#include "storage/file_id.h"
#include "storage/index/object_file/object_file.h"

class ExtendibleHash {
public:
    static constexpr auto DEFAULT_GLOBAL_DEPTH = 10;

    ExtendibleHash(ObjectFile& object_file, const std::string& filename);
    ~ExtendibleHash();

    uint64_t get_id(const std::string& str, bool insert_if_not_present = false);

private:
    ObjectFile& object_file;
    FileId dir_file_id;
    FileId buckets_file_id;

    uint_fast8_t global_depth;

    // array of size 2^global_depth
    uint_fast32_t* dir;

    void duplicate_dirs();
};

#endif // STORAGE__EXTENDIBLE_HASH_H_
