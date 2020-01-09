#include "builtin.h"

#include "print.h"
#include "eval.h"
#include "sym.h"

/**************/
/* Arithmetic */
/**************/
data inc(data d)
{
    if (cintp(car(d)))
        return(makeint(cint(car(d)) + 1));
    else if (cdblp(car(d)))
        return(makedbl(cdbl(car(d)) + 1.0));
    error(L"invalid argument type.\n");
}

data dec(data d)
{
    if (cintp(car(d)))
        return(makeint(cint(car(d)) - 1));
    else if (cdblp(car(d)))
        return(makedbl(cdbl(car(d)) - 1.0));
    error(L"invalid argument type.\n");
}

data add(data d)
{
    int i = 0;
    double f = 0;
    int contains_float = 0;
    while (cnnilp(car(d)))
    {
        if (contains_float)
        {
            if (cintp(car(d)))
                f += (double)cint(car(d));
            else if (cdblp(car(d)))
                f += cdbl(car(d));
            else
                error(L"invalid argument type.\n");
        }
        else
        {
            if (cintp(car(d)))
                i += cint(car(d));
            else if (cdblp(car(d)))
            {
                f = (double)i;
                f += cdbl(car(d));
                contains_float = 1;
            }
            else
                error(L"invalid argument type.\n");
        }
        d = cdr(d);
    }

    if (contains_float)
        return(makedbl(f));
    return(makeint(i));
}

data sub(data d)
{
    if (cnilp(car(d)))
        return(makeint(0));

    int i = 0;
    double f = 0;
    int contains_float = 0;

    if (cintp(car(d)))
    {
        i = cint(car(d));
    }
    else if (cdblp(car(d)))
    {
        f = cdbl(car(d));
        contains_float = 1;
    }
    d = cdr(d);

    while (cnnilp(car(d)))
    {
        if (contains_float)
        {
            if (cintp(car(d)))
                f -= (double)cint(car(d));
            else if (cdblp(car(d)))
                f -= cdbl(car(d));
            else
                error(L"invalid argument type.\n");
        }
        else
        {
            if (cintp(car(d)))
                i -= cint(car(d));
            else if (cdblp(car(d)))
            {
                f = (double)i;
                f -= cdbl(car(d));
                contains_float = 1;
            }
            else
                error(L"invalid argument type.\n");
        }
        d = cdr(d);
    }

    if (contains_float)
        return(makedbl(f));
    return(makeint(i));
}

data mul(data d)
{
    int i = 1;
    double f = 1;
    int contains_float = 0;
    while (cnnilp(car(d)))
    {
        if (contains_float)
        {
            if (cintp(car(d)))
                f *= (double)cint(car(d));
            else if (cdblp(car(d)))
                f *= cdbl(car(d));
            else
                error(L"invalid argument type.\n");
        }
        else
        {
            if (cintp(car(d)))
                i *= cint(car(d));
            else if (cdblp(car(d)))
            {
                f = (double)i;
                f *= cdbl(car(d));
                contains_float = 1;
            }
            else
                error(L"invalid argument type.\n");
        }
        d = cdr(d);
    }

    if (contains_float)
        return(makedbl(f));
    return(makeint(i));
}

data div_(data d)
{
    if (cnilp(car(d)))
        return(makeint(0));

    int i = 0;
    double f = 0;
    int contains_float = 0;

    if (cintp(car(d)))
        i = cint(car(d));
    else if (cdblp(car(d)))
    {
        f = cdbl(car(d));
        contains_float = 1;
    }
    else
        error(L"invalid argument type.\n");
    d = cdr(d);

    while (cnnilp(car(d)))
    {
        if (contains_float)
        {
            if (cintp(car(d)))
                f /= (double)cint(car(d));
            else if (cdblp(car(d)))
                f /= cdbl(car(d));
            else
                error(L"invalid argument type.\n");
        }
        else
        {
            if (cintp(car(d)))
                i /= cint(car(d));
            else if (cdblp(car(d)))
            {
                f = (double)i;
                f /= cdbl(car(d));
                contains_float = 1;
            }
            else
                error(L"invalid argument type.\n");
        }
        d = cdr(d);
    }

    if (contains_float)
        return(makedbl(f));
    return(makeint(i));
}

data mod(data d)
{
    if (cnilp(car(d)))
        return(makeint(0));

    int i = 0;

    if (cintp(car(d)))
        i = cint(car(d));
    else
        error(L"invalid argument type.\n");
    d = cdr(d);

    while (cnnilp(car(d)))
    {
        if (cintp(car(d)))
            i /= cint(car(d));
        else
            error(L"invalid argument type.\n");
        d = cdr(d);
    }

    return(makeint(i));
}

data if_(data d)
{
    if (cnnilp(car(d)))
        return(eval(car(cdr(d))));
    return(eval(car(cdr(cdr(d)))));
}

data and_(data d1)
{
    while (cnnilp(car(d1)))
    {
        data d2 = eval(car(d1));
        if (cnnilp(d2))
            return(nil);
        d1 = cdr(d1);
    }
    return(d1);
}

data or_(data d1)
{
    while (cnnilp(car(d1)))
    {
        data d2 = eval(car(d1));
        if (cnnilp(d2))
            return(d1);
        d1 = cdr(d1);
    }
    return(nil);
}

data not_(data d)
{
    return(nilp(d));
}

data call(data d)
{
    if (cfuncp(car(d)))
        return(cfunc(car(d))(cdr(d)));
    error(L"Call failed\n");
}

data resolvesym(data d)
{
    data value;
    if (cnilp(d))
        return(nil);
    if (cconsp(d))
        return(makecons(resolvesym(car(d)), resolvesym(cdr(d))));
    if (csymp(d))
    {
        value = findsym(d);
        if (value)
            return(value);
    }
    return(d);
}

data lambda(data d)
{

}

data function(data d)
{

}

data length(data d)
{
}

data setcar(data d)
{
    if (!cconsp(car(d)))
        error(L"setcar failed.\n");
    csetcar(car(d), cadr(d));
    return(cadr(d));
}

data setcdr(data d)
{
    if (!cconsp(car(d)))
        error(L"setcdr failed.\n");
    csetcdr(car(d), cadr(d));
    return(cadr(d));
}

data list(data d)
{
    return(d);
}

data zipfirst(data d)
{
    if (cnilp(caar(d)))
        return(nil);
    return(makecons(caar(d), zipfirst(cdr(d))));
}

data ziprest(data d)
{
    if (cnilp(cdar(d)))
        return(nil);
    return(makecons(cdar(d), ziprest(cdr(d))));
}

// (zip (a b c) (1 2 3))
// -> ((a 1) (b 2) (c 3))
data zip(data d)
{
    if (cnilp(d))
        return(nil);
    return(makecons(zipfirst(d), zip(ziprest(d))));
}
