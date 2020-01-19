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
} context;

typedef struct saved_context_tag
{
    size_t count;
} saved_context_t;

FILE * input_stream;

/********************************/
/* Private function declaration */
/********************************/
void            context_init(context *);
wint_t          context_read_char(context *);
void            context_read_back(context *, int count);
void            context_push_char(context *, wchar_t);
void            context_pop_char(context *);
void            context_clear_token(context * c);
const wchar_t * context_get_token_string(context *);
data            context_get_token_data(context *);
void            context_set_token_data(context *, data);
void            context_remove_token_string_suffix(context * c, int length);
void            context_save(context * c, saved_context_t * temp);
void            context_restore(context * c, const saved_context_t * temp);
void            context_debug_print(context * c);

data read_internal(context *);

int skip_space_and_at_eof(context * c);
int parse_single_char(context * c, wchar_t wc);
int parse_single_char_binary_operator(context * c, wchar_t wc);
int parse_lparen(context * c);
int parse_rparen(context * c);
int parse_curly_lparen(context * c);
int parse_curly_rparen(context * c);
int parse_paragraph(context * c);
int parse_block(context * c);
int parse_block_internal(context * c);
int parse_list(context * c);
int parse_list_internal(context *);
int parse_token(context * c);
int parse_statement(context * c);
int parse_expression(context * c);
int parse_unsigned_number_literal(context *);
int parse_string_literal(context *);
int parse_symbol(context *);
int parse_binary_operator(context *);
int parse_prefix_operator(context *);

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
    context c;
    context_init(&c);
    return(read_internal(&c));
}

/******************/
/* Read character */
/******************/
void context_init(context * c)
{
    memset(c, 0, sizeof(context));
    c->last_read_buffer_offset = -1;
    c->total_count = 0;
}

wint_t context_read_char(context * c)
{
    if (c->read_back_count > 0)
    {
        c->read_back_count -= 1;
        return(c->buffer[(c->last_read_buffer_offset + context_buffer_length - c->read_back_count) % context_buffer_length]);
    }
    c->last_read_buffer_offset += 1;
    c->last_read_buffer_offset %= context_buffer_length;
    c->total_count += 1;
    return(c->buffer[c->last_read_buffer_offset] = fgetc(input_stream));
}

void context_read_back(context * c, int count)
{
    c->read_back_count += count;
}

void context_push_char(context * c, wchar_t wc)
{
    c->token.string[c->token.length++] = wc;
    c->token.string[c->token.length] = L'\0';
}

void context_pop_char(context * c)
{
    c->token.string[--(c->token.length)] = L'\0';
}

void context_clear_token(context * c)
{
    c->token.data = NULL;
    c->token.length = 0;
    c->token.string[0] = L'\0';
}

const wchar_t * context_get_token_string(context * c)
{
    return(c->token.string);
}

data context_get_token_data(context * c)
{
    return(c->token.data);
}

void context_set_token_data(context * c, data d)
{
    c->token.data = d;
}

void context_remove_token_string_suffix(context * c, int length)
{
    c->token.string[c->token.length - length] = L'\0';
    c->token.length -= length;
}

void context_save(context * c, saved_context_t * saved_context)
{
    saved_context->count = c->total_count - c->read_back_count;
}

void context_restore(context * c, const saved_context_t * saved_context)
{
    if (c->total_count - c->read_back_count > saved_context->count)
        context_read_back(c, ((int)(c->total_count) - c->read_back_count - saved_context->count));
}

void context_debug_print(context * c)
{
    wint_t last_char;
    saved_context_t saved_context;
    wprintf(L"CONTEXT: ");
    while ((!is_eof(last_char = context_read_char(c))) && (!is_crlf(last_char)))
        wprintf(L"%c", last_char);
    wprintf(L"\n");
    context_restore(c, &saved_context);
}

/**************/
/* Read input */
/**************/
data read_internal(context * c)
{
    if (skip_space_and_at_eof(c))
        return(nil);
    if (parse_paragraph(c))
        return(context_get_token_data(c));
    return(nil);
}

int skip_space_and_at_eof(context * c)
{
    wint_t last_char;
    while ((!is_eof(last_char = context_read_char(c))) && ((is_space(last_char) || (is_crlf(last_char)))));
    if (is_eof(last_char))
        return(1);
    context_read_back(c, 1);
    return(0);
}

int parse_single_char(context * c, wchar_t wc)
{
    wint_t last_char;
    last_char = context_read_char(c);
    if (last_char == wc)
    {
        context_push_char(c, last_char);
        context_set_token_data(c, NULL);
        return(1);
    }
    context_read_back(c, 1);
    return(0);
}

int parse_single_char_binary_operator(context * c, wchar_t wc)
{
    wint_t last_char;
    last_char = context_read_char(c);
    if (last_char == wc)
    {
        context_push_char(c, last_char);
        context_set_token_data(c, find_binary_operator(context_get_token_string(c)));
        return(1);
    }
    context_read_back(c, 1);
    return(0);
}

int parse_lparen(context * c)
{
    return(parse_single_char(c, L'('));
}

int parse_rparen(context * c)
{
    return(parse_single_char(c, L')'));
}

int parse_curly_lparen(context * c)
{
    return(parse_single_char(c, L'{'));
}

int parse_curly_rparen(context * c)
{
    return(parse_single_char(c, L'}'));
}

int parse_comma(context * c)
{
    return(parse_single_char(c, L','));
}

int parse_semicolon(context * c)
{
    return(parse_single_char(c, L';'));
}

/* <list> */
/* <block> */
/* <statement> */
int parse_paragraph(context * c)
{
    data internal;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_list(c) || parse_block(c) || parse_statement(c))
        return(1);

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* "{" <block internal> "}" */
int parse_block(context * c)
{
    data internal;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_curly_lparen(c))
    {
        context_clear_token(c);
        if (parse_block_internal(c))
        {
            internal = context_get_token_data(c);
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_curly_rparen(c))
            {
                context_set_token_data(c, internal);
                return(1);
            }
        }
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <statement> <block internal> */
/* <statement> */
int parse_block_internal(context * c)
{
    data first_statement, rest_block_internal;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_curly_rparen(c))
    {
        context_restore(c, &saved_context);
        context_clear_token(c);
        context_set_token_data(c, nil);
        return(1);
    }
    else if (parse_statement(c))
    {
        first_statement = context_get_token_data(c);
        context_clear_token(c);
        if (parse_block_internal(c))
        {
            rest_block_internal = context_get_token_data(c);
            context_clear_token(c);
            context_set_token_data(c, make_pair(progn_v, make_pair(first_statement, make_pair(rest_block_internal, nil))));
            return(1);
        }
        return(1);
    }

    context_restore(c, &saved_context);
    return(0);
}

/* "(" <list internal> ")" */
int parse_list(context * c)
{
    data internal;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_lparen(c))
    {
        context_clear_token(c);
        if (parse_list_internal(c))
        {
            internal = context_get_token_data(c);
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_rparen(c))
            {
                context_set_token_data(c, internal);
                return(1);
            }
        }
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <token> <list internal> */
/* <token> */
int parse_list_internal(context * c)
{
    data first_token, rest_token;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_rparen(c))
    {
        context_restore(c, &saved_context);
        context_clear_token(c);
        context_set_token_data(c, nil);
        return(1);
    }
    else if (parse_token(c))
    {
        first_token = context_get_token_data(c);
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_list_internal(c))
        {
            rest_token = context_get_token_data(c);
            context_clear_token(c);
            context_set_token_data(c, make_pair(first_token, rest_token));
            return(1);
        }
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <symbol> */
/* <signed number> */
/* <string> */
/* <paragraph> */
int parse_token(context * c)
{
    if (parse_symbol(c) || parse_signed_number(c) || parse_string_literal(c) || parse_paragraph(c))
        return(1);
    return(0);
}

/* <assignment expression> , <argument> */
/* <assignment expression> */
int parse_argument(context * c)
{
    data assignment_expression, argument;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_assignment_expression(c))
    {
        assignment_expression = context_get_token_data(c);
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_comma(c))
        {
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_argument(c))
            {
                argument = context_get_token_data(c);
                context_clear_token(c);
                if (skip_space_and_at_eof(c))
                    goto restore;
                context_set_token_data(c, make_pair(progn_v, make_pair(assignment_expression, make_pair(argument, nil))));
                return(1);
            }
        }
        context_clear_token(c);
        context_set_token_data(c, assignment_expression);
        return(1);
    }


restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <symbol> "(" <argument> ")" */
int parse_c_function_call(context * c)
{
    data function_symbol, argument;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_symbol(c))
    {
        function_symbol = context_get_token_data(c);
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_lparen(c))
        {
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_argument(c))
            {
                argument = context_get_token_data(c);
                context_clear_token(c);
                if (skip_space_and_at_eof(c))
                    goto restore;
                if (parse_rparen(c))
                {
                    context_clear_token(c);
                    context_set_token_data(c, make_pair(function_symbol, make_pair(argument, nil)));
                    return(1);
                }
            }
        }
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <symbol> "=" <assignment expression> */
/* <expression> */
int parse_assignment_expression(context * c)
{
    data symbol, assign, expression;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_symbol(c))
    {
        symbol = context_get_token_data(c);
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_single_char_binary_operator(c, L'='))
        {
            assign = context_get_token_data(c);
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_assignment_expression(c))
            {
                expression = context_get_token_data(c);
                context_clear_token(c);
                context_set_token_data(c, make_pair(assign, make_pair(symbol, make_pair(expression, nil))));
                return(1);
            }
        }
        context_restore(c, &saved_context);
    }

    if (parse_expression(c))
        return(1);

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <c function call> */
/* <assignment expression> */
int parse_function_expression(context * c)
{
    if (parse_c_function_call(c) || parse_assignment_expression(c))
        return(1);
    return(1);
}

/* <function expression> ";" */
/* <function expression> "," <statement> */
int parse_statement(context * c)
{
    data first_token, rest_statement;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_function_expression(c))
    {
        first_token = context_get_token_data(c);
        debug(first_token);
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_semicolon(c))
        {
            context_clear_token(c);
            context_set_token_data(c, first_token);
            return(1);
        }
        else if (parse_comma(c))
        {
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_statement(c))
            {
                rest_statement = context_get_token_data(c);
                context_clear_token(c);
                context_set_token_data(c, make_pair(progn_v, make_pair(first_token, make_pair(rest_statement, nil))));
                return(1);
            }
        }
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* "(" <expression> ")" */
/* <polynominal> */
int parse_expression(context * c)
{
    data expression;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_lparen(c))
    {
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_expression(c))
        {
            expression = context_get_token_data(c);
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_rparen(c))
            {
                context_clear_token(c);
                context_set_token_data(c, expression);
                return(1);
            }
        }
        context_restore(c, &saved_context);
    }
    
    if (parse_polynominal(c))
    {
        return(1);
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <monominal> */
/* <monominal> + <polynominal> */
/* <monominal> - <polynominal>*/
int parse_polynominal(context * c)
{
    data first_monominal, rest_monominal, plus_or_minus_operator;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_monominal(c))
    {
        first_monominal = context_get_token_data(c);
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_single_char_binary_operator(c, L"+")
            || parse_single_char_binary_operator(c, L"-"))
        {
            plus_or_minus_operator = context_get_token_data(c);
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_polynominal(c))
            {
                rest_monominal = context_get_token_data(c);
                context_clear_token(c);
                context_set_token_data(c, make_pair(plus_or_minus_operator, make_pair(first_monominal, make_pair(rest_monominal, nil))));
                return(1);
            }
            goto restore;
        }
        context_clear_token(c);
        context_set_token_data(c, first_monominal);
        return(1);
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <signed number> */
/* <signed number> * <monominal> */
/* <signed number> / <monominal> */
/* <signed number> % <monominal> */
/* <string expression> */
int parse_monominal(context * c)
{
    data left, right, binary_operator;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_signed_number(c) || parse_symbol(c))
    {
        left = context_get_token_data(c);
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_single_char_binary_operator(c, L'*')
            || parse_single_char_binary_operator(c, L'/')
            || parse_single_char_binary_operator(c, L'%'))
        {
            binary_operator = context_get_token_data(c);
            context_clear_token(c);
            if (skip_space_and_at_eof(c))
                goto restore;
            if (parse_monominal(c))
            {
                right = context_get_token_data(c);
                context_clear_token(c);
                context_set_token_data(c, make_pair(binary_operator, make_pair(left, make_pair(right, nil))));
                return(1);
            }
        }
        context_clear_token(c);
        context_set_token_data(c, left);
        return(1);
    }

    if (parse_string_expression(c))
        return(1);

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* + <signed number> */
/* - <signed number> */
/* <unsigned number> */
int parse_signed_number(context * c)
{
    data signed_number;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_single_char(c, L'+'))
    {
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_signed_number(c))
            return(1);
        goto restore;
    }
    else if (parse_single_char(c, L'-'))
    {
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_signed_number(c))
        {
            signed_number = context_get_token_data(c);
            context_clear_token(c);
            context_get_token_data(make_pair(_mul_2op_v, make_pair(signed_number, make_pair(make_int(-1), nil))));
            return(1);
        }
        goto restore;
    }
    else if (parse_unsigned_number(c))
        return(1);

restore:
    context_restore(c, &saved_context);
    return(0);
}

int parse_number_symbol(context * c)
{
    if (parse_symbol(c))
        return(1); //TODO
    return(0);
}

/* <unsigned number literal> */
/* <number symbol> */
/* <c function call> */
int parse_unsigned_number(context * c)
{
    if (parse_unsigned_number_literal(c) || parse_number_symbol(c) || parse_c_function_call(c))
        return(1);
    return(0);
}

/* [0-9]+ */
/* [0-9]+.[0-9]+ */
int parse_unsigned_number_literal(context * c)
{
    int i;
    double d, div;
    wint_t last_char;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    last_char = context_read_char(c);

    if (is_digit(last_char))
    {
        i = ((int)(last_char - L'0'));
        last_char = context_read_char(c);
        while ((!is_eof(last_char)) && (is_digit(last_char)))
        {
            i = i * 10 + ((int)(last_char - L'0'));
            context_push_char(c, (wchar_t)last_char);
            last_char = context_read_char(c);
        }
        if (is_dot(last_char))
        {
            context_push_char(c, L'.');
            d = 0.0;
            div = 1.0;
            last_char = context_read_char(c);
            while ((!is_eof(last_char)) && (is_digit(last_char)))
            {
                d = d * 10.0 + (double)(last_char - L'0');
                div *= 10;
                context_push_char(c, (wchar_t)last_char);
                last_char = context_read_char(c);
            }
            context_read_back(c, 1);
            context_set_token_data(c, make_double(((double)i + d / div)));
            return(1);
        }
        else
        {
            context_read_back(c, 1);
            context_set_token_data(c, make_int(i));
            return(1);
        }
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

/* <string literal> + <string expression> */
/* <string literal> */
int parse_string_expression(context * c)
{
    data left, right, concatenate_operator;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    if (parse_string_literal(c))
    {
        left = context_get_token_data(c);
        context_clear_token(c);
        if (skip_space_and_at_eof(c))
            goto restore;
        if (parse_single_char_binary_operator(c, L"+"))
        {
            concatenate_operator = context_get_token_data(c);
            if (skip_space_and_at_eof(c))
                goto restore;

            if (parse_string_expression(c))
            {
                right = context_get_token_data(c);
                context_set_token_data(c, make_pair(concatenate_operator, make_pair(left, make_pair(right, nil))));
                return(1);
            }
        }
        context_clear_token(c);
        context_set_token_data(c, left);
        return(1);
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

int parse_string_literal(context * c)
{
    wint_t last_char;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    last_char = context_read_char(c);
    if (last_char == L'\"')
    {
        while (!is_eof((last_char = context_read_char(c))) && (last_char != L'\"'))
        {
            if (last_char == L'\\')
            {
                last_char = context_read_char(c);
                switch (last_char)
                {
                case L'n':  context_push_char(c, L'\n'); break;
                case L't':  context_push_char(c, L'\t'); break;
                case L'\\': context_push_char(c, L'\\'); break;
                case L'\"': context_push_char(c, L'\"'); break;
                default: error(L"Unknown escape sequence (\\n -> newline, \\t -> tab, \\\\ -> \\, \\\" -> double quotation(\"))\n");
                }
            }
            else if (!is_print(last_char))
                goto restore;
            else
                context_push_char(c, (wchar_t)last_char);
        }
        context_set_token_data(c, make_string(context_get_token_string(c)));
        return(1);
    }
    
restore:
    context_restore(c, &saved_context);
    return(0);
}

int parse_symbol(context * c)
{
    wint_t last_char;
    data prefix_operator;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    while ((!is_eof(last_char = context_read_char(c))) && (is_symbol_char(last_char)))
        context_push_char(c, (wchar_t)last_char);
    context_read_back(c, 1);

    if (c->token.length > 0)
    {
        context_set_token_data(c, make_symbol(context_get_token_string(c)));
        return(1);
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

int parse_binary_operator(context * c)
{
    wint_t last_char;
    data longest_matched_operator;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    longest_matched_operator = NULL;
    while (1)
    {
        if (is_eof(last_char = context_read_char(c)) || !is_symbol_char(last_char))
        {
            if (!longest_matched_operator)
                goto restore;
            context_set_token_data(c, longest_matched_operator);
            context_read_back(c, 1);
            return(1);
        }

        context_push_char(c, last_char);
        longest_matched_operator = find_binary_operator(context_get_token_string(c));
        if (!longest_matched_operator)
            goto restore;
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}

int parse_prefix_operator(context * c)
{
    wint_t last_char;
    data longest_matched_operator;
    saved_context_t saved_context;
    context_save(c, &saved_context);

    longest_matched_operator = NULL;
    while (1)
    {
        if (is_eof(last_char = context_read_char(c)) || !is_symbol_char(last_char))
        {
            if (!longest_matched_operator)
                goto restore;
            context_set_token_data(c, longest_matched_operator);
            context_read_back(c, 1);
            return(1);
        }

        context_push_char(c, last_char);
        longest_matched_operator = find_prefix_operator(context_get_token_string(c));
        if (!longest_matched_operator)
            goto restore;
    }

restore:
    context_restore(c, &saved_context);
    return(0);
}
