/**
 * @file parser.h
 * @brief Rozhraní syntaktického a sémantického analyzátoru jazyka IFJ26.
 *
 * Rozhraní soubor definuje funkce pro syntaktickou analýzu.
 * Řídicí struktury a deklarace jsou zpracovány metodou rekurzivního sestupu (shora dolů),
 * zatímco výrazy a operace jsou delegovány na precedenční syntaktickou analýzu (zdola nahoru).
 *
 * @author Zdeněk Michalec, Jonáš Javůrek, Tadeáš Trynkl
 */
#ifndef PARSER_H
#define PARSER_H

#include "token.h"

/**
 * @brief Hlavní vstupní bod parseru.
 *
 * Zahájí syntaktickou a sémantickou analýzu zdrojového kódu ze stdin.
 *
 * @return int Chybový kód (0 = ERR_OK, 1 = ERR_LEX, 2 = ERR_SYN, 3–7 = sémantické chyby).
 */
int parse();

/**
 * @brief Ověří shodu aktuálního tokenu s očekávaným typem a posune scanner.
 *
 * Pokud typ aktuálního tokenu odpovídá očekávanému vrátí zkonzumovaný token
 * a načte další token získaný ze scanneru.
 *
 * V případě neshody ukončí program s chybou č. 2.
 *
 * @param expected Očekávaný typ tokenu
 * @return Token ukazatel na zkonzumovaný token
 */
Token match(TokenType expected);

/**
 * @brief Zpracuje definici jedné funkce.
 *
 * Parsuje hlavičku funkce,
 * zapíše jej do symtable,
 * zpracuje její tělo.
 */
void parse_function();

/**
 * @brief Zpracuje parametry jdené funkce.
 *
 * Parsuje dvojice ve tvaru [identifikátor : Typ] oddělené čárkami.
 */
void parse_parameters();

/**
 * @brief Ověří a zkonzumuje jeden token datového typu.
 */
void match_type();

/**
 * @brief Zpracuje jeden odsazený blok příkazů.
 *
 * Kontroluje úroveň odsazení na začátku každého řádku
 */
void parse_block();

/**
 * @brief Provede syntaktickou a sémantickou analýzu výrazu.
 *
 * Spouští precedenční syntaktickou analýzu zdola nahoru řízenou precedenční tabulkou
 * a zásobníkem
 */
void parse_expression();

/******************************
 *          DEBUG FUNKCE
 *******************************/

/**
 * @brief Debug funkce pro postupné čtení a výpis všech tokenů až do EOF.
 */
void read_file_by_tokens();
/**
 * @brief Debug funkce pro převod kódu typu tokenu na textový řetězec.
 *
 * @param type Typ tokenu (TokenType).
 * @return const char* Textový názvu tokenu.
 */
const char *getTokenName(TokenType type);

#endif