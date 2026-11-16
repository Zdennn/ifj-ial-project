/**
 * @file precendece_stack.c
 *
 * @brief Implementace operací precedenčního zásobníku.
 *
 * @author Zdeněk Michalec
 */

#include "precedence_stack.h"
#include "error.h"
#include "precedence_table.h"
#include "symtable.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

void stack_init(PrecStack *stack)
{
    stack->capacity = 16;
    stack->top = 0;

    stack->items = malloc(stack->capacity * sizeof(StackItem));

    if (stack->items == NULL)
    {
        printError(ERR_INTERNAL);
    }
}

void stack_push(PrecStack *stack, StackItem new_item)
{
    if (stack->top >= stack->capacity)
    {
        size_t new_capacity = stack->capacity * 2;

        StackItem *temp = realloc(stack->items, new_capacity * sizeof(StackItem));

        if (temp == NULL)
        {
            stack_free(stack);
            printError(ERR_INTERNAL);
        }

        stack->items = temp;
        stack->capacity = new_capacity;
    }

    stack->items[stack->top] = new_item;
    stack->top++;
}

StackItem stack_pop(PrecStack *stack)
{
    StackItem retun_item = {0};
    if (stack->top > 0)
    {
        stack->top--;
        retun_item = stack->items[stack->top];
        StackItem empty = {0};
        stack->items[stack->top] = empty;
    }
    return retun_item;
}

StackItem *stack_top(PrecStack *stack)
{
    if (stack->top == 0)
    {
        return NULL;
    }

    return &stack->items[stack->top - 1];
}

StackItem *stack_top_terminal(PrecStack *stack)
{
    for (int i = stack->top - 1; i >= 0; i--)
    {
        if (stack->items[i].sem_type == SEM_TERMINAL || stack->items[i].sem_type == SEM_STOP)
        {
            return &stack->items[i];
        }
    }

    return NULL;
}

void stack_vlozeni_zarazky_za_nevyssi_terminal(PrecStack *stack)
{
    // TODO
}

void stack_free(PrecStack *stack)
{
    if (stack->items != NULL)
    {
        free(stack->items);
        stack->items = NULL;
    }
    stack->top = 0;
    stack->capacity = 0;
}