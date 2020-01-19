#ifndef BUILTIN_H
#define BUILTIN_H

#include "data.h"

void init_builtin();

data print_v;

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

data progn_v;
data let_v;
data mapcar_v;

data strcat_v;
data substr_v;

/**************/
/* Arithmetic */
/**************/
data _inc(data);
data _dec(data);

/*********************************/
/* Variacdic arithmetic function */
/*********************************/
data _add(data);
data _sub(data);
data _mul(data);
data _div(data);
data _mod(data);

/*******************/
/* Binary operator */
/*******************/
data _add_2op(data);
data _sub_2op(data);
data _mul_2op(data);
data _div_2op(data);
data _mod_2op(data);
data _logical_left_shift(data);
data _logical_right_shift(data);
data _arithmetic_left_shift(data);
data _arithmetic_right_shift(data);

/**************/
/* Comparator */
/**************/
data _less_2op(data);
data _less_equal_2op(data);
data _greater_2op(data);
data _greater_equal_2op(data);
data _equal_2op(data);
data _not_equal_2op(data);

data _assign(data);

/***************/
/* Conditional */
/***************/
data _if(data);
data _and(data);
data _or(data);
data _not(data);

/**************/
/* Functional */
/**************/
data _eval(data);
data _print(data);
data _call(data);
data _apply(data);
data _bind_symbol(data);
data _unnamed_macro(data);
data _unnamed_function(data);
data _macro(data);
data _function(data);

/************************/
/* Operator declaration */
/************************/
data _left_associative_operator(data);
data _right_associative_operator(data);

/**********************/
/* Cons-cell function */
/**********************/
data _pair(data);
data _set_first(data);
data _set_rest(data);

/*****************/
/* List function */
/*****************/
data _list(data);
data _length(data);

/******************/
/* Zip algorithm */
/******************/
data _zip_first(data);
data _zip_rest(data);
data _zip(data);

/*************************/
/* Symbol stack function */
/*************************/
data _push_symbol(data);
data _pop_symbol(data);

/****************************/
/* String integer interface */
/****************************/
data _to_char_code(data);
data _from_char_code(data);

data _progn(data);
data _let(data);
data _mapcar(data);
data _strcat(data);
data _substr(data);

#endif
