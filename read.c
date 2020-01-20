#include "read.h"

#include <stdlib.h>
#include <stdio.h>
#include "char.h"
#include "print.h"
#include "eval.h"
#include "sym.h"
#include "builtin.h"

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
int parse_single_char(context_t * c, wchar_t context);
int parse_single_char_binary_operator(context_t * c, wchar_t context);
int parse_lparen(context_t * context);
int parse_rparen(context_t * context);
int parse_curly_lparen(context_t * context);
int parse_curly_rparen(context_t * context);
int parse_paragraph(context_t * context);
int parse_block(context_t * context);
int parse_block_internal(context_t * context);
int parse_list(context_t * context);
int parse_list_internal(context_t * context);
int parse_element(context_t * context);
int parse_argument(context_t * context);
int parse_argument_internal(context_t * context);
int parse_statement(context_t * context);
int parse_expression(context_t * context);
int parse_unsigned_number_literal(context_t * context);
int parse_string_literal(context_t * context);
int parse_symbol(context_t * context);
int parse_binary_operator(context_t * context);
int parse_prefix_operator(context_t * context);

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
    context_t c;
    context_init(&c);
    return(read_internal(&c));
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
    while ((!is_eof(last_char = context_read_char(context))) && ((is_space(last_char) || (is_crlf(last_char)))));
    context_read_back(context, 1);
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

int parse_single_char_binary_operator(context_t * context, wchar_t wc)
{
    data operator;
    wint_t last_char;
    last_char = context_read_char(context);
    if (last_char == wc)
    {
        context_push_char(context, last_char);
        operator = find_binary_operator(context_get_token_string(context));
        if (!operator)
        {
            error(L"<binary operator %c> not found", wc);
            return(0);
        }
        context_set_token_data(context, get_operator_impl(operator));
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

/* <list> */
/* <block> */
/* <statement> */
int parse_paragraph(context_t * context)
{
    data internal;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_list(context) || parse_block(context) || parse_statement(context))
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

/* "(" <list internal> */
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

/* ")" */
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

/* ")" */
/* <assignment expression> ")" */
/* <assignment expression> "," <argument internal> */
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

/* <symbol> "=" <assignment expression> */
/* <expression> */
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
        if (parse_single_char_binary_operator(context, L'='))
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

    if (parse_expression(context))
        return(1);

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
    if (parse_lparen(context))
        return(1);
    return(0);
}

/* <symbol> <dummy argument> "{" <statement> "}" */
int parse_c_function_declaration(context_t * context)
{
    data type, symbol, dummy_argument, statement;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_type(context))
    {
        type = context_get_token_data(context);
        context_clear_token(context);
        if (parse_symbol(context))
        {
            symbol = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_lparen(context))
            {
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
                                context_clear_token(context);
                                skip_space(context);
                                context_set_token_data(context,
                                    make_pair(_function, make_pair(symbol, make_pair(dummy_argument, make_pair(statement, nil))))
                                );
                                return(1);
                            }
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

/* <assignment expression> ";" */
/* <assignment expression> "," <statement> */
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

/* "(" <expression> ")" */
/* <polynominal> */
int parse_expression(context_t * context)
{
    data expression;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_lparen(context))
    {
        context_clear_token(context);
        skip_space(context);
        if (parse_expression(context))
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
        context_restore(context, &saved_context);
    }
    
    if (parse_polynominal(context))
    {
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <monominal> + <polynominal> */
/* <monominal> - <polynominal>*/
/* <monominal> */
int parse_polynominal(context_t * context)
{
    data first_monominal, rest_monominal, plus_or_minus_operator;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_monominal(context))
    {
        first_monominal = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_single_char_binary_operator(context, L'+')
            || parse_single_char_binary_operator(context, L'-'))
        {
            plus_or_minus_operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_polynominal(context))
            {
                rest_monominal = context_get_token_data(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(plus_or_minus_operator, make_pair(first_monominal, make_pair(rest_monominal, nil))));
                return(1);
            }
            goto restore;
        }
        context_clear_token(context);
        context_set_token_data(context, first_monominal);
        return(1);
    }

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <signed number> * <monominal> */
/* <signed number> / <monominal> */
/* <signed number> % <monominal> */
/* <signed number> */
/* <string expression> */
int parse_monominal(context_t * context)
{
    data left, right, binary_operator;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_signed_number(context) || parse_symbol(context))
    {
        left = context_get_token_data(context);
        context_clear_token(context);
        skip_space(context);
        if (parse_single_char_binary_operator(context, L'*')
            || parse_single_char_binary_operator(context, L'/')
            || parse_single_char_binary_operator(context, L'%'))
        {
            binary_operator = context_get_token_data(context);
            context_clear_token(context);
            skip_space(context);
            if (parse_monominal(context))
            {
                right = context_get_token_data(context);
                context_clear_token(context);
                context_set_token_data(context, make_pair(binary_operator, make_pair(left, make_pair(right, nil))));
                return(1);
            }
        }
        context_clear_token(context);
        context_set_token_data(context, left);
        return(1);
    }

    if (parse_string_expression(context))
        return(1);

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* + <signed number> */
/* - <signed number> */
/* <unsigned number> */
int parse_signed_number(context_t * context)
{
    data signed_number;
    saved_context_t saved_context;
    context_save(context, &saved_context);

    if (parse_single_char(context, L'+'))
    {
        context_clear_token(context);
        skip_space(context);
        if (parse_signed_number(context))
            return(1);
        goto restore;
    }
    else if (parse_single_char(context, L'-'))
    {
        context_clear_token(context);
        skip_space(context);
        if (parse_signed_number(context))
        {
            signed_number = context_get_token_data(context);
            context_clear_token(context);
            context_get_token_data(make_pair(_mul_2op_v, make_pair(signed_number, make_pair(make_int(-1), nil))));
            return(1);
        }
        goto restore;
    }
    else if (parse_unsigned_number(context))
        return(1);

restore:
    context_restore(context, &saved_context);
    return(0);
}

int parse_number_symbol(context_t * context)
{
    if (parse_symbol(context))
        return(1); //TODO
    return(0);
}

/* <unsigned number literal> */
/* <c function call> */
/* <number symbol> */
int parse_unsigned_number(context_t * context)
{
    if (parse_unsigned_number_literal(context) || parse_c_function_call(context) || parse_number_symbol(context))
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

restore:
    context_restore(context, &saved_context);
    return(0);
}

/* <string literal> + <string expression> */
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
        if (parse_single_char_binary_operator(context, L"+"))
        {
            concatenate_operator = context_get_token_data(context);
            skip_space(context);
            if (parse_string_expression(context))
            {
                right = context_get_token_data(context);
                context_set_token_data(context, make_pair(concatenate_operator, make_pair(left, make_pair(right, nil))));
                return(1);
            }
        }
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
    
restore:
    context_restore(context, &saved_context);
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
