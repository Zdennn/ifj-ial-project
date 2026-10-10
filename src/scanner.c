/**
 * @file scanner.c
 * @brief Implementace lexikálního analyzátoru.
 *
 * Čte vstup po znacích ze stdin a převádí je na tokeny, které předává syntaktickému analyzátoru (parser).
 *
 * @author ...
 */
#include "scanner.h"
#include "error.h"
#include "token.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_next_token(Token *token)
{
    int c;

    while ((c = getchar()) != EOF)
    {

        if (c == ' ')
            continue;
        if (c == '\n')
        {
            token->type = TOKEN_EOL;
            return ERR_OK;
        }

        // Tokeny pro identifikátory nebo while, if...
        if (isalpha(c))
        {
            char buffer[256];
            int len = 0;
            while ((c = getchar()) != EOF && (isalnum(c) || c == '_'))
            {
                if (len < 255)
                    buffer[len++] = c;
            }
            buffer[len] = '\0';
            ungetc(c, stdin);

            if (strcmp(buffer, "while") == 0)
            {
                token->type = TOKEN_WHILE;
            }
            else if (strcmp(buffer, "do") == 0)
            {
                token->type = TOKEN_DO;
            }
            else if (strcmp(buffer, "if") == 0)
            {
                token->type = TOKEN_IF;
            }
            else if (strcmp(buffer, "var") == 0)
            {
                token->type = TOKEN_VAR;
            }
            else if (strcmp(buffer, "val") == 0)
            {
                token->type = TOKEN_VAL;
            }
            else if (strcmp(buffer, "import") == 0)
            {
                token->type = TOKEN_IMPORT;
            }
            else if (strcmp(buffer, "return") == 0)
            {
                token->type = TOKEN_RETURN;
            }
            else if (strcmp(buffer, "then") == 0)
            {
                token->type = TOKEN_THEN;
            }
            else if (strcmp(buffer, "int") == 0)
            {
                token->type = TOKEN_INT;
            }
            else if (strcmp(buffer, "double") == 0)
            {
                token->type = TOKEN_DOUBLE;
            }
            else if (strcmp(buffer, "string") == 0)
            {
                token->type = TOKEN_STRING;
            }
            else if (strcmp(buffer, "unit") == 0)
            {
                token->type = TOKEN_UNIT;
            }
            else
            {
                token->type = TOKEN_ID;
                token->string = strdup(buffer);
            }
            return ERR_OK;
        }
    }

    // Tokeny pro čísla
    if (isdigit(c))
    {
        char buffer[256];
        int len = 0;

        while ((c = getchar()) != EOF && (isdigit(c) || c == '_' || c == '.'))
        {
            if (len < 255 && c != '_')
                buffer[len++] = c;
        }
        ungetc(c, stdin);
        if (strchr(buffer, '.') != NULL)
        {
            strtod(buffer, NULL);
        }
        else
        {
            strtol(buffer, NULL, 10);
        }
        return ERR_OK;
    }

    // Tokeny pro operace
    if (c == '=')
    {
        token->type = TOKEN_ROVNO;
        return ERR_OK;
    }
    if (c == '+')
    {
        token->type = TOKEN_PLUS;
        return ERR_OK;
    }
    if (c == '-')
    {
        token->type = TOKEN_MINUS;
        return ERR_OK;
    }
    if (c == '*')
    {
        token->type = TOKEN_NAS;
        return ERR_OK;
    }
    if (c == '/')
    {
        token->type = TOKEN_DEL;
        return ERR_OK;
    }
    if (c == '(')
    {
        token->type = TOKEN_LZAVORKA;
        return ERR_OK;
    }
    if (c == ')')
    {
        token->type = TOKEN_PZAVORKA;
        return ERR_OK;
    }
    if (c == ':')
    {
        token->type = TOKEN_DVOJTEC;
        return ERR_OK;
    }

    // Tokeny pro porovnávání
    if (c == '<')
    {
        token->type = TOKEN_MENCI;
        return ERR_OK;
    }
    if (c == '>')
    {
        token->type = TOKEN_VETSI;
        return ERR_OK;
    }
    if (c == '<=')
    {
        token->type = TOKEN_MENSI_ROVNO;
        return ERR_OK;
    }
    if (c == '>=')
    {
        token->type = TOKEN_VETSI_ROVNO;
        return ERR_OK;
    }
    if (c == '==')
    {
        token->type = TOKEN_STEJNO;
        return ERR_OK;
    }
    if (c == '!=')
    {
        token->type = TOKEN_NE_ROVNO;
        return ERR_OK;
    }
    

    return ERR_LEX;

    // Token konce souboru
    token->type = TOKEN_EOF;
    return ERR_OK;
}