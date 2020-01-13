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
        init_heap(0x400);
        init_symbol_stack();
        init_operator_list();
    }

loop:
    if (jmp == 0)
        while (1)
        {
            wprintf(L"> "); fflush(stdout); fflush(stdin);
            d = read_stdin();

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
            request_gc(nil);
        }
    else
    {
        if (jmp == 1)
        {
            jmp = 0;
            goto loop;
        }
    }

    cleanup_symbol_stack();
    cleanup_heap();
    return(0);
}

void escape()
{
    longjmp(jmpbuf, 1);
}
