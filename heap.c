#include "heap.h"
#include <stdlib.h>
#include "string.h"
#include "print.h"
#include "sym.h"

/************************/
/* Heap memory function */
/************************/
data heap;
data restheap;
int heaplen;
int heapcnt;

void init_heap(int len)
{
    int i;
    heap = (data)malloc(sizeof(dataimpl) * len);
    if (!heap)
        error(L"bad alloc.\n");

    memset(heap, 0, sizeof(dataimpl) * len);

    restheap = heap;
    heaplen = len;
    heapcnt = 0;
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
    int i;
    for (i = 0; i < heaplen; ++i)
        free_data(&heap[i]);
    free(heap);
    heap = NULL;
    restheap = NULL;
    heaplen = 0;
    heapcnt = 0;
}

void free_data(data d)
{
    if (!used(d))
        error(L"Invalid freedata to unused heap\n");
    if (is_symbol(d) || is_string(d))
        free(d->buffer._string);
    init_data(d);
    insert_node(restheap, d);
    heapcnt -= 1;
}

data _dump_heap(data d)
{
    wprintf(L"%05.1lf%% (%d/%d)\n", ((double)heapcnt / heaplen * 100), heapcnt, heaplen);
    wprintf(L"   addr info\n");
    for (int i = 0; i < heaplen; ++i)
    {
        wprintf(L"%c%c %04x ", (used(&heap[i]) ? L'u' : L' '), (marked(&heap[i]) ? L'm' : L' '), (int)heap_addr(&heap[i]));
        print(&heap[i]);
        wprintf(L"\n");
    }
    return(t);
}

void mark_data(data d)
{
    set_marked(d, 1);
    if (is_pair(d))
    {
        if (is_not_nil(car(d)))
            mark_data(car(d));
        if (is_not_nil(cdr(d)))
            mark_data(cdr(d));
    }
    else if (is_unnamed_function(d))
    {
        mark_data(get_args(d));
        mark_data(get_impl(d));
    }
}

void unmark_heap()
{
    int i;
    for (i = 0; i < heaplen; ++i)
        set_marked(&heap[i], 0);
}

data sweep_unmarked(data d)
{
    int i, cnt;
    cnt = 0;
    for (i = 0; i < heaplen; ++i)
    {
        if (used(&heap[i]) && !marked(&heap[i]))
        {
            free_data(&heap[i]);
            cnt += 1;
        }
    }
    return(make_int(cnt));
}

data request_gc(data d)
{
    if (((double)heapcnt / heaplen) > 0.75)
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
    if (heapcnt == heaplen)
        error(L"Heap memory allocation failed.\n");

    data newdata = pull_node(&restheap);
    heapcnt += 1;

    memset((void *)newdata, 0, sizeof(dataimpl));
    set_used(newdata, 1);
    return(newdata);
}

int heap_addr(data d)
{
    return((int)(d - heap));
}
