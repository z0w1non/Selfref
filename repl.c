#include "repl.h"

#include <stdio.h>
#include <setjmp.h>
#include "read.h"
#include "eval.h"
#include "print.h"
#include "heap.h"
#include "sym.h"

/********/
/* REPL */
/********/
jmp_buf jmpbuf;
int repl()
{
    data d;
    int jmp = setjmp(jmpbuf);
    if (!jmp)
    {
        initheap(0x400);
        initsym();
    }

loop:
    if (jmp == 0)
        while (1)
        {
            wprintf(L"> "); fflush(stdout); fflush(stdin);
            d = readstdin();

#if 0
            wprintf(L"read: ");
            print(d);
            wprintf(L"\n");
#endif

            if (d)
            {
                //d = listize(d);
                print(eval(d));
                wprintf(L"\n"); fflush(stdout);
            }
            requestgc(nil);
        }
    else
    {
        if (jmp == 1)
        {
            jmp = 0;
            goto loop;
        }
    }

    freesym();
    freeheap();
    return(0);
}

void escape()
{
    longjmp(jmpbuf, 1);
}
