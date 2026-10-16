/* Test selhání alokace: malloc je přesměrován přes linker. */
/**
 * @file test_oom.c
 * @brief Testy obecného AVL stromu (bst.c / bst.h).
 *
 * Tento soubor vygenerovala AI (Claude)
 * "Napiš testy pro obecný AVL strom v C: kontrola invariantů, rotace, mazání, duplicity, uvolňování paměti."
 *
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/bst.h"

static int fail_after = -1; // kolikátý malloc má selhat (-1 = nikdy)

void *__real_malloc(size_t n);
void *__wrap_malloc(size_t n)
{
    if (fail_after == 0)
        return NULL;
    if (fail_after > 0)
        fail_after--;
    return __real_malloc(n);
}

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static size_t count(const bst_node_t *n)
{
    return n ? 1 + count(n->left) + count(n->right) : 0;
}

int main(void)
{
    static int keys[100];
    bst_t t;
    bst_init(&t, cmp_int, NULL, NULL);

    for (int i = 0; i < 50; i++)
    {
        keys[i] = i;
        assert(bst_insert(&t, &keys[i], NULL) == BST_OK);
    }
    bst_node_t *root_before = t.root;
    size_t n_before = count(t.root);

    fail_after = 0; // další malloc selže
    keys[99] = 999;
    assert(bst_insert(&t, &keys[99], NULL) == BST_NO_MEMORY);
    fail_after = -1;

    assert(t.root == root_before); // strom se nezměnil
    assert(count(t.root) == n_before);
    assert(bst_search(&t, &keys[10]) != NULL);
    assert(bst_search(&t, &keys[99]) == NULL);

    // po chybě musí strom dál fungovat
    assert(bst_insert(&t, &keys[99], NULL) == BST_OK);
    assert(count(t.root) == n_before + 1);

    bst_dispose(&t);
    puts("ok: test_oom");
    return 0;
}
