#include "print.h"
#include <stdio.h>
#include "repl.h"
#include "heap.h"

data print(data d)
{
    if (cconsp(d))
    {
        if (!used(d))
            wprintf(L"<unused heap>");
        else
        {
            wprintf(L"(");
            printlist(d);
            wprintf(L")");
        }
    }
    else if (cnilp(d))
        wprintf(L"nil");
    else if (ctp(d))
        wprintf(L"t");
    else if (csymp(d))
        wprintf(L"<symbol %s>", csym(d));
    else if (cfuncp(d))
        wprintf(L"<function %04x>", heapaddr(d));
    else if (cmacrop(d))
        wprintf(L"<macro %04x>", heapaddr(d));
    else if (cintp(d))
        wprintf(L"%d", cint(d));
    else if (cdblp(d))
        wprintf(L"%lf", cdbl(d));
    else if (cstrp(d))
        wprintf(L"\"%s\"", cstr(d));
    else if (clambdap(d))
        wprintf(L"<lambda %04x>", heapaddr(d));
    return(d);
}

data printlist(data d)
{
    print(car(d));
    if(cnnilp(cdr(d)))
    {
        wprintf(L" ");
        printlist(cdr(d));
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
