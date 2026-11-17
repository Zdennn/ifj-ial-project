/**
 * @file precendece_table.c
 *
 * @brief Implementace precedenční tabulky a mapovací funkce.
 *
 * @author Zdeněk Michalec
 */

#include "precedence_table.h"
#include "symtable.h"
#include "token.h"

/**
 * @brief Dvourozměrné pole reprezentující precedenční tabulku.
 * Řádek je terminál na vrcholu zásobníku, sloupec je token na vstupu.
 */
static const PrecOperation prec_table[8][8] = {
    //              * /     + -     rel     ==      (       )       id     $
    /* * /  */ {PREC_R, PREC_R, PREC_R, PREC_R, PREC_S, PREC_R, PREC_S, PREC_R},
    /* + -  */ {PREC_S, PREC_R, PREC_R, PREC_R, PREC_S, PREC_R, PREC_S, PREC_R},
    /* rel  */ {PREC_S, PREC_S, PREC_ERR, PREC_ERR, PREC_S, PREC_R, PREC_S, PREC_R},
    /* ==   */ {PREC_S, PREC_S, PREC_ERR, PREC_ERR, PREC_S, PREC_R, PREC_S, PREC_R},
    /* (    */ {PREC_S, PREC_S, PREC_S, PREC_S, PREC_S, PREC_E, PREC_S, PREC_ERR},
    /* )    */ {PREC_R, PREC_R, PREC_R, PREC_R, PREC_ERR, PREC_R, PREC_ERR, PREC_R},
    /* id   */ {PREC_R, PREC_R, PREC_R, PREC_R, PREC_ERR, PREC_R, PREC_ERR, PREC_R},
    /* $    */ {PREC_S, PREC_S, PREC_S, PREC_S, PREC_S, PREC_ERR, PREC_S, PREC_ERR}};

PrecOperation get_prec_operation(PrecIndex stack_top, PrecIndex input_token)
{
    if (stack_top == PREC_INVALID || input_token == PREC_INVALID)
    {
        return PREC_ERR;
    }
    return prec_table[stack_top][input_token];
}

PrecIndex token_to_index(TokenType type)
{
    switch (type)
    {
    case TOKEN_NAS:
    case TOKEN_DEL:
        return PREC_MUL_DIV;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return PREC_PLUS_MINUS;
    case TOKEN_MENSI:
    case TOKEN_MENSI_ROVNO:
    case TOKEN_VETSI:
    case TOKEN_VETSI_ROVNO:
        return PREC_REL;
    case TOKEN_STEJNO:
    case TOKEN_NE_ROVNO:
        return PREC_EQUAL;
    case TOKEN_LZAVORKA:
        return PREC_LZAVORKA;
    case TOKEN_PZAVORKA:
        return PREC_PZAVORKA;
    case TOKEN_ID:
    case TOKEN_INT_HOD:
    case TOKEN_DOUBLE_HOD:
    case TOKEN_STRING_HOD:
    case TOKEN_UNIT:
        return PREC_ID;
    case TOKEN_EOL:
    case TOKEN_THEN:
    case TOKEN_DO:
    case TOKEN_EOF:
        return PREC_STOP;
    default:
        return (PrecIndex){0};
    }
}