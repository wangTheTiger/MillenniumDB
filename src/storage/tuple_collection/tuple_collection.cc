#include "tuple_collection.h"

#include <cassert>
#include <iostream>
#include <cstring>
#include <cmath>

#include "storage/file_id.h"
#include "storage/file_manager.h"
#include "storage/buffer_manager.h"

uint64_t PAGE_SIZE = 4096;

TupleCollection::TupleCollection(Page& page, const uint64_t tuple_size) :
    page(page),
    tuple_size(tuple_size)
{
    tuple_count = reinterpret_cast<uint64_t*>(page.get_bytes());
    tuples = reinterpret_cast<uint64_t*>(page.get_bytes() + sizeof(tuple_count));
}


TupleCollection::~TupleCollection() {
	buffer_manager.unpin(page);
}


bool TupleCollection::is_full() const {
    if (((*tuple_count) + 1) * tuple_size + sizeof(tuple_count) < PAGE_SIZE) {
      return true;
    }
    return false;
}


void TupleCollection::add(std::vector<uint64_t> new_tuple) {
	  for (size_t i = 0; i < tuple_size; i++) {
    	tuples[(*tuple_count) * tuple_size + i] = new_tuple[i];
  	}
    page.make_dirty();
  	(*tuple_count)++;
}


std::vector<uint64_t> TupleCollection::get(int id) const {
    std::vector<uint64_t> n_tuple = std::vector<uint64_t>(tuple_size);
    for (size_t i = 0; i < tuple_size; i++) {
        n_tuple[i] = tuples[id * tuple_size + i];
    }
    return n_tuple;
}


void TupleCollection::override_tuple(std::vector<uint64_t>& tuple, int position) {
    for (size_t i = 0; i < tuple_size; i++) {
        tuples[position * tuple_size + i] = tuple[i];
    }
}


void TupleCollection::swap(int x, int y) {
    std::vector<uint64_t> x_tuple = get(x);
    std::vector<uint64_t> y_tuple = get(y);
    override_tuple(x_tuple, y);
    override_tuple(y_tuple, x);
}


void TupleCollection::sort( bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y)) {
    quicksort(0, (*tuple_count) - 1, is_leq);
    page.make_dirty();
}


void TupleCollection::quicksort(int i, int f, bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y)) {
    if (i <= f) {
        int p = partition(i, f, is_leq);
        quicksort(i, p - 1, is_leq);
        quicksort(p + 1, f, is_leq);
    }
}


int TupleCollection::partition(int i, int f, bool (*is_leq)(std::vector<uint64_t> x, std::vector<uint64_t> y)) {
    int x = i + (rand() % (f - i + 1));
    std::vector<uint64_t> p = get(x);
    TupleCollection::swap(x,f);
    override_tuple(p, f);
    int j = i;
    int k = i;
    while (k <= f) {
        if (is_leq(get(k), p)) {
            TupleCollection::swap(j, k);
        }
        k++;
    }
    TupleCollection::swap(j, f);
    return j;
}
