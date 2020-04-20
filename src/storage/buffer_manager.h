/* !!!! IMPORTANT !!!! DON'T include this file in a header file
 *
 * BufferManager contains all pages in memory and is used to get a page, making transparent if the page is
 * already in memory or needs to be readed from disk.
 *
 * `buffer_manager` is a global object and is available when this file is included. `buffer_manager` is only
 * constructed if it will be used in the execution. Construction is performed before the first line of main()
 * is executed and destrcution is performed after the last line of main() is executed.
 */

#ifndef STORAGE__BUFFER_MANAGER_H_
#define STORAGE__BUFFER_MANAGER_H_

#include "storage/file_id.h"
#include "storage/page_id.h"

#include <map>
#include <mutex>
#include <string>

class Page;

class BufferManager {
friend class BufferManagerInitializer;  // needed to access private constructor
public:
    static constexpr int DEFAULT_BUFFER_POOL_SIZE = 1024;

    // necesary to be called before first usage
    void init(int _buffer_pool_size = DEFAULT_BUFFER_POOL_SIZE);

    // Get a page. It will search in the buffer and if it is not on it, it will read from disk and put in the buffer.
    // Also it will pin the page, so calling buffer_manager.unpin(page) is expected when the caller doesn't need 
    // the returned page anymore.
    Page& get_page(FileId file_id, uint_fast32_t page_number);

    // Similar to get_page, but the page_number is the greatest number such that page number exist on disk.
    Page& get_last_page(FileId file_id);

    // Similar to get_page, but the page_number is the smallest number such that page number does not exist on disk.
    // The page returned has all its bytes initialized to 0. This operation perform a disk write inmediately
    // so 2 append_page in a row will work as expected.
    Page& append_page(FileId file_id);

    // write all dirty pages to disk
    void flush();

    // reduces the count of objects using the page. Should be called when a object using the page is destroyed.
    void unpin(Page& page);


private:
    // maximum pages the buffer can have
    int buffer_pool_size;

    // map used to search the index in the `buffer_pool` of a certain page
    std::map<PageId, int> pages;

    // array of `BUFFER_POOL_SIZE` pages
    Page* buffer_pool;

    // begining of the allocated memory for the pages
    char* bytes;

    // simple clock used to page replacement
    int clock_pos;

    // to avoid pin/unpin synchronization problems
    std::mutex pin_mutex;

    BufferManager();
    ~BufferManager();

    // returns the index of an unpined page from `buffer_pool`
    int get_buffer_available();
};

extern BufferManager& buffer_manager; // global object

static struct BufferManagerInitializer {
    BufferManagerInitializer();
    ~BufferManagerInitializer();
} bufferManager_initializer; // static initializer for every translation unit

#endif // STORAGE__BUFFER_MANAGER_H_
