#include "heap.h"
#include <stdlib.h>
#include "string.h"
#include "print.h"
#include "symbol.h"
#include "eval.h"
#include "object.h"

/************************/
/* Heap memory function */
/************************/
data heap;
data unused_heap;
int heap_length;
int heap_used_count;

void init_heap(int len)
{
    size_t i;
    heap = (data)malloc(sizeof(dataimpl) * len);
    if (!heap)
    {
        error(L"bad alloc");
        return;
    }
    memset(heap, 0, sizeof(dataimpl) * len);
    unused_heap = heap;
    heap_length = len;
    heap_used_count = 0;
    for (i = 0; i < len; ++i)
    {
        set_car(&heap[i], &heap[((i - 1 + len) % len)]);
        set_cdr(&heap[i], &heap[((i + 1) % len)]);
    }
    nil = make_nil();
    t = make_t();
    quote = make_quote();
}

void cleanup_heap()
{
    size_t i;
    for (i = 0; i < heap_length; ++i)
        free_data(&heap[i]);
    free(heap);
    heap = NULL;
    unused_heap = NULL;
    heap_length = 0;
    heap_used_count = 0;
}

void free_data(data d)
{
    if (!used(d))
        error(L"invalid free data to unused heap\n");
    if (is_symbol(d) || is_string(d) || is_builtin_macro(d) || is_builtin_macro(d))
        free(d->buffer._string);
    if (is_object(d))
        object_cleanup(d);
    init_data(d);
    insert_node(unused_heap, d);
    heap_used_count -= 1;
}

data _dump_heap(data d)
{
    size_t i;
    wprintf(L"%05.1lf%% (%d/%d)\n", ((double)heap_used_count / heap_length * 100), heap_used_count, heap_length);
    wprintf(L"   addr info\n");
    for (i = 0; i < heap_length; ++i)
    {
        wprintf(L"%c%c %04zu ", (used(&heap[i]) ? L'u' : L' '), (marked(&heap[i]) ? L'm' : L' '), (size_t)heap_address(&heap[i]));
        print(&heap[i]);
        wprintf(L"\n");
    }
    return(t);
}

void unmark_heap()
{
    size_t i;
    for (i = 0; i < heap_length; ++i)
        set_marked(&heap[i], 0);
}

data sweep_unmarked(data d)
{
    size_t i, count;
    count = 0;
    for (i = 0; i < heap_length; ++i)
    {
        if (used(&heap[i]) && !marked(&heap[i]))
        {
            free_data(&heap[i]);
            count += 1;
        }
    }
    return(make_int(count));
}

data request_gc(data d)
{
    if (((double)heap_used_count / heap_length) > 0.75)
        return(_gc(d));
    return(nil);
}

data _gc(data d)
{
    unmark_heap();
    mark_symbol();
    return(sweep_unmarked(nil));
}

data alloc()
{
    if (heap_used_count == heap_length)
    {
        error(L"heap memory allocation failed");
        return(nil);
    }

    data new_data = pull_node(&unused_heap);
    heap_used_count += 1;

    memset((void *)new_data, 0, sizeof(dataimpl));
    set_used(new_data, 1);
    return(new_data);
}

int heap_address(data d)
{
    return((int)(d - heap));
}
