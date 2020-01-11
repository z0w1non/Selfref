#include "print.h"
#include <stdio.h>
#include "repl.h"
#include "heap.h"

data print(data d)
{
    if (is_pair(d))
    {
        if (!used(d))
            wprintf(L"<unused heap>");
        else
        {
            wprintf(L"(");
            print_list(d);
            wprintf(L")");
        }
    }
    else if (is_nil(d))
        wprintf(L"nil");
    else if (is_t(d))
        wprintf(L"t");
    else if (is_symbol(d))
        wprintf(L"<symbol %s>", raw_string(d));
    else if (is_builtin_macro(d))
        wprintf(L"<builtin_macro %04x>", heap_addr(d));
    else if (is_builtin_function(d))
        wprintf(L"<builtin_function %04x>", heap_addr(d));
    else if (is_unnamed_macro(d))
        wprintf(L"<macro %04x>", heap_addr(d));
    else if (is_unnamed_function(d))
        wprintf(L"<function %04x>", heap_addr(d));
    else if (is_int(d))
        wprintf(L"%d", raw_int(d));
    else if (is_double(d))
        wprintf(L"%lf", raw_double(d));
    else if (is_string(d))
        wprintf(L"\"%s\"", raw_string(d));
    else if (is_unnamed_function(d))
        wprintf(L"<lambda %04x>", heap_addr(d));
    return(d);
}

data print_list(data d)
{
    print(car(d));
    if(is_not_nil(cdr(d)))
    {
        wprintf(L" ");
        print_list(cdr(d));
    }
    return(d);
}

void error(const wchar_t * format, ...)
{
    va_list args;
    fwprintf(stdout, L"ERROR: ");
    va_start(args, format);
    vfwprintf(stdout, format, args);
    va_end(args);
    fwprintf(stdout, L"\n");
    escape();
}
