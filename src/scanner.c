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

static int current_line = 1;
static int interp_depth = 0; /* INTERPOLATEDS: hloubka vnoření do ${...}, 0 = mimo interpolaci */

/* INTERPOLATEDS: fronta hotových tokenů. Interpolovaný řetězec se rozloží na více tokenů, ty se vydávají postupně. */
static Token *pq = NULL;
static int pq_len = 0;
static int pq_pos = 0;

static int read_char()
{
    int c = getchar();
    if (c == '\n')
    {
        current_line++;
    }
    return c;
}
static void unread_char(int c, FILE *stream)
{
    if (c == '\n')
    {
        current_line--;
    }
    ungetc(c, stream);
}

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
            int next_c = read_char();
            if (!isdigit(next_c))
            {
                return ERR_LEX;
            }

            unread_char(next_c, stdin);
        }

        if (c == 'e')
        {
            (*ma_e)++;

            if ((*ma_e) > 1)
            {
                return ERR_LEX;
            }

            buffer[new_len++] = 'e';
            c = read_char();
            if (c == '+' || c == '-')
            {
                buffer[new_len++] = c;
                c = read_char();
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

            c = read_char();
            if (isalnum(c) || c == '_' || c == '.' || (*ma_d) > 1)
            {
                return ERR_LEX;
            }

            buffer[new_len++] = 'd';
            unread_char(c, stdin);
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
            int next_c = read_char();

            if (!isdigit(next_c))
            {
                return ERR_LEX;
            }

            unread_char(next_c, stdin);
        }

        if ((*tecka) > 1)
        {
            return ERR_LEX;
        }

        if (new_len < 255 && c != '_')
            buffer[new_len++] = c;
        c = read_char();
    }

    unread_char(c, stdin);
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

/* INTERPOLATEDS: přidá hotový token do dynamického pole (pole přebírá vlastnictví t.string) */
static int tok_push(Token **arr, int *n, int *cap, Token t)
{
    if (*n >= *cap)
    {
        int new_cap = (*cap == 0) ? 16 : *cap * 2;
        Token *tmp = (Token *)realloc(*arr, sizeof(Token) * new_cap);
        if (tmp == NULL)
        {
            return ERR_INTERNAL;
        }
        *arr = tmp;
        *cap = new_cap;
    }
    (*arr)[(*n)++] = t;
    return ERR_OK;
}

/* INTERPOLATEDS: vytvoří token zadaného typu (s kopií textu, pokud je zadán) a přidá ho do pole */
static int tok_add(Token **arr, int *n, int *cap, TokenType type, const char *str, int line)
{
    Token t;
    t.type = type;
    t.string = NULL;
    t.int_val = 0;
    t.double_val = 0;
    t.line = line;
    if (str != NULL)
    {
        t.string = str_dup(str);
        if (t.string == NULL)
        {
            return ERR_INTERNAL;
        }
    }
    return tok_push(arr, n, cap, t);
}

/* INTERPOLATEDS: přidá znak na konec dynamického bufferu, buffer je vždy ukončen znakem \0 */
static int buf_add(char **buf, int *len, int *cap, int c)
{
    if (*len + 1 >= *cap)
    {
        int new_cap = (*cap == 0) ? 32 : *cap * 2;
        char *tmp = (char *)realloc(*buf, new_cap);
        if (tmp == NULL)
        {
            return ERR_INTERNAL;
        }
        *buf = tmp;
        *cap = new_cap;
    }
    (*buf)[(*len)++] = (char)c;
    (*buf)[*len] = '\0';
    return ERR_OK;
}

/* INTERPOLATEDS: přečte interpolovaný řetězec s"..." nebo s"""...""" (úvodní s" už je přečteno).
 * Vyrobí tokeny ( "text" + toStr(hodnota) + "text" ... ), první vrátí a ostatní uloží do fronty. */
static int scan_interpolated(Token *token)
{
    Token *arr = NULL;
    int n = 0;
    int cap = 0;
    char *part = NULL;
    int plen = 0;
    int pcap = 0;
    int multi = 0;
    int closed = 0;
    int line = current_line;
    int rc = ERR_OK;
    int c;

    c = read_char();
    if (c == '"')
    {
        int c2 = read_char();
        if (c2 == '"')
        {
            multi = 1; /* s""" začíná víceřádkový řetězec */
        }
        else
        {
            unread_char(c2, stdin);
            closed = 1; /* s"" je prázdný řetězec */
        }
    }
    else
    {
        unread_char(c, stdin);
    }

    if ((rc = tok_add(&arr, &n, &cap, TOKEN_LZAVORKA, NULL, line)) != ERR_OK)
    {
        goto fail;
    }

    while (!closed)
    {
        c = read_char();
        if (c == EOF || (!multi && c == '\n'))
        {
            rc = ERR_LEX;
            goto fail;
        }

        if (c == '"')
        {
            if (!multi)
            {
                closed = 1;
                continue;
            }
            int c2 = read_char();
            if (c2 == '"')
            {
                int c3 = read_char();
                if (c3 == '"')
                {
                    closed = 1;
                    continue;
                }
                unread_char(c3, stdin);
            }
            unread_char(c2, stdin);
            /* jednotlivá uvozovka uvnitř víceřádkového řetězce je obyčejný znak */
        }
        else if (c == '\\' && !multi)
        {
            c = read_char();
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
                rc = ERR_LEX;
                goto fail;
            }
        }
        else if (c == '$')
        {
            int next = read_char();
            if (next == '$')
            {
                c = '$'; /* $$ je jeden znak dolaru */
            }
            else if (next == '{' || isalpha(next) || next == '_')
            {
                /* dosavadní text se vydá jako řetězec, pak následuje + toStr( ... ) + */
                if ((rc = tok_add(&arr, &n, &cap, TOKEN_STRING_HOD, part != NULL ? part : "", line)) != ERR_OK ||
                    (rc = tok_add(&arr, &n, &cap, TOKEN_PLUS, NULL, line)) != ERR_OK ||
                    (rc = tok_add(&arr, &n, &cap, TOKEN_ID, "toStr", line)) != ERR_OK ||
                    (rc = tok_add(&arr, &n, &cap, TOKEN_LZAVORKA, NULL, line)) != ERR_OK)
                {
                    goto fail;
                }
                plen = 0;
                if (part != NULL)
                {
                    part[0] = '\0';
                }

                if (next == '{')
                {
                    /* ${výraz}: tokeny výrazu se čtou, dokud scanner nevrátí TOKEN_INTERP_END */
                    interp_depth++;
                    while (1)
                    {
                        Token t;
                        int pk = read_char();
                        if (pk == EOF)
                        {
                            rc = ERR_LEX;
                            goto fail;
                        }
                        unread_char(pk, stdin);
                        rc = get_next_token(&t);
                        if (rc != ERR_OK)
                        {
                            goto fail;
                        }
                        if (t.type == TOKEN_INTERP_END)
                        {
                            break;
                        }
                        if (t.type == TOKEN_EOL)
                        {
                            free(t.string);
                            rc = ERR_LEX;
                            goto fail;
                        }
                        if ((rc = tok_push(&arr, &n, &cap, t)) != ERR_OK)
                        {
                            goto fail;
                        }
                    }
                    interp_depth--;
                }
                else
                {
                    /* $jméno: vloží se hodnota proměnné */
                    char name[256];
                    int k = 0;
                    name[k++] = (char)next;
                    while ((c = read_char()) != EOF && (isalnum(c) || c == '_'))
                    {
                        if (k < 255)
                        {
                            name[k++] = (char)c;
                        }
                    }
                    name[k] = '\0';
                    unread_char(c, stdin);
                    if ((rc = tok_add(&arr, &n, &cap, TOKEN_ID, name, line)) != ERR_OK)
                    {
                        goto fail;
                    }
                }

                if ((rc = tok_add(&arr, &n, &cap, TOKEN_PZAVORKA, NULL, line)) != ERR_OK ||
                    (rc = tok_add(&arr, &n, &cap, TOKEN_PLUS, NULL, line)) != ERR_OK)
                {
                    goto fail;
                }
                continue;
            }
            else
            {
                rc = ERR_LEX; /* za $ musí být $, { nebo jméno proměnné */
                goto fail;
            }
        }

        if ((rc = buf_add(&part, &plen, &pcap, c)) != ERR_OK)
        {
            goto fail;
        }
    }

    if ((rc = tok_add(&arr, &n, &cap, TOKEN_STRING_HOD, part != NULL ? part : "", line)) != ERR_OK ||
        (rc = tok_add(&arr, &n, &cap, TOKEN_PZAVORKA, NULL, line)) != ERR_OK)
    {
        goto fail;
    }

    free(part);
    *token = arr[0];
    pq = arr; /* zbylé tokeny se vydají z fronty při dalších voláních */
    pq_len = n;
    pq_pos = 1;
    return ERR_OK;

fail:
    free(part);
    for (int i = 0; i < n; i++)
    {
        free(arr[i].string);
    }
    free(arr);
    return rc;
}

int get_next_token(Token *token)
{
    int c;
    token->double_val = 0;
    token->int_val = 0;
    token->string = NULL;

    /* INTERPOLATEDS: nejdřív se vydají tokeny ze fronty */
    if (pq_pos < pq_len)
    {
        *token = pq[pq_pos++];
        if (pq_pos == pq_len)
        {
            free(pq);
            pq = NULL;
            pq_len = 0;
            pq_pos = 0;
        }
        return ERR_OK;
    }

    while ((c = read_char()) != EOF)
    {
        /* INTERPOLATEDS: uvnitř ${...} znak } ukončuje výraz */
        if (c == '}' && interp_depth > 0)
        {
            token->type = TOKEN_INTERP_END;
            token->line = current_line;
            return ERR_OK;
        }


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

        if (c == '\n' || c == '\r')
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
                int token_start_line = current_line;
                token->type = TOKEN_EOL;
                token->line = token_start_line;
                return ERR_OK;
            }

            continue;
        }

        zacatek = 0;

        if (c == '/')
        {
            c = read_char();

            // jednořádkový komentář
            if (c == '/')
            {
                while ((c = read_char()) != '\n' && c != EOF)
                    ;

                if (c == '\n')
                {
                    unread_char(c, stdin);
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
                    c = read_char();
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
                        int token_start_line = current_line;
                        token->type = TOKEN_EOL;
                        token->line = token_start_line;
                        return ERR_OK;
                    }
                }

                continue;
            }
            else
            {
                if (c != EOF)
                {
                    unread_char(c, stdin);
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
            unread_char(c, stdin);
            letter = 2;
            odsazeni = space;

            if (tab != 0)
                return ERR_LEX;

            if (space == 0)
                continue;

            token->type = TOKEN_ODSAZENI;
            token->line = current_line;
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

            while ((c = read_char()) != EOF && (isalnum(c) || c == '_'))
            {
                if (len < 255)
                    buffer[len++] = c;
            }
            buffer[len] = '\0';
            unread_char(c, stdin);

            /* INTERPOLATEDS: identifikátor s přímo před uvozovkou je interpolovaný řetězec s"..." */
            if (len == 1 && buffer[0] == 's' && c == '"')
            {
                read_char(); /* přečte uvozovku, kterou jsme před chvílí vrátili */
                return scan_interpolated(token);
            }

            if (strcmp(buffer, "while") == 0)
            {
                token->type = TOKEN_WHILE;
                token->line = current_line;
            }
            else if (strcmp(buffer, "extension") == 0) /* EXTKW */
            {
                token->type = TOKEN_EXTENSION;
                token->line = current_line;
            }
            else if (strcmp(buffer, "for") == 0) /* CONTROL */
            {
                token->type = TOKEN_FOR;
                token->line = current_line;
            }
            else if (strcmp(buffer, "do") == 0)
            {
                token->type = TOKEN_DO;
                token->line = current_line;
            }
            else if (strcmp(buffer, "if") == 0)
            {
                token->type = TOKEN_IF;
                token->line = current_line;
            }
            else if (strcmp(buffer, "else") == 0)
            {
                token->type = TOKEN_ELSE;
                token->line = current_line;
            }
            else if (strcmp(buffer, "var") == 0)
            {
                token->type = TOKEN_VAR;
                token->line = current_line;
            }
            else if (strcmp(buffer, "val") == 0)
            {
                token->type = TOKEN_VAL;
                token->line = current_line;
            }
            else if (strcmp(buffer, "import") == 0)
            {
                token->type = TOKEN_IMPORT;
                token->line = current_line;
            }
            else if (strcmp(buffer, "return") == 0)
            {
                token->type = TOKEN_RETURN;
                token->line = current_line;
            }
            else if (strcmp(buffer, "then") == 0)
            {
                token->type = TOKEN_THEN;
                token->line = current_line;
            }
            else if (strcmp(buffer, "Int") == 0)
            {
                token->type = TOKEN_INT;
                token->line = current_line;
            }
            else if (strcmp(buffer, "Double") == 0)
            {
                token->type = TOKEN_DOUBLE;
                token->line = current_line;
            }
            else if (strcmp(buffer, "String") == 0)
            {
                token->type = TOKEN_STRING;
                token->line = current_line;
            }
            else if (strcmp(buffer, "Unit") == 0)
            {
                token->type = TOKEN_UNIT;
                token->line = current_line;
            }
            else if (strcmp(buffer, "def") == 0)
            {
                token->type = TOKEN_DEF;
                token->line = current_line;
            }
            else if (strcmp(buffer, "Boolean") == 0)
            {
                token->type = TOKEN_BOOLEAN;
                token->line = current_line;
            }
            else if (strcmp(buffer, "true") == 0)
            {
                token->type = TOKEN_TRUE_HOD;
                token->line = current_line;
            }
            else if (strcmp(buffer, "false") == 0)
            {
                token->type = TOKEN_FALSE_HOD;
                token->line = current_line;
            }
            else
            {
                if (len == 1 && buffer[0] == '_')
                    return ERR_LEX;

                token->type = TOKEN_ID;
                token->line = current_line;
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
                int next_c = read_char();

                if (isdigit(next_c) || next_c == '_')
                {
                    return ERR_LEX;
                }

                unread_char(next_c, stdin);
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
                token->line = current_line;
                token->double_val = strtod(buffer, NULL);
            }
            else
            {
                token->type = TOKEN_INT_HOD;
                token->line = current_line;
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
            if ((c = read_char()) != '"')
            {
                unread_char(c, stdin);
                while ((c = read_char()) != '"')
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
                        c = read_char();

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
                token->line = current_line;
                token->string = buffer;

                return ERR_OK;
            }
            // Více řádkový řetězec
            else
            {
                if ((c = read_char()) == '"')
                {

                    while (1)
                    {
                        c = read_char();
                        if (c == EOF)
                        {
                            free(buffer);
                            return ERR_LEX;
                        }

                        if (c == '"')
                        {
                            int c2 = read_char();
                            if (c2 == '"')
                            {
                                int c3 = read_char();
                                if (c3 == '"')
                                {
                                    break;
                                }
                                unread_char(c3, stdin);
                            }
                            unread_char(c2, stdin);
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
                    token->line = current_line;
                    token->string = buffer;

                    return ERR_OK;
                }
                else
                {
                    unread_char(c, stdin);
                }

                buffer[0] = '\0';
                token->type = TOKEN_STRING_HOD;
                token->line = current_line;
                token->string = buffer;

                return ERR_OK;
            }
        }

        // Tokeny pro operace
        if (c == '=')
        {
            if ((c = read_char()) != EOF && c == '=') // ==
            {
                token->type = TOKEN_STEJNO;
                token->line = current_line;
            }
            else
            {
                token->type = TOKEN_ROVNO;
                token->line = current_line;
                if (c != EOF)
                {
                    unread_char(c, stdin);
                }
            }
            return ERR_OK;
        }
        if (c == '+')
        {
            token->type = TOKEN_PLUS;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == '-')
        {
            token->type = TOKEN_MINUS;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == '*')
        {
            token->type = TOKEN_NAS;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == '/')
        {
            token->type = TOKEN_DEL;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == '(')
        {
            zavorky++;
            token->type = TOKEN_LZAVORKA;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == ')')
        {
            if (zavorky > 0)
                zavorky--;
            token->type = TOKEN_PZAVORKA;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == ':')
        {
            token->type = TOKEN_DVOJTEC;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == '.')
        {
            c = read_char();
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
                token->line = current_line;

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
                unread_char(c, stdin);
                token->type = TOKEN_TECKA;
                token->line = current_line;
            }

            return ERR_OK;
        }
        if (c == ',')
        {
            token->type = TOKEN_CARKA;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == ';')
        {
            token->type = TOKEN_STREDNIK;
            token->line = current_line;
            return ERR_OK;
        }
        if (c == '@')
        {
            token->type = TOKEN_ZAVINAC;
            token->line = current_line;
            return ERR_OK;
        }

        // Tokeny pro porovnávání
        if (c == '<')
        {
            if ((c = read_char()) != EOF && c == '=') // <=
            {
                token->type = TOKEN_MENSI_ROVNO;
                token->line = current_line;
            }
            else
            {
                token->type = (c == '-') ? TOKEN_LARROW : TOKEN_MENSI; /* CONTROL: šipka <- */
                token->line = current_line;
                if (c != EOF && c != '-')
                {
                    unread_char(c, stdin);
                }
            }
            return ERR_OK;
        }
        if (c == '>')
        {
            if ((c = read_char()) != EOF && c == '=') // >=
            {
                token->type = TOKEN_VETSI_ROVNO;
                token->line = current_line;
            }
            else
            {
                token->type = TOKEN_VETSI;
                token->line = current_line;
                if (c != EOF)
                {
                    unread_char(c, stdin);
                }
            }
            return ERR_OK;
        }
        if (c == '!')
        {
            if ((c = read_char()) != EOF && c == '=') // !=
            {
                token->type = TOKEN_NE_ROVNO;
                token->line = current_line;
                return ERR_OK;
            }
            else
            {
                if (c != EOF)
                {
                    unread_char(c, stdin);
                }
                token->type = TOKEN_NOT; //BOOLTHEN
                token->line = current_line;
                return ERR_OK;
            }
        }
        if (c == '&') //BOOLTHEN
        {
            if ((c = read_char()) != EOF && c == '&') // &&
            {
                token->type = TOKEN_AND;
                token->line = current_line;
                return ERR_OK;
            }
            return ERR_LEX;
        }
        if (c == '|') //BOOLTHEN
        {
            if ((c = read_char()) != EOF && c == '|') // ||
            {
                token->type = TOKEN_OR;
                token->line = current_line;
                return ERR_OK;
            }
            return ERR_LEX;
        }

        return ERR_LEX;
    }

    if (letter > 0)
    {
        letter = 0;
        int token_start_line = current_line;
        token->type = TOKEN_EOL;
        token->line = token_start_line;
        return ERR_OK;
    }

    // Token konce souboru
    token->type = TOKEN_EOF;
    token->line = current_line;
    return ERR_OK;
}