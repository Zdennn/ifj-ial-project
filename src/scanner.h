/**
 * @file scanner.h
 * @brief Rozhraní Lexikálního analyzátoru.
 * 
 * Rozhraní pro čtení zdrojového kódu ze stdin a převádění na tokeny, které předává syntaktickému analyzátoru (parser).
 * 
 * @author ...
 */

#ifndef SCANNER_H
#define SCANNER_H

#include "token.h" 

/**
 * @brief Načte a vrátí další token
 * 
 * @param token Ukazatel na strukturu tokenu, do které se uloží načtený token.
 * @return int Chybový kód (0 = ERR_OK, 1 = ERR_LEX)
 */
int get_next_token(Token* token);

#endif