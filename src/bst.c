/**
 * @file bst.c
 * @brief Implementace AVL stromu.
 *
 *
 *
 * @author Jonáš Javůrek
 */
#include "bst.h"
#include <stdbool.h>
#include <stdlib.h>

void bst_init(bst_t *tree, bst_cmp_func cmp, bst_key_free_func key_free, bst_data_free_func data_free)
{
    tree->root = NULL;
    tree->cmp = cmp;
    tree->key_free = key_free;
    tree->data_free = data_free;
}

static bst_node_t *bst_new_node(void *key, void *data)
{
    bst_node_t *new_node = (bst_node_t *)malloc(sizeof(bst_node_t));
    if (new_node == NULL)
        return NULL;

    new_node->key = key;
    new_node->data = data;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->height = 0;

    return new_node;
}

static int get_height(bst_node_t *node)
{
    if (node == NULL)
        return -1;

    return node->height;
}

static int get_balance(bst_node_t *node)
{
    if (node == NULL)
        return 0;

    return get_height(node->left) - get_height(node->right);
}

static int height(bst_node_t *a, bst_node_t *b)
{
    int a_h = get_height(a);
    int b_h = get_height(b);

    if (a_h > b_h)
        return 1 + a_h;
    else
        return 1 + b_h;
}

bst_node_t *rotate_right(bst_node_t *root)
{
    bst_node_t *x = root->left;
    bst_node_t *y = x->right;

    x->right = root;
    root->left = y;

    root->height = height(root->left, root->right);
    x->height = height(x->left, x->right);

    return x;
}

bst_node_t *rotate_left(bst_node_t *root)
{
    bst_node_t *x = root->right;
    bst_node_t *y = x->left;

    x->left = root;
    root->right = y;

    root->height = height(root->left, root->right);
    x->height = height(x->left, x->right);

    return x;
}

bst_node_t *insert_node(bst_t *tree, bst_node_t *node, void *key, void *data, bst_status_t *state)
{

    if (node == NULL)
    {
        bst_node_t *new_node = bst_new_node(key, data);
        if (new_node == NULL)
        {
            *state = BST_NO_MEMORY;
            return NULL;
        }
        *state = BST_OK;
        return new_node;
    }

    int cmp = tree->cmp(node->key, key);
    if (cmp < 0)
    {
        node->right = insert_node(tree, node->right, key, data, state);
    }
    else if (cmp > 0)
    {
        node->left = insert_node(tree, node->left, key, data, state);
    }
    else
    {
        *state = BST_DUPLICATE;
        return node;
    }

    if (*state == BST_NO_MEMORY)
    {
        return node;
    }

    node->height = height(node->left, node->right);

    int balance = get_balance(node);

    // LL
    if (balance > 1 && tree->cmp(node->left->key, key) > 0)
        return rotate_right(node);

    // RR
    if (balance < -1 && tree->cmp(node->right->key, key) < 0)
    {
        return rotate_left(node);
    }

    // LR
    if (balance > 1 && tree->cmp(node->left->key, key) < 0)
    {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    // RL
    if (balance < -1 && tree->cmp(node->right->key, key) > 0)
    {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}

bst_status_t bst_insert(bst_t *tree, void *key, void *data)
{
    if (tree == NULL)
        return BST_ERROR;

    bst_status_t state = BST_OK;
    tree->root = insert_node(tree, tree->root, key, data, &state);

    return state;
}

bst_node_t *bst_search(bst_t *tree, void *key)
{
    if (tree == NULL)
        return NULL;

    bst_node_t *actual = tree->root;

    while (actual != NULL)
    {
        int cmp = tree->cmp(actual->key, key);

        if (cmp < 0)
            actual = actual->right;
        else if (cmp > 0)
            actual = actual->left;
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