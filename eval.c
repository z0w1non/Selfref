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
        value = find_symbol(csym(d));
        if (!value)
            error(L"<symbol %s> is not found.\n", csym(d));
        return(value);
    }
    else if (is_cons(d))
    {
        value = eval(car(d));
        if (is_builtin_function(value))
            return(_call(makecons(value, eval_list(cdr(d)))));
        else if (is_builtin_macro(value))
            return(expand_macro(makecons(value, cdr(d))));
        else if (is_unnamed_function(value))
            return(call_function(makecons(value, cdr(d))));
    }
   return(d);
}

data eval_list(data d)
{
    if (is_nil(d))
        return(nil);
    if (!is_cons(d))
        error(L"evallist failed.\n");
    return(makecons(eval(car(d)), eval_list(cdr(d))));
}

data expand_macro(data d)
{
    if (!is_builtin_macro(car(d)))
        error(L"invalid expandmacro call.\n");
    return(cmacro(car(d))(cdr(d)));
}

// call dummyargs actualargs
data call_function(data d)
{
    data args, ret;
    if(!is_unnamed_function(car(d)))
        error(L"invalid calllambda call.\n");
    args = _zip(makecons(getargs(car(d)), makecons(cdr(d), nil)));
    _push_args(args);
    ret = eval(getimpl(car(d)));
    _pop_args(args);
    return(ret);
}
