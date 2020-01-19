#include "repl.h"

#pragma warning(disable: 4996)

#include <stdio.h>
#include <setjmp.h>
#include "read.h"
#include "eval.h"
#include "print.h"
#include "heap.h"
#include "sym.h"

int parse_command_line_argment(int argc, const char ** argv);
void close_file_stream();

/********/
/* REPL */
/********/
jmp_buf jmpbuf;
int repl(int argc, const char ** argv)
{
    int ret;

    ret = 0;

    if (!parse_command_line_argment(argc, argv))
    {
        ret = -1;
        goto cleanup;
    }

    data d;
    int jmp = setjmp(jmpbuf);
    if (!jmp)
    {
        init_heap(0x400);
        init_symbol_stack();
    }

loop:
    if (jmp == 0)
        while (1)
        {
            wprintf(L"> "); fflush(stdout); fflush(stdin);
            d = read();

#if 0
            wprintf(L"read: ");
            print(d);
            wprintf(L"\n");
#endif

            if (d)
            {
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

cleanup:
    cleanup_symbol_stack();
    cleanup_heap();
    close_file_stream();
    return(ret);
}

void escape()
{
    longjmp(jmpbuf, 1);
}

int parse_command_line_argment(int argc, const char ** argv)
{
    FILE * file;
    const char * filename;

    if (argc < 2)
    {
        set_input_stream(stdin);
        return(1);
    }

    filename = argv[1];
    file = fopen(filename, "r");
    if (file)
    {
        set_input_stream(file);
        return(0);
    }

    return(0);
}

void close_file_stream()
{
    if (get_input_stream() != stdin)
    {
        fclose(get_input_stream());
        set_input_stream(stdin);
    }
}
