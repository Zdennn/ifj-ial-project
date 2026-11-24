/**
 * @file precedence_table.h
 * @brief Deklarace precedenční tabulky a typů.
 *
 * Obsahuje indexy pro mapování tokenů do tabulky a definici
 * operací pro syntaktickou analýzu.
 *
 * @author ...
 */

#ifndef PREC_TABLE_H
#define PREC_TABLE_H

#include "token.h"

/**
 * @enum PrecIndex
 * @brief Indexy zastupující skupiny operátorů v precedenční tabulce.
 */
typedef enum
{
    PREC_NOT,        // !           BOOLTHEN
    PREC_MUL_DIV,    // / *
    PREC_PLUS_MINUS, // + -
    PREC_REL,        // < > <= >=
    PREC_EQUAL,      // == !=
    PREC_AND,        // &&          BOOLTHEN
    PREC_OR,         // ||          BOOLTHEN
    PREC_LZAVORKA,   // (
    PREC_PZAVORKA,   // )
    PREC_ID,         // identifikátor, proměnná, číslo, string ...
    PREC_STOP,       // $ zarážka
    PREC_INVALID     // nepovoleny znak
} PrecIndex;

/**
 * @enum PrecOperation
 * @brief Operace, které vrací precedenční tabulka.
 */
typedef enum
{
    PREC_ERR, // Chyba
    PREC_S,   // < shift, vloží na zásobník zarážku a načte token
    PREC_R,   // > reduce redukuj na E
    PREC_E    // = shoda
} PrecOperation;

/**
 * @brief Převede typ tokenu na index v precedenční tabulce.
 *
 * @param type Typ tokenu (TokenType) ze scanneru.
 * @return PrecIndex Index pro řádek/sloupec precedenční tabulky.
 */
PrecIndex token_to_index(TokenType type);

/**
 * @brief Vrátí operaci z tabulky.
 *
 * @param stack_top Hodnota na vrcholu zásobníku.
 * @param input_token Hodnota inputu.
 * @return PrecOperation Výsledná operace.
 */
PrecOperation get_prec_operation(PrecIndex stack_top, PrecIndex input_token);

#endif