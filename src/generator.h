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

/**
 * @brief Vypíše jednu instrukci IFJcode26 ve tvaru "OPCODE a b c\n".
 *
 * Operandy jsou hotový text (např. "LF@x", "int@5", "string@ahoj").
 * Operandy, které instrukce nemá, se předají jako NULL a nevypíšou se.
 * Operandy se řadí zleva, tj. NULL v prostředku (a, NULL, c) není povolen.
 *
 * @param opcode Název instrukce (např. "ADD", "WRITE"), nesmí být NULL.
 * @param a      První operand, nebo NULL.
 * @param b      Druhý operand, nebo NULL (smí být jen pokud je i c NULL, nebo a/b zadány).
 * @param c      Třetí operand, nebo NULL.
 */
void gen_instr(const char *opcode, const char *a, const char *b, const char *c);


#endif
