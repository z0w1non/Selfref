#include "builtin.h"

#include "print.h"
#include "eval.h"
#include "sym.h"
#include "heap.h"
#include "eval.h"

void init_builtin()
{
    cpush_symbol(L"nil", nil);
    cpush_symbol(L"t", t);
    cpush_symbol(L"quote", quote);

    cpush_symbol(L"eval", make_builtin_function(_eval));
    cpush_symbol(L"print", make_builtin_function(_print));

    cpush_symbol(L"dump_symbol", make_builtin_function(_dump_symbol));
    cpush_symbol(L"dump_heap", make_builtin_function(_dump_heap));
    cpush_symbol(L"gc", make_builtin_function(_gc));

    cpush_symbol(L"push_args", make_builtin_macro(_push_args));
    cpush_symbol(L"pop_args", make_builtin_macro(_pop_args));

    cpush_symbol(L"pair", make_builtin_function(_pair));
    cpush_symbol(L"set_first", make_builtin_function(_set_first));
    cpush_symbol(L"set_rest", make_builtin_function(_set_rest));

    cpush_symbol(L"list", make_builtin_function(_list));
    cpush_symbol(L"length", make_builtin_function(_length));

    cpush_symbol(L"if", make_builtin_macro(_if));
    cpush_symbol(L"and", make_builtin_macro(_and));
    cpush_symbol(L"or", make_builtin_macro(_or));
    cpush_symbol(L"not", make_builtin_function(_not));

    cpush_symbol(L"eval", make_builtin_macro(eval));
    cpush_symbol(L"call", make_builtin_macro(_call));
    cpush_symbol(L"bind_symbol", make_builtin_macro(_bind_symbol));
    cpush_symbol(L"unnamed_macro", make_builtin_macro(_unnamed_macro));
    cpush_symbol(L"unnamed_function", make_builtin_macro(_unnamed_function));
    cpush_symbol(L"macro", make_builtin_macro(_macro));
    cpush_symbol(L"function", make_builtin_macro(_function));

    cpush_symbol(L"left_associative_operator", make_builtin_macro(_left_associative_operator));
    cpush_symbol(L"right_associative_operator", make_builtin_macro(_right_associative_operator));

    cpush_symbol(L"is_cons", make_builtin_function(_is_pair));
    cpush_symbol(L"is_builtin_macro", make_builtin_function(_is_builtin_macro));
    cpush_symbol(L"is_builtin_function", make_builtin_function(_is_builtin_function));
    cpush_symbol(L"is_unnamed_macro", make_builtin_function(_is_unnamed_macro));
    cpush_symbol(L"is_unnamed_function", make_builtin_function(_is_unnamed_function));
    cpush_symbol(L"is_symbol", make_builtin_function(_is_symbol));
    cpush_symbol(L"is_nil", make_builtin_function(_is_nil));
    cpush_symbol(L"is_int", make_builtin_function(_is_int));
    cpush_symbol(L"is_double", make_builtin_function(_is_double));
    cpush_symbol(L"is_number", make_builtin_function(_is_number));
    cpush_symbol(L"is_string", make_builtin_function(_is_string));
    cpush_symbol(L"is_zero", make_builtin_function(_is_zero));

    cpush_symbol(L"inc", make_builtin_function(_inc));
    cpush_symbol(L"dec", make_builtin_function(_dec));
    cpush_symbol(L"add", make_builtin_function(_add));
    cpush_symbol(L"sub", make_builtin_function(_sub));
    cpush_symbol(L"mul", make_builtin_function(_mul));
    cpush_symbol(L"div", make_builtin_function(_div));
    cpush_symbol(L"mod", make_builtin_function(_mod));

    cpush_symbol(L"zip_first", make_builtin_function(_zip_first));
    cpush_symbol(L"zip_rest", make_builtin_function(_zip_rest));
    cpush_symbol(L"zip", make_builtin_function(_zip));

    cpush_symbol(L"push_symbol", make_builtin_macro(_push_symbol));
    cpush_symbol(L"pop_symbol", make_builtin_macro(_pop_symbol));

    cpush_symbol(L"to_char_code", make_builtin_function(_to_char_code));
    cpush_symbol(L"from_char_code", make_builtin_function(_from_char_code));

    cpush_symbol(L"progn", make_builtin_macro(_progn));
    cpush_symbol(L"let", make_builtin_macro(_let));
}

/**************/
/* Arithmetic */
/**************/
data _inc(data d)
{
    if (is_int(car(d)))
        return(make_int(raw_int(car(d)) + 1));
    else if (is_double(car(d)))
        return(make_double(raw_double(car(d)) + 1.0));
    error(L"invalid argument type.\n");
    return(nil);
}

data _dec(data d)
{
    if (is_int(car(d)))
        return(make_int(raw_int(car(d)) - 1));
    else if (is_double(car(d)))
        return(make_double(raw_double(car(d)) - 1.0));
    error(L"invalid argument type.\n");
    return(nil);
}

/*********************************/
/* Variacdic arithmetic function */
/*********************************/
data _add(data d)
{
    int i = 0;
    double f = 0;
    int contains_float = 0;
    while (is_not_nil(car(d)))
    {
        if (contains_float)
        {
            if (is_int(car(d)))
                f += (double)raw_int(car(d));
            else if (is_double(car(d)))
                f += raw_double(car(d));
            else
                error(L"invalid argument type.\n");
        }
        else
        {
            if (is_int(car(d)))
                i += raw_int(car(d));
            else if (is_double(car(d)))
            {
                f = (double)i;
                f += raw_double(car(d));
                contains_float = 1;
            }
            else
                error(L"invalid argument type.\n");
        }
        d = cdr(d);
    }

    if (contains_float)
        return(make_double(f));
    return(make_int(i));
}

data _sub(data d)
{
    if (is_nil(car(d)))
        return(make_int(0));

    int i = 0;
    double f = 0;
    int contains_float = 0;

    if (is_int(car(d)))
    {
        i = raw_int(car(d));
    }
    else if (is_double(car(d)))
    {
        f = raw_double(car(d));
        contains_float = 1;
    }
    d = cdr(d);

    while (is_not_nil(car(d)))
    {
        if (contains_float)
        {
            if (is_int(car(d)))
                f -= (double)raw_int(car(d));
            else if (is_double(car(d)))
                f -= raw_double(car(d));
            else
                error(L"invalid argument type.\n");
        }
        else
        {
            if (is_int(car(d)))
                i -= raw_int(car(d));
            else if (is_double(car(d)))
            {
                f = (double)i;
                f -= raw_double(car(d));
                contains_float = 1;
            }
            else
                error(L"invalid argument type.\n");
        }
        d = cdr(d);
    }

    if (contains_float)
        return(make_double(f));
    return(make_int(i));
}

data _mul(data d)
{
    int i = 1;
    double f = 1;
    int contains_float = 0;
    while (is_not_nil(car(d)))
    {
        if (contains_float)
        {
            if (is_int(car(d)))
                f *= (double)raw_int(car(d));
            else if (is_double(car(d)))
                f *= raw_double(car(d));
            else
                error(L"invalid argument type.\n");
        }
        else
        {
            if (is_int(car(d)))
                i *= raw_int(car(d));
            else if (is_double(car(d)))
            {
                f = (double)i;
                f *= raw_double(car(d));
                contains_float = 1;
            }
            else
                error(L"invalid argument type.\n");
        }
        d = cdr(d);
    }

    if (contains_float)
        return(make_double(f));
    return(make_int(i));
}

data _div(data d)
{
    if (is_nil(car(d)))
        return(make_int(0));

    int i = 0;
    double f = 0;
    int contains_float = 0;

    if (is_int(car(d)))
        i = raw_int(car(d));
    else if (is_double(car(d)))
    {
        f = raw_double(car(d));
        contains_float = 1;
    }
    else
        error(L"invalid argument type.\n");
    d = cdr(d);

    while (is_not_nil(car(d)))
    {
        if (contains_float)
        {
            if (is_int(car(d)))
                f /= (double)raw_int(car(d));
            else if (is_double(car(d)))
                f /= raw_double(car(d));
            else
                error(L"invalid argument type.\n");
        }
        else
        {
            if (is_int(car(d)))
                i /= raw_int(car(d));
            else if (is_double(car(d)))
            {
                f = (double)i;
                f /= raw_double(car(d));
                contains_float = 1;
            }
            else
                error(L"invalid argument type.\n");
        }
        d = cdr(d);
    }

    if (contains_float)
        return(make_double(f));
    return(make_int(i));
}

data _mod(data d)
{
    if (is_nil(car(d)))
        return(make_int(0));

    int i = 0;

    if (is_int(car(d)))
        i = raw_int(car(d));
    else
        error(L"invalid argument type.\n");
    d = cdr(d);

    while (is_not_nil(car(d)))
    {
        if (is_int(car(d)))
            i /= raw_int(car(d));
        else
            error(L"invalid argument type.\n");
        d = cdr(d);
    }

    return(make_int(i));
}

/*******************/
/* Binary operator */
/*******************/
data _add_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(make_int(raw_int(car(d)) + raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(make_double(raw_int(car(d)) + raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(make_double(raw_double(car(d)) + raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(make_double(raw_double(car(d)) + raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _sub_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(make_int(raw_int(car(d)) - raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(make_double(raw_int(car(d)) - raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(make_double(raw_double(car(d)) - raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(make_double(raw_double(car(d)) - raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _mul_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(make_int(raw_int(car(d)) * raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(make_double(raw_int(car(d)) * raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(make_double(raw_double(car(d)) * raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(make_double(raw_double(car(d)) * raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _div_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(make_int(raw_int(car(d)) / raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(make_double(raw_int(car(d)) / raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(make_double(raw_double(car(d)) / raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(make_double(raw_double(car(d)) / raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _mod_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(make_int(raw_int(car(d)) % raw_int(cadr(d))));
    error(L"invalid argument.\n");
}

/**************/
/* Comparator */
/**************/
data _less_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nilort(raw_int(car(d)) < raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nilort(raw_int(car(d)) < raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nilort(raw_double(car(d)) < raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nilort(raw_double(car(d)) < raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _less_equal_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nilort(raw_int(car(d)) <= raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nilort(raw_int(car(d)) <= raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nilort(raw_double(car(d)) <= raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nilort(raw_double(car(d)) <= raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _greater_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nilort(raw_int(car(d)) > raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nilort(raw_int(car(d)) > raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nilort(raw_double(car(d)) > raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nilort(raw_double(car(d)) > raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _greater_equal_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nilort(raw_int(car(d)) >= raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nilort(raw_int(car(d)) >= raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nilort(raw_double(car(d)) >= raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nilort(raw_double(car(d)) >= raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _equal_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nilort(raw_int(car(d)) == raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nilort(raw_int(car(d)) == raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nilort(raw_double(car(d)) == raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nilort(raw_double(car(d)) == raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _not_equal_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nilort(raw_int(car(d)) != raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nilort(raw_int(car(d)) != raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nilort(raw_double(car(d)) != raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nilort(raw_double(car(d)) != raw_double(cadr(d))));
    error(L"invalid argument.\n");
}

data _assign(data d)
{
    data ret;
    if (!is_symbol(car(d)))
        error(L"An invalid value is specified as the assignment destination.\n");
    ret = replace_symbol(raw_string(car(d)), eval(cadr(d)));
    if (!ret)
        error(L"<symbol %s> is not found.\n", raw_string(car(d)));
    return(ret);
}

/***************/
/* Conditional */
/***************/
data _if(data d)
{
    if (is_not_nil(eval(car(d))))
        return(eval(car(cdr(d))));
    return(eval(car(cdr(cdr(d)))));
}

data _and(data d1)
{
    while (is_not_nil(car(d1)))
    {
        data d2 = eval(car(d1));
        if (is_not_nil(d2))
            return(nil);
        d1 = cdr(d1);
    }
    return(d1);
}

data _or(data d1)
{
    while (is_not_nil(car(d1)))
    {
        data d2 = eval(car(d1));
        if (is_not_nil(d2))
            return(d1);
        d1 = cdr(d1);
    }
    return(nil);
}

data _not(data d)
{
    return(_is_nil(d));
}

/**************/
/* Functional */
/**************/
data _eval(data d)
{
    return(car(d));
}

data _print(data d)
{
    return(car(d));
}

data _call(data d)
{
    if (is_builtin_function(car(d)))
        return(raw_function(car(d))(cdr(d)));
    error(L"Call failed\n");
    return(nil);
}

data _bind_symbol(data d)
{
    data value;
    if (is_nil(d))
        return(nil);
    if (is_pair(d))
        return(make_pair(_bind_symbol(car(d)), _bind_symbol(cdr(d))));
    if (is_symbol(d))
    {
        value = find_symbol(raw_string(d));
        if (value)
            return(value);
    }
    return(d);
}

data _unnamed_macro(data d)
{
    return(make_macro(car(d), _bind_symbol(cadr(d))));
}

data _unnamed_function(data d)
{
    return(make_function(car(d), _bind_symbol(cadr(d))));
}

data _macro(data d)
{
    if (!is_symbol(car(d)))
        error(L"invalid macro name.\n");
    cpush_symbol(raw_string(car(d)), _unnamed_macro(cdr(d)));
    return(car(d));
}

data _function(data d)
{
    if (!is_symbol(car(d)))
        error(L"invalid function name.\n");
    cpush_symbol(raw_string(car(d)), _unnamed_function(cdr(d)));
    return(car(d));
}

/************************/
/* Operator declaration */
/************************/
// (left_associative_operator => (args impl) (unnamed_function args impl))
data _left_associative_operator(data d)
{
    if (!is_symbol(car(d)))
        error(L"invalid operator name.\n");
    make_left_associative_operator(raw_string(car(d)), _unnamed_function(cdr(d)));
    return(car(d));
}

// (right_associative_operator = (destination value) (push_symbol destination value))
data _right_associative_operator(data d)
{
    if (!is_symbol(car(d)))
        error(L"invalid operator name.\n");
    make_right_associative_operator(raw_string(car(d)), _unnamed_function(cdr(d)));
    return(car(d));
}

/**********************/
/* Cons-cell function */
/**********************/
data _pair(data d)
{
    return(make_pair(car(d), cadr(d)));
}

data _set_first(data d)
{
    if (!is_pair(car(d)))
        error(L"setcar failed.\n");
    set_car(car(d), cadr(d));
    return(cadr(d));
}

data _set_rest(data d)
{
    if (!is_pair(car(d)))
        error(L"setcdr failed.\n");
    set_cdr(car(d), cadr(d));
    return(cadr(d));
}

/*****************/
/* List function */
/*****************/
data _list(data d)
{
    return(d);
}

data _length(data d)
{
    int len;
    len = 0;
    d = car(d);
    while (is_not_nil(d))
    {
        len += 1;
        d = cdr(d);
    }
    return(make_int(len));
}

/******************/
/* Zip algorithm */
/******************/
data _zip_first(data d)
{
    if (is_nil(caar(d)))
        return(nil);
    return(make_pair(caar(d), _zip_first(cdr(d))));
}

data _zip_rest(data d)
{
    if (is_nil(cdar(d)))
        return(nil);
    return(make_pair(cdar(d), _zip_rest(cdr(d))));
}

// (zip (a b c) (1 2 3))
// -> ((a 1) (b 2) (c 3))
data _zip(data d)
{
    if (is_nil(d))
        return(nil);
    return(make_pair(_zip_first(d), _zip(_zip_rest(d))));
}

/*************************/
/* Symbol stack function */
/*************************/
data _push_symbol(data d)
{
    if (!is_symbol(car(d)))
        error(L"pushsym failed.\n");
    cpush_symbol(raw_string(car(d)), cadr(d));
    return(d);
}

data _pop_symbol(data d)
{
    if (!is_symbol(car(d)))
        error(L"popsym failed.\n");
    cpop_symbol(raw_string(car(d)));
    return(d);
}

/****************************/
/* String integer interface */
/****************************/
data _to_char_code(data d)
{
    data list;
    const wchar_t * first, * last;

    list = nil;
    if (!is_string(car(d)))
        error(L"charcode source is must be a string literal.\n");
    first = raw_string(car(d));
    for (last = first; *last; ++last);
    --last;
    while (first <= last)
    {
        list = make_pair(make_int((int)(*last)), list);
        --last;
    }
    return(list);
}

data _from_char_code(data d)
{
    wchar_t buf[1024];
    wchar_t * s;
    s = buf;
    while ((is_int(car(d))))
    {
        if (s - buf < 1024 - 1)
            error(L"fromcharcode buffer overrun.\n");
        *(s++) = (wchar_t)raw_int(car(d));
        d = cdr(d);
    }
    *s = L'\0';
    return(make_string(buf));
}

data _progn(data d)
{
    data last;
    last = nil;
    while (is_not_nil(car(d)))
    {
        last = eval(car(d));
        d = cdr(d);
    }
    return(last);
}

// (let (a b c) (...))
// -> (let ((a nil) (b nil) (c nil)) (...))
data complement_args(data args)
{
    if (is_nil(args))
        return(nil);
    if (is_pair(car(args)))
        return(make_pair(car(args), cdr(args)));
    return(make_pair(make_pair(car(args), make_pair(nil, nil)), complement_args(cdr(args))));
}

data _let(data d)
{
    data last, args;
    args = complement_args(car(d));
    debug(args);
    _push_args(args);
    d = cdr(d);
    last = nil;
    while (is_not_nil(car(d)))
    {
        last = eval(car(d));
        d = cdr(d);
    }
    _pop_args(args);
    return(last);
}
