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

void initheap(int len)
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
        csetcar(&heap[i], &heap[((i - 1 + len) % len)]);
        csetcdr(&heap[i], &heap[((i + 1) % len)]);
    }
    nil = makenil();
    t = maket();
    quote = makequote();
}

void freeheap()
{
    int i;
    for (i = 0; i < heaplen; ++i)
        freedata(&heap[i]);
    free(heap);
    heap = NULL;
    restheap = NULL;
    heaplen = 0;
    heapcnt = 0;
}

void freedata(data d)
{
    if (!used(d))
        error(L"Invalid freedata to unused heap\n");
    if (csymp(d) || cstrp(d))
        free(((wchar_t **)getbuf(d))[0]);
    initdata(d);
    insertnode(restheap, d);
    heapcnt -= 1;
}

data dumpheap(data d)
{
    wprintf(L"%05.1lf%% (%d/%d)\n", ((double)heapcnt / heaplen * 100), heapcnt, heaplen);
    wprintf(L"   addr info\n");
    for (int i = 0; i < heaplen; ++i)
    {
        wprintf(L"%c%c %04x ", (used(&heap[i]) ? L'u' : L' '), (marked(&heap[i]) ? L'm' : L' '), (int)(&heap[i] - heap));
        print(&heap[i]);
        wprintf(L"\n");
    }
    return(t);
}

void markdata(data d)
{
    setmarked(d, 1);
    if (cconsp(d))
    {
        if (cnnilp(car(d)))
            markdata(car(d));
        if (cnnilp(cdr(d)))
            markdata(cdr(d));
    }
    else if (clambdap(d))
    {
        markdata(getargs(d));
        markdata(getimpl(d));
    }
}

void unmarkheap()
{
    int i;
    for (i = 0; i < heaplen; ++i)
        setmarked(&heap[i], 0);
}

data sweep(data d)
{
    int i, cnt;
    cnt = 0;
    for (i = 0; i < heaplen; ++i)
    {
        if (used(&heap[i]) && !marked(&heap[i]))
        {
            freedata(&heap[i]);
            cnt += 1;
        }
    }
    return(makeint(cnt));
}

data requestgc(data d)
{
    if (((double)heapcnt / heaplen) > 0.75)
        return(gc(d));
    return(nil);
}

data gc(data d)
{
    unmarkheap();
    marksym();
    return(sweep(nil));
}

data alloc()
{
    if (heapcnt == heaplen)
        error(L"Heap memory allocation failed.\n");

    data newdata = pullnode(&restheap);
    heapcnt += 1;

    memset((void *)newdata, 0, sizeof(dataimpl));
    setused(newdata, 1);
    return(newdata);
}

int heapaddr(data d)
{
    return((int)(heap - d));
}
