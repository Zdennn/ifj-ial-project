/**
 * @file test_stress.c
 * @brief Zátěžové a diferenciální testy AVL stromu (bst.c / bst.h).
 *
 * Soubor vygenerovala AI (Claude)
 * "Napiš testy pro obecný AVL strom v C: kontrola invariantů, rotace, mazání, duplicity, uvolňování paměti. Dej větší
 * data"
 *
 */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/bst.h"

/* ---------- společné ---------- */

static unsigned long long rng_state = 88172645463325252ULL;

static void seed(unsigned long long s)
{
    rng_state = s ? s : 88172645463325252ULL;
}

static unsigned long long rnd(void) // xorshift64
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return rng_state;
}

static int rnd_range(int n)
{
    return (int)(rnd() % (unsigned long long)n);
}

/* Kontrola invariantů pro libovolný typ klíče přes cmp stromu. */
static int check_node(const bst_t *t, const bst_node_t *n, const void *lo, const void *hi, size_t *count)
{
    if (n == NULL)
        return -1;

    assert(lo == NULL || t->cmp(n->key, lo) > 0);
    assert(hi == NULL || t->cmp(n->key, hi) < 0);

    int hl = check_node(t, n->left, lo, n->key, count);
    int hr = check_node(t, n->right, n->key, hi, count);
    int h = 1 + (hl > hr ? hl : hr);

    assert(n->height == h);
    assert(hl - hr >= -1 && hl - hr <= 1);

    (*count)++;
    return h;
}

static size_t check_tree(const bst_t *t)
{
    size_t count = 0;
    check_node(t, t->root, NULL, NULL, &count);
    return count;
}

/* Horní mez výšky AVL stromu: 1.4405 * log2(n + 2). */
static int height_bound(size_t n)
{
    double x = (double)(n + 2);
    int lg = 0;
    while (x > 1.0)
    {
        x /= 2.0;
        lg++;
    }
    return (int)(1.4405 * lg) + 1;
}

/* ---------- A: celočíselné klíče, diferenciální test proti poli ---------- */

static long live_int_keys = 0;
static long live_int_data = 0;

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static void free_int_key(void *p)
{
    free(p);
    live_int_keys--;
}

static void free_int_data(void *p)
{
    free(p);
    live_int_data--;
}

static void *new_int(int v, long *counter)
{
    int *p = malloc(sizeof(int));
    assert(p != NULL);
    *p = v;
    (*counter)++;
    return p;
}

/*
 * Náhodné operace (insert / delete / search) proti referenčnímu modelu.
 * universe = rozsah klíčů, ops = počet operací, check_every = jak často
 * kontrolovat celé invarianty.
 */
static void differential_int(int universe, long ops, long check_every, unsigned long long s, int insert_pct)
{
    seed(s);
    bool *model = calloc((size_t)universe, sizeof(bool));
    assert(model != NULL);
    size_t model_count = 0;

    bst_t t;
    bst_init(&t, cmp_int, free_int_key, free_int_data);

    for (long i = 0; i < ops; i++)
    {
        int k = rnd_range(universe);
        int op = rnd_range(100);

        if (op < insert_pct)
        {
            void *key = new_int(k, &live_int_keys);
            void *data = new_int(k * 3, &live_int_data);
            bst_status_t st = bst_insert(&t, key, data);
            if (model[k])
            {
                assert(st == BST_DUPLICATE);
                free_int_key(key);
                free_int_data(data);
            }
            else
            {
                assert(st == BST_OK);
                model[k] = true;
                model_count++;
            }
        }
        else if (op < insert_pct + (100 - insert_pct) / 2)
        {
            bool deleted = bst_delete(&t, &k);
            assert(deleted == model[k]);
            if (model[k])
            {
                model[k] = false;
                model_count--;
            }
        }
        else
        {
            bst_node_t *n = bst_search(&t, &k);
            assert((n != NULL) == model[k]);
            if (n != NULL)
            {
                assert(*(int *)n->key == k);
                assert(*(int *)n->data == k * 3);
            }
        }

        if (i % check_every == 0)
        {
            assert(check_tree(&t) == model_count);
            if (t.root != NULL)
                assert(t.root->height <= height_bound(model_count));
        }
    }

    // finální úplná shoda s modelem
    assert(check_tree(&t) == model_count);
    for (int k = 0; k < universe; k++)
    {
        bst_node_t *n = bst_search(&t, &k);
        assert((n != NULL) == model[k]);
    }
    assert(live_int_keys == (long)model_count);
    assert(live_int_data == (long)model_count);

    bst_dispose(&t);
    assert(t.root == NULL);
    assert(live_int_keys == 0 && live_int_data == 0);
    free(model);
}

static void test_differential(void)
{
    // malý rozsah = hodně kolizí, duplicit a mazání; kontrola po každé operaci
    for (unsigned long long s = 1; s <= 30; s++)
        differential_int(32, 6000, 1, s, 50);

    // střední rozsah, převaha vkládání, pak převaha mazání
    for (unsigned long long s = 100; s < 110; s++)
        differential_int(1000, 60000, 7, s, 70);
    for (unsigned long long s = 200; s < 210; s++)
        differential_int(1000, 60000, 7, s, 30);

    // velký rozsah
    differential_int(100000, 600000, 5000, 4242, 60);
    differential_int(100000, 600000, 5000, 777, 45);

    puts("ok: test_differential");
}

/* ---------- B: řetězcové klíče a složitá data s vnitřní alokací ---------- */

typedef struct
{
    char *name;  // alokovaný řetězec
    int *values; // alokované pole
    size_t n;
} symbol_t;

static long live_str_keys = 0;
static long live_symbols = 0;

static int cmp_str(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

static void free_str_key(void *p)
{
    free(p);
    live_str_keys--;
}

static void free_symbol(void *p)
{
    symbol_t *s = p;
    free(s->name);
    free(s->values);
    free(s);
    live_symbols--;
}

static char *make_name(int id)
{
    // různé délky a společné prefixy, ať se strcmp opravdu zapotí
    char buf[64];
    int len = snprintf(buf, sizeof buf, "ident_%d_%s", id, (id % 3 == 0) ? "x" : (id % 3 == 1) ? "yy" : "zzz");
    char *s = malloc((size_t)len + 1);
    assert(s != NULL);
    memcpy(s, buf, (size_t)len + 1);
    live_str_keys++;
    return s;
}

static symbol_t *make_symbol(int id)
{
    symbol_t *s = malloc(sizeof *s);
    assert(s != NULL);
    s->n = (size_t)(id % 7) + 1;
    s->name = malloc(16);
    assert(s->name != NULL);
    snprintf(s->name, 16, "sym%d", id);
    s->values = malloc(s->n * sizeof(int));
    assert(s->values != NULL);
    for (size_t i = 0; i < s->n; i++)
        s->values[i] = id + (int)i;
    live_symbols++;
    return s;
}

static void test_strings_and_structs(void)
{
    enum
    {
        UNIVERSE = 5000
    };
    static bool model[UNIVERSE];
    memset(model, 0, sizeof model);
    size_t model_count = 0;

    seed(2026);
    bst_t t;
    bst_init(&t, cmp_str, free_str_key, free_symbol);

    for (long i = 0; i < 150000; i++)
    {
        int id = rnd_range(UNIVERSE);
        int op = rnd_range(100);

        if (op < 55)
        {
            char *key = make_name(id);
            symbol_t *sym = make_symbol(id);
            bst_status_t st = bst_insert(&t, key, sym);
            if (model[id])
            {
                assert(st == BST_DUPLICATE);
                free_str_key(key);
                free_symbol(sym);
            }
            else
            {
                assert(st == BST_OK);
                model[id] = true;
                model_count++;
            }
        }
        else if (op < 80)
        {
            char *probe = make_name(id);
            bool deleted = bst_delete(&t, probe);
            assert(deleted == model[id]);
            if (model[id])
            {
                model[id] = false;
                model_count--;
            }
            free_str_key(probe);
        }
        else
        {
            char *probe = make_name(id);
            bst_node_t *n = bst_search(&t, probe);
            assert((n != NULL) == model[id]);
            if (n != NULL)
            {
                const symbol_t *s = n->data;
                assert(s->n == (size_t)(id % 7) + 1);
                for (size_t j = 0; j < s->n; j++)
                    assert(s->values[j] == id + (int)j);
            }
            free_str_key(probe);
        }

        if (i % 997 == 0)
            assert(check_tree(&t) == model_count);
    }

    assert(check_tree(&t) == model_count);
    assert(live_str_keys == (long)model_count);
    assert(live_symbols == (long)model_count);

    bst_dispose(&t);
    assert(live_str_keys == 0 && live_symbols == 0);
    puts("ok: test_strings_and_structs");
}

/* ---------- C: velké objemy a speciální vzory ---------- */

static void insert_range_keys(bst_t *t, int *keys, int n)
{
    for (int i = 0; i < n; i++)
    {
        keys[i] = i;
        assert(bst_insert(t, &keys[i], NULL) == BST_OK);
    }
}

static void test_large_volume(void)
{
    enum
    {
        N = 300000
    };
    int *keys = malloc(N * sizeof(int));
    int *order = malloc(N * sizeof(int));
    assert(keys != NULL && order != NULL);

    bst_t t;
    bst_init(&t, cmp_int, NULL, NULL);

    insert_range_keys(&t, keys, N); // vzestupné pořadí = nejhorší případ pro obyčejný BVS
    assert(check_tree(&t) == N);
    assert(t.root->height <= height_bound(N));
    // dokonale vyvážený strom o 300000 uzlech má výšku 18, AVL nesmí být výrazně horší
    assert(t.root->height <= 26);

    for (int i = 0; i < N; i += 997)
        assert(bst_search(&t, &keys[i]) != NULL);

    // smazání v náhodném pořadí s občasnou kontrolou
    for (int i = 0; i < N; i++)
        order[i] = i;
    seed(99);
    for (int i = N - 1; i > 0; i--)
    {
        int j = rnd_range(i + 1);
        int tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }
    for (int i = 0; i < N; i++)
    {
        assert(bst_delete(&t, &keys[order[i]]));
        if (i % 20011 == 0)
            assert(check_tree(&t) == (size_t)(N - i - 1));
    }
    assert(t.root == NULL);

    free(keys);
    free(order);
    puts("ok: test_large_volume");
}

static void test_special_patterns(void)
{
    enum
    {
        N = 20000
    };
    static int keys[N];
    bst_t t;

    // mazání vždy od minima
    bst_init(&t, cmp_int, NULL, NULL);
    insert_range_keys(&t, keys, N);
    for (int i = 0; i < N; i++)
    {
        assert(bst_delete(&t, &keys[i]));
        if (i % 500 == 0)
            assert(check_tree(&t) == (size_t)(N - i - 1));
    }
    assert(t.root == NULL);

    // mazání vždy od maxima
    insert_range_keys(&t, keys, N);
    for (int i = N - 1; i >= 0; i--)
    {
        assert(bst_delete(&t, &keys[i]));
        if (i % 500 == 0)
            assert(check_tree(&t) == (size_t)i);
    }
    assert(t.root == NULL);

    // zigzag vkládání: střídavě malé a velké
    for (int i = 0; i < N; i++)
        keys[i] = (i % 2 == 0) ? i / 2 : N - 1 - i / 2;
    for (int i = 0; i < N; i++)
        assert(bst_insert(&t, &keys[i], NULL) == BST_OK);
    assert(check_tree(&t) == N);
    // zigzag mazání: střídavě od kořene (vždy smaž aktuální kořen)
    for (int i = 0; i < N; i++)
    {
        assert(t.root != NULL);
        int rk = *(int *)t.root->key;
        assert(bst_delete(&t, &rk));
        if (i % 500 == 0)
            assert(check_tree(&t) == (size_t)(N - i - 1));
    }
    assert(t.root == NULL);

    // opakované plnění a vyprazdňování stejného stromu
    for (int round = 0; round < 50; round++)
    {
        for (int i = 0; i < 200; i++)
        {
            keys[i] = (i * 37 + round) % 1009;
            (void)bst_insert(&t, &keys[i], NULL); // duplicity jsou v pořádku
        }
        check_tree(&t);
        bst_dispose(&t);
        assert(t.root == NULL);
    }

    // jediný prvek: vložit, najít, smazat, znovu vložit
    int one = 7;
    assert(bst_insert(&t, &one, NULL) == BST_OK);
    assert(t.root->height == 0);
    assert(bst_search(&t, &one) == t.root);
    assert(bst_delete(&t, &one));
    assert(t.root == NULL);
    assert(!bst_delete(&t, &one));
    assert(bst_insert(&t, &one, NULL) == BST_OK);
    bst_dispose(&t);

    puts("ok: test_special_patterns");
}

int main(void)
{
    test_special_patterns();
    test_strings_and_structs();
    test_differential();
    test_large_volume();
    puts("Vsechny zatezove testy prosly.");
    return 0;
}
