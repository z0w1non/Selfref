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

    if (is_symbol(d))
    {
        value = find_symbol(raw_string(d));
        if (!value)
            error(L"<symbol %s> is not found.\n", raw_string(d));
        return(value);
    }
    else if (is_pair(d))
    {
        value = eval(car(d));
        if (is_builtin_function(value))
            return(_call(make_pair(value, _eval_list(cdr(d)))));
        else if (is_builtin_macro(value))
            return(call_macro(make_pair(value, cdr(d))));
        else if (is_unnamed_macro(value))
            return(call_unnamed_macro(make_pair(value, cdr(d))));
        else if (is_unnamed_function(value))
            return(call_unnamed_function(make_pair(value, cdr(d))));
        return(value); //???
    }
   return(d);
}

data _eval_list(data d)
{
    if (is_nil(d))
        return(nil);
    if (!is_pair(d))
        error(L"eval list failed.\n");
    return(make_pair(eval(car(d)), _eval_list(cdr(d))));
}

// (call_macro actualargs)
data call_macro(data d)
{
    if (!is_builtin_macro(car(d)))
        error(L"invalid macro call.\n");
    return(raw_macro(car(d))(cdr(d)));
}

// (call_macro actualargs)
data call_unnamed_macro(data d)
{
    data args, ret;
    if (!is_unnamed_macro(car(d)))
        error(L"invalid unnnamed macro call.\n");
    args = _zip(make_pair(get_args(car(d)), make_pair(cdr(d), nil)));
    if (is_not_nil(car(args)))
        _push_args(args);
    ret = eval(get_impl(car(d)));
    if (is_not_nil(car(args)))
        _pop_args(args);
    return(ret);
}

// (call_function actualargs)
data call_unnamed_function(data d)
{
    data args, ret;
    if(!is_unnamed_function(car(d)))
        error(L"invalid function call.\n");
    args = _zip(make_pair(get_args(car(d)), make_pair(_eval_list(cdr(d)), nil)));
    if (is_not_nil(car(args)))
        _push_args(args);
    ret = eval(get_impl(car(d)));
    if (is_not_nil(car(args)))
        _pop_args(args);
    return(ret);
}

/*****************/
/* Operator list */
/*****************/
data operator_list;

void init_operator_list()
{
    operator_list = nil;

    //add_operator(make_suffix_operator(L";", _listize));

    add_operator(make_left_associative_operator(L"=>", _unnamed_function));

    add_operator(make_left_associative_operator(L"=", _push_symbol));

    //add_operator(make_binary_operator(L"<", _less));
    //add_operator(make_binary_operator(L"<=", _less_equal));
    //add_operator(make_binary_operator(L">", _greater));
    //add_operator(make_binary_operator(L">=", _greater_equal));

    add_operator(make_left_associative_operator(L"+", _add));
    add_operator(make_left_associative_operator(L"-", _sub));

    add_operator(make_left_associative_operator(L"*", _mul));
    add_operator(make_left_associative_operator(L"/", _div));
    add_operator(make_left_associative_operator(L"%", _mod));
}

void add_operator(data d)
{
    operator_list = make_pair(d, operator_list);
}

void remove_operator(const wchar_t * name)
{
    data list, temp;
    list = operator_list;
    while (is_not_nil(list))
    {
        if (wcscmp(raw_string(car(list)), name) == 0)
        {
            //TODO
        }
        list = cdr(list);
    }
    error(L"<operator %s> not found.\n", name);
}

data find_operator(const wchar_t * name)
{
    data list;
    list = operator_list;
    while (is_not_nil(list))
    {
        if (wcscmp(raw_string(car(list)), name) == 0)
            return(car(list));
        list = cdr(list);
    }
    error(L"<operator %s> not found.\n", name);
}

int compare_operator_priority(const wchar_t * a, const wchar_t * b)
{
    if (wcscmp(a, b) == 0)
        return(0);
    data list;
    list = operator_list;
    while (is_not_nil(list))
    {
        if (wcscmp(raw_string(car(list)), a) == 0)
            return(1);
        if (wcscmp(raw_string(car(list)), b) == 0)
            return(-1);
        list = cdr(list);
    }
    error(L"<operator %s> and <operator %s> not found.\n", a, b);
}

data sort_operator(data d)
{
    stack operator_stack, stack_machine;
    queue output_queue;
    data front_operator, front_token, left, right, sorted;

    operator_stack = NULL;
    stack_machine = NULL;
    output_queue = NULL;

    if (!(operator_stack = stack_create(sizeof(data))))
        goto error;

    if (!(stack_machine = stack_create(sizeof(data))))
        goto error;

    if (!(output_queue = queue_create(sizeof(data))))
        goto error;

    while (is_not_nil(d))
    {
        if (is_operator(car(d)))
        {
            stack_front(operator_stack, &front_operator);
            while (!stack_is_empty(operator_stack)
                && (((is_left_associative_operator(front_operator) && (compare_operator_priority(car(d), front_operator) <= 0)))
                || (compare_operator_priority(car(d), front_operator) < 0)))
            {
                stack_pop(operator_stack, &front_operator);
                if (!queue_enqueue(output_queue, &front_operator))
                    goto error;
            }
            if (!queue_enqueue(output_queue, car(d)))
                return error;
        }
        else
        {
            if (!queue_enqueue(output_queue, car(d)))
                goto error;
        }
        d = cdr(d);
    }

    while (!stack_is_empty(operator_stack))
    {
        stack_pop(operator_stack, &front_operator);
        if (!queue_enqueue(output_queue, &front_operator))
            goto error;
    }

    while (!queue_is_empty(output_queue))
    {
        queue_dequeue(output_queue, &front_token);
        if (is_operator(front_token))
        {
            if (stack_is_empty(stack_machine))
                goto error;
            stack_pop(stack_machine, &left);
            if (stack_is_empty(stack_machine))
                goto error;
            stack_pop(stack_machine, &right);

            sorted = make_pair(front_token, make_pair(left, make_pair(right, nil)));
            if (queue_is_empty(output_queue))
                return(sorted);

            if (!stack_push(stack_machine, sorted))
                goto error;
        }
    }

error:
    stack_cleanup(operator_stack);
    stack_cleanup(stack_machine);
    queue_cleanup(output_queue);
    error(L"bac alloc.\n");
    return(nil);
}