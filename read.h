#ifndef READ_H
#define READ_H

#include <wchar.h>
#include "data.h"

typedef wint_t(*getchar_t)();

struct context_tag;
typedef struct context_tag context;

void   initcontext(context *, getchar_t);
wint_t readchar(context *);
void   readback(context *);
void   pushchar(context *);
void   popchar(context *);
void cleartoken(context * c);

int             tokenkind(context *);
const wchar_t * tokenstr(context *);
data            tokendata(context *);
void            settokenkind(context *, int);
void            settokendata(context *, data);

data readstdin();
data read(context *);
void readtoken(context *);
data readlist(context *);

int parselparen(context * c);
int parserparen(context * c);
int parsenum(context *);
int parsestr(context *);
int parsesym(context *);

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

data listize(data);

#endif
