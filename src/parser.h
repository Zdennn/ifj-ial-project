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

#include "error.h"
#include "precedence_table.h"
#include "symtable.h"
#include "token.h"

/**
 * @brief Hlavní vstupní bod parseru.
 *
 * Zahájí syntaktickou a sémantickou analýzu zdrojového kódu ze stdin.
 *
 * @return ErrorCodes Chybový kód (0 = ERR_OK, 1 = ERR_LEX, 2 = ERR_SYN, 3–7 = sémantické chyby).
 */
ErrorCodes parse();

/**
 * @brief Získání dalšího tokenu z parseru.
 *
 * Zavolá funkci get_next_token().
 *
 * @return ErrorCodes Chybový kód ze scanneru.
 */
ErrorCodes get_n_token();

/**
 * @brief Získání dalšího tokenu z pole token_array.
 *
 * Hlídá indexaci a uloží do current_token další token
 */
void get_next_token_from_array();

/**
 * @brief Restartuje index pro průchod v poli token_array.
 */
void reset_parser_position();

/**
 * @brief Zpracování hlavičky souboru.
 */
void beginning();

/**
 * @brief Ověří shodu aktuálního tokenu s očekávaným typem a posune index pole.
 *
 * Pokud typ aktuálního tokenu odpovídá očekávanému vrátí zkonzumovaný token
 * a načte další token získaný ze pole token_array.
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
 * Zjistí parametry funkce,
 * zpracuje její tělo.
 */
void parse_function();

/**
 * @brief Zpracuje hlavičku jedné funkce.
 *
 * Parsuje hlavičku funkce,
 * zapíše jej do symtable.
 *
 * @return ErrorCodes, chybový kód
 */
ErrorCodes define_function_header();

/**
 * @brief Zpracuje parametry jdené funkce.
 *
 * Parsuje dvojice ve tvaru [identifikátor : Typ] oddělené čárkami.
 *
 * @return DataType, pole typů parametrů
 */
DataType *parse_parameters();

/**
 * @brief Ověří a zkonzumuje jeden token datového typu.
 *
 * @return Token, zkonzumovaný token
 */
Token match_type();

/**
 * @brief Převede token s datovým typen na hodnotu struktury DataType.
 *
 * @return DataType, typ proměnné hodnoty DataType
 */
DataType token_to_dataType();

/**
 * @brief Zpracuje jeden odsazený blok příkazů.
 *
 * Kontroluje úroveň odsazení na začátku každého řádku
 */
DataType parse_block(DataType expected_return_type);

/**
 * @brief Provede syntaktickou a sémantickou analýzu výrazu.
 *
 * Spouští precedenční syntaktickou analýzu zdola nahoru řízenou precedenční tabulkou
 * a zásobníkem.
 *
 * @return DataType, typ danných proměnných
 */
DataType parse_expression();

/**
 * @brief Funkce pro postupné čtení a zápis všech tokenů do token_array až do EOF.
 */
void read_file_by_tokens();

/**
 * @brief Provede kontrolu semantiky výrazu.
 */
DataType kontrola_typu(DataType left_type, PrecIndex op, DataType right_type, TokenType op_token);

/******************************
 *          DEBUG FUNKCE
 *******************************/

/**
 * @brief Debug funkce pro převod kódu typu tokenu na textový řetězec.
 *
 * @param type Typ tokenu (TokenType).
 * @return const char* Textový názvu tokenu.
 */
const char *getTokenName(TokenType type);

#endif