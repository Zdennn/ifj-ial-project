/**
 * @file bst.c
 * @brief Implementace AVL stromu.
 *
 *
 *
 * @author Jonáš Javůrek
 */
#include "bst.h"
#include "stdbool.h"
#include <stdlib.h>

void bst_init(bst_t *tree, bst_cmp_func cmp, bst_key_free_func key_free, bst_data_free_func data_free)
{
    tree->root = NULL;
    tree->cmp = cmp;
    tree->key_free = key_free;
    tree->data_free = data_free;
}

bst_status_t bst_insert(bst_t *tree, void *key, void *data)
{
    (void)tree;
    (void)key;
    (void)data;

    return 0;
}

bst_node_t *bst_search(bst_t *tree, void *key)
{
    bst_node_t *actual = tree->root;

    while (actual->key == NULL)
    {
        if (tree->cmp(actual->key, key) < 0)
            actual = actual->left;
        else if (tree->cmp(actual->key, key) > 0)
            actual = actual->right;
        else
            return actual;
    }

    return NULL;
}

bool bst_delete(bst_t *tree, void *key)
{
    (void)tree;
    (void)key;

    return false;
}

void bst_dispose(bst_t *tree)
{
    (void)tree;
}