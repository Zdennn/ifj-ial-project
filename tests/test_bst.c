/**
 * @file test_bst.c
 * @brief Testy obecného AVL stromu (bst.c / bst.h).
 *
 * Tento soubor vygenerovala AI (Claude)
 * "Napiš testy pro obecný AVL strom v C: kontrola invariantů, rotace, mazání, duplicity, uvolňování paměti."
 *
 */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/bst.h"

/* ---------- pomocné prostředky: klíče a data jsou alokovaná int ---------- */

static long live_keys = 0; // kolik klíčů je právě naalokováno
static long live_data = 0; // kolik datových bloků je právě naalokováno

static int *make_int(int v, long *counter)
{
    int *p = malloc(sizeof(int));
    assert(p != NULL);
    *p = v;
    (*counter)++;
    return p;
}

static void *make_key(int v)
{
    return make_int(v, &live_keys);
}

static void *make_data(int v)
{
    return make_int(v * 10, &live_data);
}

static void free_key(void *k)
{
    free(k);
    live_keys--;
}

static void free_data(void *d)
{
    free(d);
    live_data--;
}

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static void init_tree(bst_t *t)
{
    bst_init(t, cmp_int, free_key, free_data);
}

/* Vloží klíč k; při neúspěchu uvolní klíč i data (vlastnictví zůstává u volajícího). */
static bst_status_t insert_int(bst_t *t, int k)
{
    void *key = make_key(k);
    void *data = make_data(k);
    bst_status_t st = bst_insert(t, key, data);
    if (st != BST_OK)
    {
        free_key(key);
        free_data(data);
    }
    return st;
}

static bool contains(bst_t *t, int k)
{
    int probe = k;
    bst_node_t *n = bst_search(t, &probe);
    if (n != NULL)
    {
        assert(*(int *)n->key == k);
        assert(*(int *)n->data == k * 10);
    }
    return n != NULL;
}

static bool delete_int(bst_t *t, int k)
{
    int probe = k;
    return bst_delete(t, &probe);
}

static void shuffle(int *a, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

/* ---------- kontrola invariantů ---------- */

/* Nejvyšší možná výška AVL stromu (v hranách) s n uzly. */
static int max_avl_height(size_t n)
{
    size_t prev = 1, cur = 2; // minimální počty uzlů pro výšku 0 a 1
    int h = 1;
    while (prev + cur + 1 <= n)
    {
        size_t next = prev + cur + 1;
        prev = cur;
        cur = next;
        h++;
    }
    return h;
}

/* Vrací skutečnou výšku podstromu; ověřuje uspořádání, uloženou výšku a balance. */
static int check_node(const bst_node_t *n, const int *lo, const int *hi, size_t *count)
{
    if (n == NULL)
        return -1;

    int k = *(const int *)n->key;
    assert(lo == NULL || k > *lo);
    assert(hi == NULL || k < *hi);

    int hl = check_node(n->left, lo, &k, count);
    int hr = check_node(n->right, &k, hi, count);
    int h = 1 + (hl > hr ? hl : hr);

    assert(n->height == h);
    assert(hl - hr >= -1 && hl - hr <= 1);

    (*count)++;
    return h;
}

static void check_tree(const bst_t *t, size_t expected)
{
    size_t count = 0;
    check_node(t->root, NULL, NULL, &count);
    assert(count == expected);
    if (t->root != NULL)
        assert(t->root->height <= max_avl_height(count));
}

/* ---------- testy ---------- */

static void test_empty(void)
{
    bst_t t;
    init_tree(&t);
    assert(t.root == NULL);
    assert(!contains(&t, 1));
    assert(!delete_int(&t, 1));
    check_tree(&t, 0);
    bst_dispose(&t);
    assert(t.root == NULL);
}

static void test_null_arguments(void)
{
    assert(bst_insert(NULL, NULL, NULL) == BST_ERROR);
    assert(bst_search(NULL, NULL) == NULL);
    assert(!bst_delete(NULL, NULL));
    bst_dispose(NULL);
}

/* Čtyři případy rotace: po vložení tří klíčů musí být v kořeni prostřední. */
static void test_rotations(void)
{
    const int orders[4][3] = {
        {3, 2, 1}, // LL
        {1, 2, 3}, // RR
        {3, 1, 2}, // LR
        {1, 3, 2}, // RL
    };

    for (int o = 0; o < 4; o++)
    {
        bst_t t;
        init_tree(&t);
        for (int i = 0; i < 3; i++)
            assert(insert_int(&t, orders[o][i]) == BST_OK);
        check_tree(&t, 3);
        assert(*(int *)t.root->key == 2);
        assert(t.root->height == 1);
        bst_dispose(&t);
    }
    assert(live_keys == 0 && live_data == 0);
}

static void test_sequential(void)
{
    const int n = 1000;

    // vzestupně
    bst_t t;
    init_tree(&t);
    for (int i = 0; i < n; i++)
    {
        assert(insert_int(&t, i) == BST_OK);
        check_tree(&t, (size_t)i + 1);
    }
    for (int i = 0; i < n; i++)
        assert(contains(&t, i));
    assert(!contains(&t, -1));
    assert(!contains(&t, n));
    for (int i = 0; i < n; i++)
    {
        assert(delete_int(&t, i));
        check_tree(&t, (size_t)(n - i - 1));
    }
    assert(t.root == NULL);

    // sestupně
    for (int i = n - 1; i >= 0; i--)
    {
        assert(insert_int(&t, i) == BST_OK);
        check_tree(&t, (size_t)(n - i));
    }
    for (int i = n - 1; i >= 0; i--)
    {
        assert(delete_int(&t, i));
        check_tree(&t, (size_t)i);
    }
    assert(t.root == NULL);

    assert(live_keys == 0 && live_data == 0);
}

static void test_random(void)
{
    enum
    {
        N = 2000
    };
    static int order[N];

    srand(12345);
    for (int i = 0; i < N; i++)
        order[i] = i;
    shuffle(order, N);

    bst_t t;
    init_tree(&t);
    for (int i = 0; i < N; i++)
    {
        assert(insert_int(&t, order[i]) == BST_OK);
        check_tree(&t, (size_t)i + 1);
    }
    for (int i = 0; i < N; i++)
        assert(contains(&t, i));

    // smaž polovinu v jiném náhodném pořadí
    shuffle(order, N);
    for (int i = 0; i < N / 2; i++)
    {
        assert(delete_int(&t, order[i]));
        check_tree(&t, (size_t)(N - i - 1));
        assert(!contains(&t, order[i]));
    }
    for (int i = N / 2; i < N; i++)
        assert(contains(&t, order[i]));

    // zbytek nech zrušit najednou
    bst_dispose(&t);
    assert(t.root == NULL);
    assert(live_keys == 0 && live_data == 0);

    // a znovu: smaž úplně všechno po jednom
    shuffle(order, N);
    for (int i = 0; i < N; i++)
        assert(insert_int(&t, order[i]) == BST_OK);
    shuffle(order, N);
    for (int i = 0; i < N; i++)
    {
        assert(delete_int(&t, order[i]));
        check_tree(&t, (size_t)(N - i - 1));
    }
    assert(t.root == NULL);
    assert(live_keys == 0 && live_data == 0);
}

static void test_duplicates(void)
{
    bst_t t;
    init_tree(&t);

    assert(insert_int(&t, 5) == BST_OK);
    assert(insert_int(&t, 3) == BST_OK);
    assert(insert_int(&t, 8) == BST_OK);
    long keys_before = live_keys;

    assert(insert_int(&t, 5) == BST_DUPLICATE);
    assert(insert_int(&t, 3) == BST_DUPLICATE);
    assert(insert_int(&t, 8) == BST_DUPLICATE);

    check_tree(&t, 3);
    assert(live_keys == keys_before); // duplicitní klíče byly uvolněny volajícím
    assert(contains(&t, 5) && contains(&t, 3) && contains(&t, 8));

    bst_dispose(&t);
    assert(live_keys == 0 && live_data == 0);
}

/* Mazání listu, uzlu s jedním potomkem, uzlu se dvěma potomky, kořene a neexistujícího klíče. */
static void test_delete_cases(void)
{
    bst_t t;
    init_tree(&t);

    const int keys[] = {4, 2, 6, 1, 3, 5, 7};
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
        assert(insert_int(&t, keys[i]) == BST_OK);
    check_tree(&t, 7);

    assert(delete_int(&t, 1)); // list
    check_tree(&t, 6);
    assert(!contains(&t, 1));

    assert(delete_int(&t, 2)); // uzel s jedním potomkem
    check_tree(&t, 5);
    assert(!contains(&t, 2));

    assert(delete_int(&t, 4)); // kořen se dvěma potomky
    check_tree(&t, 4);
    assert(!contains(&t, 4));

    assert(!delete_int(&t, 99)); // neexistující klíč
    assert(!delete_int(&t, 4));  // už smazaný klíč
    check_tree(&t, 4);

    assert(contains(&t, 3) && contains(&t, 5) && contains(&t, 6) && contains(&t, 7));
    assert(live_keys == 4 && live_data == 4);

    bst_dispose(&t);
    assert(live_keys == 0 && live_data == 0);
}

/* Callbacky mohou být NULL: strom pak klíče ani data neuvolňuje. */
static void test_null_callbacks(void)
{
    static int keys[10];

    bst_t t;
    bst_init(&t, cmp_int, NULL, NULL);
    for (int i = 0; i < 10; i++)
    {
        keys[i] = i;
        assert(bst_insert(&t, &keys[i], NULL) == BST_OK);
    }
    check_tree(&t, 10);

    assert(bst_delete(&t, &keys[3]));
    assert(bst_delete(&t, &keys[0]));
    check_tree(&t, 8);

    bst_dispose(&t);
    assert(t.root == NULL);
}

int main(void)
{
    test_empty();
    puts("ok: test_empty");
    test_null_arguments();
    puts("ok: test_null_arguments");
    test_rotations();
    puts("ok: test_rotations");
    test_sequential();
    puts("ok: test_sequential");
    test_random();
    puts("ok: test_random");
    test_duplicates();
    puts("ok: test_duplicates");
    test_delete_cases();
    puts("ok: test_delete_cases");
    test_null_callbacks();
    puts("ok: test_null_callbacks");
    puts("Vsechny testy prosly.");
    return 0;
}
