/**
 * @file parser.h
 * @brief Rozhraní syntaktického analyzátoru.
 *
 * @author Zdeněk Michalec, Jonáš Javůrek, Tadeáš Trynkl
 */
#ifndef PARSER_H
#define PARSER_H

/**
 * @brief Provede syntaktickou analýzu programu načteného ze stdin.
 * @return int Chybový kód (0 = ERR_OK, 1 = ERR_LEX, 2 = ERR_SYN)
 */
int parse();

#endif