#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdlib.h>

extern "C"
{
    int allocator_init();
    int allocator_cleanup();
    void * allocate(size_t bytes);
    void destroy(void * ptr);
    void allocator_gc_begin();
    void allocator_gc_mark(void * ptr);
    void allocator_gc_end();
}

#endif
