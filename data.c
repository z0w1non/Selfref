#include "data.h"

#include <stdlib.h>
#include "heap.h"
#include "print.h"
#include "hash_table.h"

enum
{
    id_pair,
    id_builtin_macro,
    id_builtin_function,
    id_unnamed_macro,
    id_unnamed_function,
    id_symbol,
    id_nil,
    id_t,
    id_int,
    id_double,
    id_string,
    id_object,

    mask_used       = 0x0200,
    mask_marked     = 0x0400,
};

/****************************/
/* Data infomation accesser */
/****************************/
void init_data(data d)
{
    memset(d, 0, sizeof(dataimpl));
}

int used(data d)
{
    return((d->info & mask_used) != 0);
}

void set_used(data d, int used)
{
    if (used)
        d->info |= mask_used;
    else
        d->info &= ~((int)mask_used);
}

int marked(data d)
{
    return((d->info & mask_marked) != 0);
}

void set_marked(data d, int marked)
{
    if (marked)
        d->info |= mask_marked;
    else
        d->info &= ~((int)mask_marked);
}

int type_id(data d)
{
    return(d->info & 15);
}

void mark_data(data d)
{
    if (marked(d))
        return;
    set_marked(d, 1);
    if (is_pair(d))
    {
        if (is_not_nil(car(d)))
            mark_data(car(d));
        if (is_not_nil(cdr(d)))
            mark_data(cdr(d));
    }
    else if (is_unnamed_function(d) || is_unnamed_macro(d))
    {
        mark_data(get_args(d));
        mark_data(get_impl(d));
    }
}

/*****************/
/* Make function */
/*****************/
data make_pair(data first, data rest)
{
    data d = alloc();
    d->info |= id_pair;
    d->buffer.first = first;
    d->buffer.rest = rest;
    return(d);
}

data make_builtin_macro(function_t f, const wchar_t * name)
{
    data d = alloc();
    d->info |= id_builtin_macro;
    d->buffer._string = clone_string(name);
    d->buffer.impl = f;
    return(d);
}

data make_builtin_function(function_t f, const wchar_t * name)
{
    data d = alloc();
    d->info |= id_builtin_function;
    d->buffer._string = clone_string(name);
    d->buffer.impl = f;
    return(d);
}

data make_macro(data args, data impl)
{
    data d = alloc();
    d->info |= id_unnamed_macro;
    d->buffer.first = args;
    d->buffer.rest = impl;
    return(d);
}

data make_function(data args, data impl)
{
    data d = alloc();
    d->info |= id_unnamed_function;
    d->buffer.first = args;
    d->buffer.rest = impl;
    return(d);
}

data make_symbol(const wchar_t * name)
{
    data d;
    d = alloc();
    d->info |= id_symbol;
    d->buffer._string = clone_string(name);
    return(d);
}

data make_nil()
{
    data d = alloc();
    d->info |= id_nil;
    return(d);
}

data make_t
()
{
    data d = alloc();
    d->info |= id_t;
    return(d);
}

data _quote(data d)
{
    return(car(d));
}

data make_quote()
{
    return(make_builtin_macro(_quote, L"\'"));
}

data make_int(int value)
{
    data d = alloc();
    d->info |= id_int;
    d->buffer._int = value;
    return(d);
}

data make_double(double value)
{
    data d = alloc();
    d->info |= id_double;
    d->buffer._double = value;
    return(d);
}

data make_string(const char * name)
{
    data d;
    d = alloc();
    d->info |= id_string;
    d->buffer._string = clone_string(name);
    return(d);
}

data make_object()
{
    data d;
    d = alloc();
    d->info |= id_object;
    object_init(d);
    return(d);
}

/**********************/
/* Cons-cell accesser */
/**********************/
data car(data d)
{
    if (!is_pair(d))
        return(nil);
    return(d->buffer.first);
}

data cdr(data d)
{
    if (!is_pair(d))
        return(nil);
    return(d->buffer.rest);
}

data cadr(data d)
{
    return(car(cdr(d)));
}

data caddr(data d)
{
    return(car(cdr(cdr(d))));
}

data cadddr(data d)
{
    return(car(cdr(cdr(cdr(d)))));
}

data cddr(data d)
{
    return(cdr(cdr(d)));
}

data cdddr(data d)
{
    return(cdr(cdr(cdr(d))));
}

data cddddr(data d)
{
    return(cdr(cdr(cdr(cdr(d)))));
}

data caar(data d)
{
    return(car(car(d)));
}

data cdar(data d)
{
    return(cdr(car(d)));
}

/*********************/
/* Raw data accesser */
/*********************/
function_t raw_function(data d)
{
    return(d->buffer.impl);
}

function_t raw_macro(data d)
{
    return(d->buffer.impl);
}

data get_args(data d)
{
    if (!is_unnamed_function(d))
        error(L"getargs failed");
    return(d->buffer.first);
}

data get_impl(data d)
{
    if (!is_unnamed_function(d))
        error(L"getargs failed");
    return(d->buffer.rest);
}

int raw_int(data d)
{
    return(d->buffer._int);
}

double raw_double(data d)
{
    return(d->buffer._double);
}

const wchar_t * raw_string(data d)
{
    return(d->buffer._string);
}

void set_car(data d, data car)
{
    d->buffer.first = car;
}

void set_cdr(data d, data cdr)
{
    d->buffer.rest = cdr;
}

data get_operator_impl(data d)
{
    return(d->buffer.impl);
}

/**********************/
/* Predicate function */
/**********************/
data _is_pair(data d)
{
    return(nil_or_t(type_id(car(d)) == id_pair));
}

data _is_builtin_macro(data d)
{
    return(nil_or_t(type_id(car(d)) == id_builtin_macro));
}

data _is_builtin_function(data d)
{
    return(nil_or_t(type_id(car(d)) == id_builtin_function));
}

data _is_unnamed_macro(data d)
{
    return(nil_or_t(type_id(car(d)) == id_unnamed_macro));
}

data _is_unnamed_function(data d)
{
    return(nil_or_t(type_id(car(d)) == id_unnamed_function));
}

data _is_symbol(data d)
{
    return(nil_or_t(type_id(car(d)) == id_symbol));
}

data _is_nil(data d)
{
    return(nil_or_t(type_id(car(d)) == id_nil));
}

data _is_not_nil(data d)
{
    return(nil_or_t(type_id(car(d)) != id_nil));
}

data _is_int(data d)
{
    return(nil_or_t(type_id(car(d)) == id_int));
}

data _is_double(data d)
{
    return(nil_or_t(type_id(car(d)) == id_double));
}

data _is_number(data d)
{
    return(nil_or_t((type_id(car(d)) == id_int) || (type_id(car(d)) == id_double)));
}

data _is_string(data d)
{
    return(nil_or_t(type_id(car(d)) == id_string));
}

data _is_object(data d)
{
    return(nil_or_t(type_id(car(d)) == id_object));
}

data _is_zero(data d)
{
    if (is_int(car(d)))
        return(nil_or_t(raw_int(car(d)) == 0));
    else if (is_double(car(d)))
        return(nil_or_t(raw_double(car(d)) == 0.0));
    return(nil);
}

/****************************/
/* Predicate function for C */
/****************************/
int is_pair(data d)
{
    return(type_id(d) == id_pair);
}

int is_builtin_macro(data d)
{
    return(type_id(d) == id_builtin_macro);
}

int is_builtin_function(data d)
{
    return(type_id(d) == id_builtin_function);
}

int is_unnamed_macro(data d)
{
    return(type_id(d) == id_unnamed_macro);
}

int is_unnamed_function(data d)
{
    return(type_id(d) == id_unnamed_function);
}

int is_symbol(data d)
{
    return(type_id(d) == id_symbol);
}

int is_nil(data d)
{
    return(type_id(d) == id_nil);
}

int is_not_nil(data d)
{
    return(type_id(d) != id_nil);
}

int is_t(data d)
{
    return(type_id(d) == id_t);
}

int is_int(data d)
{
    return(type_id(d) == id_int);
}

int is_double(data d)
{
    return(type_id(d) == id_double);
}

int is_number(data d)
{
    return((type_id(d) == id_int) || (type_id(d) == id_double));
}

int is_string(data d)
{
    return(type_id(d) == id_string);
}

int is_object(data d)
{
    return(type_id(d) == id_object);
}

int is_zero(data d)
{
    if (is_int(d))
        return(raw_int(d) == 0);
    else if (is_double(d))
        return(raw_double(d) == 0.0);
    return(0);
}

/**************************************/
/* Bidirectional linked list function */
/**************************************/
void link_node(data a, data b)
{
    if (a == b)
    {
        error(L"linknode failed");
        return;
    }
    set_cdr(a, b);
    set_car(b, a);
}

data insert_node(data list, data node)
{
    if (list == node)
    {
        error(L"insertnode failed");
        return(nil);
    }
    data prev = car(list);
    link_node(prev, node);
    link_node(node, list);
    return(node);
}

data pull_node(data * list)
{
    data d1, d2, pop;
    if ((car(*list) == *list) || (cdr(*list) == *list))
    {
        error(L"pullnode failed");
        return(nil);
    }
    d1 = car(*list);
    d2 = cdr(*list);
    link_node(d1, d2);
    pop = *list;
    *list = d2;
    return(pop);
}

/***********/
/* Utility */
/***********/
data nil_or_t(int b)
{
    return(b ? t : nil);
}

wchar_t * clone_string(const wchar_t * s)
{
    int len;
    wchar_t * newstr;
    len = wcslen(s) + 1;
    newstr = (wchar_t *)malloc(sizeof(wchar_t) * len);
    if (!newstr)
    {
        error(L"Heap memory allocation failed");
        return(NULL);
    }
    wcscpy_s(newstr, len, s);
    return(newstr);
}

/*********/
/* Queue */
/*********/
typedef struct queue_tag
{
    void * data;
    size_t size;
    size_t head;
    size_t count;
    size_t size_of_type;
} * queue;

queue queue_create(size_t size_of_type)
{
    queue q;
    q = malloc(sizeof(struct queue_tag));
    if (!q)
        return(NULL);
    q->size = 32;
    q->data = malloc(size_of_type * q->size);
    q->head = 0;
    q->count = 0;
    q->size_of_type = size_of_type;
    return(q);
}

void queue_cleanup(queue q)
{
    if(q)
    {
        free(q->data);
        q->data = NULL;
        free(q);
    }
}

int queue_enqueue(queue q, const void * data)
{
    if (q->count >= q->size)
        if (!(q->data = realloc(q->data, q->size_of_type * (q->size *= 2))))
            return(0);
    memcpy(((char *)(q->data) + q->size_of_type * ((q->head + q->count) % q->size)), data, q->size_of_type);
    q->count += 1;
    return(1);
}

int queue_dequeue(queue q, void * data)
{
    if (q->count <= 0)
        return(0);
    memcpy(data, ((char *)(q->data) + q->size_of_type * q->head), q->size_of_type);
    q->count -= 1;
    q->head = (q->head + 1) % q->size;
    return(1);
}

int queue_front(queue q, void * data)
{
    if (q->count <= 0)
        return(0);
    memcpy(data, ((char *)(q->data) + q->size_of_type * q->head), q->size_of_type);
    return(1);
}

int queue_is_empty(queue q)
{
    return(q->count == 0);
}

int queue_print_as_data(queue q)
{
    size_t i;
    wprintf(L"(queue ");
    for (i = 0; i < q->count; ++i)
    {
        if (i > 0)
            wprintf(L" ");
        print(*((data *)(q->data) + ((q->head + i) % q->size)));
    }
    wprintf(L")\n");
    return(1);
}

/*********/
/* Stack */
/*********/
typedef struct stack_tag
{
    void * data;
    size_t size;
    size_t count;
    size_t size_of_type;
} * stack;

stack stack_create(size_t size_of_type)
{
    stack s;
    s = (stack)malloc(sizeof(struct stack_tag));
    if (!s)
        return(NULL);
    s->size = 32;
    s->data = malloc(size_of_type * s->size);
    s->count = 0;
    s->size_of_type = size_of_type;
    return(s);
}

void stack_cleanup(stack s)
{
    if (s)
    {
        free(s->data);
        free(s);
    }
}

int stack_push(stack s, const void * data)
{
    if (s->count >= s->size)
        if (!(s->data = realloc(s->data, s->size_of_type * (s->size *= 2))))
            return(0);
    memcpy(((char *)(s->data) + s->size_of_type * s->count), data, s->size_of_type);
    s->count += 1;
    return(1);
}

int stack_pop(stack s, void * data)
{
    if (s->count <= 0)
        return(0);
    s->count -= 1;
    memcpy(data, ((char *)(s->data) + s->size_of_type * s->count), s->size_of_type);
    return(1);
}

int stack_front(stack s, void * data)
{
    if (s->count <= 0)
        return(0);
    memcpy(data, ((char *)(s->data) + s->size_of_type * (s->count - 1)), s->size_of_type);
    return(1);
}

int stack_is_empty(stack s)
{
    return(s->count == 0);
}

int stack_print_as_data(stack s)
{
    size_t i;
    wprintf(L"(stack ");
    for (i = 0; i < s->count; ++i)
    {
        if (i > 0)
            wprintf(L" ");
        print(*((data *)(s->data) + i));
    }
    wprintf(L")\n");
    return(1);
}

/****************/
/* Forward list */
/****************/
data forward_list_create()
{
    return(nil);
}

data forward_list_push_front(data * list, data element)
{
    data front;
    front = make_pair(element, *list);
    *list = front;
    return(t);
}

data forward_list_pop_front(data * list)
{
    data front;
    front = car(*list);
    *list = cdr(*list);
    return(front);
}

data forward_list_find(data list, predicate_t pred)
{
    data d = list;
    while (is_not_nil(d))
    {
        if (pred(car(d)))
            return(car(d));
        d = cdr(d);
    }
    return(NULL);
}

data forward_list_remove(data * list, predicate_t pred)
{
    data d, temp;
    if (is_nil(*list))
        return(nil);
    if (pred(car(*list)))
    {
        temp = car(*list);
        *list = cdr(*list);
        return(temp);
    }
    d = *list;
    while (is_not_nil(cdr(d)))
    {
        if (pred(cadr(d)))
        {
            temp = cadr(d);
            set_cdr(d, cddr(d));
            return(temp);
        }
        d = cdr(d);
    }
    return(NULL);
}

data forward_list_mark(data list)
{
    while (is_not_nil(list))
    {
        mark_data(list);
        mark_data(car(list));
        list = cdr(list);
    }
    return(list);
}
