#include "read.h"

#include <stdlib.h>
#include <stdio.h>
#include "char.h"
#include "print.h"

enum
{
    tokenmax = 256,
};

typedef struct token_tag
{
    int kind;
    wchar_t string[tokenmax];
    int length;
    data data;
} token;

enum
{
    contextbuflen = 256,
};

typedef struct context_tag
{
    token tkn;
    wint_t buf[contextbuflen];
    int lastreadbuf;
    int backcnt;
} context;

enum
{
    token_unknown,
    token_eof,
    token_lparen,
    token_rparen,
    token_literal,
    token_symbol,
    token_quote,
};

FILE * input_stream;

/********************************/
/* Private function declaration */
/********************************/
void            context_init(context *);
wint_t          context_read_char(context *);
void            context_read_back(context *);
void            context_push_char(context *, wchar_t);
void            context_pop_char(context *);
void            context_clear_token(context * c);
int             context_get_token_kind(context *);
const wchar_t * context_get_token_str(context *);
data            context_get_token_data(context *);
void            context_set_token_kind(context *, int);
void            context_set_token_data(context *, data);

data read_internal(context *);
void read_token(context *);
data read_list(context *);

int parse_nonprintable(context * c);
int parse_lparen(context * c);
int parse_rparen(context * c);
int parse_number(context *);
int parse_string(context *);
int parse_symbol(context *);

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
    c->lastreadbuf = -1;
}

wint_t context_read_char(context * c)
{
    if (c->backcnt > 0)
    {
        c->backcnt -= 1;
        return(c->buf[(c->lastreadbuf + contextbuflen - c->backcnt) % contextbuflen]);
    }
    c->lastreadbuf += 1;
    c->lastreadbuf %= contextbuflen;
    return(c->buf[c->lastreadbuf] = fgetc(input_stream));
}

void context_read_back(context * c)
{
    c->backcnt += 1;
}

void context_push_char(context * c, wchar_t wc)
{
    c->tkn.string[c->tkn.length++] = wc;
    c->tkn.string[c->tkn.length] = L'\0';
}

void context_pop_char(context * c)
{
    c->tkn.string[--(c->tkn.length)] = L'\0';
}

void context_clear_token(context * c)
{
    c->tkn.data = NULL;
    c->tkn.kind = 0;
    c->tkn.length = 0;
    c->tkn.string[0] = L'\0';
}

int context_get_token_kind(context * c)
{
    return(c->tkn.kind);
}

const wchar_t * context_get_token_str(context * c)
{
    return(c->tkn.string);
}

data context_get_token_data(context * c)
{
    return(c->tkn.data);
}

void context_set_token_kind(context * c, int kind)
{
    c->tkn.kind = kind;
}

void context_set_token_data(context * c, data d)
{
    c->tkn.data = d;
}

/**************/
/* Read input */
/**************/
data read_internal(context * c)
{
    read_token(c);
    switch (context_get_token_kind(c))
    {
    case token_eof:
        return(NULL);
    case token_lparen:
        return(read_list(c));
    case token_literal:
    case token_symbol:
    //case token_quote:
        return(context_get_token_data(c));
    default:
        error(L"Unexpected character\n");
    }
    return(nil);
}

void read_token(context * c)
{
    context_clear_token(c);
    if (!parse_nonprintable(c))
    {
        if (parse_lparen(c))
            ;
        else if (parse_rparen(c))
            ;
        else if (parse_number(c))
            ;
        else if (parse_string(c))
            ;
        else if (parse_symbol(c))
            ;
        else if (parse_nonprintable(c))
            context_set_token_kind(c, token_eof);
        else
            error(L"Undefined token.\n");
    }
}

data read_list(context * c)
{
    data car;
    read_token(c);
    if (context_get_token_kind(c) == token_rparen)
        return(nil);
    else if (context_get_token_kind(c) == token_lparen)
    {
        car = read_list(c);
        return(make_pair(car, read_list(c)));
    }
    car = context_get_token_data(c);
    return(make_pair(car, read_list(c)));
}

int parse_nonprintable(context * c)
{
    wint_t lastchar;
    while ((!is_eof(lastchar = context_read_char(c))) && ((is_space(lastchar) || (is_crlf(lastchar)))));
    if (is_eof(lastchar))
        return(1);
    context_read_back(c);
    return(0);
}

int parse_lparen(context * c)
{
    wint_t lastchar;
    lastchar = context_read_char(c);
    if (is_lparen(lastchar))
    {
        context_push_char(c, lastchar);
        context_set_token_kind(c, token_lparen);
        context_set_token_data(c, NULL);
        return(1);
    }
    context_read_back(c);
    return(0);
}

int parse_rparen(context * c)
{
    wint_t lastchar;
    lastchar = context_read_char(c);
    if (is_rparen(lastchar))
    {
        context_push_char(c, lastchar);
        context_set_token_kind(c, token_rparen);
        context_set_token_data(c, NULL);
        return(1);
    }
    context_read_back(c);
    return(0);
}

int parse_number(context * c)
{
    int i;
    double d, div;
    wint_t lastchar;
    lastchar = context_read_char(c);
    if (is_digit(lastchar))
    {
        i = ((int)(lastchar - L'0'));
        lastchar = context_read_char(c);
        while ((!is_eof(lastchar)) && (is_digit(lastchar)))
        {
            i = i * 10 + ((int)(lastchar - L'0'));
            context_push_char(c, (wchar_t)lastchar);
            lastchar = context_read_char(c);
        }
        if (is_dot(lastchar))
        {
            context_push_char(c, L'.');
            d = 0.0;
            div = 1.0;
            lastchar = context_read_char(c);
            while ((!is_eof(lastchar)) && (is_digit(lastchar)))
            {
                d = d * 10.0 + (double)(lastchar - L'0');
                div *= 10;
                context_push_char(c, (wchar_t)lastchar);
                lastchar = context_read_char(c);
            }
            context_set_token_kind(c, token_literal);
            context_set_token_data(c, make_double((double)i + d / div));
        }
        else
        {
            context_set_token_kind(c, token_literal);
            context_set_token_data(c, make_int(i));
        }
        if (is_symbol_char(lastchar))
        {
            context_set_token_kind(c, token_unknown);
            context_set_token_data(c, NULL);
            context_push_char(c, (wchar_t)lastchar);
            error(L"Number literal parsing failed. (%s)\n", c->tkn.string);
        }
        context_read_back(c);
        return(1);
    }
    context_read_back(c);
    return(0);
}

int parse_string(context * c)
{
    wint_t lastchar;
    lastchar = context_read_char(c);
    if (lastchar == L'\"')
    {
        while (!is_eof((lastchar = context_read_char(c))) && (lastchar != L'\"'))
        {
            if (lastchar == L'\\')
            {
                lastchar = context_read_char(c);
                switch (lastchar)
                {
                case L'n':  context_push_char(c, L'\n'); break;
                case L't':  context_push_char(c, L'\t'); break;
                case L'\\': context_push_char(c, L'\\'); break;
                case L'\"': context_push_char(c, L'\"'); break;
                default: error(L"Unknown escape sequence (\\n -> newline, \\t -> tab, \\\\ -> \\, \\\" -> double quotation(\"))\n");
                }
            }
            else if (!is_print(lastchar))
                error(L"String literal contains a unprintable character.\n");
            else
                context_push_char(c, (wchar_t)lastchar);
        }
        context_set_token_kind(c, token_literal);
        context_set_token_data(c, make_string(context_get_token_str(c)));
        return(1);
    }
    context_read_back(c);
    return(0);
}

int parse_symbol(context * c)
{
    wint_t lastchar;
    while ((!is_eof(lastchar = context_read_char(c))) && (is_symbol_char(lastchar)))
        context_push_char(c, (wchar_t)lastchar);
    if(c->tkn.length > 0)
    {
        context_set_token_kind(c, token_symbol);
        context_set_token_data(c, make_symbol(context_get_token_str(c)));
    }
    else
        return(0);
    context_read_back(c);
    return(1);
}
