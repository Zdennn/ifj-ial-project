/**
 * @file precedence_stack.h
 *
 * @brief Definice zásobníku pro precedenční syntaktickou analýzu.
 *
 * @author ...
 */

#ifndef STACK_H
#define STACK_H

#include "precedence_table.h"
#include "symtable.h"
#include "token.h"

/**
 * @enum PrecType
 * @brief Typ symbolu na zásobníku.
 */
typedef enum
{
    SEM_TERMINAL,    // Token ze vstupu (scanneru), jsou to operátory => / * + - ...
    SEM_NONTERMINAL, // Výsledek po redukci => E
    SEM_STOP         // Zarážka => shift < nebo konec $
} PrecType;

/**
 * @struct StackItem
 * @brief Položka uložená v precedenčním zásobníku.
 */
typedef struct
{
    PrecType sem_type;  // Terminál, Neterminál nebo Zarážka
    PrecIndex prec_sym; // Mapovaný index do precedenční tabulky
    DataType data_type; // Datový typ (Int, Double...)
} StackItem;

/**
 * @struct PrecStack
 * @brief Dynamicky alokovaný zásobník pro výrazy.
 */
typedef struct
{
    StackItem *items; // Pole prvků na zásobníku
    int top;          // Index dalšího místa na vrcholu
    int capacity;     // Maximální kapacita zásobníku
} PrecStack;

/**
 * @brief Inicializuje nově vytvořený zásobník.
 * @param stack Ukazatel na zásobník.
 */
void stack_init(PrecStack *stack);
/**
 * @brief Vloží nový prvek na vrchol zásobníku.
 *
 * Pokud nedostačuje kapacita, zásobník je dynamicky zvětšen.
 *
 * @param stack Ukazatel na zásobník.
 * @param new_item Struktura k vložení.
 */
void stack_push(PrecStack *stack, StackItem new_item);

/**
 * @brief Odebere a vrátí prvek z vrcholu zásobníku.
 *
 * @param stack Ukazatel na zásobník.
 * @return StackItem Odebraný prvek. Vrací prázdnou strukturu, pokud je prázdný.
 */
StackItem stack_pop(PrecStack *stack);

/**
 * @brief Vrátí prvek na vrcholu zásobníku bez jeho odebrání.
 *
 * @param stack Ukazatel na zásobník.
 * @return StackItem* Prvek na vrcholu.
 */
StackItem *stack_top(PrecStack *stack);

/**
 * @brief Najde a vrátí nejvrchnější terminál na zásobníku.
 *
 * Ignoruje neterminály (E).
 * Používá se pro určení řádku v precedenční tabulce.
 *
 * @param stack Ukazatel na zásobník.
 * @return StackItem* Nejvrchnější terminál nebo zarážka.
 */
StackItem *stack_top_terminal(PrecStack *stack);

/**
 * @brief Vloží zarážku za první / nejvyšší teminal.
 *
 * @param stack Ukazatel na zásobník.
 */
void stack_vlozeni_zarazky_za_nevyssi_terminal(PrecStack *stack);

/**
 * @brief Uvolní dynamickou paměť alokovanou zásobníkem.
 * @param stack Ukazatel na zásobník.
 */
void stack_free(PrecStack *stack);

#endif