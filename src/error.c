/**
 * @file error.c
 * @brief Implementace funkce pro tisk chyb.
 *
 * @author Zdeněk Michalec, Jonáš Javůrek, Tadeáš Trynkl
 */
#ifndef ERROR_C
#define ERROR_C

#include "error.h"
#include <stdio.h>
#include <stdlib.h>

char *getErrorName(ErrorCodes error)
{
    switch (error)
    {
    case ERR_OK:
        return "ERR_OK";
    case ERR_LEX:
        return "ERR_LEX";
    case ERR_SYN:
        return "ERR_SYN";
    case ERR_SEM_UNDEF:
        return "ERR_SEM_UNDEF";
    case ERR_SEM_REDEF:
        return "ERR_SEM_REDEF";
    case ERR_SEM_ARG:
        return "ERR_SEM_ARG";
    case ERR_SEM_TYPE:
        return "ERR_SEM_TYPE";
    case ERR_SEM_UNMOD:
        return "ERR_SEM_UNMOD";
    case ERR_SEM_OTHER:
        return "ERR_SEM_OTHER";
    case ERR_INTERNAL:
        return "ERR_INTERNAL";
    default:
        return "NEZNAMY_KOD_CHYBY";
    }
}

void printError(ErrorCodes error)
{
    if (error != ERR_OK)
    {
        fprintf(stderr, "Chyba: %s (kod: %d)\n", getErrorName(error), error);
    }
    exit(error);
}

#endif