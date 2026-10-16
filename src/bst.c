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

// alokace nové node
bst_node_t *bst_new_node(void *key, void *data)
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

// získání výšky
int get_height(bst_node_t *node)
{
    if (node == NULL)
        return -1;

    return node->height;
}

// vypočtení balance pro výšky node
int get_balance(bst_node_t *node)
{
    if (node == NULL)
        return 0;

    return get_height(node->left) - get_height(node->right);
}

// zjištění výšky
int height(bst_node_t *a, bst_node_t *b)
{
    int a_h = get_height(a);
    int b_h = get_height(b);

    if (a_h > b_h)
        return 1 + a_h;
    else
        return 1 + b_h;
}

// pravá rotace
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

// levá rotace
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

// rekurze pro vkládání node
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
        node->right = insert_node(tree, node->right, key, data, state);
    else if (cmp > 0)
        node->left = insert_node(tree, node->left, key, data, state);
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
        return rotate_left(node);

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

// nalezení nejmenšího uzlu v podstromu
bst_node_t *find_min(bst_node_t *node)
{
    bst_node_t *actual = node;
    while (actual->left != NULL)
        actual = actual->left;
    return actual;
}

// rekurze pro mazání node
bst_node_t *delete_node(bst_t *tree, bst_node_t *node, void *key, bool *deleted)
{
    if (node == NULL)
    {
        *deleted = false;
        return NULL;
    }

    int cmp = tree->cmp(node->key, key);
    if (cmp < 0)
        node->right = delete_node(tree, node->right, key, deleted);
    else if (cmp > 0)
        node->left = delete_node(tree, node->left, key, deleted);
    else
    {
        // 0/1 potomek
        if (node->left == NULL || node->right == NULL)
        {
            bst_node_t *delete = NULL;

            if (node->left != NULL)
            {
                delete = node;
                node = node->left;
            }
            else if (node->right != NULL)
            {
                delete = node;
                node = node->right;
            }
            else
            {
                delete = node;
                node = NULL;
            }

            if (tree->key_free != NULL && delete->key != NULL)
                tree->key_free(delete->key);
            if (tree->data_free != NULL && delete->data != NULL)
                tree->data_free(delete->data);

            free(delete);
            *deleted = true;
        }
        else
        {
            // 2 potomci
            bst_node_t *node_min = find_min(node->right);

            void *old_key = node->key;
            void *old_data = node->data;

            node->key = node_min->key;
            node->data = node_min->data;

            node_min->key = old_key;
            node_min->data = old_data;

            node->right = delete_node(tree, node->right, old_key, deleted);
            *deleted = true;
        }
    }

    if (node == NULL)
        return NULL;

    node->height = height(node->left, node->right);

    int balance = get_balance(node);

    // LL
    if (balance > 1 && get_balance(node->left) >= 0)
        return rotate_right(node);

    // LR
    if (balance > 1 && get_balance(node->left) < 0)
    {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    // RR
    if (balance < -1 && get_balance(node->right) <= 0)
        return rotate_left(node);

    // RL
    if (balance < -1 && get_balance(node->right) > 0)
    {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}

// rekurze pro mazání celého stromu
bool bst_delete(bst_t *tree, void *key)
{
    if (tree == NULL || tree->root == NULL || key == NULL)
        return false;

    bool deleted = false;
    tree->root = delete_node(tree, tree->root, key, &deleted);
    return deleted;
}

void dispose_node(bst_node_t *node, bst_key_free_func key_free, bst_data_free_func data_free)
{
    if (node == NULL)
        return;

    dispose_node(node->left, key_free, data_free);
    dispose_node(node->right, key_free, data_free);

    if (key_free != NULL && node->key != NULL)
        key_free(node->key);
    if (data_free != NULL && node->data != NULL)
        data_free(node->data);

    free(node);
}

void bst_dispose(bst_t *tree)
{
    if (tree == NULL)
        return;

    dispose_node(tree->root, tree->key_free, tree->data_free);
    tree->root = NULL;
}