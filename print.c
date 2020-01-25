#include "print.h"
#include <stdio.h>
#include "repl.h"
#include "heap.h"

data print_pair(data);
data print_list_internal(data);
data print_function(data);

/*******************/
/* Public function */
/*******************/
data print(data d)
{
    if (!used(d))
        wprintf(L"<unused heap>");
    else if (is_pair(d))
        return(print_pair(d));
    else if (is_nil(d))
        wprintf(L"nil");
    else if (is_t(d))
        wprintf(L"t");
    else if (is_symbol(d))
        wprintf(L"<symbol %s>", raw_string(d));
    else if (is_builtin_macro(d))
        wprintf(L"<builtin macro %s>", raw_string(d));
    else if (is_builtin_function(d))
        wprintf(L"<builtin function %s>", raw_string(d));
    else if (is_unnamed_macro(d))
        wprintf(L"<macro %04x>", heap_address(d));
    else if (is_unnamed_function(d))
        print_function(d);
    else if (is_int(d))
        wprintf(L"%d", raw_int(d));
    else if (is_double(d))
        wprintf(L"%lf", raw_double(d));
    else if (is_string(d))
        wprintf(L"\"%s\"", raw_string(d));
    else if (is_unnamed_function(d))
        wprintf(L"<lambda %04x>", heap_address(d));
    return(d);
}

data print_pair(data d)
{
    if (is_pair(cdr(d)) && !is_nil(cdr(d)))
        return(print_list(d));
    wprintf(L"(");
    print(car(d));
    wprintf(L" . ");
    print(cdr(d));
    wprintf(L")");
    return(d);
}

data print_list(data d)
{
    wprintf(L"(");
    print_list_internal(d);
    wprintf(L")");
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

/********************/
/* Private function */
/********************/
data print_list_internal(data d)
{
    print(car(d));
    if (is_not_nil(cdr(d)))
    {
        wprintf(L" ");
        print_list_internal(cdr(d));
    }
    return(d);
}

data print_function(data d)
{
    if (!is_unnamed_function(d))
        error(L"print function failed");
    wprintf(L"<function ");
    print(get_args(d));
    wprintf(L" ");
    print(get_impl(d));
    wprintf(L">");
    return(d);
}
