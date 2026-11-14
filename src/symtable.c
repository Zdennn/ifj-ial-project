/**
 * @file symbol_table.c
 * @brief Implementace tabulky symbolů.
 *
 * Ukládá informace o identifikátorech (proměnných) a jejich vlastnostech (typ, hodnota).
 * Varianta vv-BVS = Výškově vyvážený binární vyhledávací strom
 *
 * @author ...
 */
#include "symtable.h"
#include "bst.h"
#include "error.h"
#include "token.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Aktuální scope
Scope *current_scope = NULL;

int cmp_func(const void *a, const void *b)
{
    return strcmp((char *)a, (char *)b);
}
void key_free_func(void *key)
{
    if (key != NULL)
    {
        free(key);
    }
}
void data_free_func(void *data)
{
    if (data == NULL)
    {
        return;
    }

    SymbolData *symData = (SymbolData *)data;

    if (symData->symbol_type == SYM_FUNCTION && symData->param_types != NULL)
    {
        free(symData->param_types);
    }
    free(symData);
}

void symtable_init()
{
    current_scope = malloc(sizeof(Scope));
    current_scope->tree = malloc(sizeof(bst_t));
    current_scope->parent = NULL;

    bst_init(current_scope->tree, cmp_func, key_free_func, data_free_func);

    symtable_insert_function("print", TYPE_UNIT, 1, (DataType[]){TYPE_ANY}, false);
    symtable_insert_function("readLine", TYPE_STRING, 0, NULL, false);
    symtable_insert_function("readInt", TYPE_INT, 0, NULL, false);
    symtable_insert_function("readDouble", TYPE_DOUBLE, 0, NULL, false);
    symtable_insert_function("toStr", TYPE_STRING, 1, (DataType[]){TYPE_ANY}, false);
    symtable_insert_function("toInt", TYPE_INT, 1, (DataType[]){TYPE_DOUBLE}, false);
    symtable_insert_function("strLength", TYPE_INT, 1, (DataType[]){TYPE_STRING}, false);
    symtable_insert_function("subStr", TYPE_STRING, 3, (DataType[]){TYPE_STRING, TYPE_INT, TYPE_INT}, false);
    symtable_insert_function("ord", TYPE_INT, 2, (DataType[]){TYPE_STRING, TYPE_INT}, false);
    symtable_insert_function("chr", TYPE_STRING, 1, (DataType[]){TYPE_INT}, false);
}

void symtable_dispose()
{
    while (current_scope != NULL)
    {
        symtable_pop_scope();
    }
}

void symtable_push_scope()
{
    Scope *new_scope = malloc(sizeof(Scope));
    new_scope->tree = malloc(sizeof(bst_t));
    bst_init(new_scope->tree, cmp_func, key_free_func, data_free_func);
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

void symtable_pop_scope()
{
    Scope *to_delete = current_scope;
    current_scope = current_scope->parent;
    bst_dispose(to_delete->tree);
    free(to_delete);
}

SymbolData *symtable_lookup(char *name)
{
    Scope *search_scope = current_scope;
    while (search_scope != NULL)
    {
        bst_node_t *node = bst_search(search_scope->tree, name);
        if (node != NULL)
        {
            return (SymbolData *)node->data;
        }
        search_scope = search_scope->parent;
    }
    return NULL;
}

SymbolData *symtable_lookup_current(char *name)
{
    bst_node_t *node = bst_search(current_scope->tree, name);
    if (node != NULL)
    {
        return (SymbolData *)node->data;
    }
    return NULL;
}

void symtable_insert_var(char *name, DataType type, bool is_var)
{
    SymbolData *data = malloc(sizeof(SymbolData));
    data->name = strdup(name);
    data->symbol_type = SYM_VARIABLE;
    data->data_type = type;
    data->is_var = is_var;

    data->param_count = 0;
    data->param_types = NULL;

    bst_insert(current_scope->tree, data->name, data);
}

void symtable_insert_function(char *name, DataType return_type, int param_count, DataType *param_types, bool is_main)
{
    SymbolData *data = malloc(sizeof(SymbolData));
    data->name = strdup(name);
    data->symbol_type = SYM_FUNCTION;
    data->data_type = return_type;
    data->param_count = param_count;
    data->is_main = is_main;
    data->is_var = false;

    if (param_count > 0 && param_types != NULL)
    {
        data->param_types = malloc(param_count * sizeof(DataType));
        memcpy(data->param_types, param_types, param_count * sizeof(DataType));
    }
    else
    {
        data->param_types = NULL;
    }
    bst_insert(current_scope->tree, data->name, data);
}