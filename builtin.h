#ifndef BUILTIN_H
#define BUILTIN_H

#include "data.h"

void init_builtin();

/**************************/
/* Builtin function value */
/**************************/
extern data print_v;
extern data print_line_v;
extern data dump_symbol_v;
extern data dump_heap_v;
extern data gc_v;
extern data push_args_v;
extern data pop_args_v;
extern data pair_v;
extern data set_first_v;
extern data set_rest_v;
extern data list_v;
extern data length_v;
extern data if_v;
extern data and_v;
extern data or_v;
extern data not_v;
extern data eval_v;
extern data call_v;
extern data apply_v;
extern data bind_symbol_v;
extern data unnamed_macro_v;
extern data unnamed_function_v;
extern data macro_v;
extern data function_v;
extern data left_associative_operator_v;
extern data right_associative_operator_v;
extern data is_cons_v;
extern data is_builtin_macro_v;
extern data is_builtin_function_v;
extern data is_unnamed_macro_v;
extern data is_unnamed_function_v;
extern data is_symbol_v;
extern data is_nil_v;
extern data is_int_v;
extern data is_double_v;
extern data is_number_v;
extern data is_string_v;
extern data is_zero_v;
extern data inc_v;
extern data dec_v;
extern data add_v;
extern data sub_v;
extern data mul_v;
extern data div_v;
extern data mod_v;
extern data zip_first_v;
extern data zip_rest_v;
extern data zip_v;
extern data push_symbol_v;
extern data pop_symbol_v;
extern data to_char_code_v;
extern data from_char_code_v;
extern data progn_v;
extern data let_v;
extern data mapcar_v;
extern data strcat_v;
extern data substr_v;

/*********************************/
/* Builtin binary operator value */
/*********************************/
extern data _assign_v;
extern data _unnamed_function_v;
extern data _less_2op_v;
extern data _less_equal_2op_v;
extern data _greater_2op_v;
extern data _greater_equal_2op_v;
extern data _equal_2op_v;
extern data _not_equal_2op_v;
extern data _sub_2op_v;
extern data _add_2op_v;
extern data _mod_2op_v;
extern data _div_2op_v;
extern data _mul_2op_v;
extern data _arithmetic_left_shift_v;
extern data _arithmetic_right_shift_v;
extern data _logical_left_shift_v;
extern data _logical_right_shift_v;

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
data _print_line(data);
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
