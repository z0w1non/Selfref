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

    if (csymp(d))
    {
        value = findsym(csym(d));
        if (!value)
            error(L"<symbol %s> is not found.\n", csym(d));
        return(value);
    }
    else if (cconsp(d))
    {
        value = eval(car(d));
        if (cfuncp(value))
            return(call(makecons(value, evallist(cdr(d)))));
        else if (cmacrop(value))
            return(expandmacro(makecons(value, cdr(d))));
        else if (clambdap(value))
            return(calllambda(makecons(value, cdr(d))));
    }
   return(d);
}

data evallist(data d)
{
    if (cnilp(d))
        return(nil);
    if (!cconsp(d))
        error(L"evallist failed.\n");
    return(makecons(eval(car(d)), evallist(cdr(d))));
}

data expandmacro(data d)
{
    if (!cmacrop(car(d)))
        error(L"invalid expandmacro call.\n");
    return(cmacro(car(d))(cdr(d)));
}

// call dummyargs actualargs
data calllambda(data d)
{
    data args, ret;
    if(!clambdap(car(d)))
        error(L"invalid calllambda call.\n");
    args = zip(makecons(getargs(car(d)), makecons(cdr(d), nil)));
    pushargs(args);
    ret = eval(getimpl(car(d)));
    popargs(args);
    return(ret);
}
