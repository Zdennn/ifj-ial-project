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

/* Nastaví výstup, do kterého se generuje kód (stdout nebo soubor dle testu) */
void gen_init(FILE *out);

void gen_header(void);
#endif 
