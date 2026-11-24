/**
 * @file bst.h
 * @brief Obecný AVL strom.
 *
 * Výškově vyvážený binární vyhledávací strom s klíči a daty libovolného typu.
 * Porovnání klíčů a uvolňování paměti zajišťují funkce předané při inicializaci.
 *
 * @author ...
 */

#ifndef BST_H
#define BST_H

#include <stdbool.h>

typedef enum
{
    BST_OK = 0,        // uzel úspěšně vložen
    BST_DUPLICATE = 1, // klíč už ve stromu existuje, strom nezměněn
    BST_NO_MEMORY = 2, // selhala alokace paměti, strom nezměněn
    BST_ERROR = 3      // neplatný vstup
} bst_status_t;

typedef struct bst_node
{
    void *key;              // klíč uzlu, jeho typ zná porovnávací funkce
    struct bst_node *left;  // ukazatel na levý podstrom (menší klíč)
    struct bst_node *right; // ukazatel na pravý podstrom (větší klíč)
    void *data;             // ukazatel na data
    int height;             // výška podstromu: list = 0, NULL = -1

} bst_node_t;

typedef int (*bst_cmp_func)(const void *, const void *); // porovnává klíče: záporné číslo a<b, 0 a==b, kladné číslo a>b

typedef void (*bst_key_free_func)(void *);  // uvolní klíč uzlu
typedef void (*bst_data_free_func)(void *); // uvolní data uzlu

typedef struct bst
{
    bst_node_t *root;             // kořen stromu
    bst_cmp_func cmp;             // porovnávací funkce klíčů, povinné (záporné číslo a<b, 0 a==b, kladné číslo a>b)
    bst_key_free_func key_free;   // uvolnění klíče, nepovinné
    bst_data_free_func data_free; // uvolnění dat, nepovinné
} bst_t;

/**
 * @brief Inicializace stromu
 *
 * @param tree ukazatel na strom
 * @param cmp funkce pro porovnání klíčů (povinná, nesmí být NULL)
 * @param key_free funkce pro uvolnění klíče (může být NULL)
 * @param data_free funkce pro uvolnění dat (může být NULL)
 */
void bst_init(bst_t *tree, bst_cmp_func cmp, bst_key_free_func key_free, bst_data_free_func data_free);

/**
 * @brief Vložení klíče s daty do stromu
 *
 * @param tree ukazatel na strom
 * @param key klíč nového uzlu
 * @param data ukazatel na data
 * @return BST_OK při úspěchu, BST_DUPLICATE pokud klíč už existuje, BST_NO_MEMORY při selhání alokace.
 */
bst_status_t bst_insert(bst_t *tree, void *key, void *data);

/**
 * @brief Vyhledávání uzlu pomocí klíče
 *
 * @param tree ukazatel na strom
 * @param key klíč hledaného uzlu
 * @return ukazatel na nalezený uzel, nebo NULL pokud neexistuje
 */
bst_node_t *bst_search(bst_t *tree, const void *key);

/**
 * @brief Odstranění uzlu pomocí klíče
 *
 * @param tree ukazatel na strom
 * @param key klíč mazaného uzlu
 * @return true, pokud odstranění proběhne úspěšně, jinak false
 */
bool bst_delete(bst_t *tree, const void *key);

/**
 * @brief Zrušení celého stromu
 *
 * @param tree ukazatel na strom
 */
void bst_dispose(bst_t *tree);

#endif