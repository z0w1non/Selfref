#include "char.h"

#include "wctype.h"

int chartype(wint_t wi)
{
    if (is_digit(wi))
        return(wctype_digit);
    if (is_symbol_char(wi))
        return(wctype_letter);
    if (is_lparen(wi))
        return(wctype_lparen);
    if (is_rparen(wi))
        return(wctype_rparen);
    if (is_dot(wi))
        return(wctype_dot);
    if (is_space(wi))
        return(wctype_space);
    return(wctype_other);
}

int is_digit(wint_t wi)
{
    return((wi >= L'0') && (wi <= L'9'));
}

int is_eof(wint_t wi)
{
    return(wi == WEOF);
}

int is_space(wint_t wi)
{
    return((wi == L' ') || (wi == L'\t'));
}

int is_lparen(wint_t wi)
{
    return(wi == L'(');
}

int is_rparen(wint_t wi)
{
    return(wi == L')');
}

int is_symbol_char(wint_t wi)
{
    return(
        ((wi >= L'a') && (wi <= L'z'))
     || ((wi >= L'A') && (wi <= L'Z'))
     || (wi == L'_')
     || (wi == L'.')
     || (wi == L',')
     || (wi == L':')
     || (wi == L';')
     || (wi == L'+')
     || (wi == L'-')
     || (wi == L'*')
     || (wi == L'/')
     || (wi == L'%')
     || (wi == L'=')
     || (wi == L'<')
     || (wi == L'>')
     || (wi == L'!')
     || (wi == L'?')
     || (wi == L'\'')
    );
}

int is_dot(wint_t wi)
{
    return(wi == L'.');
}

int is_print(wint_t wi)
{
    return(iswprint(wi));
}

int is_crlf(wint_t wi)
{
    return(wi == L'\n' || wi == L'\r');
}

int is_semicolon(wint_t wi)
{
    return(wi == L';');
}

int is_comma(wint_t wi)
{
    return(wi == L',');
}
