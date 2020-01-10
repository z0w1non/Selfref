#ifndef READ_H
#define READ_H

#include <wchar.h>
#include "data.h"

struct context_tag;
typedef struct context_tag context;

void   init_context(context *);
wint_t read_char(context *);
void   read_back(context *);
void   push_char(context *);
void   pop_char(context *);
void clear_token(context * c);

int             token_kind(context *);
const wchar_t * token_str(context *);
data            token_data(context *);
void            set_token_kind(context *, int);
void            set_token_data(context *, data);

data read_stdin();
data read_file(const wchar_t * filename);
data read(context *);
void read_token(context *);
data read_list(context *);

int parse_lparen(context * c);
int parse_rparen(context * c);
int parse_number(context *);
int parse_string(context *);
int parse_symbol(context *);

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
