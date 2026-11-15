#include "token_array.h"
#include "error.h"
#include <stdio.h>

void token_array_init(TokenArray *arr)
{
    arr->capacity = 64;
    arr->count = 0;

    arr->items = malloc(arr->capacity * sizeof(Token));

    if (arr->items == NULL)
    {
        printError(ERR_INTERNAL);
    }
}

void token_array_append(TokenArray *arr, Token new_token)
{
    if (arr->count >= arr->capacity)
    {
        size_t new_capacity = arr->capacity * 2;

        Token *temp = realloc(arr->items, new_capacity * sizeof(Token));

        if (temp == NULL)
        {
            token_array_free(arr);
            printError(ERR_INTERNAL);
        }

        arr->items = temp;
        arr->capacity = new_capacity;
    }

    arr->items[arr->count] = new_token;
    arr->count++;
}

void token_array_free(TokenArray *arr)
{
    if (arr->items != NULL)
    {
        free(arr->items);
        arr->items = NULL;
    }
    arr->count = 0;
    arr->capacity = 0;
}