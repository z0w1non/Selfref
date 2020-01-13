#include "read.h"

#pragma warning(disable: 4996)

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
    wint_t (*getchar)(void *);
    void * (*callback)(void *);
    void * impldata;
} context;

/********************/
/* Private function */
/********************/
void   setgetchar(context * c, wint_t(*getchar)(void *));
void   seteofcallback(context * c, void (*callback)(void *));
void   setimpldata(context * c, void * impldata);
void * calleofcallback(context * c);
wint_t callgetchar(context * c);
void * callback_fclose(void * impldata);
wint_t getchar_from_file(void * impldata);
wint_t getchar_from_stdin(void * impldata);

/******************/
/* Read character */
/******************/
void init_context(context * c)
{
    memset(c, 0, sizeof(context));
    c->lastreadbuf = -1;
    c->getchar = NULL;
    c->callback = NULL;
    c->impldata = NULL;
}

wint_t read_char(context * c)
{
    if (c->backcnt > 0)
    {
        c->backcnt -= 1;
        return(c->buf[(c->lastreadbuf + contextbuflen - c->backcnt) % contextbuflen]);
    }
    c->lastreadbuf += 1;
    c->lastreadbuf %= contextbuflen;
    return(c->buf[c->lastreadbuf] = c->getchar(c->impldata));
}

void read_back(context * c)
{
    c->backcnt += 1;
}

void push_char(context * c, wchar_t wc)
{
    c->tkn.string[c->tkn.length++] = wc;
    c->tkn.string[c->tkn.length] = L'\0';
}

void pop_char(context * c)
{
    c->tkn.string[--(c->tkn.length)] = L'\0';
}

void clear_token(context * c)
{
    c->tkn.data = NULL;
    c->tkn.kind = 0;
    c->tkn.length = 0;
    c->tkn.string[0] = L'\0';
}

int token_kind(context * c)
{
    return(c->tkn.kind);
}

const wchar_t * token_str(context * c)
{
    return(c->tkn.string);
}

data token_data(context * c)
{
    return(c->tkn.data);
}

void set_token_kind(context * c, int kind)
{
    c->tkn.kind = kind;
}

void set_token_data(context * c, data d)
{
    c->tkn.data = d;
}

/**************/
/* Read input */
/**************/
data read_stdin()
{
    context c;
    init_context(&c, getwchar);
    setgetchar(&c, getchar_from_stdin);
    return(read(&c));
}

data read_file(const wchar_t * filename)
{
    context c;
    void * fileptr;
    char mbs[1024];
    wcstombs(mbs, filename, sizeof(mbs) / sizeof(*mbs));
    fileptr = (void *)(fopen(mbs, "r"));
    init_context(&c);
    setgetchar(&c, getchar_from_file);
    setimpldata(&c, fileptr);
    seteofcallback(&c, callback_fclose);
}

data read(context * c)
{
    read_token(c);
    switch (token_kind(c))
    {
    case token_eof:
        return(NULL);
    case token_lparen:
        return(read_list(c));
    case token_literal:
    case token_symbol:
    //case token_quote:
        return(token_data(c));
    default:
        error(L"Unexpected character\n");
    }
}

void read_token(context * c)
{
    clear_token(c);
    if (!skipnprint(c))
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
        else if (skipnprint(c))
            set_token_kind(c, token_eof);
        else
            error(L"Undefined token.\n");
    }
}

data read_list(context * c)
{
    data car;
    read_token(c);
    if (token_kind(c) == token_rparen)
        return(nil);
    else if (token_kind(c) == token_lparen)
    {
        car = read_list(c);
        return(make_pair(car, read_list(c)));
    }
    car = token_data(c);
    return(make_pair(car, read_list(c)));
}

int skipnprint(context * c)
{
    wint_t lastchar;
    while ((!is_eof(lastchar = read_char(c))) && ((is_space(lastchar) || (is_crlf(lastchar)))));
    if (is_eof(lastchar))
        return(1);
    read_back(c);
    return(0);
}

int parse_lparen(context * c)
{
    wint_t lastchar;
    lastchar = read_char(c);
    if (is_lparen(lastchar))
    {
        push_char(c, lastchar);
        set_token_kind(c, token_lparen);
        set_token_data(c, NULL);
        return(1);
    }
    read_back(c);
    return(0);
}

int parse_rparen(context * c)
{
    wint_t lastchar;
    lastchar = read_char(c);
    if (is_rparen(lastchar))
    {
        push_char(c, lastchar);
        set_token_kind(c, token_rparen);
        set_token_data(c, NULL);
        return(1);
    }
    read_back(c);
    return(0);
}

int parse_number(context * c)
{
    int i;
    double d, div;
    wint_t lastchar;
    lastchar = read_char(c);
    if (is_digit(lastchar))
    {
        i = ((int)(lastchar - L'0'));
        lastchar = read_char(c);
        while ((!is_eof(lastchar)) && (is_digit(lastchar)))
        {
            i = i * 10 + ((int)(lastchar - L'0'));
            push_char(c, (wchar_t)lastchar);
            lastchar = read_char(c);
        }
        if (is_dot(lastchar))
        {
            push_char(c, L'.');
            d = 0.0;
            div = 1.0;
            lastchar = read_char(c);
            while ((!is_eof(lastchar)) && (is_digit(lastchar)))
            {
                d = d * 10.0 + (double)(lastchar - L'0');
                div *= 10;
                push_char(c, (wchar_t)lastchar);
                lastchar = read_char(c);
            }
            set_token_kind(c, token_literal);
            set_token_data(c, make_double((double)i + d / div));
        }
        else
        {
            set_token_kind(c, token_literal);
            set_token_data(c, make_int(i));
        }
        if (is_symbol_char(lastchar))
        {
            set_token_kind(c, token_unknown);
            set_token_data(c, NULL);
            push_char(c, (wchar_t)lastchar);
            error(L"Number literal parsing failed. (%s)\n", c->tkn.string);
        }
        read_back(c);
        return(1);
    }
    read_back(c);
    return(0);
}

int parse_string(context * c)
{
    wint_t lastchar;
    lastchar = read_char(c);
    if (lastchar == L'\"')
    {
        while (!is_eof((lastchar = read_char(c))) && (lastchar != L'\"'))
        {
            if (lastchar == L'\\')
            {
                lastchar = read_char(c);
                switch (lastchar)
                {
                case L'n':  push_char(c, L'\n'); break;
                case L't':  push_char(c, L'\t'); break;
                case L'\\': push_char(c, L'\\'); break;
                case L'\"': push_char(c, L'\"'); break;
                default: error(L"Unknown escape sequence (\\n -> newline, \\t -> tab, \\\\ -> \\, \\\" -> double quotation(\"))\n");
                }
            }
            else if (!is_print(lastchar))
                error(L"String literal contains a unprintable character.\n");
            else
                push_char(c, (wchar_t)lastchar);
        }
        set_token_kind(c, token_literal);
        set_token_data(c, make_string(token_str(c)));
        return(1);
    }
    read_back(c);
    return(0);
}

int parse_symbol(context * c)
{
    wint_t lastchar;
    while ((!is_eof(lastchar = read_char(c))) && (is_symbol_char(lastchar)))
        push_char(c, (wchar_t)lastchar);
    if(c->tkn.length > 0)
    {
        set_token_kind(c, token_symbol);
        set_token_data(c, make_symbol(token_str(c)));
    }
    else
        return(0);
    read_back(c);
    return(1);
}

data listize(data d)
{
    if (is_symbol(d))
        return(make_pair(d, nil));
    return(d);
}

/********************/
/* Private function */
/********************/
void setgetchar(context * c, wint_t(*getchar)(void *))
{
    c->getchar = getchar;
}

void seteofcallback(context * c, void (*callback)(void *))
{
    c->callback = callback;
}

void setimpldata(context * c, void * impldata)
{
    c->impldata = impldata;
}

void * calleofcallback(context * c)
{
    void * ret;
    ret = NULL;
    if (c->callback)
    {
        ret = c->callback(c->impldata);
        c->callback = NULL;
        c->impldata = NULL;
    }
    return ret;
}

wint_t callgetchar(context * c)
{
    return(c->getchar(c->impldata));
}

void * callback_fclose(void * impldata)
{
    fclose((FILE *)impldata);
    return(NULL);
}

wint_t getchar_from_file(void * impldata)
{
    wint_t wi;
    int ret;
    ret = fwscanf((FILE *)impldata, L"%c", &wi);
    if (ret == EOF)
        return(WEOF);
    return(wi);
}

wint_t getchar_from_stdin(void * impldata)
{
    return(getwchar());
}