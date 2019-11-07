#include "file/page.h"

Page::Page(int page_number, char* bytes, const std::string& filename)
    : page_number(page_number), filename(filename), pins(1), dirty(false), bytes(bytes)
{
}

Page::~Page(){
    if (pins > 0) {
        std::cout << "Destroying pinned page";
    }
    flush();
    // std::cout << "destroying page (" << page_number << ", " << filename << ")\n";
}

uint_fast32_t Page::get_page_number() {
    return page_number;
}

void Page::unpin() {
    if (pins == 0) {
        throw std::logic_error("Inconsistent unpin when pins == 0");
    }
    pins--;
}

void Page::make_dirty() {
    dirty = true;
}

void Page::pin() {
    pins++;
}

char* Page::get_bytes() {
    return bytes;
}

void Page::flush() {
    if (dirty) {
        // std::cout << "Page::flush() " << page_number << ", " << filename << "\n";
        std::fstream file;
        file.open(filename, std::fstream::in|std::fstream::out|std::fstream::binary);
        file.seekp(page_number*PAGE_SIZE);
        // std::cout << file.tellp() << "\n";
        file.write(bytes, PAGE_SIZE);
        file.close();
        dirty = false;
    }
}
