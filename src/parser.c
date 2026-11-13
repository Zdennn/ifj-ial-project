/**
 * @file parser.c
 * @brief Syntaktický analizátor.
 *
 * Část programu oveřující, zda vstupní tokeny odpovidají syntaktické a sémantické struktuře jazyka IFJ26.
 *
 * Řesenou pomocí metody rekurzivního sestupu a precedenční syntaktickou analýzu
 *
 * @author Zdeněk Michalec
 */
#include "parser.h"
#include "error.h"
#include "scanner.h"
#include "token.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Token current_token;
const char *getTokenName(TokenType type)
{
    static const char *TokenNames[] = {[TOKEN_EOF] = "TOKEN_EOF",
                                       [TOKEN_EOL] = "TOKEN_EOL",
                                       [TOKEN_ID] = "TOKEN_ID",
                                       [TOKEN_INT] = "TOKEN_INT",
                                       [TOKEN_DOUBLE] = "TOKEN_DOUBLE",
                                       [TOKEN_STRING] = "TOKEN_STRING",
                                       [TOKEN_DEF] = "TOKEN_DEF",
                                       [TOKEN_DO] = "TOKEN_DO",
                                       [TOKEN_WHILE] = "TOKEN_WHILE",
                                       [TOKEN_IF] = "TOKEN_IF",
                                       [TOKEN_ELSE] = "TOKEN_ELSE",
                                       [TOKEN_VAR] = "TOKEN_VAR",
                                       [TOKEN_VAL] = "TOKEN_VAL",
                                       [TOKEN_IMPORT] = "TOKEN_IMPORT",
                                       [TOKEN_RETURN] = "TOKEN_RETURN",
                                       [TOKEN_THEN] = "TOKEN_THEN",
                                       [TOKEN_UNIT] = "TOKEN_UNIT",
                                       [TOKEN_STRING_HOD] = "TOKEN_STRING_HOD",
                                       [TOKEN_INT_HOD] = "TOKEN_INT_HOD",
                                       [TOKEN_DOUBLE_HOD] = "TOKEN_DOUBLE_HOD",
                                       [TOKEN_ODSAZENI] = "TOKEN_ODSAZENI",
                                       [TOKEN_ROVNO] = "TOKEN_ROVNO",
                                       [TOKEN_PLUS] = "TOKEN_PLUS",
                                       [TOKEN_MINUS] = "TOKEN_MINUS",
                                       [TOKEN_NAS] = "TOKEN_NAS",
                                       [TOKEN_DEL] = "TOKEN_DEL",
                                       [TOKEN_LZAVORKA] = "TOKEN_LZAVORKA",
                                       [TOKEN_PZAVORKA] = "TOKEN_PZAVORKA",
                                       [TOKEN_DVOJTEC] = "TOKEN_DVOJTEC",
                                       [TOKEN_TECKA] = "TOKEN_TECKA",
                                       [TOKEN_CARKA] = "TOKEN_CARKA",
                                       [TOKEN_STREDNIK] = "TOKEN_STREDNIK",
                                       [TOKEN_ZAVINAC] = "TOKEN_ZAVINAC",
                                       [TOKEN_MENSI] = "TOKEN_MENSI",
                                       [TOKEN_VETSI] = "TOKEN_VETSI",
                                       [TOKEN_MENSI_ROVNO] = "TOKEN_MENSI_ROVNO",
                                       [TOKEN_VETSI_ROVNO] = "TOKEN_VETSI_ROVNO",
                                       [TOKEN_NE_ROVNO] = "TOKEN_NE_ROVNO",
                                       [TOKEN_STEJNO] = "TOKEN_STEJNO"};

    if (type >= 0 && type <= TOKEN_STEJNO)
    {
        return TokenNames[type];
    }
    return "NEZNAMY_TOKEN";
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorCodes get_n_token()
{
    ErrorCodes error = get_next_token(&current_token);
    if (error)
    {
        printError(error);
        return error;
    }
    return ERR_OK;
}

Token match(TokenType expected)
{
    printf("current: %s, expected: %s, line: %d\n", getTokenName(current_token.type), getTokenName(expected),
           current_token.line);
    if (current_token.type == expected)
    {
        Token old_token = current_token;
        get_n_token();
        return old_token;
    }
    else
    {
        printError(ERR_SYN);
    }
    return (Token){0};
}
void match_type()
{
    switch (current_token.type)
    {
    case TOKEN_INT:
        match(TOKEN_INT);
        break;
    case TOKEN_DOUBLE:
        match(TOKEN_DOUBLE);
        break;
    case TOKEN_STRING:
        match(TOKEN_STRING);
        break;
    case TOKEN_UNIT:
        match(TOKEN_UNIT);
        break;
    default:
        printError(ERR_SYN);
        break;
    }
}

void parse_block()
{

    int pocet_odsazeni = current_token.int_val;

    printf("parse_block\n");

    while (current_token.type != TOKEN_EOF)
    {
        if (current_token.type == TOKEN_EOL)
        {
            match(TOKEN_EOL);
            continue;
        }
        if (current_token.type == TOKEN_ODSAZENI)
        {

            if (current_token.int_val < pocet_odsazeni)
            {
                // Konec bloku
                return;
            }
            else if (current_token.int_val > pocet_odsazeni)
            {
                printError(ERR_SYN);
            }
            if (current_token.int_val == pocet_odsazeni)
            {
                // Pokracuje ve stejnem bloku
                match(TOKEN_ODSAZENI);
            }
        }
        else
        {
            if (pocet_odsazeni > 0)
            {
                // Konec bloku, radek nezacina odsazenim, ale jsem v v odsazenem bloku
                return;
            }
        }

        Token id_token;

        switch (current_token.type)
        {
        case TOKEN_VAR:
            match(TOKEN_VAR);
            id_token = match(TOKEN_ID);
            if (current_token.type == TOKEN_DVOJTEC)
            {
                match(TOKEN_DVOJTEC);
                match_type();
            }
            match(TOKEN_ROVNO);
            parse_expression();

            break;
        case TOKEN_VAL:
            match(TOKEN_VAL);
            id_token = match(TOKEN_ID);
            if (current_token.type == TOKEN_DVOJTEC)
            {
                match(TOKEN_DVOJTEC);
                match_type();
            }
            match(TOKEN_ROVNO);
            match(TOKEN_ID);
            if (current_token.type == TOKEN_LZAVORKA)
            {
                // Volání funkce
                match(TOKEN_LZAVORKA);
                parse_expression();
                match(TOKEN_PZAVORKA);
            }
            else
            {
                parse_expression();
            }

            break;
        case TOKEN_IF:
            match(TOKEN_IF);
            parse_expression();
            match(TOKEN_THEN);
            match(TOKEN_EOL);
            // Then musí mít větší odsazení než if
            if (current_token.type != TOKEN_ODSAZENI || current_token.int_val <= pocet_odsazeni)
            {
                printError(ERR_SYN);
            }
            parse_block();
            // Else musí mít stejné odsazení jako if!
            if (current_token.type != TOKEN_ODSAZENI || current_token.int_val != pocet_odsazeni)
            {
                printError(ERR_SYN);
            }
            match(TOKEN_ODSAZENI);
            match(TOKEN_ELSE);
            match(TOKEN_EOL);
            // Else musí mít větší odsazení než if/else
            if (current_token.type != TOKEN_ODSAZENI || current_token.int_val <= pocet_odsazeni)
            {
                printError(ERR_SYN);
            }
            parse_block();

            break;
        case TOKEN_WHILE:
            match(TOKEN_WHILE);
            parse_expression(); // Podmínka
            match(TOKEN_DO);
            match(TOKEN_EOL);
            // Musí být odsazeni s vyšším počtem mezer
            if (current_token.type != TOKEN_ODSAZENI || current_token.int_val <= pocet_odsazeni)
            {
                printError(ERR_SYN);
            }

            parse_block();
            break;

        case TOKEN_ID:
            id_token = match(TOKEN_ID);
            if (current_token.type == TOKEN_ROVNO)
            {
                match(TOKEN_ROVNO);
                parse_expression();
            }
            else if (current_token.type == TOKEN_LZAVORKA)
            {
                // Volání funkce
                match(TOKEN_LZAVORKA);
                parse_expression();
                match(TOKEN_PZAVORKA);
            }
            else
            {
                // Konec bloku
                parse_expression();
            }

            break;
        case TOKEN_RETURN:
            match(TOKEN_RETURN);
            if (current_token.type != TOKEN_EOL && current_token.type != TOKEN_EOF)
            {
                parse_expression();
            }
            break;

        default:
            printError(ERR_SYN);
            break;
        }

        if (current_token.type == TOKEN_EOL)
        {
            match(TOKEN_EOL);
        }
    }
}

void read_file_by_tokens()
{
    get_n_token();

    while (current_token.type != TOKEN_EOF)
    {
        printf("Token name: %s, line: %d, int val: %d, double val: %f, string val: %s\n",
               getTokenName(current_token.type), current_token.line, current_token.int_val, current_token.double_val,
               current_token.string);
        get_n_token();
    }
    printf("Token name: %s, line: %d\n", getTokenName(current_token.type), current_token.line);

    printError(ERR_OK);
}

int parse()
{
    // read_file_by_tokens();

    // Získání prvního tokenu
    get_n_token();
    // Zpracování: import ifj26.*
    match(TOKEN_IMPORT);
    Token id = match(TOKEN_ID);
    if (strcmp(id.string, "ifj26") != 0)
    {
        printError(ERR_SYN);
    }
    match(TOKEN_TECKA);
    match(TOKEN_NAS);
    match(TOKEN_EOL);

    while (current_token.type != TOKEN_EOF)
    {
        parse_function();
    }

    return ERR_OK;
}

void parse_parameters()
{
    if (current_token.type == TOKEN_PZAVORKA)
    {
        return;
    }
    match(TOKEN_ID);
    match(TOKEN_DVOJTEC);
    match_type();

    if (current_token.type == TOKEN_CARKA)
    {
        int comma_line = current_token.line;
        match(TOKEN_CARKA);

        if (current_token.type == TOKEN_PZAVORKA)
        {
            if (current_token.line == comma_line)
            {
                printError(ERR_SYN);
            }
            return;
        }
        if (current_token.type == TOKEN_ID)
        {
            parse_parameters();
        }
        else
        {
            printError(ERR_SYN);
        }
    }
    else if (current_token.type == TOKEN_PZAVORKA)
    {
        return;
    }
    else
    {
        printError(ERR_SYN);
    }
}

void parse_expression()
{
    printf("EXPRESION\n");
    while (current_token.type != TOKEN_EOL && current_token.type != TOKEN_DO && current_token.type != TOKEN_THEN &&
           current_token.type != TOKEN_CARKA && current_token.type != TOKEN_PZAVORKA && current_token.type != TOKEN_EOF)
    {
        get_n_token();
    }
}

void parse_function()
{
    bool main_flag = false;

    // TODO Anotace může být na samostatném řádku nebo na stejném řádku s klíčovým slovem def
    if (current_token.type == TOKEN_ZAVINAC)
    {
        match(TOKEN_ZAVINAC);
        if (current_token.type == TOKEN_ID && current_token.string != NULL && strcmp(current_token.string, "main") == 0)
        {
            match(TOKEN_ID);
            main_flag = true;
        }
        else
        {
            printError(ERR_SYN);
        }
    }

    match(TOKEN_DEF);
    match(TOKEN_ID);
    match(TOKEN_LZAVORKA);

    if (main_flag && current_token.type == TOKEN_ID)
    {
        printError(ERR_SEM_ARG);
    }
    parse_parameters();

    match(TOKEN_PZAVORKA);

    // TODO Více vstupních funkcí na chybu 4. ASI AŽ VE STROMU
    match(TOKEN_DVOJTEC);
    if (main_flag && current_token.type != TOKEN_UNIT)
    {
        printError(ERR_SEM_TYPE);
    }
    match_type();
    match(TOKEN_ROVNO);

    if (current_token.type == TOKEN_EOL)
    {
        // Odsazený blok
        match(TOKEN_EOL);

        parse_block();
    }
    else
    {
        // Jednořádková funkce
        parse_expression();

        if (current_token.type == TOKEN_EOL)
        {
            match(TOKEN_EOL);
        }
    }
}
