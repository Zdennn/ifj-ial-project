/**
 * @file symbol_table.h
 * @brief Rozhraní Tabulky symbolů realizovaná jako výškově vyvážený binární vyhledávací strom.
 *
 * Ukládá informace o identifikátorech (proměnných) a jejich vlastnostech (typ, hodnota).
 * Varianta vv-BVS = Výškově vyvážený binární vyhledávací strom
 *
 * @author ...
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "bst.h"
#include <stdbool.h>

typedef enum
{
    SYM_VARIABLE,
    SYM_FUNCTION
} SymbolType;

typedef enum
{
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_UNIT,
    TYPE_BOOLEAN,
    TYPE_ANY,
    TYPE_UNDEF
} DataType;

typedef struct
{
    char *name;             // Identifikátor
    SymbolType symbol_type; // Proměnná nebo funkce
    DataType data_type;     // Typ proměnné / návratový typ funkce

    // Pro proměnné
    bool is_var;    // true = var, false = val
    char *gen_name; // Jedinečné jméno v generovaném kódu (jméno$číslo)

    // Pro funkce
    int param_count;        // Počet seznamů parametrů
    int *param_sizes;       // Pole počtů parametrů v seznamech
    DataType **param_types; // Pole polí typů parametrů             PARAMLISTS
    bool is_main;           // Příznak vstupní funkce @main

} SymbolData;

typedef struct Scope
{
    bst_t *tree;          // Lokální strom pro tento blok
    struct Scope *parent; // Nadřazený strom
} Scope;

/**
 * @brief Inicializuje tabulku symbolů a zaregistruje vestavěné funkce.
 */
void symtable_init();

/**
 * @brief Uvolní paměť tabulky symbolů při ukončení překladu.
 */
void symtable_dispose();

/**
 * @brief Otevře nový scope (blok funkce, cyklu while, větve if).
 */
void symtable_push_scope();

/**
 * @brief Uzavře aktuální scope.
 */
void symtable_pop_scope();

/**
 * @brief Hledá symbol od aktuálního scope až po kořen.
 *
 * @param name Název vyhledánaného identifikátoru
 * @return Najitý uzel SymbolData
 */
SymbolData *symtable_lookup(char *name);

/**
 * @brief Hledá symbol v aktuálním scope.
 *
 * @param name Název vyhledánaného identifikátoru
 * @return Najitý uzel SymbolData
 */
SymbolData *symtable_lookup_current(char *name);

/**
 * @brief Vloží novou lokální proměnnou nebo parametr do aktuálního scope.
 * Vyhodí chybu č. 4, pokud proměnná v tomto rámci již existuje.
 *
 * @param name Název identifikátoru
 * @param type Datový typ proměnné
 * @param is_var Boolean příznak, zda je var
 */
void symtable_insert_var(char *name, DataType type, bool is_var);

/**
 * @brief Vloží funkci do globální tabulky funkcí.
 * Vyhodí chybu č. 4 - 7.
 *
 * @param name Název identifikátoru
 * @param return_type Návratový datový typ funkce
 * @param param_count Počet parametrů
 * @param param_types Pole parametrů
 * @param is_main Boolean příznak, zda je main
 */
void symtable_insert_function(char *name, DataType return_type, int param_count, int *param_sizes,
                              DataType **param_types, bool is_main);

#endif