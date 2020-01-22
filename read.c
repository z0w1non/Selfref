#include "read.h"

#include <stdlib.h>
#include <stdio.h>
#include "char.h"
#include "print.h"
#include "eval.h"
#include "sym.h"
#include "builtin.h"
#include "repl.h"

enum
{
    tokenmax = 256,
};

typedef struct token_tag
{
    wchar_t string[tokenmax];
    size_t length;
    data data;
} token;

enum
{
    context_buffer_length = 256,
};

typedef struct context_tag
{
    token token;
    wint_t buffer[context_buffer_length];
    size_t last_read_buffer_offset;
    size_t read_back_count;
    size_t total_count;
} context_t;

typedef struct saved_context_tag
{
    size_t count;
} saved_context_t;

FILE * input_stream;

/********************************/
/* Private function declaration */
/********************************/
void            context_init(context_t * context);
wint_t          context_read_char(context_t * context);
void            context_read_back(context_t * context, int count);
void            context_push_char(context_t * context, wchar_t wc);
void            context_pop_char(context_t * context);
void            context_clear_token(context_t * context);
const wchar_t * context_get_token_string(context_t * context);
data            context_get_token_data(context_t * context);
void            context_set_token_data(context_t * context, data);
void            context_remove_token_string_suffix(context_t * context, int length);
void            context_save(context_t * context, saved_context_t * temp);
void            context_restore(context_t * context, const saved_context_t * temp);
void            context_debug_print(context_t * context);

data read_internal(context_t *);

int at_eof(context_t * context);
void skip_space(context_t * context);

typedef int (*parse_function_t)(context_t *);
int parse_empty(context_t * context);

int parse_single_char(context_t * c, wchar_t context);

int parse_lparen(context_t * context);
int parse_rparen(context_t * context);
int parse_curly_lparen(context_t * context);
int parse_curly_rparen(context_t * context);
int parse_comma(context_t * context);
int parse_semicolon(context_t * context);
int parse_block(context_t * context);
int parse_block_internal(context_t * context);
int parse_list(context_t * context);
int parse_list_internal(context_t * context);
int parse_abstract_list(context_t * context, wchar_t lparen, parse_function_t parse_function, wchar_t separator, wchar_t rparen);
int parse_abstract_list_internal(context_t * context, parse_function_t parse_function, wchar_t separator, wchar_t rparen);

/**************/
/* Expression */
/**************/
int parse_assignment_expression(context_t * context);
int parse_logical_or_expression(context_t * context);
int parse_logical_and_expression(context_t * context);
int parse_bit_or_expression(context_t * context);
int parse_bit_xor_expression(context_t * context);
int parse_bit_and_expression(context_t * context);
int parse_relational_expression(context_t * context);
int parse_shift_expression(context_t * context);
int parse_polynomial_expression(context_t * context);
int parse_monomial_expression(context_t * context);
int parse_signed_number(context_t * context);
int parse_unsigned_number(context_t * context);
int parse_unsigned_number_literal(context_t * context);
int parse_string_expression(context_t * context);
int parse_string_literal(context_t * context);

int parse_paragraph(context_t * context);
int parse_element(context_t * context);
int parse_argument(context_t * context);
int parse_argument_internal(context_t * context);
int parse_statement(context_t * context);
int parse_expression(context_t * context);
int parse_symbol(context_t * context);
int parse_reserved_word(context_t * context, const wchar_t * reserved_word);

int parse_abstract_operator(context_t * context, const wchar_t * operator_string, data builtin_operator);
int parse_equal_operator(context_t * context);
int parse_not_equal_operator(context_t * context);
int parse_less_operator(context_t * context);
int parse_less_equal_operator(context_t * context);
int parse_greater_operator(context_t * context);
int parse_greater_equal_operator(context_t * context);
int parse_arithmetic_left_shift(context_t * context);
int parse_arithmetic_right_shift(context_t * context);
int parse_logical_left_shift(context_t * context);
int parse_logical_right_shift(context_t * context);
int parse_logical_or_operator(context_t * context);
int parse_logical_and_operator(context_t * context);
int parse_bit_or_operator(context_t * context);
int parse_bit_xor_operator(context_t * context);
int parse_bit_or_operator(context_t * context);
int parse_addition_operator(context_t * context);
int parse_substraction_operator(context_t * context);
int parse_multiplication_operator(context_t * context);
int parse_division_operator(context_t * context);
int parse_modulo_operator(context_t * context);
int parse_positive_operator(context_t * context);
int parse_negative_operator(context_t * context);

/*******************/
/* Public function */
/*******************/
void set_input_stream(FILE * s)
{
    input_stream = s;
}

FILE * get_input_stream()
{
    return(input_stream);
}

data read()
{
    context_t context;
    data result;
    context_init(&context);
    result = read_internal(&context);
    if (interactive_mode()) /* discard new line character */
        context_read_char(&context);
    return(result);
}

/******************/
/* Read character */
/******************/
void context_init(context_t * context)
{
    memset(context, 0, sizeof(context_t));
    context->last_read_buffer_offset = -1;
    context->total_count = 0;
}

wint_t context_read_char(context_t * context)
{
    if (context->read_back_count > 0)
    {
        context->read_back_count -= 1;
        return(context->buffer[(context->last_read_buffer_offset + context_buffer_length - context->read_back_count) % context_buffer_length]);
    }
    context->last_read_buffer_offset += 1;
    context->last_read_buffer_offset %= context_buffer_length;
    context->total_count += 1;
    return(context->buffer[context->last_read_buffer_offset] = fgetc(input_stream));
}

void context_read_back(context_t * context, int count)
{
    context->read_back_count += count;
}

void context_push_char(context_t * context, wchar_t wc)
{
    context->token.string[context->token.length++] = wc;
    context->token.string[context->token.length] = L'\0';
}

void context_pop_char(context_t * context)
{
    context->token.string[--(context->token.length)] = L'\0';
}

void context_clear_token(context_t * context)
{
    context->token.data = NULL;
    context->token.length = 0;
    context->token.string[0] = L'\0';
}

const wchar_t * context_get_token_string(context_t * context)
{
    return(context->token.string);
}

data context_get_token_data(context_t * context)
{
    return(context->token.data);
}

void context_set_token_data(context_t * context, data d)
{
    context->token.data = d;
}

void context_remove_token_string_suffix(context_t * context, int length)
{
    context->token.string[context->token.length - length] = L'\0';
    context->token.length -= length;
}

void context_save(context_t * context, saved_context_t * saved_context)
{
    saved_context->count = context->total_count - context->read_back_count;
}

void context_restore(context_t * context, const saved_context_t * saved_context)
{
    if (context->total_count - context->read_back_count > saved_context->count)
        context_read_back(context, ((int)(context->total_count) - context->read_back_count - saved_context->count));
}

int contesxt_read_back_to_last_crlf(context_t * context)
{
    size_t i, j;
    for (i = 0; i < context_buffer_length && i < context->total_count; ++i)
    {
        j = (context_buffer_length * 2 + context->last_read_buffer_offset - context->read_back_count - i) % context_buffer_length;
        if (is_crlf(context->buffer[j]))
        {
            context_read_back(context, i);
            return(i);
        }
    }
}

void context_debug_print(context_t * context)
{
    size_t current_position, i;
    wint_t last_char;
    saved_context_t saved_context;
    current_position = contesxt_read_back_to_last_crlf(context);
    while ((!is_eof(last_char = context_read_char(context))) && (!is_crlf(last_char)))
        wprintf(L"%c", last_char);
    wprintf(L"\n");
    for (i = 0; i < current_position - 1; ++i)
        wprintf(L" ");
    wprintf(L"^\n");
    context_restore(context, &saved_context);
}

/**************/
/* Read input */
/**************/
data read_internal(context_t * context)
{
    skip_space(context);
    if (parse_paragraph(context))
        return(context_get_token_data(context));
    error(L"read failed");
    return(nil);
}

int at_eof(context_t * context)
{
    int eof;
    wint_t last_char;
    last_char = context_read_char(context);
    eof = is_eof(last_char);
    context_read_back(context, 1);
    return(eof);
}

void skip_space(context_t * context)
{
    wint_t last_char;
    while ((!is_eof(last_char = context_read_char(context))) && ((is_space(last_char) || (!interactive_mode() && is_crlf(last_char)))));
    context_read_back(context, 1);
}

int parse_empty(context_t * context)
{
    return(1);
}

int parse_single_char(context_t * context, wchar_t wc)
{
    wint_t last_char;
    last_char = context_read_char(context);
    if (last_char == wc)
    {
        context_push_char(context, last_char);
        context_set_token_data(context, NULL);
        return(1);
    }
    context_read_back(context, 1);
    return(0);
}

int parse_lparen(context_t * context)
{
    return(parse_single_char(context, L'('));
}

int parse_rparen(context_t * context)
{
    return(parse_single_char(context, L')'));
}

int parse_curly_lparen(context_t * context)
{
    return(parse_single_char(context, L'{'));
}

int parse_curly_rparen(context_t * context)
{
    return(parse_single_char(context, L'}'));
}

int parse_comma(context_t * context)
{
    return(parse_single_char(context, L','));
}

int parse_semicolon(context_t * context)
{
    return(parse_single_char(context, L';'));
}

/* <specified lparen> <abstract list internal> */
int parse_abstract_list(context_t * context, wchar_t lparen, parse_function_t parse_function, wchar_t separator, wchar_t rparen)
{
    if (parse_single_char(context, lparen))
        if (parse_abstract_list_internal(context, parse_function, separator, rparen))
            return(1);
    return(0);
}

/* <specified rparen> */
/* <token> <specified rparen> */
/* <token> <specified separator> <abstract list internal> */
int parse_abstract_list_internal(context_t * context, parse_function_t parse_function, wchar_t separator, wchar_t rparen)
{
    data first, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_single_char(context, rparen))
    {
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, nil);
        return(1);
    }
    else if (parse_function(context))
    {
        first = context_get_token_data(context);
        skip_space(context);
        context_clear_token(context);
        if (parse_single_char(context, rparen))
        {
            skip_space(context);
            context_clear_token(context);
            context_set_token_data(context, first);
            return(1);
        }
        else if (parse_single_char(context, separator))
        {
            skip_space(context);
            context_clear_token(context);
            if (parse_abstract_list_internal(context, parse_function, separator, rparen))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(first, rest));
                return(1);
            }
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <list> */
/* <block> */
/* <statement> */
/* <function declaration> */
int parse_paragraph(context_t * context)
{
    data internal;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_list(context) || parse_block(context) || parse_statement(context) || parse_function_declaration(context))
        return(1);

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* "{" <block internal> "}" */
int parse_block(context_t * context)
{
    data internal;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_curly_lparen(context))
    {
        context_clear_token(context);
        if (parse_block_internal(context))
        {
            internal = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_curly_rparen(context))
            {
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, internal);
                return(1);
            }
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <statement> <block internal> */
/* <statement> */
int parse_block_internal(context_t * context)
{
    data first_statement, rest_block_internal;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_curly_rparen(context))
    {
        context_restore(context, &saved_context);
        context_clear_token(context);
        context_set_token_data(context, nil);
        return(1);
    }
    else if (parse_statement(context))
    {
        first_statement = context_get_token_data(context);
        context_clear_token(context);
        if (parse_block_internal(context))
        {
            rest_block_internal = context_get_token_data(context);
            skip_space(context);
            context_clear_token(context);
            context_set_token_data(context, make_pair(progn_v, make_pair(first_statement, make_pair(rest_block_internal, nil))));
            return(1);
        }
        return(1);
    }

    context_restore(context, &saved_context);
    return(0);
}

/* <lparen> <list internal> */
int parse_list(context_t * context)
{
    data internal;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_lparen(context))
    {
        context_clear_token(context);
        if (parse_list_internal(context))
        {
            internal = context_get_token_data(context);
            skip_space(context);
            context_clear_token(context);
            context_set_token_data(context, internal);
            return(1);
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <rparen> */
/* <element> <list internal> */
int parse_list_internal(context_t * context)
{
    data first_token, rest_token;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_rparen(context))
    {
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, nil);
        return(1);
    }
    else if (parse_element(context))
    {
        first_token = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_list_internal(context))
        {
            rest_token = context_get_token_data(context);
            skip_space(context);
            context_clear_token(context);
            context_set_token_data(context, make_pair(first_token, rest_token));
            return(1);
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <symbol> */
/* <signed number> */
/* <string> */
/* <paragraph> */
int parse_element(context_t * context)
{
    if (parse_symbol(context) || parse_signed_number(context) || parse_string_literal(context) || parse_paragraph(context))
        return(1);
    return(0);
}

/* "(" <argument internal> */
int parse_argument(context_t * context)
{
    data argument;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_lparen(context))
        if (parse_argument_internal(context))
            return(1);

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <rparen> */
/* <assignment expression> <rparen> */
/* <assignment expression> <comma> <argument internal> */
int parse_argument_internal(context_t * context)
{
    data first, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_rparen(context))
    {
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, nil);
        return(1);
    }
    else if (parse_assignment_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_rparen(context))
        {
            skip_space(context);
            context_clear_token(context);
            context_set_token_data(context, make_pair(first, nil));
            return(1);
        }
        else if (parse_comma(context))
        {
            skip_space(context);
            context_clear_token(context);
            if (parse_argument_internal(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(first, rest));
                return(1);
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, make_pair(first, nil));
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <symbol> <argument> */
int parse_c_function_call(context_t * context)
{
    data function_symbol, argument;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_symbol(context))
    {
        function_symbol = context_get_token_data(context);
        context_clear_token(context);
        if (parse_argument(context))
        {
            argument = context_get_token_data(context);
            skip_space(context);
            context_clear_token(context);
            context_set_token_data(context, make_pair(function_symbol, argument));
            return(1);
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/**************/
/* Expression */
/**************/

/* <symbol> <assignment operator> <assignment expression> */
/* <logical and expression> */
int parse_assignment_expression(context_t * context)
{
    data symbol, assign, expression;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_symbol(context))
    {
        symbol = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_assignment_operator(context))
        {
            assign = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_assignment_expression(context))
            {
                expression = context_get_token_data(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(assign, make_pair(symbol, make_pair(expression, nil))));
                return(1);
            }
        }
        context_restore(context, &saved_context);
    }

    if (parse_logical_or_expression(context))
        return(1);

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <logical and expression> <logical or operator> <logical or expression> */
/* <logical and expression> */
int parse_logical_or_expression(context_t * context)
{
    data first, operator, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_logical_and_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_logical_or_operator(context))
        {
            operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_logical_or_expression(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(operator, make_pair(first, make_pair(rest, nil))));
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <bit or expression> <logical and operator> <logical and expression> */
/* <bit or expression> */
int parse_logical_and_expression(context_t * context)
{
    data first, operator, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_bit_or_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_logical_and_operator(context))
        {
            operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_logical_and_expression(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(operator, make_pair(first, make_pair(rest, nil))));
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <bit xor expression> <bit or operator> <bit or expression> */
/* <bit xor expression> */
int parse_bit_or_expression(context_t * context)
{
    data first, operator, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_bit_xor_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_bit_or_operator(context))
        {
            operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_bit_or_expression(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(operator, make_pair(first, make_pair(rest, nil))));
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <bit and expression> <bit xor operator> <bit xor expression> */
/* <bit and expression> */
int parse_bit_xor_expression(context_t * context)
{
    data first, operator, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_bit_and_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_bit_xor_operator(context))
        {
            operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_bit_xor_expression(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(operator, make_pair(first, make_pair(rest, nil))));
                return(1);
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <polynomial> <bit and operator> <bit and expression> */
/* <polynomial> */
int parse_bit_and_expression(context_t * context)
{
    data first, operator, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_shift_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_bit_and_operator(context))
        {
            operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_bit_and_expression(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(operator, make_pair(first, make_pair(rest, nil))));
                return(1);
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <shift expression> <equal operator> <relational expression> */
/* <shift expression> <not equal operator> <relational expression> */
/* <shift expression> <less operator> <relational expression> */
/* <shift expression> <less equal operator> <relational expression> */
/* <shift expression> <greater operator> <relational expression> */
/* <shift expression> <greater equal oparator> <relational expression> */
/* <shift expression> */
int parse_relational_expression(context_t * context)
{
    data first, relational_operator, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_shift_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_equal_operator(context)
            || parse_not_equal_operator(context)
            || parse_less_operator(context)
            || parse_less_equal_operator(context)
            || parse_greater_operator(context)
            || parse_greater_equal_operator(context))
        {
            relational_operator = context_get_token_data(context);
            skip_space(context);
            context_clear_token(context);
            if (parse_relational_expression(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(relational_operator, make_pair(first, make_pair(rest, nil))));
                return(1);
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <polymial expression> <arithmetic right shift> <shift expression> */
/* <polymial expression> <arithmetic left shift> <shift expression> */
/* <polymial expression> <logical right shift> <shift expression> */
/* <polymial expression> <logical left shift> <shift expression> */
/* <polymial expression> */
int parse_shift_expression(context_t * context)
{
    data first, shift_operator, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_polynomial_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_arithmetic_left_shift(context)
            || parse_arithmetic_right_shift(context)
            || parse_logical_left_shift(context)
            || parse_logical_right_shift(context))
        {
            shift_operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_shift_expression(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(shift_operator, make_pair(first, make_pair(rest, nil))));
                return(1);
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}


/* <monomial expression> <addition operator> <polynomial expression> */
/* <monomial expression> <substraction operator> <polynomial expression> */
/* <monomial expression> */
int parse_polynomial_expression(context_t * context)
{
    data first, operator, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_monomial_expression(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_addition_operator(context) || parse_substraction_operator(context))
        {
            operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_polynomial_expression(context))
            {
                rest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(operator, make_pair(first, make_pair(rest, nil))));
                return(1);
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <signed number> <multiplication operator> <monomial> */
/* <signed number> <division operator> <monomial> */
/* <signed number> <modulo operator> <monomial> */
/* <signed number> */
/* <string expression> */
int parse_monomial_expression(context_t * context)
{
    data first, operator, lest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_signed_number(context) || parse_symbol(context))
    {
        first = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_multiplication_operator(context)
            || parse_division_operator(context)
            || parse_modulo_operator(context))
        {
            operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_monomial_expression(context))
            {
                lest = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(operator, make_pair(first, make_pair(lest, nil))));
                return(1);
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, first);
        return(1);
    }
    if (parse_string_expression(context))
        return(1);

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <positive operator> <signed number> */
/* <negative operator> <signed number> */
/* <unsigned number> */
int parse_signed_number(context_t * context)
{
    data operator, signed_number;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_positive_operator(context) || parse_negative_operator(context))
    {
        operator = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_signed_number(context))
        {
            signed_number = context_get_token_data(context);
            skip_space(context);
            context_clear_token(context);
            context_get_token_data(make_pair(operator, make_pair(signed_number, nil)));
            return(1);
        }
    }
    if (parse_unsigned_number(context) || parse_nested_expression(context))
        return(1);

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <unsigned number literal> */
/* <c function call> */
/* <number symbol> */
/* <nested expression> */
int parse_unsigned_number(context_t * context)
{
    if (parse_unsigned_number_literal(context) || parse_c_function_call(context) || parse_symbol(context) || parse_nested_expression(context))
        return(1);
    return(0);
}

/* [0-9]+ */
/* [0-9]+.[0-9]+ */
int parse_unsigned_number_literal(context_t * context)
{
    int i;
    double d, div;
    wint_t last_char;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    last_char = context_read_char(context);

    if (is_digit(last_char))
    {
        i = ((int)(last_char - L'0'));
        last_char = context_read_char(context);
        while ((!is_eof(last_char)) && (is_digit(last_char)))
        {
            i = i * 10 + ((int)(last_char - L'0'));
            context_push_char(context, (wchar_t)last_char);
            last_char = context_read_char(context);
        }
        if (is_dot(last_char))
        {
            context_push_char(context, L'.');
            d = 0.0;
            div = 1.0;
            last_char = context_read_char(context);
            while ((!is_eof(last_char)) && (is_digit(last_char)))
            {
                d = d * 10.0 + (double)(last_char - L'0');
                div *= 10;
                context_push_char(context, (wchar_t)last_char);
                last_char = context_read_char(context);
            }
            context_read_back(context, 1);
            context_set_token_data(context, make_double(((double)i + d / div)));
            return(1);
        }
        else
        {
            context_read_back(context, 1);
            context_set_token_data(context, make_int(i));
            return(1);
        }
    }
    context_clear_token(context);

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <string literal> <addition operator> <string expression> */
/* <string literal> */
int parse_string_expression(context_t * context)
{
    data left, right, concatenate_operator;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_string_literal(context))
    {
        left = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_addition_operator(context))
        {
            concatenate_operator = context_get_token_data(context);
            skip_space(context);
            if (parse_string_expression(context))
            {
                right = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(concatenate_operator, make_pair(left, make_pair(right, nil))));
                return(1);
            }
        }
        skip_space(context);
        context_clear_token(context);
        context_set_token_data(context, left);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

int parse_string_literal(context_t * context)
{
    wint_t last_char;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    last_char = context_read_char(context);
    if (last_char == L'\"')
    {
        while (!is_eof((last_char = context_read_char(context))) && (last_char != L'\"'))
        {
            if (last_char == L'\\')
            {
                last_char = context_read_char(context);
                switch (last_char)
                {
                case L'n':  context_push_char(context, L'\n'); break;
                case L't':  context_push_char(context, L'\t'); break;
                case L'\\': context_push_char(context, L'\\'); break;
                case L'\"': context_push_char(context, L'\"'); break;
                default: error(L"Unknown escape sequence (\\n -> newline, \\t -> tab, \\\\ -> \\, \\\" -> double quotation(\"))\n");
                }
            }
            else if (!is_print(last_char))
                goto restore;
            else
                context_push_char(context, (wchar_t)last_char);
        }
        skip_space(context);
        context_set_token_data(context, make_string(context_get_token_string(context)));
        return(1);
    }
    context_clear_token(context);

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* Private */
int is_builtin_type(const wchar_t * name)
{
    static const wchar_t * builtin_types[] = {
        L"int",
        L"double",
        L"string",
    };
}

// test
int parse_type(context_t * context)
{
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_symbol(context))
    {
        if (is_builtin_type(context_get_token_string(context)))
        {
            return(1);
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* ")" */
/* <symbol> <dummy argument internal> */
int parse_dummy_argument_internal(context_t * context)
{
    data first, rest;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_rparen(context))
    {
        skip_space(context);
        context_set_token_data(context, nil);
        return(1);
    }
    else if (parse_symbol(context))
    {
        first = context_get_token_data(context);
        skip_space(context);
        context_clear_token(context);
        if (parse_dummy_argument_internal(context))
        {
            rest = context_get_token_data(context);
            skip_space(context);
            context_clear_token(context);
            context_set_token_data(context, make_pair(first, rest));
            return(1);
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* "(" <dummy argument internal> */
int parse_dummy_argument(context_t * context)
{
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_lparen(context))
    {
        context_clear_token(context);
        skip_space(context);
        if (parse_dummy_argument_internal(context))
            return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* "function" <symbol> <dummy argument> "{" <statement> "}" */
int parse_function_declaration(context_t * context)
{
    data symbol, dummy_argument, statement;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_reserved_word(context, L"function"))
    {
        context_clear_token(context);
        if (parse_symbol(context))
        {
            symbol = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_dummy_argument(context))
            {
                dummy_argument = context_get_token_data(context);
                context_clear_token(context);
                skip_space(context);
                if (parse_curly_lparen(context))
                {
                    context_clear_token(context);
                    skip_space(context);
                    if (parse_statement(context))
                    {
                        statement = context_get_token_data(context);
                        context_clear_token(context);
                        skip_space(context);
                        if (parse_curly_rparen(context))
                        {
                            skip_space(context);
                            context_clear_token(context);
                            context_set_token_data(context, make_pair(function_v, make_pair(symbol, make_pair(dummy_argument, make_pair(statement, nil)))));
                            return(1);
                        }
                    }
                }
            }
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <assignment expression> <semicolon> */
/* <assignment expression> <comma> <statement> */
int parse_statement(context_t * context)
{
    data first_token, rest_statement;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_assignment_expression(context))
    {
        first_token = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_semicolon(context))
        {
            context_clear_token(context);
            context_set_token_data(context, first_token);
            return(1);
        }
        else if (parse_comma(context))
        {
            context_clear_token(context);
            skip_space(context);
            if (parse_statement(context))
            {
                rest_statement = context_get_token_data(context);
                skip_space(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(progn_v, make_pair(first_token, make_pair(rest_statement, nil))));
                return(1);
            }
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* "(" <polynomial expression> ")" */
int parse_nested_expression(context_t * context)
{
    data expression;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_lparen(context))
    {
        context_clear_token(context);
        skip_space(context);
        if (parse_polynomial_expression(context))
        {
            expression = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_rparen(context))
            {
                context_clear_token(context);
                context_set_token_data(context, expression);
                return(1);
            }
        }
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <polynomial> */
int parse_expression(context_t * context)
{
    if (parse_polynomial_expression(context))
        return(1);
    return(0);
}

int parse_symbol(context_t * context)
{
    wint_t last_char;
    data prefix_operator;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    while ((!is_eof(last_char = context_read_char(context))) && (is_symbol_char(last_char)))
        context_push_char(context, (wchar_t)last_char);
    context_read_back(context, 1);

    if (context->token.length > 0)
    {
        skip_space(context);
        context_set_token_data(context, make_symbol(context_get_token_string(context)));
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

int parse_binary_operator(context_t * context)
{
    wint_t last_char;
    data longest_matched_operator;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    longest_matched_operator = NULL;
    while (1)
    {
        if (is_eof(last_char = context_read_char(context)) || !is_operator_char(last_char))
        {
            if (!longest_matched_operator)
                goto restore;
            context_set_token_data(context, longest_matched_operator);
            context_read_back(context, 1);
            return(1);
        }

        context_push_char(context, last_char);
        longest_matched_operator = find_binary_operator(context_get_token_string(context));
        if (!longest_matched_operator)
            goto restore;
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

int parse_prefix_operator(context_t * context)
{
    wint_t last_char;
    data longest_matched_operator;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    longest_matched_operator = NULL;
    while (1)
    {
        if (is_eof(last_char = context_read_char(context)) || !is_operator_char(last_char))
        {
            if (!longest_matched_operator)
                goto restore;
            context_set_token_data(context, longest_matched_operator);
            context_read_back(context, 1);
            skip_space(context);
            return(1);
        }

        context_push_char(context, last_char);
        longest_matched_operator = find_prefix_operator(context_get_token_string(context));
        if (!longest_matched_operator)
            goto restore;
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

int parse_reserved_word(context_t * context, const wchar_t * reserved_word)
{
    wint_t last_char;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    while ((!is_eof(last_char = context_read_char(context))) && ((is_symbol_char(last_char) || is_operator_char(last_char))))
        context_push_char(context, (wchar_t)last_char);
    context_read_back(context, 1);

    if (context->token.length > 0)
    {
        if (wcscmp(context_get_token_string(context), reserved_word) == 0)
        {
            skip_space(context);
            context_set_token_data(context, make_symbol(context_get_token_string(context)));
            return(1);
        }
    }
    context_clear_token(context);

restore:
    context_restore(context, &saved_context);
    return(0);
}

int parse_abstract_operator(context_t * context, const wchar_t * operator_string, data builtin_operator)
{
    wint_t last_char;
    size_t specified_operator_string_length, read_string_length;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    specified_operator_string_length = wcslen(operator_string);
    read_string_length = 0;
    while ((!is_eof(last_char = context_read_char(context))) && is_operator_char(last_char))
    {
        context_push_char(context, (wchar_t)last_char);
        read_string_length += 1;
        if (read_string_length > specified_operator_string_length)
            break;
        if (wcscmp(context_get_token_string(context), operator_string) == 0)
        {
            skip_space(context);
            context_set_token_data(context, builtin_operator);
            return(1);
        }
    }
    context_clear_token(context);

restore:
    context_restore(context, &saved_context);
    return(0);
}

int parse_assignment_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"=", _assign_v));
}

int parse_equal_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"==", _equal_2op_v));
}

int parse_not_equal_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"!=", _not_equal_2op_v));
}

int parse_less_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"<", _less_2op_v));
}

int parse_less_equal_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"<=", _less_equal_2op_v));
}

int parse_greater_operator(context_t * context)
{
    return(parse_abstract_operator(context, L">", _greater_2op_v));
}

int parse_greater_equal_operator(context_t * context)
{
    return(parse_abstract_operator(context, L">=", _greater_equal_2op_v));
}

int parse_arithmetic_left_shift(context_t * context)
{
    return(parse_abstract_operator(context, L"<<<", _arithmetic_left_shift_v));
}

int parse_arithmetic_right_shift(context_t * context)
{
    return(parse_abstract_operator(context, L">>>", _arithmetic_right_shift_v));
}

int parse_logical_left_shift(context_t * context)
{
    return(parse_abstract_operator(context, L"<<<", _logical_left_shift_v));
}

int parse_logical_right_shift(context_t * context)
{
    return(parse_abstract_operator(context, L">>>", _logical_right_shift_v));
}

int parse_logical_or_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"||", _logical_or_2op_v));
}

int parse_logical_and_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"&%", _logical_and_2op_v));
}

int parse_bit_or_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"|", _bit_or_2op_v));
}

int parse_bit_xor_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"^", _bit_xor_2op_v));
}

int parse_bit_and_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"&", _bit_and_2op_v));
}

int parse_addition_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"+", _add_2op_v));
}

int parse_substraction_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"-", _sub_2op_v));
}

int parse_multiplication_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"*", _mul_2op_v));
}

int parse_division_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"/", _div_2op_v));
}

int parse_modulo_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"%", _mod_2op_v));
}

int parse_positive_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"+", _positive_v));
}

int parse_negative_operator(context_t * context)
{
    return(parse_abstract_operator(context, L"-", _negative_v));
}
