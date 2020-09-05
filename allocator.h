#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <memory>

class gc_allocator
{
public:
    using pointer = void *;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    gc_allocator();
    ~gc_allocator();
    pointer allocate(size_type size);
    void deallocate(pointer ptr);
    void gc_begin();
    void gc_mark(pointer ptr);
    void gc_end();
    size_type get_used_memory();
    size_type get_committed_memory();

private:
    class impl_t;
    std::unique_ptr<impl_t> impl;
};

#endif
