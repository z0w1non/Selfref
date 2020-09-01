#include "eval.h"

#include <stdlib.h>
#include "symbol.h"
#include "builtin.h"
#include "print.h"
#include "heap.h"

/********/
/* Eval */
/********/
data eval(data d)
{
    data value;
    if (is_symbol(d))
    {
        value = find_symbol(raw_string(d));
        if (!value)
            error(L"<symbol %s> is not found", raw_string(d));
        return(value);
    }
    else if (is_pair(d))
    {
        value = eval(car(d));
        if (is_builtin_macro(value))
            return(call_builtin_macro(make_pair(value, cdr(d))));
        else if (is_builtin_function(value))
            return(call_builtin_function(make_pair(value, cdr(d))));
        else if (is_unnamed_macro(value))
            return(call_unnamed_macro(make_pair(value, cdr(d))));
        else if (is_unnamed_function(value))
            return(call_unnamed_function(make_pair(value, cdr(d))));
        error(L"first element in list must be a function or macro");
    }
   return(d);
}

data _eval_list(data d)
{
    if (is_nil(d))
        return(nil);
    if (!is_pair(d))
        error(L"eval list failed");
    return(make_pair(eval(car(d)), _eval_list(cdr(d))));
}

// (call_macro arg1 arg2 arg3 ...)
data call_builtin_macro(data d)
{
    if (!is_builtin_macro(car(d)))
        error(L"invalid macro call");
    return(raw_macro(car(d))(cdr(d)));
}

// (call_function arg1 arg2 arg3 ...)
data call_builtin_function(data d)
{
    if (!is_builtin_function(car(d)))
        error(L"invalid function call");
    return(raw_function(car(d))(_eval_list(cdr(d))));
}

// (call_unnamed_macro arg1 arg2 arg3 ...)
data call_unnamed_macro(data d)
{
    data args, ret;
    if (!is_unnamed_macro(car(d)))
        error(L"invalid unnnamed macro call");
    args = nil;
    if (is_not_nil(cadr(d)))
        _push_args(args = _zip(make_pair(get_args(car(d)), make_pair(cdr(d), nil))));
    ret = eval(get_impl(car(d)));
    if (is_not_nil(cadr(d)))
        _pop_args(args);
    return(ret);
}

// (call_unnamed_function arg1 arg2 arg3 ...)
data call_unnamed_function(data d)
{
    data args, ret;
    if(!is_unnamed_function(car(d)))
        error(L"invalid function call");
    args = nil;
    if (is_not_nil(cadr(d)))
        _push_args(args = _zip(make_pair(get_args(car(d)), make_pair(_eval_list(cdr(d)), nil))));
    ret = eval(get_impl(car(d)));
    if (is_not_nil(cadr(d)))
        _pop_args(args);
    return(ret);
}