#ifndef HEAP_H
#define HEAP_H

#include "data.h"

void initheap(int len);
void freeheap();
void freedata(data);
data dumpheap(data);
void markdata(data);
void unmarkheap();
data requestgc(data);
data sweep(data);
data gc(data);
data alloc();
int heapaddr(data);

#endif
