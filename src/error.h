/**
 * @file error.h
 * @brief Definice chybových kódů - enum.
 * 
 * @author Zdeněk Michalec
 */

#ifndef ERROR_H
#define ERROR_H

typedef enum {
    ERR_OK = 0, 
    ERR_LEX = 1, // Chyba v lexikální analýze (chybná struktura tokenu - aktuálního lexému, komentáře nebo odsazení).
    ERR_SYN = 2, // chyba v syntaktické analýze (chybná syntaxe, chybějící prolog, nepovolená konstrukce nebo porušení struktury odsazených bloků).
    ERR_SEM_UNDEF = 3, // Nedefinované funkce, proměnné nebo typu; chybějící funkce @main.
    ERR_SEM_REDEF = 4, // Nepovolená redefinice funkce, proměnné, typu.
    ERR_SEM_ARG = 5, // Nesprávný počet nebo typ argumentů při volání funkce, nepovolené parametry funkce s anotací @main.
    ERR_SEM_TYPE = 6, // Chyba typové kompatibility nebo odvození typu ve výrazu, přiřazení, inicializaci, návratové hodnotě či řídicí konstrukci.
    ERR_SEM_UNMOD = 7, // Pokus o zápis do nemodifikovatelné proměnné nebo formálního parametru funkce.
    ERR_SEM_OTHER = 10, // Ostatní sémantické chyby (např. nepovolený return, nevyčerpávající match nebo nulovýkrok rozsahu).
    ERR_INTERNAL = 99 // Interní chyba překladače neovlivněná vstupním programem (např. chyba alokace paměti).
} ErrorCodes;

#endif