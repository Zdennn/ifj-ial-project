/**
 * @file scanner.c
 * @brief Implementace lexikálního analyzátoru.
 *
 * Čte vstup po znacích ze stdin a převádí je na tokeny, které předává syntaktickému analyzátoru (parser).
 *
 * @author Zdeněk Michalec, Jonáš Javůrek, Tadeáš Trynkl
 */
#include "scanner.h"
#include "error.h"
#include "token.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *str_dup(const char *src)
{
    int len = (int)strlen(src);
    char *copy = (char *)malloc(len + 1);
    if (copy == NULL)
        return NULL;
    memcpy(copy, src, len + 1);
    return copy;
}

static int pruchod_cislo(int c, char *buffer, int *len, int *ma_e, int *ma_d, int *tecka)
{
    int new_len = (*len);
    while (c != EOF && (isdigit(c) || c == '_' || c == '.' || tolower(c) == 'e' || tolower(c) == 'd'))
    {
        if (new_len >= 250)
        {
            return ERR_LEX;
        }

        c = tolower(c);

        if (c == '_')
        {
            int next_c = getchar();
            if (!isdigit(next_c))
            {
                return ERR_LEX;
            }

            ungetc(next_c, stdin);
        }

        if (c == 'e')
        {
            (*ma_e)++;

            if ((*ma_e) > 1)
            {
                return ERR_LEX;
            }

            buffer[new_len++] = 'e';
            c = getchar();
            if (c == '+' || c == '-')
            {
                buffer[new_len++] = c;
                c = getchar();
                if (!isdigit(c))
                {
                    return ERR_LEX;
                }
            }
            else if (!isdigit(c))
            {
                return ERR_LEX;
            }
        }

        if (c == 'd')
        {
            (*ma_d)++;

            c = getchar();
            if (isalnum(c) || c == '_' || c == '.' || (*ma_d) > 1)
            {
                return ERR_LEX;
            }

            buffer[new_len++] = 'd';
            ungetc(c, stdin);
            *len = new_len;

            return ERR_OK;
        }

        if (c == '.')
        {
            if ((*ma_e) > 0)
            {
                return ERR_LEX;
            }

            (*tecka)++;
            int next_c = getchar();

            if (!isdigit(next_c))
            {
                return ERR_LEX;
            }

            ungetc(next_c, stdin);
        }

        if ((*tecka) > 1)
        {
            return ERR_LEX;
        }

        if (new_len < 255 && c != '_')
            buffer[new_len++] = c;
        c = getchar();
    }

    ungetc(c, stdin);
    *len = new_len;

    return ERR_OK;
}

static int letter = 0;
static int space = 0;
static int tab = 0;
static int zacatek = 1;
static int zavorky = 0;
static int odsazeni = 0;
static int kontrola = 0;
static int po_kom = 0;

void scanner_reset()
{
    letter = 0;
    space = 0;
    tab = 0;
    zacatek = 1;
    zavorky = 0;
    odsazeni = 0;
    kontrola = 0;
    po_kom = 0;
}

int get_next_token(Token *token)
{
    int c;
    token->double_val = 0;
    token->int_val = 0;
    token->string = NULL;

    while ((c = getchar()) != EOF)
    {

        if (c == ' ')
        {
            if (zacatek)
            {
                space++;
            }

            continue;
        }

        if (c == '\t')
        {
            if (zacatek)
            {
                tab++;
            }

            continue;
        }

        if (c == '\n')
        {
            // jen white space
            if (zavorky > 0)
            {
                space = 0;
                tab = 0;
                zacatek = 1;
                kontrola = 1;
                continue;
            }

            int byl_token = letter;
            space = 0;
            letter = 0;
            tab = 0;
            zacatek = 1;
            kontrola = 0;
            po_kom = 0;

            if (byl_token > 0)
            {
                token->type = TOKEN_EOL;
                return ERR_OK;
            }

            continue;
        }

        zacatek = 0;

        if (c == '/')
        {
            c = getchar();

            // jednořádkový komentář
            if (c == '/')
            {
                while ((c = getchar()) != '\n' && c != EOF)
                    ;

                if (c == '\n')
                {
                    ungetc(c, stdin);
                }

                continue;
            }
            // víceřádkový komentář
            else if (c == '*')
            {
                int zanoreni = 1;
                int prev_c = 0;
                int radek = 0;

                while (zanoreni > 0)
                {
                    c = getchar();
                    if (c == EOF)
                    {
                        return ERR_LEX;
                    }

                    if (c == '\n')
                    {
                        radek = 1;
                    }

                    if (prev_c == '/' && c == '*')
                    {
                        zanoreni++;
                        prev_c = c;
                    }
                    else if (prev_c == '*' && c == '/')
                    {
                        zanoreni--;
                        prev_c = 0;
                    }
                    else
                    {
                        prev_c = c;
                    }
                }

                if (radek == 1 && zavorky == 0)
                {
                    int byl_token = letter;
                    space = 0;
                    letter = 0;
                    tab = 0;
                    zacatek = 1;
                    kontrola = 0;
                    po_kom = 1;

                    if (byl_token > 0)
                    {
                        token->type = TOKEN_EOL;
                        return ERR_OK;
                    }
                }

                continue;
            }
            else
            {
                if (c != EOF)
                {
                    ungetc(c, stdin);
                }
                c = '/';
            }
        }

        if (po_kom)
            return ERR_LEX;

        if (kontrola)
        {
            kontrola = 0;
            if (c != ')' && space <= odsazeni)
                return ERR_LEX;
        }

        letter++;
        if (letter == 1)
        {
            ungetc(c, stdin);
            letter = 2;
            odsazeni = space;

            if (tab != 0)
                return ERR_LEX;

            if (space == 0)
                continue;

            token->type = TOKEN_ODSAZENI;
            token->int_val = space;
            space = 0;
            return ERR_OK;
        }

        // Tokeny pro identifikátory nebo while, if...
        if (isalpha(c) || c == '_')
        {
            char buffer[256];
            int len = 0;

            if (len < 255)
                buffer[len++] = c;

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
                if (len == 1 && buffer[0] == '_')
                    return ERR_LEX;

                token->type = TOKEN_ID;
                token->string = str_dup(buffer);

                if (token->string == NULL)
                {
                    return ERR_INTERNAL;
                }
            }
            return ERR_OK;
        }

        // Tokeny pro čísla
        if (isdigit(c))
        {
            char buffer[256];
            int len = 0;
            int ma_e = 0;
            int ma_d = 0;

            if (c == '0')
            {
                int next_c = getchar();

                if (isdigit(next_c) || next_c == '_')
                {
                    return ERR_LEX;
                }

                ungetc(next_c, stdin);
            }

            int tecka = 0;

            int status = pruchod_cislo(c, buffer, &len, &ma_e, &ma_d, &tecka);

            if (status != ERR_OK)
            {
                return status;
            }

            buffer[len] = '\0';

            if (ma_e == 1 || tecka == 1 || ma_d == 1)
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
                        free(buffer);
                        return ERR_LEX;
                    }

                    if (c == '\n')
                    {
                        free(buffer);
                        return ERR_LEX;
                    }

                    if (c == '\\')
                    {
                        c = getchar();

                        if (c == EOF)
                        {
                            free(buffer);
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
                            free(buffer);
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
                            free(buffer);
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

                    while (1)
                    {
                        c = getchar();
                        if (c == EOF)
                        {
                            free(buffer);
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

                        if (len >= size)
                        {
                            size = size * 2;
                            char *new_buff =
                                (char *)realloc(buffer, sizeof(char) * (size + 1)); // +1 kvuli ukončení řetezce \0
                            if (new_buff == NULL)
                            {
                                free(buffer);
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
            zavorky++;
            token->type = TOKEN_LZAVORKA;
            return ERR_OK;
        }
        if (c == ')')
        {
            if (zavorky > 0)
                zavorky--;
            token->type = TOKEN_PZAVORKA;
            return ERR_OK;
        }
        if (c == ':')
        {
            token->type = TOKEN_DVOJTEC;
            return ERR_OK;
        }
        if (c == '.')
        {
            c = getchar();
            int tecka = 1;
            char buffer[256];
            int len = 0;
            int ma_e = 0;
            int ma_d = 0;

            buffer[len++] = '.';

            // Desetinné číslo
            if (isdigit(c))
            {
                token->type = TOKEN_DOUBLE_HOD;

                int status = pruchod_cislo(c, buffer, &len, &ma_e, &ma_d, &tecka);

                if (status != ERR_OK)
                {
                    return status;
                }

                buffer[len] = '\0';
                token->double_val = strtod(buffer, NULL);
            }
            else
            {
                ungetc(c, stdin);
                token->type = TOKEN_TECKA;
            }

            return ERR_OK;
        }
        if (c == ',')
        {
            token->type = TOKEN_CARKA;
            return ERR_OK;
        }
        if (c == ';')
        {
            token->type = TOKEN_STREDNIK;
            return ERR_OK;
        }
        if (c == '@')
        {
            token->type = TOKEN_ZAVINAC;
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

    if (letter > 0)
    {
        letter = 0;
        token->type = TOKEN_EOL;
        return ERR_OK;
    }

    // Token konce souboru
    token->type = TOKEN_EOF;
    return ERR_OK;
}