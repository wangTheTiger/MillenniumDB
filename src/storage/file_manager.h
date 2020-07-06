/*
 * FileManager mantains a list (`opened_files`) with all files that were opened (even if the file is closed),
 * and another list (`filenames`) with the string of the file paths. Both list must have the same size
 * and objects at the same index are related to each other.
 *
 * All the other clases that need to work with files should use the FileManager to obtain a reference to a
 * fstream.
 *
 * `file_manager` is a global object and is available when this file is included. Before using it, somebody
 * needs to call the method FileManager::init(), usually is the responsability of the model (e.g. RelationalModel)
 * to call it.
 *
 * The instance `file_manager` cannot be destroyed before the BufferManager flushes its dirty pages on exit
 * because BufferManager needs to access the file paths from FileManager. Nifty counter trick should handle this
 * automagically.
 */

#ifndef STORAGE__FILE_MANAGER_H_
#define STORAGE__FILE_MANAGER_H_

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "storage/file_id.h"
#include "storage/page_id.h"

class FileManager {
public:
    static constexpr auto DEFAULT_DB_FOLDER = "test_files/default_db";

    ~FileManager();

    // necesary to be called before first usage
    static void init(std::string db_folder = DEFAULT_DB_FOLDER);

    // Get an id for the corresponding file, creating it if it's necessary
    FileId get_file_id(const std::string& filename);

    // returns the filename assignated to `file_id`
    std::string get_absolute_path(FileId file_id);

    // returns the filename assignated to `file_id`
    std::string get_filename(FileId file_id);

    // get the file stream assignated to `file_id` as a reference
    std::fstream& get_file(FileId file_path);

    // count how many pages a file have
    uint_fast32_t count_pages(FileId file_id);

    // if the file is closed, open it
    void ensure_open(FileId file_id);

    // close the file represented by `file_id`
    void close(FileId file_id);

    // delete the file represented by `file_id`, pages in buffer using that file_id are cleared
    void remove(FileId file_id);

    // rename the file represented by `file_id` to `new_name`
    void rename(FileId file_id, std::string new_name);

    // write the data pointed by `bytes` page represented by `page_id` to disk.
    // `bytes` must point to the start memory position of `PAGE_SIZE` allocated bytes
    void flush(PageId page_id, char* bytes);

    // read a page from disk into memory pointed by `bytes`.
    // `bytes` must point to the start memory position of `PAGE_SIZE` allocated bytes
    void read_page(PageId page_id, char* bytes);

private:
    FileManager(std::string db_folder);

    // folder where all the used files will be
    std::string db_folder;

    // contains all file streams that have been opened, including closed ones
    std::vector< std::unique_ptr<std::fstream> > opened_files;

    // contains all filenames that have been used. The position in this vector is equivalent to the FileId
    // representing that file
    std::vector<std::string> absolute_file_paths;
};

extern FileManager& file_manager; // global object

#endif // STORAGE__FILE_MANAGER_H_
