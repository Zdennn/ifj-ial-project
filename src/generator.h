/**
 * @file generator.h
 * @brief Rozhraní Generátoru kódu.
 *
 * Implementace překladače imperativního jazyka IFJ26
 *
 * @author Trynkl Tadeáš, Javůrek Jonáš, Zdeněk Michalec
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>

/**
 * @brief Nastaví výstupní proud pro generovaný kód.
 * @param out Cílový proud, nesmí být NULL.
 */
void gen_init(FILE *out);

/**
 * @brief Vypíše úvodní řádek .IFJcode26.
 */
void gen_header(void);

/**
 * @brief Vypíše celočíselný operand ve tvaru int@<hodnota> (bez odřádkování).
 * @param value Hodnota typu Int.
 */
void gen_write_int(int value);

/**
 * @brief Vypíše desetinný operand ve tvaru float@<hodnota> (formát %a, bez odřádkování).
 * @param value Hodnota typu Double.
 */
void gen_write_float(double value);

/**
 * @brief Vypíše řetězcový operand ve tvaru string@<hodnota>.
 *        Znaky s kódem 0-32, 35 a 92 zapisuje jako escape sekvenci \xyz.
 * @param str Řetězec zakončený nulou, nesmí být NULL.
 */
void gen_write_string(const char *str);


#endif
