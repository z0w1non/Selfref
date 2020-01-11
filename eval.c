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
    else if (is_cons(d))
    {
        value = eval(car(d));
        if (is_builtin_function(value))
            return(_call(make_pair(value, eval_list(cdr(d)))));
        else if (is_builtin_macro(value))
            return(call_macro(make_pair(value, cdr(d))));
        else if (is_unnamed_function(value))
            return(call_function(make_pair(value, cdr(d))));
    }
   return(d);
}

data eval_list(data d)
{
    if (is_nil(d))
        return(nil);
    if (!is_cons(d))
        error(L"evallist failed.\n");
    return(make_pair(eval(car(d)), eval_list(cdr(d))));
}

data call_macro(data d)
{
    if (!is_builtin_macro(car(d)))
        error(L"invalid expandmacro call.\n");
    return(raw_macro(car(d))(cdr(d)));
}

// call dummyargs actualargs
data call_function(data d)
{
    data args, ret;
    if(!is_unnamed_function(car(d)))
        error(L"invalid calllambda call.\n");
    args = _zip(make_pair(get_args(car(d)), make_pair(cdr(d), nil)));
    _push_args(args);
    ret = eval(get_impl(car(d)));
    _pop_args(args);
    return(ret);
}
