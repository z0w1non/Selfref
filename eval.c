#include "eval.h"

#include "sym.h"
#include "builtin.h"
#include "print.h"

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
            error(L"<symbol %s> is not found.\n", raw_string(d));
        return(value);
    }
    else if (is_pair(d))
    {
        value = eval(car(d));
        if (is_builtin_function(value))
            return(_call(make_pair(value, _eval_list(cdr(d)))));
        else if (is_builtin_macro(value))
            return(call_macro(make_pair(value, cdr(d))));
        else if (is_unnamed_macro(value))
            return(call_macro(make_pair(value, cdr(d))));
        else if (is_unnamed_function(value))
            return(call_function(make_pair(value, cdr(d))));
        return(value); //???
    }
   return(d);
}

data _eval_list(data d)
{
    if (is_nil(d))
        return(nil);
    if (!is_pair(d))
        error(L"eval list failed.\n");
    return(make_pair(eval(car(d)), _eval_list(cdr(d))));
}

// (call_macro actualargs)
data call_macro(data d)
{
    if (!is_builtin_macro(car(d)))
        error(L"invalid macro call.\n");
    return(raw_macro(car(d))(cdr(d)));
}

// (call_function actualargs)
data call_function(data d)
{
    data args, ret;
    if(!is_unnamed_function(car(d)))
        error(L"invalid function call.\n");
    args = _zip(make_pair(get_args(car(d)), make_pair(cdr(d), nil)));
    if (is_not_nil(car(args)))
        _push_args(args);
    ret = eval(get_impl(car(d)));
    if (is_not_nil(car(args)))
        _pop_args(args);
    return(ret);
}
