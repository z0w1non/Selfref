#include "read.h"

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
    getchar_t getchar;
    token tkn;
    wint_t buf[contextbuflen];
    int lastreadbuf;
    int backcnt;
} context;

/******************/
/* Read character */
/******************/
void initcontext(context * c, getchar_t getchar)
{
    memset(c, 0, sizeof(context));
    c->getchar = getchar;
    c->lastreadbuf = -1;
}

wint_t readchar(context * c)
{
    if (c->backcnt > 0)
    {
        c->backcnt -= 1;
        return(c->buf[(c->lastreadbuf + contextbuflen - c->backcnt) % contextbuflen]);
    }
    c->lastreadbuf += 1;
    c->lastreadbuf %= contextbuflen;
    return(c->buf[c->lastreadbuf] = c->getchar());
}

void readback(context * c)
{
    c->backcnt += 1;
}

void pushchar(context * c, wchar_t wc)
{
    c->tkn.string[c->tkn.length++] = wc;
    c->tkn.string[c->tkn.length] = L'\0';
}

void popchar(context * c)
{
    c->tkn.string[--(c->tkn.length)] = L'\0';
}

void cleartoken(context * c)
{
    c->tkn.data = NULL;
    c->tkn.kind = 0;
    c->tkn.length = 0;
    c->tkn.string[0] = L'\0';
}

int tokenkind(context * c)
{
    return(c->tkn.kind);
}

const wchar_t * tokenstr(context * c)
{
    return(c->tkn.string);
}

data tokendata(context * c)
{
    return(c->tkn.data);
}

void settokenkind(context * c, int kind)
{
    c->tkn.kind = kind;
}

void settokendata(context * c, data d)
{
    c->tkn.data = d;
}

/**************/
/* Read input */
/**************/
data readstdin()
{
    context c;
    initcontext(&c, getwchar);
    return(read(&c));
}

data read(context * c)
{
    readtoken(c);
    switch (tokenkind(c))
    {
    case token_eof:
        return(NULL);
    case token_lparen:
        return(readlist(c));
    case token_literal:
    case token_symbol:
    //case token_quote:
        return(tokendata(c));
    default:
        error(L"Unexpected character\n");
    }
}

void readtoken(context * c)
{
    cleartoken(c);
    if (!skipnprint(c))
    {
        if (parselparen(c))
            ;
        else if (parserparen(c))
            ;
        else if (parsenum(c))
            ;
        else if (parsestr(c))
            ;
        else if (parsesym(c))
            ;
        else if (skipnprint(c))
            settokenkind(c, token_eof);
        else
            error(L"Undefined token.\n");
    }
}

data readlist(context * c)
{
    data car;
    readtoken(c);
    if (tokenkind(c) == token_rparen)
        return(nil);
    else if (tokenkind(c) == token_lparen)
    {
        car = readlist(c);
        return(makecons(car, readlist(c)));
    }
    car = tokendata(c);
    return(makecons(car, readlist(c)));
}

int skipnprint(context * c)
{
    wint_t lastchar;
    while ((!iseof(lastchar = readchar(c))) && ((isspace(lastchar) || (iscrlf(lastchar)))));
    if (iseof(lastchar))
        return(1);
    readback(c);
    return(0);
}

int parselparen(context * c)
{
    wint_t lastchar;
    lastchar = readchar(c);
    if (islparen(lastchar))
    {
        pushchar(c, lastchar);
        settokenkind(c, token_lparen);
        settokendata(c, NULL);
        return(1);
    }
    readback(c);
    return(0);
}

int parserparen(context * c)
{
    wint_t lastchar;
    lastchar = readchar(c);
    if (isrparen(lastchar))
    {
        pushchar(c, lastchar);
        settokenkind(c, token_rparen);
        settokendata(c, NULL);
        return(1);
    }
    readback(c);
    return(0);
}

int parsenum(context * c)
{
    int i;
    double d, div;
    wint_t lastchar;
    lastchar = readchar(c);
    if (isdigit(lastchar))
    {
        i = ((int)(lastchar - L'0'));
        lastchar = readchar(c);
        while ((!iseof(lastchar)) && (isdigit(lastchar)))
        {
            i = i * 10 + ((int)(lastchar - L'0'));
            pushchar(c, (wchar_t)lastchar);
            lastchar = readchar(c);
        }
        if (isdot(lastchar))
        {
            pushchar(c, L'.');
            d = 0.0;
            div = 1.0;
            lastchar = readchar(c);
            while ((!iseof(lastchar)) && (isdigit(lastchar)))
            {
                d = d * 10.0 + (double)(lastchar - L'0');
                div *= 10;
                pushchar(c, (wchar_t)lastchar);
                lastchar = readchar(c);
            }
            settokenkind(c, token_literal);
            settokendata(c, makedbl((double)i + d / div));
        }
        else
        {
            settokenkind(c, token_literal);
            settokendata(c, makeint(i));
        }
        if (isletter(lastchar))
        {
            settokenkind(c, token_unknown);
            settokendata(c, NULL);
            pushchar(c, (wchar_t)lastchar);
            error(L"Number literal parsing failed. (%s)\n", c->tkn.string);
        }
        readback(c);
        return(1);
    }
    readback(c);
    return(0);
}

int parsestr(context * c)
{
    wint_t lastchar;
    lastchar = readchar(c);
    if (lastchar == L'\"')
    {
        while (!iseof((lastchar = readchar(c))) && (lastchar != L'\"'))
        {
            if (lastchar == L'\\')
            {
                lastchar = readchar(c);
                switch (lastchar)
                {
                case L'n':  pushchar(c, L'\n'); break;
                case L't':  pushchar(c, L'\t'); break;
                case L'\\': pushchar(c, L'\\'); break;
                case L'\"': pushchar(c, L'\"'); break;
                default: error(L"Unknown escape sequence (\\n -> newline, \\t -> tab, \\\\ -> \\, \\\" -> double quotation(\"))\n");
                }
            }
            else if (!isprint(lastchar))
                error(L"String literal contains a unprintable character.\n");
            else
                pushchar(c, (wchar_t)lastchar);
        }
        settokenkind(c, token_literal);
        settokendata(c, makestr(tokenstr(c)));
        return(1);
    }
    readback(c);
    return(0);
}

int parsesym(context * c)
{
    wint_t lastchar;
    while ((!iseof(lastchar = readchar(c))) && (isletter(lastchar)))
        pushchar(c, (wchar_t)lastchar);
    if(c->tkn.length > 0)
    {
        settokenkind(c, token_symbol);
        settokendata(c, makesym(tokenstr(c)));
    }
    else
        return(0);
    readback(c);
    return(1);
}

data listize(data d)
{
    if (csymp(d))
        return(makecons(d, nil));
    return(d);
}