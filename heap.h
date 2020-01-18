#ifndef HEAP_H
#define HEAP_H

#include "data.h"

void init_heap(int len);
void cleanup_heap();
void free_data(data);
data _dump_heap(data);
void unmark_heap();
data request_gc(data);
data sweep_unmarked(data);
data _gc(data);
data alloc();
int heap_addr(data);

#endif
