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
            else if (strcmp(buffer, "else") == 0)
            {
                token->type = TOKEN_ELSE;
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
            else if (strcmp(buffer, "Int") == 0)
            {
                token->type = TOKEN_INT;
            }
            else if (strcmp(buffer, "Double") == 0)
            {
                token->type = TOKEN_DOUBLE;
            }
            else if (strcmp(buffer, "String") == 0)
            {
                token->type = TOKEN_STRING;
            }
            else if (strcmp(buffer, "Unit") == 0)
            {
                token->type = TOKEN_UNIT;
            }
            else if (strcmp(buffer, "def") == 0)
            {
                token->type = TOKEN_DEF;
            }
            else
            {
                token->type = TOKEN_ID;
                token->string = strdup(buffer);
            }
            return ERR_OK;

            // Podle rozšíření přidávat další tokeny
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
            buffer[len] = '\0';
            ungetc(c, stdin);

            if (strchr(buffer, '.') != NULL)
            {
                token->type = TOKEN_DOUBLE_HOD;
                token->double_val = strtod(buffer, NULL);
            }
            else
            {
                token->type = TOKEN_INT_HOD;
                token->int_val = (int)strtol(buffer, NULL, 10);
            }
            return ERR_OK;
        }

        // Tokeny pro řetězce
        if (c == '"')
        {
            char *buffer = (char *)malloc(sizeof(char) * 3); // *3 kvuli koncovemu znaku size=2
            if (buffer == NULL)
                return ERR_INTERNAL;

            int len = 0;
            int size = 2;

            // Jednořádkový řetězec
            if ((c = getchar()) != '"')
            {
                ungetc(c, stdin);
                while ((c = getchar()) != '"')
                {
                    if (c == EOF)
                    {
                        return ERR_LEX;
                    }

                    if (c == '\n')
                    {
                        return ERR_LEX;
                    }

                    if (c == '\\')
                    {
                        c = getchar();

                        if (c == EOF)
                        {
                            return ERR_LEX;
                        }

                        switch (c)
                        {
                        case '\\':
                            c = '\\';
                            break;
                        case '"':
                            c = '"';
                            break;
                        case 'n':
                            c = '\n';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        case 't':
                            c = '\t';
                            break;

                        default:
                            return ERR_LEX;
                        }
                    }
                    if (len >= size)
                    {
                        size = size * 2;
                        char *new_buff =
                            (char *)realloc(buffer, sizeof(char) * (size + 1)); // +1 kvuli ukončení řetezce \0
                        if (new_buff == NULL)
                        {
                            return ERR_INTERNAL;
                        }
                        buffer = new_buff;
                    }

                    buffer[len++] = c;
                }

                buffer[len] = '\0';
                token->type = TOKEN_STRING_HOD;
                token->string = buffer;

                return ERR_OK;
            }
            // Více řádkový řetězec
            else
            {
                if ((c = getchar()) == '"')
                {
                    if ((c = getchar()) == '"')
                    {
                        char *buffer = (char *)malloc(sizeof(char) * 3); // *3 kvuli koncovemu znaku size=2
                        if (buffer == NULL)
                            return ERR_INTERNAL;

                        int len = 0;
                        int size = 2;

                        while (1)
                        {
                            c = getchar();
                            if (c == EOF)
                            {
                                return ERR_LEX;
                            }

                            if (c == '\\')
                            {
                                c = getchar();

                                if (c == EOF)
                                {
                                    return ERR_LEX;
                                }

                                if (c == '"')
                                {
                                    int c2 = getchar();
                                    if (c2 == '"')
                                    {
                                        int c3 = getchar();
                                        if (c3 == '"')
                                        {
                                            break;
                                        }
                                        ungetc(c3, stdin);
                                    }
                                    ungetc(c2, stdin);
                                }

                                switch (c)
                                {
                                case '\\':
                                    c = '\\';
                                    break;
                                case '"':
                                    c = '"';
                                    break;
                                case 'n':
                                    c = '\n';
                                    break;
                                case 'r':
                                    c = '\r';
                                    break;
                                case 't':
                                    c = '\t';
                                    break;

                                default:
                                    return ERR_LEX;
                                }
                            }
                            if (len >= size)
                            {
                                size = size * 2;
                                char *new_buff =
                                    (char *)realloc(buffer, sizeof(char) * (size + 1)); // +1 kvuli ukončení řetezce \0
                                if (new_buff == NULL)
                                {
                                    return ERR_INTERNAL;
                                }
                                buffer = new_buff;
                            }

                            buffer[len++] = c;
                        }

                        buffer[len] = '\0';
                        token->type = TOKEN_STRING_HOD;
                        token->string = buffer;

                        return ERR_OK;
                    }
                    else
                    {
                        ungetc(c, stdin);
                    }
                }
                else
                {
                    ungetc(c, stdin);
                }

                char *buffer = (char *)malloc(sizeof(char) * 2);

                if (buffer == NULL)
                    return ERR_INTERNAL;

                buffer[0] = '\0';
                token->type = TOKEN_STRING_HOD;
                token->string = buffer;

                return ERR_OK;
            }
        }

        // Tokeny pro operace
        if (c == '=')
        {
            if ((c = getchar()) != EOF && c == '=') // ==
            {
                token->type = TOKEN_STEJNO;
            }
            else
            {
                token->type = TOKEN_ROVNO;
                if (c != EOF)
                {
                    ungetc(c, stdin);
                }
            }
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
            if ((c = getchar()) != EOF && c == '=') // <=
            {
                token->type = TOKEN_MENSI_ROVNO;
            }
            else
            {
                token->type = TOKEN_MENSI;
                if (c != EOF)
                {
                    ungetc(c, stdin);
                }
            }
            return ERR_OK;
        }
        if (c == '>')
        {
            if ((c = getchar()) != EOF && c == '=') // >=
            {
                token->type = TOKEN_VETSI_ROVNO;
            }
            else
            {
                token->type = TOKEN_VETSI;
                if (c != EOF)
                {
                    ungetc(c, stdin);
                }
            }
            return ERR_OK;
        }
        if (c == '!')
        {
            if ((c = getchar()) != EOF && c == '=') // !=
            {
                token->type = TOKEN_NE_ROVNO;
                return ERR_OK;
            }
            else if (c != EOF)
            {
                ungetc(c, stdin);
                return ERR_LEX;
            }
        }

        return ERR_LEX;
    }

    // Token konce souboru
    token->type = TOKEN_EOF;
    return ERR_OK;
}