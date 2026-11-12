/**
 * @file parser.h
 * @brief Rozhraní syntaktického analyzátoru.
 *
 * @author Zdeněk Michalec, Jonáš Javůrek, Tadeáš Trynkl
 */
#include "token.h"
#ifndef PARSER_H
#define PARSER_H

/**
 * @brief Provede syntaktickou analýzu programu načteného ze stdin.
 * @return int Chybový kód (0 = ERR_OK, 1 = ERR_LEX, 2 = ERR_SYN)
 */
int parse();
Token match(TokenType expected);
// Rekurzní průchod pro zkontrolování návaznosti tokenů
void parse_statement();
// Průchod přes zásobník, kontroluje proměnné a operace
void parse_expression();
// Zpracování funkce
void parse_function();
// Zpracování parametru funkce
void parse_parameters();
// Zpracování typu
void match_type();

#endif