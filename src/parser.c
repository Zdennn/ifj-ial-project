/**
 * @file parser.c
 * @brief Syntaktický analizátor.
 *
 * Část programu oveřující, zda vstupní tokeny odpovidají syntaktické struktuře jazyka IFJ26.
 *
 * Řesenou pomocí metody rekurzivního sestupu
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

int parse()
{
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

    parse_function();

    return ERR_OK;
}

// void parse_parameters()
// {
//     if (current_token.type == TOKEN_PZAVORKA)
//     {
//         return;
//     }
//     match(TOKEN_ID);
//     match(TOKEN_DVOJTEC);
//     match_type();
//     if (current_token.type == TOKEN_CARKA)
//     {
//         match(TOKEN_CARKA);
//         if (current_token.type == TOKEN_LZAVORKA)
//         {
//             printError(ERR_SYN);
//         }

//         parse_parameters();
//     }
// }
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
}

void parse_statement()
{
    switch (current_token.type)
    {
    case TOKEN_IF:
        match(TOKEN_IF);
        match(TOKEN_ID);
        parse_expression();
        match(TOKEN_THEN);
        break;

    default:
        printError(ERR_SYN);
        break;
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
    int comma_line = current_token.line;
    match(TOKEN_LZAVORKA);

    if (current_token.line != comma_line)
    {
        // Víceřádkové parametry
        // def foo(
        //   a: Int,
        //   b: String, // může být koncová čárka
        // ): Unit = ()
        if (main_flag && current_token.type == TOKEN_ID)
        {
            printError(ERR_SEM_ARG);
        }
        parse_parameters();
    }
    else
    {
        // Jednořádkové parametry
        // def foo( a: Int, b: String ): Unit = ()
        if (main_flag && current_token.type == TOKEN_ID)
        {
            printError(ERR_SEM_ARG);
        }
        parse_parameters();
    }

    match(TOKEN_PZAVORKA);

    // if (current_token.type == TOKEN_DVOJTEC)
    //{
    // TODO Více vstupních funkcí na chybu 4. ASI AŽ VE STROMU
    match(TOKEN_DVOJTEC);
    if (main_flag && current_token.type != TOKEN_UNIT)
    {
        printError(ERR_SEM_TYPE);
    }
    match_type();
    //}
    match(TOKEN_ROVNO);

    if (current_token.type == TOKEN_EOL)
    {
        match(TOKEN_EOL);
        match(TOKEN_ODSAZENI);
        // TODO: parse_block()
        printf("current: %s\n", getTokenName(current_token.type));
    }
    else
    {
        parse_expression();
    }
}
