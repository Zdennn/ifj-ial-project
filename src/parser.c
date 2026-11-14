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
#include "symtable.h"
#include "token.h"
#include "token_array.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO main def run() : Unit = () ====> parser nevezme druhou )

Token current_token;
TokenArray tokens;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

int token_index = 0;
void get_next_token_from_array()
{
    // printf("token index: %d\n", token_index);
    if (token_index < tokens.count)
    {
        current_token = tokens.items[token_index++];
        // printf("current: %s\n", getTokenName(current_token.type));
    }
    else
    {
        current_token = tokens.items[tokens.count - 1];
    }
}
void reset_parser_position()
{
    token_index = 0;
}

Token match(TokenType expected)
{
    printf("current: %s, expected: %s, line: %d\n", getTokenName(current_token.type), getTokenName(expected),
           current_token.line);
    if (current_token.type == expected)
    {
        Token old_token = current_token;
        get_next_token_from_array();
        return old_token;
    }
    else
    {
        printError(ERR_SYN);
    }
    return (Token){0};
}

Token match_type()
{
    switch (current_token.type)
    {
    case TOKEN_INT:
        return match(TOKEN_INT);
        break;
    case TOKEN_DOUBLE:
        return match(TOKEN_DOUBLE);
        break;
    case TOKEN_STRING:
        return match(TOKEN_STRING);
        break;
    case TOKEN_UNIT:
        return match(TOKEN_UNIT);
        break;
    default:
        printError(ERR_SYN);
        return (Token){0};
        break;
    }
}
DataType token_to_dataType(Token token)
{
    switch (token.type)
    {
    case TOKEN_INT:
        return TYPE_INT;
        break;
    case TOKEN_DOUBLE:
        return TYPE_DOUBLE;
        break;
    case TOKEN_STRING:
        return TYPE_STRING;
        break;
    case TOKEN_UNIT:
        return TYPE_UNIT;
        break;
    default:
        printError(ERR_SYN);
        return (DataType){0};
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

        Token token_id;
        Token token_type = {0};

        switch (current_token.type)
        {
        case TOKEN_VAR:
            match(TOKEN_VAR);
            token_id = match(TOKEN_ID);

            if (symtable_lookup_current(token_id.string) != NULL)
            {
                printError(ERR_SEM_REDEF);
            }

            if (current_token.type == TOKEN_DVOJTEC)
            {
                match(TOKEN_DVOJTEC);
                token_type = match_type();
            }
            match(TOKEN_ROVNO);
            DataType type_of_expresion = parse_expression();
            DataType final_var_type;
            if (token_type.type == TOKEN_EOF) // token_type = {0}, typ nebyl uveden
            {
                final_var_type = type_of_expresion;
            }
            else // token_type != {0}, typ byl uveden
            {
                DataType declared_type = token_to_dataType(token_type);

                if (type_of_expresion == declared_type)
                {
                    final_var_type = declared_type;
                }
                else if (type_of_expresion == TYPE_INT && declared_type == TYPE_DOUBLE)
                {
                    final_var_type = TYPE_DOUBLE;
                }
                else
                {
                    printError(ERR_SEM_TYPE);
                }
            }
            symtable_insert_var(token_id.string, final_var_type, true);

            break; ////////////////////////////////////////////////////////// TODO TADY POKRACOVAT
        case TOKEN_VAL:
            match(TOKEN_VAL);
            token_id = match(TOKEN_ID);
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
            token_id = match(TOKEN_ID);
            if (current_token.type == TOKEN_ROVNO)
            {
                match(TOKEN_ROVNO);
                parse_expression();
            }
            else if (current_token.type == TOKEN_LZAVORKA)
            {
                // Volání funkce
                if (symtable_lookup(token_id.string) == NULL)
                {
                    printError(ERR_SEM_UNDEF);
                }
                // TODO sedí parametry (chyba 5) a jaký typ funkce vrací (chyba 6)??
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
    token_array_init(&tokens);
    get_n_token();

    while (current_token.type != TOKEN_EOF)
    {
        // printf("Token name: %s, line: %d, int val: %d, double val: %f, string val: %s\n",
        //        getTokenName(current_token.type), current_token.line, current_token.int_val, current_token.double_val,
        //        current_token.string);
        token_array_append(&tokens, current_token);
        get_n_token();
    }
    // printf("Token name: %s, line: %d\n", getTokenName(current_token.type), current_token.line);
    token_array_append(&tokens, current_token);
}

void beginning()
{
    // Získání prvního tokenu
    get_next_token_from_array();
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
}

int pocet_main = 0;

ErrorCodes define_function_header()
{
    bool main_flag = false;

    if (current_token.type == TOKEN_ZAVINAC)
    {
        match(TOKEN_ZAVINAC);
        if (current_token.type == TOKEN_EOL)
        {
            match(TOKEN_EOL);
        }

        if (current_token.type == TOKEN_ID && current_token.string != NULL && strcmp(current_token.string, "main") == 0)
        {
            match(TOKEN_ID);
            pocet_main++;

            if (pocet_main > 1)
            {
                printError(ERR_SEM_REDEF);
            }
        }
        else
        {
            printError(ERR_SYN);
        }
    }

    match(TOKEN_DEF);

    char *name = match(TOKEN_ID).string;

    printf("%s\n", name);

    SymbolData *symbol = symtable_lookup(name);
    if (symbol != NULL)
    {
        printError(ERR_SEM_REDEF);
    }

    match(TOKEN_LZAVORKA);

    if (main_flag && current_token.type == TOKEN_ID)
    {
        printError(ERR_SEM_ARG);
    }

    int out_param_count = 0;
    DataType *param_types = parse_parameters(&out_param_count);

    match(TOKEN_PZAVORKA);

    match(TOKEN_DVOJTEC);
    if (main_flag && current_token.type != TOKEN_UNIT)
    {
        printError(ERR_SEM_TYPE);
    }
    Token return_type_token = match_type();

    printf("Zapsi to symtable\n");
    symtable_insert_function(name, token_to_dataType(return_type_token), sizeof(param_types), param_types, main_flag);

    match(TOKEN_ROVNO);

    while (current_token.type != TOKEN_ZAVINAC && current_token.type != TOKEN_DEF && current_token.type != TOKEN_EOF)
    {
        get_next_token_from_array();
    }

    return 0;
}

ErrorCodes parse()
{
    read_file_by_tokens();

    symtable_init();

    beginning();

    while (current_token.type != TOKEN_EOF)
    {
        define_function_header();
    }
    if (pocet_main == 0)
    {
        printError(ERR_SEM_UNDEF);
    }

    printf("Konec privniho cteni\n");

    reset_parser_position();
    beginning();

    while (current_token.type != TOKEN_EOF)
    {
        parse_function();
    }

    return ERR_OK;
}

DataType *parse_parameters(int *out_param_count)
{

    *out_param_count = 0;
    if (current_token.type == TOKEN_PZAVORKA)
    {
        return NULL;
    }

    int capacity = 4;
    DataType *typy_parametru = malloc(capacity * sizeof(DataType));
    if (typy_parametru == NULL)
    {
        printError(ERR_INTERNAL);
    }

    while (true)
    {
        match(TOKEN_ID);
        match(TOKEN_DVOJTEC);
        Token param_type_token = match_type();
        if (*out_param_count >= capacity)
        {
            capacity *= 2;
            DataType *temp = realloc(typy_parametru, capacity * sizeof(DataType));
            if (temp == NULL)
            {
                free(typy_parametru);
                printError(ERR_INTERNAL);
                return NULL;
            }
            typy_parametru = temp;
        }
        typy_parametru[*out_param_count] = token_to_dataType(param_type_token);
        (*out_param_count)++;

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
                break;
            }
            else if (current_token.type != TOKEN_ID)
            {
                printError(ERR_SYN);
            }
        }
        else if (current_token.type == TOKEN_PZAVORKA)
        {
            break;
        }
        else
        {
            printError(ERR_SYN);
        }
    }
    return typy_parametru;
}

DataType parse_expression()
{
    // TODO !!!
    printf("EXPRESION\n");
    while (current_token.type != TOKEN_EOL && current_token.type != TOKEN_DO && current_token.type != TOKEN_THEN &&
           current_token.type != TOKEN_CARKA && current_token.type != TOKEN_PZAVORKA && current_token.type != TOKEN_EOF)
    {
        get_next_token_from_array();
    }

    return (DataType){0};
}

void parse_function()
{
    symtable_push_scope();

    while (current_token.type != TOKEN_LZAVORKA)
    {
        get_next_token_from_array();
    }

    match(TOKEN_LZAVORKA);

    while (current_token.type != TOKEN_PZAVORKA)
    {
        Token token_name = match(TOKEN_ID);
        match(TOKEN_DVOJTEC);
        Token token_type = match_type();
        symtable_insert_var(token_name.string, token_to_dataType(token_type), false);
        if (current_token.type == TOKEN_CARKA)
        {
            match(TOKEN_CARKA);
        }
    }

    match(TOKEN_PZAVORKA);

    match(TOKEN_DVOJTEC);
    Token return_type_token = match_type();
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

    symtable_pop_scope();
}
