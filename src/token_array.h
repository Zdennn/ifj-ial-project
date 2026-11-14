#ifndef TOKEN_ARRAY_H
#define TOKEN_ARRAY_H

#include "token.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct
{
    Token *items;
    int count;
    int capacity;
} TokenArray;

void token_array_init(TokenArray *arr);
void token_array_append(TokenArray *arr, Token new_token);
void token_array_free(TokenArray *arr);

#endif