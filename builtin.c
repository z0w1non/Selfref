#include "builtin.h"

#include <stdlib.h>
#include "print.h"
#include "eval.h"
#include "symbol.h"
#include "heap.h"
#include "eval.h"
#include "fileio.h"

/**************************/
/* Builtin function value */
/**************************/
data print_v;
data print_line_v;
data dump_symbol_v;
data dump_heap_v;
data gc_v;
data push_args_v;
data pop_args_v;
data pair_v;
data set_first_v;
data set_rest_v;
data list_v;
data length_v;
data reverse_v;
data if_v;
data and_v;
data or_v;
data not_v;
data eval_v;
data call_v;
data apply_v;
data bind_symbol_v;
data unnamed_macro_v;
data unnamed_function_v;
data macro_v;
data function_v;
data left_associative_operator_v;
data right_associative_operator_v;
data is_cons_v;
data is_builtin_macro_v;
data is_builtin_function_v;
data is_unnamed_macro_v;
data is_unnamed_function_v;
data is_symbol_v;
data is_nil_v;
data is_int_v;
data is_double_v;
data is_number_v;
data is_string_v;
data is_zero_v;
data inc_v;
data dec_v;
data add_v;
data sub_v;
data mul_v;
data div_v;
data mod_v;
data zip_first_v;
data zip_rest_v;
data zip_v;
data push_symbol_v;
data pop_symbol_v;
data to_char_code_v;
data from_char_code_v;
data acons_v;
data assoc_v;
data progn_v;
data let_v;
data while_v;
data mapcar_v;
data strcat_v;
data substr_v;
data read_file_v;
data write_file_v;

/*********************************/
/* Builtin binary operator value */
/*********************************/
data _assignment_v;
data _addition_assignment_v;
data _substraction_assignment_v;
data _multiplication_assignment_v;
data _division_assignment_v;
data _modulo_assignment_v;
data _unnamed_function_v;
data _less_2op_v;
data _less_equal_2op_v;
data _greater_2op_v;
data _greater_equal_2op_v;
data _equal_2op_v;
data _not_equal_2op_v;
data _sub_2op_v;
data _add_2op_v;
data _mod_2op_v;
data _div_2op_v;
data _mul_2op_v;
data _arithmetic_left_shift_v;
data _arithmetic_right_shift_v;
data _logical_left_shift_v;
data _logical_right_shift_v;
data _logical_or_2op_v;
data _logical_and_2op_v;
data _bit_or_2op_v;
data _bit_xor_2op_v;
data _bit_and_2op_v;

data _positive_v;
data _negative_v;

void init_builtin()
{
    /* symbol */
    push_symbol(L"nil", nil);
    push_symbol(L"t", t);
    push_symbol(L"quote", quote);
    push_symbol(L"print", print_v = make_builtin_function(_print, L"print"));
    push_symbol(L"print_line", print_line_v = make_builtin_function(_print_line, L"print_line"));
    push_symbol(L"dump_symbol", dump_symbol_v = make_builtin_function(_dump_symbol, L"dump_symbol"));
    push_symbol(L"dump_heap", dump_heap_v = make_builtin_function(_dump_heap, L"dump_heap"));
    push_symbol(L"gc", gc_v = make_builtin_function(_gc, L"gc"));
    push_symbol(L"push_args", push_args_v = make_builtin_macro(_push_args, L"push_args"));
    push_symbol(L"pop_args", pop_args_v = make_builtin_macro(_pop_args, L"pop_args"));
    push_symbol(L"pair", pair_v = make_builtin_function(_pair, L"pair"));
    push_symbol(L"set_first", set_first_v = make_builtin_function(_set_first, L"set_first"));
    push_symbol(L"set_rest", set_rest_v = make_builtin_function(_set_rest, L"set_rest"));
    push_symbol(L"list", list_v = make_builtin_function(_list, L"list"));
    push_symbol(L"length", length_v = make_builtin_function(_length, L"length"));
    push_symbol(L"reverse", reverse_v = make_builtin_function(_reverse, L"reverse"));
    push_symbol(L"if", if_v = make_builtin_macro(_if, L"if"));
    push_symbol(L"and", and_v = make_builtin_macro(_and, L"and"));
    push_symbol(L"or", or_v = make_builtin_macro(_or, L"or"));
    push_symbol(L"not", not_v = make_builtin_function(_not, L"not"));
    push_symbol(L"eval", eval_v = make_builtin_macro(eval, L"eval"));
    push_symbol(L"call", call_v = make_builtin_macro(_call, L"call"));
    push_symbol(L"apply", apply_v = make_builtin_macro(_apply, L"apply"));
    push_symbol(L"bind_symbol", bind_symbol_v = make_builtin_macro(_bind_symbol, L"bind_symbol"));
    push_symbol(L"unnamed_macro", unnamed_macro_v = make_builtin_macro(_unnamed_macro, L"unnamed_macro"));
    push_symbol(L"unnamed_function", unnamed_function_v = make_builtin_macro(_unnamed_function, L"unnamed_function"));
    push_symbol(L"macro", macro_v = make_builtin_macro(_macro, L"macro"));
    push_symbol(L"function", function_v = make_builtin_macro(_function, L"function"));
    push_symbol(L"is_cons", is_cons_v = make_builtin_function(_is_pair, L"is_pair"));
    push_symbol(L"is_builtin_macro", is_builtin_macro_v = make_builtin_function(_is_builtin_macro, L"is_builtin_macro"));
    push_symbol(L"is_builtin_function", is_builtin_function_v = make_builtin_function(_is_builtin_function, L"is_builtin_function"));
    push_symbol(L"is_unnamed_macro", is_unnamed_macro_v = make_builtin_function(_is_unnamed_macro, L"is_unnamed_macro"));
    push_symbol(L"is_unnamed_function", is_unnamed_function_v = make_builtin_function(_is_unnamed_function, L"is_unnamed_function"));
    push_symbol(L"is_symbol", is_symbol_v = make_builtin_function(_is_symbol, L"is_symbol"));
    push_symbol(L"is_nil", is_nil_v = make_builtin_function(_is_nil, L"is_nil"));
    push_symbol(L"is_int", is_int_v = make_builtin_function(_is_int, L"is_int"));
    push_symbol(L"is_double", is_double_v = make_builtin_function(_is_double, L"is_double"));
    push_symbol(L"is_number", is_number_v = make_builtin_function(_is_number, L"is_number"));
    push_symbol(L"is_string", is_string_v = make_builtin_function(_is_string, L"is_string"));
    push_symbol(L"is_zero", is_zero_v = make_builtin_function(_is_zero, L"is_zero"));
    push_symbol(L"inc", inc_v = make_builtin_function(_inc, L"inc"));
    push_symbol(L"dec", dec_v = make_builtin_function(_dec, L"dec"));
    push_symbol(L"add", add_v = make_builtin_function(_add, L"add"));
    push_symbol(L"sub", sub_v = make_builtin_function(_sub, L"sub"));
    push_symbol(L"mul", mul_v = make_builtin_function(_mul, L"mul"));
    push_symbol(L"div", div_v = make_builtin_function(_div, L"div"));
    push_symbol(L"mod", mod_v = make_builtin_function(_mod, L"mod"));
    push_symbol(L"zip_first", zip_first_v = make_builtin_function(_zip_first, L"zip_first"));
    push_symbol(L"zip_rest", zip_rest_v = make_builtin_function(_zip_rest, L"zip_rest"));
    push_symbol(L"zip", zip_v = make_builtin_function(_zip, L"zip"));
    push_symbol(L"push_symbol", push_symbol_v = make_builtin_macro(_push_symbol, L"push_symbol"));
    push_symbol(L"pop_symbol", pop_symbol_v = make_builtin_macro(_pop_symbol, L"pop_symbol"));
    push_symbol(L"to_char_code", to_char_code_v = make_builtin_function(_to_char_code, L"to_char_code"));
    push_symbol(L"from_char_code", from_char_code_v = make_builtin_function(_from_char_code, L"from_char_code"));
    push_symbol(L"acons", acons_v = make_builtin_function(_acons, L"acons"));
    push_symbol(L"assoc", assoc_v = make_builtin_function(_assoc, L"assoc"));
    push_symbol(L"progn", progn_v = make_builtin_macro(_progn, L"progn"));
    push_symbol(L"let", let_v = make_builtin_macro(_let, L"let"));
    push_symbol(L"while", while_v = make_builtin_macro(_while, L"while"));
    push_symbol(L"mapcar", mapcar_v = make_builtin_function(_mapcar, L"mapcar"));
    push_symbol(L"strcat", strcat_v = make_builtin_function(_strcat, L"strcat"));
    push_symbol(L"substr", substr_v = make_builtin_function(_substr, L"substr"));
    push_symbol(L"read_file", read_file_v = make_builtin_function(_read_file, L"read_file"));
    push_symbol(L"write_file", write_file_v = make_builtin_function(_write_file, L"write_file"));

    /* Arithmetic operator */
    _add_2op_v = make_builtin_function(_add_2op, L"+");
    _sub_2op_v = make_builtin_function(_sub_2op, L"-");
    _mul_2op_v = make_builtin_function(_mul_2op, L"*");
    _div_2op_v = make_builtin_function(_div_2op, L"/");
    _mod_2op_v = make_builtin_function(_mod_2op, L"%");

    /* Assignment operator */
    _assignment_v = make_builtin_macro(_assignment, L"=");
    _addition_assignment_v = make_builtin_macro(_addition_assignment, L"+=");
    _substraction_assignment_v = make_builtin_macro(_substraction_assignment, L"-=");
    _multiplication_assignment_v = make_builtin_macro(_multiplication_assignment, L"*=");
    _division_assignment_v = make_builtin_macro(_division_assignment, L"/=");
    _modulo_assignment_v = make_builtin_macro(_modulo_assignment, L"%=");

    /* Relational operator */
    _equal_2op_v = make_builtin_function(_equal_2op, L"==");
    _not_equal_2op_v = make_builtin_function(_not_equal_2op, L"!=");
    _less_2op_v = make_builtin_function(_less_2op, L"<");
    _less_equal_2op_v = make_builtin_function(_less_equal_2op, L"<=");
    _greater_2op_v = make_builtin_function(_greater_2op, L">");
    _greater_equal_2op_v = make_builtin_function(_greater_equal_2op, L">=");

    /* Logical / bit operator */
    _logical_or_2op_v = make_builtin_macro(_logical_or_2op, L"||");
    _logical_and_2op_v = make_builtin_macro(_logical_and_2op, L"&&");
    _bit_or_2op_v = make_builtin_function(_bit_or_2op, L"|");
    _bit_xor_2op_v = make_builtin_function(_bit_xor_2op, L"^");
    _bit_and_2op_v = make_builtin_function(_bit_and_2op, L"&");

    /* Shift operator */
    _arithmetic_left_shift_v = make_builtin_function(_arithmetic_left_shift, L"<<");
    _arithmetic_right_shift_v = make_builtin_function(_arithmetic_right_shift, L">>");
    _logical_left_shift_v = make_builtin_function(_logical_left_shift, L"<<<");
    _logical_right_shift_v = make_builtin_function(_logical_right_shift, L">>>");

    /* Unry operator */
    _positive_v = make_builtin_function(_positive, L"+");
    _negative_v = make_builtin_function(_negative, L"-");
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
    error(L"invalid argument type");
    return(nil);
}

data _dec(data d)
{
    if (is_int(car(d)))
        return(make_int(raw_int(car(d)) - 1));
    else if (is_double(car(d)))
        return(make_double(raw_double(car(d)) - 1.0));
    error(L"invalid argument type");
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
                error(L"invalid argument type");
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
                error(L"invalid argument type");
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
                error(L"invalid argument type");
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
                error(L"invalid argument type");
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
                error(L"invalid argument type");
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
                error(L"invalid argument type");
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
        error(L"invalid argument type");
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
                error(L"invalid argument type");
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
                error(L"invalid argument type");
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
        error(L"invalid argument type");
    d = cdr(d);

    while (is_not_nil(car(d)))
    {
        if (is_int(car(d)))
            i /= raw_int(car(d));
        else
            error(L"invalid argument type");
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
    else if (is_string(car(d)) && is_string(cadr(d)))
        return(_strcat(d));
    error(L"invalid argument");
    return(nil);
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
    error(L"invalid argument");
    return(nil);
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
    error(L"invalid argument");
    return(nil);
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
    error(L"invalid argument");
    return(nil);
}

data _mod_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(make_int(raw_int(car(d)) % raw_int(cadr(d))));
    error(L"invalid argument");
    return(nil);
}

data _logical_left_shift(data d)
{
    if (!is_int(car(d)) || !is_int(cadr(d)))
        error(L"logical left shift can not be applied to non-integer value");
    return(make_int(((unsigned int)raw_int(car(d))) << raw_int(cadr(d))));
}

data _logical_right_shift(data d)
{
    if (!is_int(car(d)) || !is_int(cadr(d)))
        error(L"logical left shift can not be applied to non-integer value");
    return(make_int(((unsigned int)raw_int(car(d))) >> raw_int(cadr(d))));
}

data _arithmetic_left_shift(data d)
{
    if (!is_int(car(d)) || !is_int(cadr(d)))
        error(L"logical left shift can not be applied to non-integer value");
    return(make_int(raw_int(car(d)) << raw_int(cadr(d))));
}

data _arithmetic_right_shift(data d)
{
    if (!is_int(car(d)) || !is_int(cadr(d)))
        error(L"logical left shift can not be applied to non-integer value");
    return(make_int(raw_int(car(d)) >> raw_int(cadr(d))));
}

data _positive(data d)
{
    if (is_int(d))
        return(make_int(+raw_int(d)));
    if (is_double(d))
        return(make_double(+raw_double(d)));
    error(L"positive operator failed");
    return(nil);
}

data _negative(data d)
{
    if (is_int(d))
        return(make_int(-raw_int(d)));
    if (is_double(d))
        return(make_double(-raw_double(d)));
    error(L"positive operator failed");
    return(nil);
}

/**************/
/* Comparator */
/**************/
data _less_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_int(car(d)) < raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_int(car(d)) < raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_double(car(d)) < raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_double(car(d)) < raw_double(cadr(d))));
    else if (is_string(car(d)) && is_string(cadr(d)))
        return(nil_or_t(wcscmp(raw_string(car(d)), raw_string(cadr(d))) < 0));
    error(L"invalid argument");
    return(nil);
}

data _less_equal_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_int(car(d)) <= raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_int(car(d)) <= raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_double(car(d)) <= raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_double(car(d)) <= raw_double(cadr(d))));
    else if (is_string(car(d)) && is_string(cadr(d)))
        return(nil_or_t(wcscmp(raw_string(car(d)), raw_string(cadr(d))) <= 0));
    error(L"invalid argument");
    return(nil);
}

data _greater_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_int(car(d)) > raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_int(car(d)) > raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_double(car(d)) > raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_double(car(d)) > raw_double(cadr(d))));
    else if (is_string(car(d)) && is_string(cadr(d)))
        return(nil_or_t(wcscmp(raw_string(car(d)), raw_string(cadr(d))) > 0));
    error(L"invalid argument");
    return(nil);
}

data _greater_equal_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_int(car(d)) >= raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_int(car(d)) >= raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_double(car(d)) >= raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_double(car(d)) >= raw_double(cadr(d))));
    else if (is_string(car(d)) && is_string(cadr(d)))
        return(nil_or_t(wcscmp(raw_string(car(d)), raw_string(cadr(d))) >= 0));
    error(L"invalid argument");
    return(nil);
}

data _equal_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_int(car(d)) == raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_int(car(d)) == raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_double(car(d)) == raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_double(car(d)) == raw_double(cadr(d))));
    else if (is_string(car(d)) && is_string(cadr(d)))
        return(nil_or_t(wcscmp(raw_string(car(d)), raw_string(cadr(d))) == 0));
    error(L"invalid argument");
    return(nil);
}

data _not_equal_2op(data d)
{
    if (is_int(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_int(car(d)) != raw_int(cadr(d))));
    else if (is_int(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_int(car(d)) != raw_double(cadr(d))));
    else if (is_double(car(d)) && is_int(cadr(d)))
        return(nil_or_t(raw_double(car(d)) != raw_int(cadr(d))));
    else if (is_double(car(d)) && is_double(cadr(d)))
        return(nil_or_t(raw_double(car(d)) != raw_double(cadr(d))));
    else if (is_string(car(d)) && is_string(cadr(d)))
        return(nil_or_t(wcscmp(raw_string(car(d)), raw_string(cadr(d))) != 0));
    error(L"invalid argument");
    return(nil);
}

data _logical_or_2op(data d)
{
    if (is_nil(eval(car(d))))
        return(nil_or_t(is_nil(eval(cadr(d)))));
    return(t);
}

data _logical_and_2op(data d)
{
    if (is_not_nil(eval(car(d))))
        return(nil_or_t(is_not_nil(eval(cadr(d)))));
    return(nil);
}

data _bit_or_2op(data d)
{
    if (!is_int(car(d)) || !is_int(cadr(d)))
        error(L"bit or failed");
    return(make_int(raw_int(car(d)) | raw_int(cadr(d))));
}

data _bit_xor_2op(data d)
{
    if (!is_int(car(d)) || !is_int(cadr(d)))
        error(L"bit xor failed");
    return(make_int(raw_int(car(d)) ^ raw_int(cadr(d))));
}

data _bit_and_2op(data d)
{
    if (!is_int(car(d)) || !is_int(cadr(d)))
        error(L"bit and failed");
    return(make_int(raw_int(car(d)) & raw_int(cadr(d))));
}

data _assignment(data d)
{
    if (!is_symbol(car(d)))
        error(L"An invalid value is specified as the assignment destination");
    override_symbol(raw_string(car(d)), eval(cadr(d)));
    return(car(d));
}

data _addition_assignment(data d)
{
    data value, old_value;
    if (!is_symbol(car(d)))
        error(L"An invalid value is specified as the assignment destination");
    value = eval(cadr(d));
    old_value = find_symbol(raw_string(car(d)));
    if (!old_value)
        error(L"assignee dose not exist");
    if (type_id(value) != type_id(old_value))
        error(L"assign type and assigned type are different");
    if (is_int(value) && is_int(old_value))
        override_symbol(raw_string(car(d)), make_int(raw_int(value) + raw_int(old_value)));
    else if (is_double(value) && is_double(old_value))
        override_symbol(raw_string(car(d)), make_double(raw_double(value) + raw_double(old_value)));
    return(car(d));
}

data _substraction_assignment(data d)
{
    data value, old_value;
    if (!is_symbol(car(d)))
        error(L"An invalid value is specified as the assignment destination");
    value = eval(cadr(d));
    old_value = find_symbol(raw_string(car(d)));
    if (!old_value)
        error(L"assignee dose not exist");
    if (type_id(value) != type_id(old_value))
        error(L"assign type and assigned type are different");
    if (is_int(value) && is_int(old_value))
        override_symbol(raw_string(car(d)), make_int(raw_int(value) - raw_int(old_value)));
    else if (is_double(value) && is_double(old_value))
        override_symbol(raw_string(car(d)), make_double(raw_double(value) - raw_double(old_value)));
    return(car(d));
}

data _multiplication_assignment(data d)
{
    data value, old_value;
    if (!is_symbol(car(d)))
        error(L"An invalid value is specified as the assignment destination");
    value = eval(cadr(d));
    old_value = find_symbol(raw_string(car(d)));
    if (!old_value)
        error(L"assignee dose not exist");
    if (type_id(value) != type_id(old_value))
        error(L"assign type and assigned type are different");
    if (is_int(value) && is_int(old_value))
        override_symbol(raw_string(car(d)), make_int(raw_int(value) * raw_int(old_value)));
    else if (is_double(value) && is_double(old_value))
        override_symbol(raw_string(car(d)), make_double(raw_double(value) * raw_double(old_value)));
    return(car(d));
}

data _division_assignment(data d)
{
    data value, old_value;
    if (!is_symbol(car(d)))
        error(L"An invalid value is specified as the assignment destination");
    value = eval(cadr(d));
    old_value = find_symbol(raw_string(car(d)));
    if (!old_value)
        error(L"assignee dose not exist");
    if (type_id(value) != type_id(old_value))
        error(L"assign type and assigned type are different");
    if (is_int(value) && is_int(old_value))
        override_symbol(raw_string(car(d)), make_int(raw_int(value) / raw_int(old_value)));
    else if (is_double(value) && is_double(old_value))
        override_symbol(raw_string(car(d)), make_double(raw_double(value) / raw_double(old_value)));
    return(car(d));
}

data _modulo_assignment(data d)
{
    data value, old_value;
    if (!is_symbol(car(d)))
        error(L"An invalid value is specified as the assignment destination");
    value = eval(cadr(d));
    old_value = find_symbol(raw_string(car(d)));
    if (!old_value)
        error(L"assignee dose not exist");
    if (type_id(value) != type_id(old_value))
        error(L"assign type and assigned type are different");
    if (is_int(value) && is_int(old_value))
        override_symbol(raw_string(car(d)), make_int(raw_int(value) % raw_int(old_value)));
    else if (is_double(value) && is_double(old_value))
        error(L"modulo assignment faield");
    return(car(d));
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
    print(car(d));
    return(t);
}

data _print_line(data d)
{
    print(car(d));
    wprintf(L"\n");
    return(t);
}

// (call <macro | function> arg1 arg2 arg3 ...)
data _call(data d)
{
    data function;
    if (is_symbol(car(d)))
        function = find_symbol(raw_string(car(d)));
    else
        function = car(d);
    if (is_builtin_macro(function))
        return(call_builtin_macro(make_pair(function, cdr(d))));
    else if (is_builtin_function(function))
        return(call_builtin_function(make_pair(function, cdr(d))));
    else if (is_unnamed_macro(function))
        return(call_unnamed_macro(make_pair(function, cdr(d))));
    else if (is_unnamed_function(function))
        return(call_unnamed_function(make_pair(function, cdr(d))));
    error(L"call failed");
    return(nil);
}

// (apply <macro | function> (arg1 arg2 arg3 ...))
data _apply(data d)
{
    return(_call(make_pair(car(d), cadr(d))));
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
        error(L"invalid macro name");
    push_symbol(raw_string(car(d)), _unnamed_macro(cdr(d)));
    return(car(d));
}

data _function(data d)
{
    if (!is_symbol(car(d)))
        error(L"invalid function name");
    push_symbol(raw_string(car(d)), _unnamed_function(cdr(d)));
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
        error(L"setcar failed");
    set_car(car(d), cadr(d));
    return(cadr(d));
}

data _set_rest(data d)
{
    if (!is_pair(car(d)))
        error(L"setcdr failed");
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

data _reverse(data d)
{

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
        error(L"pushsym failed");
    push_symbol(raw_string(car(d)), cadr(d));
    return(d);
}

data _pop_symbol(data d)
{
    if (!is_symbol(car(d)))
        error(L"popsym failed");
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
        error(L"charcode source is must be a string literal");
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
            error(L"fromcharcode buffer overrun");
        *(s++) = (wchar_t)raw_int(car(d));
        d = cdr(d);
    }
    *s = L'\0';
    return(make_string(buf));
}

/********************/
/* Association list */
/********************/
data _acons(data d)
{
    return(make_pair(make_pair(car(d), cadr(d)), caddr(d)));
}

data _assoc(data d)
{
    data key, list;
    key = car(d);
    list = cadr(d);
    while (is_not_nil(list))
    {
        if (is_not_nil(_equal_2op(make_pair(caar(list), make_pair(key, nil)))))
            return(car(list));
        list = cdr(list);
    }
    return(nil);
}

/***************/
/* Basic macro */
/***************/
data _progn(data d)
{
    data last;
    last = nil;
    while (is_not_nil(d))
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

// (while condition impl)
data _while(data d)
{
    data last;
    last = nil;
    while (is_not_nil(eval(car(d))))
    {
        last = eval(cadr(d));
    }
    return last;
}

data _mapcar(data d)
{
    if (is_nil(d))
        return(nil);
    return(make_pair(raw_function(car(d))(d), _mapcar(make_pair(car(d), cdr(d)))));
}

/**********/
/* String */
/**********/
data _strcat(data d)
{
    size_t a_length, b_length;
    wchar_t * new_string;
    if (!is_string(car(d)) || !is_string(cadr(d)))
        goto error;
    a_length = wcslen(raw_string(car(d)));
    b_length = wcslen(raw_string(cadr(d)));
    new_string = malloc(sizeof(wchar_t) * (a_length + b_length + 1));
    if (!new_string)
        goto error;
    memcpy(new_string, raw_string(car(d)), sizeof(wchar_t) * a_length);
    memcpy(new_string + a_length, raw_string(cadr(d)), sizeof(wchar_t) * b_length);
    new_string[a_length + b_length] = L'\0';
    return(make_string(new_string));

error:
    error(L"strcat failed");
    return(nil);
}

data _substr(data d)
{
    size_t length;
    wchar_t * new_string;
    if (!is_string(car(d)))
        goto error;
    length = wcslen(raw_string(car(d)));
    if (!is_int(cadr(d)) || (!is_int(caddr(d)) && is_not_nil(cddr(d))))
        goto error;
    if (is_nil(cddr(d)))
        return(make_string(clone_string(raw_string(car(d)) + raw_int(cadr(d)))));
    if ((size_t)raw_int(cadr(d)) + (size_t)raw_int(caddr(d)) > wcslen(raw_string(car(d))))
        goto error;
    length = wcslen(raw_string(car(d)));
    new_string = malloc(sizeof(wchar_t) * raw_int(caddr(d)));
    if (!new_string)
        goto error;
    memcpy(new_string, raw_string(car(d)) + raw_int(cadr(d)), sizeof(wchar_t) * raw_int(caddr(d)));
    new_string[raw_int(caddr(d))] = L'\0';
    return(make_string(new_string));

error:
    error(L"substr failed");
    return(nil);
}
