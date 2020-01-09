#include "sym.h"

#include <stdlib.h>
#include "builtin.h"
#include "eval.h"
#include "heap.h"
#include "print.h"

/**************/
/* Hash table */
/**************/
enum
{
    state_unused,
    state_used,
    //state_deleted,
};

typedef struct tablecell_tag
{
    int state;
    wchar_t * key;
    data stack;
} tablecell;

typedef struct hashtable_tag
{
    tablecell * data;
    int len;
    int usedcnt;
} hashtable;

void inithashtable(hashtable * table, int len);
void freehashtable(hashtable * table);

hashtable table;

/**********/
/* Rehash */
/**********/
void rehash();
void tryrehash();

/*********************/
/* Internal function */
/*********************/
int strhash(const wchar_t * s);
wchar_t * copystr(const wchar_t * s);
void movestack(hashtable * table, const wchar_t * key, data stack);

/**************/
/* Hash table */
/**************/
void inithashtable(hashtable * table, int len)
{
    int i;
    table->data = malloc(sizeof(tablecell) * len);
    if (!table->data)
        error(L"bad alloc.\n");
    for (i = 0; i < len; ++i)
    {
        table->data[i].state = state_unused;
        table->data[i].key = NULL;
        table->data[i].stack = nil;
    }
    table->len = len;
    table->usedcnt = 0;
}

void freehashtable(hashtable * table)
{
    int i;
    for (i = 0; i < table->len; ++i)
        free(table->data[i].key);
    memset(table, 0, sizeof(table));
}

/**********/
/* Rehash */
/**********/
void rehash()
{
    hashtable newtable;
    int newlen, i;

    if (table.len == 0)
        newlen = 0x400;
    else
        newlen = table.len * 2;

    inithashtable(&newtable, newlen);

    for (i = 0; i < table.len; ++i)
        if (table.data[i].state == state_used)
        {
            movestack(&newtable, table.data[i].key, table.data[i].stack);
            table.data[i].key = NULL;
        }

    freehashtable(&table);
    table = newtable;
}

void tryrehash()
{
    if (((double)table.usedcnt / table.len) >= 0.75)
        rehash();
}

/*********************/
/* Internal function */
/*********************/
unsigned int strhash(const wchar_t * s)
{
    unsigned int i = 0;
    while (*s)
    {
        i = (i * 257) ^ (*s);
        s += 1;
    }
    return(i);
}

wchar_t * copystr(const wchar_t * s)
{
    int len;
    wchar_t * newstr;
    len = wcslen(s) + 1;
    newstr = (wchar_t *)malloc(sizeof(wchar_t) * len);
    if (!newstr)
        error(L"Heap memory allocation failed.\n");
    wcscpy_s(newstr, len, s);
    return(newstr);
}

void movestack(hashtable * table, const wchar_t * key, data stack)
{
    int i;
    data d;
    i = (strhash(key) % table->len);
    table->data[i].state = state_used;
    table->data[i].key = key;
    table->data[i].stack = stack;
}

/***********************/
/* Symbol map operator */
/***********************/
void initsym()
{
    rehash();

    cpushsym(L"nil", nil);
    cpushsym(L"t", t);  
    cpushsym(L"quote", quote);
    cpushsym(L"eval", makemacro(eval));
    cpushsym(L"call", makemacro(call));
    cpushsym(L"lambda", makemacro(lambda));
    cpushsym(L"function", makemacro(function));

    cpushsym(L"dumpsym", makefunc(dumpsym));
    cpushsym(L"dumpheap", makefunc(dumpheap));
    cpushsym(L"gc", makefunc(gc));

    cpushsym(L"pushargs", makemacro(pushargs));
    cpushsym(L"popargs", makemacro(popargs));

    cpushsym(L"setcar", makefunc(setcar));
    cpushsym(L"setcdr", makefunc(setcdr));
    cpushsym(L"length", makefunc(length));

    cpushsym(L"if", makemacro(if_));
    cpushsym(L"and", makemacro(and_));
    cpushsym(L"or", makemacro(or_));
    cpushsym(L"not", makefunc(not_));

    cpushsym(L"consp", makefunc(consp));
    cpushsym(L"funcp", makefunc(funcp));
    cpushsym(L"macrop", makefunc(macrop));
    cpushsym(L"symp", makefunc(symp));
    cpushsym(L"nilp", makefunc(nilp));
    cpushsym(L"intp", makefunc(intp));
    cpushsym(L"dblp", makefunc(dblp));
    cpushsym(L"nump", makefunc(nump));
    cpushsym(L"strp", makefunc(strp));
    cpushsym(L"zerop", makefunc(zerop));

    cpushsym(L"add", makefunc(add));
    cpushsym(L"sub", makefunc(sub));
    cpushsym(L"mul", makefunc(mul));
    cpushsym(L"div", makefunc(div_));
    cpushsym(L"mod", makefunc(mod));

    cpushsym(L"list", makefunc(list));
    cpushsym(L"zipfirst", makefunc(zipfirst));
    cpushsym(L"ziprest", makefunc(ziprest));
    cpushsym(L"zip", makefunc(zip));

    cpushsym(L"pushsym", makemacro(pushsym));
    cpushsym(L"popsym", makemacro(popsym));

    cpushsym(L"tocharcode", makefunc(tocharcode));
    cpushsym(L"fromcharcode", makefunc(fromcharcode));

    //pushsym(L"less", makemacro(less));
    //pushsym(L"less-equal", makemacro(less_equal));
    //pushsym(L"greater", makemacro(greater));
    //pushsym(L"greater-equal", makemacro(greater_equal));
    //pushsym(L"equal", makemacro(equal));
    //pushsym(L"not-equal", makemacro(not_equal));
}

void freesym()
{
    freehashtable(&table);
}

void cpushsym(const wchar_t * key, data value)
{
    unsigned int hash;
    int i, j;
    data d;

    tryrehash();

    hash = strhash(key);
    for (i = 0; i < table.len; ++i)
    {
        j = ((hash + i) % table.len);
        if ((table.data[j].state == state_unused))
        {
            table.data[j].state = state_used;
            table.data[j].key = copystr(key);
            table.data[j].stack = makecons(value, table.data[j].stack);
            table.usedcnt += 1;
            return;
        }
        else if(((table.data[j].state == state_used) && (wcscmp(table.data[j].key, key) == 0)))
        {
            table.data[j].stack = makecons(value, table.data[j].stack);
        }
    }
    error(L"pushsymimpl failed.\n");
}

void cpopsym(const wchar_t * key)
{
    unsigned int hash;
    int i, j;
    data d;

    hash = strhash(key);
    for (i = 0; i < table.len; ++i)
    {
        j = ((hash + i) % table.len);
        if ((table.data[j].state == state_used) && (wcscmp(table.data[j].key, key) == 0))
        {
            if (cnnilp(table.data[j].stack))
            {
                d = cdr(table.data[j].stack);
                freedata(table.data[j].stack);
                table.data[j].stack = d;
            }
            return;
        }
    }
    error(L"popsym failed.\n");
}

data findsym(const wchar_t * key)
{
    unsigned int hash;
    int i, j;
    hash = strhash(key);
    for (i = 0; i < table.len; ++i)
    {
        j = ((hash + i) % table.len);
        if ((table.data[j].state == state_used) && (wcscmp(table.data[j].key, key) == 0))
        {
            if (cnnilp(table.data[j].stack))
                return(car(table.data[j].stack));
            return(NULL);
        }
        if (table.data[j].state == state_unused)
            return(NULL);
    }
    return(NULL);
}

void marksym()
{
    int i;
    data d;
    for (i = 0; i < table.len; ++i)
        if (table.data[i].state == state_used)
        {
            d = table.data[i].stack;
            while(cnnilp(d))
            {
                markdata(car(d));
                d = cdr(d);
            }
        }
}

data dumpsym(data d)
{
    int i;
    for (i = 0; i < table.len; ++i)
        if (table.data[i].state == state_used)
        {
            wprintf(L"(\"%s\" ", table.data[i].key);
            print(car(table.data[i].stack));
            wprintf(L")\n");
        }
    return(t);
}

data pushargs(data d)
{
    if (cnilp(d))
        return(d);
    if (!cconsp(car(d)))
        error(L"pushargs arguments is must be list.\n");
    if ((!csymp(caar(d))) || (!cconsp(cdar(d))))
        error(L"invalid key at pushargs.\n");
    cpushsym(csym(caar(d)), car(cdar(d)));
    return(pushargs(cdr(d)));
}

data popargs(data d)
{
    if (cnilp(d))
        return(d);
    if (!cconsp(car(d)))
        error(L"popargs arguments is must be list.\n");
    if ((!csymp(caar(d))))
        error(L"invalid key at popargs.\n");
    cpopsym(csym(caar(d)));
    return(popargs(cdr(d)));
}
