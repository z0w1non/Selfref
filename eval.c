#include "eval.h"

#include <stdlib.h>
#include "sym.h"
#include "builtin.h"
#include "print.h"
#include "heap.h"

/********/
/* Eval */
/********/
data eval(data d)
{
    data value;
    if (is_pair(d))
    {
        if (has_prefix_operator(d))
            d = resolve_prefix_operator(d);
    }

    if (has_binary_operator(d))
        d = sort_binary_operator(d);
    if (is_symbol(d))
    {
        value = find_symbol(raw_string(d));
        if (!value)
            error(L"<symbol %s> is not found", raw_string(d));
        return(value);
    }
    else if (is_pair(d))
    {
        value = eval(car(d));
        if (is_builtin_macro(value))
            return(call_builtin_macro(make_pair(value, cdr(d))));
        else if (is_builtin_function(value))
            return(call_builtin_function(make_pair(value, cdr(d))));
        else if (is_unnamed_macro(value))
            return(call_unnamed_macro(make_pair(value, cdr(d))));
        else if (is_unnamed_function(value))
            return(call_unnamed_function(make_pair(value, cdr(d))));
        if (has_binary_operator(d))
            return(eval(d));
        error(L"The first token in the list is must be callable or a symbol that bound to a callable");
    }
   return(d);
}

data _eval_list(data d)
{
    if (is_nil(d))
        return(nil);
    if (!is_pair(d))
        error(L"eval list failed");
    return(make_pair(eval(car(d)), _eval_list(cdr(d))));
}

// (call_macro arg1 arg2 arg3 ...)
data call_builtin_macro(data d)
{
    if (!is_builtin_macro(car(d)))
        error(L"invalid macro call");
    return(raw_macro(car(d))(cdr(d)));
}

// (call_function arg1 arg2 arg3 ...)
data call_builtin_function(data d)
{
    if (!is_builtin_function(car(d)))
        error(L"invalid function call");
    return(raw_function(car(d))(_eval_list(cdr(d))));
}

// (call_unnamed_macro arg1 arg2 arg3 ...)
data call_unnamed_macro(data d)
{
    data args, ret;
    if (!is_unnamed_macro(car(d)))
        error(L"invalid unnnamed macro call");
    args = nil;
    if (is_not_nil(cadr(d)))
        _push_args(args = _zip(make_pair(get_args(car(d)), make_pair(cdr(d), nil))));
    ret = eval(get_impl(car(d)));
    if (is_not_nil(cadr(d)))
        _pop_args(args);
    return(ret);
}

// (call_unnamed_function arg1 arg2 arg3 ...)
data call_unnamed_function(data d)
{
    data args, ret;
    if(!is_unnamed_function(car(d)))
        error(L"invalid function call");
    args = nil;
    if (is_not_nil(cadr(d)))
        _push_args(args = _zip(make_pair(get_args(car(d)), make_pair(_eval_list(cdr(d)), nil))));
    ret = eval(get_impl(car(d)));
    if (is_not_nil(cadr(d)))
        _pop_args(args);
    return(ret);
}

/*****************/
/* Operator list */
/*****************/
data binary_operator_list;

void init_binary_operator_list()
{
    binary_operator_list = nil;
}

void mark_binary_operator_list()
{
    forward_list_mark(binary_operator_list);
}

data add_builtin_left_associative_operator_function(const wchar_t * name, function_t f)
{
    data d = make_left_associative_operator(name, make_builtin_function(f));
    add_binary_operator(d);
    return(d);
}

data add_builtin_right_associative_operator_function(const wchar_t * name, function_t f)
{
    data d = make_right_associative_operator(name, make_builtin_function(f));
    add_binary_operator(d);
    return(d);
}

data add_builtin_left_associative_operator_macro(const wchar_t * name, function_t f)
{
    data d = make_left_associative_operator(name, make_builtin_macro(f));
    add_binary_operator(d);
    return(d);
}

data add_builtin_right_associative_operator_macro(const wchar_t * name, function_t f)
{
    data d = make_right_associative_operator(name, make_builtin_macro(f));
    add_binary_operator(d);
    return(d);
}

void add_binary_operator(data d)
{
    if (!is_left_associative_operator(d) && !is_right_associative_operator(d))
        error(L"add operator failed");
    binary_operator_list = make_pair(d, binary_operator_list);
}

void remove_binary_operator(const wchar_t * name)
{
    data list;
    list = binary_operator_list;
    while (is_not_nil(list))
    {
        if (wcscmp(raw_string(car(list)), name) == 0)
        {
            //TODO
        }
        list = cdr(list);
    }
    error(L"<operator %s> not found", name);
}

data find_binary_operator(const wchar_t * name)
{
    data d;
    d = binary_operator_list;
    while (is_not_nil(d))
    {
        if (wcscmp(raw_string(car(d)), name) == 0)
            return(car(d));
        d = cdr(d);
    }
    return(NULL);
}

int compare_binary_operator_priority(const wchar_t * a, const wchar_t * b)
{
    if (wcscmp(a, b) == 0)
        return(0);
    data list;
    list = binary_operator_list;
    while (is_not_nil(list))
    {
        if (wcscmp(raw_string(car(list)), a) == 0)
            return(1);
        if (wcscmp(raw_string(car(list)), b) == 0)
            return(-1);
        list = cdr(list);
    }
    error(L"<operator %s> and <operator %s> not found", a, b);
    return(0);
}

int has_binary_operator(data d)
{
    while (is_not_nil(car(d)))
    {
        if (is_symbol(car(d)) && find_binary_operator(raw_string(car(d))))
            return(1);
        d = cdr(d);
    }
    return(0);
}

data sort_binary_operator(data d)
{
    stack operator_stack, stack_machine;
    queue output_queue;
    data list, op1, op2, temp, front_token, left, right, sorted;
    const wchar_t * op1_name, * op2_name;

    operator_stack = NULL;
    stack_machine = NULL;
    output_queue = NULL;

    if (!(operator_stack = stack_create(sizeof(data))))
        goto error;

    if (!(stack_machine = stack_create(sizeof(data))))
        goto error;

    if (!(output_queue = queue_create(sizeof(data))))
        goto error;
    
    list = d;
    while (is_not_nil(list))
    {
        if (is_symbol(car(list)))
        {
            op1_name = raw_string(car(list));
            op1 = find_binary_operator(op1_name);
            if (op1)
            {
                if (!stack_is_empty(operator_stack))
                {
                    stack_front(operator_stack, &op2);
                    op2_name = raw_string(op2);

                    while (((is_left_associative_operator(op2) && (compare_binary_operator_priority(op1_name, op2_name) <= 0)))
                        || (compare_binary_operator_priority(op1_name, op2_name) < 0))
                    {
                        stack_pop(operator_stack, &op2);
                        if (!queue_enqueue(output_queue, &op2))
                            goto error;

                        if (stack_is_empty(operator_stack))
                            break;
                        stack_front(operator_stack, &op2);
                        op2_name = raw_string(op2);
                    }
                }
                if (!stack_push(operator_stack, &op1))
                    goto error;
            }
            else
            {
                temp = car(list);
                if (!queue_enqueue(output_queue, &temp))
                    goto error;
            }
        }
        else
        {
            temp = car(list);
            if (!queue_enqueue(output_queue, &temp))
                goto error;
        }
        list = cdr(list);
    }

    while (!stack_is_empty(operator_stack))
    {
        stack_pop(operator_stack, &op1);
        if (!queue_enqueue(output_queue, &op1))
            goto error;
    }
    sorted = nil;
    while (!queue_is_empty(output_queue))
    {
#ifndef NDEBUG
        if (find_symbol(L"debug_sort_operator"))
        {
            queue_print_as_data(output_queue);
            stack_print_as_data(stack_machine);
        }
#endif
        queue_dequeue(output_queue, &front_token);
        if (is_left_associative_operator(front_token) || is_right_associative_operator(front_token))
        {
            if (stack_is_empty(stack_machine))
                goto error;
            stack_pop(stack_machine, &right);
            if (stack_is_empty(stack_machine))
                goto error;
            stack_pop(stack_machine, &left);

            op1_name = raw_string(front_token);
            op1 = find_binary_operator(op1_name);
            if (!op1)
                goto error;

            temp = make_pair(get_operator_impl(op1), make_pair(left, make_pair(right, nil)));
            if (queue_is_empty(output_queue))
            {
                stack_cleanup(operator_stack);
                stack_cleanup(stack_machine);
                queue_cleanup(output_queue);
#ifndef NDEBUG
                if (find_symbol(L"debug_sort_operator"))
                {
                    print(temp);
                    wprintf(L"\n");
                }
#endif
                return(temp);
            }

            if (!stack_push(stack_machine, &temp))
                goto error;
        }
        else
        {
            if (!stack_push(stack_machine, &front_token))
                goto error;
        }
    }

    while (!stack_is_empty(stack_machine))
    {
        stack_pop(stack_machine, &temp);
        sorted = make_pair(temp, sorted);
    }
#ifndef NDEBUG
    if (find_symbol(L"debug_sort_operator"))
    {
        print(sorted);
        wprintf(L"\n");
    }
#endif
    return(sorted);

error:
    stack_cleanup(operator_stack);
    stack_cleanup(stack_machine);
    queue_cleanup(output_queue);
    error(L"bac alloc");
    return(nil);
}

/************************/
/* Prefix operator list */
/************************/
data prefix_operator_list;

void init_prefix_operator_list()
{
    prefix_operator_list = forward_list_create();
}

void mark_prefix_operator_list()
{
    forward_list_mark(prefix_operator_list);
}

data add_builtin_prefix_operator_macro(const wchar_t * name, function_t f)
{
    data d;
    d = make_prefix_operator(name, make_builtin_macro(f));
    return(add_prefix_operator(d));
}

data add_builtin_prefix_operator_function(const wchar_t * name, function_t f)
{
    data d;
    d = make_prefix_operator(name, make_builtin_function(f));
    return(add_prefix_operator(d));
}

data add_prefix_operator(data d)
{
    forward_list_push_front(&prefix_operator_list, d);
    return(d);
}

/* Private callback function */
const wchar_t * prefix_operator_string;
int prefix_operator_internal_predicate(data d)
{
    return(wcscmp(prefix_operator_string, raw_string(d)) == 0);
}

data remove_prefix_operator(const wchar_t * name)
{
    data d;
    prefix_operator_string = name;
    d = forward_list_remove(&prefix_operator_list, prefix_operator_internal_predicate);
    prefix_operator_string = NULL;
    if(!d)
        error(L"<prefix operator %s> not found", name);
    return(d);
}

data find_prefix_operator(const wchar_t * name)
{
    data d;
    prefix_operator_string = name;
    d = forward_list_find(prefix_operator_list, prefix_operator_internal_predicate);
    prefix_operator_string = NULL;
    return(d);
}

int has_prefix_operator(data d)
{
    data temp;
    while (is_not_nil(d))
    {
        if (is_symbol(car(d)))
        {
            temp = find_prefix_operator(raw_string(car(d)));
            if (temp)
                return(1);
        }
        d = cdr(d);
    }
    return(0);
}

// (resolve_prefix_operator (a 'b c ...))
data resolve_prefix_operator(data d)
{
    data prefix_operator;
    if (is_nil(d))
        return(nil);
    if (is_symbol(car(d)))
    {
        prefix_operator = find_prefix_operator(raw_string(car(d)));
        if (prefix_operator)
        {
            if (is_nil(cdr(d)))
                error(L"token expected at after prefix operator");
            return(make_pair(
                    make_pair(get_operator_impl(prefix_operator), make_pair(resolve_prefix_operator(cadr(d)), nil)),
                    resolve_prefix_operator(cddr(d))));
        }
    }
    return(make_pair(car(d), resolve_prefix_operator(cdr(d))));
}

// ">"   : (">>" ">>>" "=>")
// ">>"  : (">>>")
// ">>>" : nil
data get_operators_start_with(const wchar_t * name)
{
    data d, list;
    size_t specified_operator_length;
    specified_operator_length = wcslen(name);
    d = prefix_operator_list;
    list = nil;
    if (is_not_nil(d))
    {
        if ((wcslen(raw_string(car(d))) > specified_operator_length)
            && (wcsncmp(raw_string(car(d)), name, specified_operator_length) == 0))
            list = make_pair(make_string(clone_string(raw_string(car(d)))), list);
        d = cdr(d);
    }
    return(list);
}
