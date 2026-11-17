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
#include "precedence_stack.h"
#include "precedence_table.h"
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
char *main_name;

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
    fprintf(stderr, "current: %s, expected: %s, line: %d\n", getTokenName(current_token.type), getTokenName(expected),
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
        break; // všechny breaky jsou zbytečné protoze tam mame return :D
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

static DataType parse_call(Token token_id)
{
    SymbolData *symbol = symtable_lookup(token_id.string);
    if (symbol == NULL)
        printError(ERR_SEM_UNDEF);

    match(TOKEN_LZAVORKA);

    int arg = 0;
    int carka = 0;

    while (current_token.type != TOKEN_PZAVORKA && current_token.type != TOKEN_EOF)
    {
        DataType arg_type = parse_expression();

        if (arg < symbol->param_count)
        {
            DataType expected = symbol->param_types[arg];

            if (expected != TYPE_ANY && arg_type != expected)
            {
                if (!(arg_type == TYPE_INT && expected == TYPE_DOUBLE))
                    printError(ERR_SEM_ARG);
            }
        }

        arg++;

        if (current_token.type == TOKEN_CARKA)
        {
            match(TOKEN_CARKA);
            carka++;
        }
        else
        {
            break;
        }
    }

    match(TOKEN_PZAVORKA);

    if (arg != symbol->param_count)
        printError(ERR_SEM_ARG);

    if (carka != arg - 1 && !(arg == 0 && carka == 0))
    {
        printError(ERR_SYN);
    }

    return symbol->data_type;
}

static DataType parse_value()
{
    if (current_token.type == TOKEN_ID)
    {
        Token next = (token_index < tokens.count) ? tokens.items[token_index] : current_token;
        if (next.type == TOKEN_LZAVORKA)
        {
            Token id_tok = match(TOKEN_ID);
            return parse_call(id_tok);
        }
    }

    return parse_expression();
}

DataType parse_block(DataType expected_return_type, bool has_explicit_return)
{

    DataType last_type = TYPE_UNIT;

    int pocet_odsazeni = current_token.int_val;

    // printf("parse_block\n");

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
                return last_type;
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
                return last_type;
            }
        }

        Token token_id;
        Token token_type = {0};
        DataType type_of_expresion = {0};
        DataType final_var_type = {0};
        DataType declared_type = {0};

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
            type_of_expresion = parse_value();
            if (token_type.type == TOKEN_EOF) // token_type = {0}, typ nebyl uveden
            {
                final_var_type = type_of_expresion;
            }
            else // token_type != {0}, typ byl uveden
            {
                declared_type = token_to_dataType(token_type);

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
            if (type_of_expresion == TYPE_BOOLEAN || final_var_type == TYPE_BOOLEAN)
            {
                printError(ERR_SEM_TYPE);
            }
            symtable_insert_var(token_id.string, final_var_type, true);
            last_type = TYPE_UNIT;
            break;
        case TOKEN_VAL:
            match(TOKEN_VAL);
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

            type_of_expresion = parse_value();

            if (token_type.type == TOKEN_EOF) // token_type = {0}, typ nebyl uveden
            {
                final_var_type = type_of_expresion;
            }
            else // token_type != {0}, typ byl uveden
            {
                declared_type = token_to_dataType(token_type);

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
            if (type_of_expresion == TYPE_BOOLEAN || final_var_type == TYPE_BOOLEAN)
            {
                printError(ERR_SEM_TYPE);
            }
            symtable_insert_var(token_id.string, final_var_type, false);
            last_type = TYPE_UNIT;
            break;
        case TOKEN_IF:
            match(TOKEN_IF);
            type_of_expresion = parse_expression();
            if (type_of_expresion != TYPE_BOOLEAN)
            {
                printError(ERR_SEM_TYPE);
            }
            match(TOKEN_THEN);
            match(TOKEN_EOL);
            // Then musí mít větší odsazení než if
            if (current_token.type != TOKEN_ODSAZENI || current_token.int_val <= pocet_odsazeni)
            {
                printError(ERR_SYN);
            }
            symtable_push_scope();
            DataType type_then = parse_block(expected_return_type, has_explicit_return);
            symtable_pop_scope();

            // Else musí mít stejné odsazení jako if!
            if (current_token.type != TOKEN_ODSAZENI || current_token.int_val != pocet_odsazeni)
            {
                printError(ERR_SYN);
            }
            match(TOKEN_ODSAZENI);
            match(TOKEN_ELSE);

            DataType type_else = TYPE_UNIT;
            if (current_token.type == TOKEN_EOL)
            {
                match(TOKEN_EOL);

                // Else musí mít větší odsazení než if/else
                if (current_token.type != TOKEN_ODSAZENI || current_token.int_val <= pocet_odsazeni)
                {
                    printError(ERR_SYN);
                }
                symtable_push_scope();
                type_else = parse_block(expected_return_type, has_explicit_return);
                symtable_pop_scope();
            }
            else
            {
                type_else = parse_value();
                if (current_token.type == TOKEN_EOL)
                {
                    match(TOKEN_EOL);
                }
            }

            if (type_then == type_else)
            {
                last_type = type_then;
            }
            else if ((type_then == TYPE_INT && type_else == TYPE_DOUBLE) ||
                     (type_then == TYPE_DOUBLE && type_else == TYPE_INT))
            {
                last_type = TYPE_DOUBLE;
            }
            else
            {
                // rozdilne typy
                last_type = TYPE_UNIT;
            }

            break;
        case TOKEN_WHILE:
            match(TOKEN_WHILE);
            type_of_expresion = parse_expression(); // Podmínka
            if (type_of_expresion != TYPE_BOOLEAN)
            {
                printError(ERR_SEM_TYPE);
            }

            match(TOKEN_DO);
            match(TOKEN_EOL);
            // Musí být odsazeni s vyšším počtem mezer
            if (current_token.type != TOKEN_ODSAZENI || current_token.int_val <= pocet_odsazeni)
            {
                printError(ERR_SYN);
            }
            symtable_push_scope();
            parse_block(expected_return_type, has_explicit_return);
            symtable_pop_scope();
            last_type = TYPE_UNIT;
            break;

        case TOKEN_ID:

            // TODO asi resit az v parse_expresion()
            // TODO bude zpracovávvat napr. a = 5, foo(5), a + b, foo(5) - a + 5
            // TODO tahkle si akorat smazu tkone_id ktere pote bude chybet v parse_expression()
            /*
                        token_id = match(TOKEN_ID);
                        SymbolData *symbol = symtable_lookup(token_id.string);
                        if (symbol == NULL)
                        {
                            printError(ERR_SEM_UNDEF);
                        }

                        if (current_token.type == TOKEN_ROVNO)
                        {
                            // výraz
                            match(TOKEN_ROVNO);
                            if (!symbol->is_var)
                            {
                                printError(ERR_SEM_UNMOD);
                            }
                            type_of_expresion = parse_expression();
                        }
                        else if (current_token.type == TOKEN_LZAVORKA)
                        {
                            // Volání funkce
                            // TODO sedí parametry (chyba 5)??
                            match(TOKEN_LZAVORKA);
                            type_of_expresion = parse_expression();
                            match(TOKEN_PZAVORKA);
                        }
                        else
                        {
                            // Konec bloku
                            parse_expression();
                        }

                        if (type_of_expresion != symbol->data_type)
                        {
                            if (!(type_of_expresion == TYPE_INT && symbol->data_type == TYPE_DOUBLE))
                            {
                                printError(ERR_SEM_TYPE);
                            }
                        }
            */
            ////////////////////////////////////////////!
            Token next = (token_index < tokens.count) ? tokens.items[token_index] : current_token;

            // Prirazeni do promenne
            if (next.type == TOKEN_ROVNO)
            {
                token_id = match(TOKEN_ID);
                match(TOKEN_ROVNO);

                SymbolData *symbol = symtable_lookup(token_id.string);
                if (symbol == NULL)
                {
                    printError(ERR_SEM_UNDEF);
                }

                if (!symbol->is_var)
                {
                    printError(ERR_SEM_UNMOD);
                }

                DataType expression_type = parse_value();

                if (expression_type != symbol->data_type)
                {
                    if (!(expression_type == TYPE_INT && symbol->data_type == TYPE_DOUBLE))
                    {
                        printError(ERR_SEM_TYPE);
                    }
                }

                last_type = TYPE_UNIT;
            }
            // Volani funkce
            else
            {
                last_type = parse_value();
            }

            if (current_token.type == TOKEN_EOL)
            {
                match(TOKEN_EOL);
            }
            break;

        case TOKEN_RETURN:
            match(TOKEN_RETURN);

            if (!has_explicit_return)
            {
                printError(ERR_SEM_TYPE);
            }

            if (current_token.type == TOKEN_EOL || current_token.type == TOKEN_EOF)
            {
                if (expected_return_type != TYPE_UNIT)
                {
                    printError(10);
                }
                last_type = TYPE_UNIT;
            }
            else
            {
                last_type = parse_value();

                if (expected_return_type != TYPE_UNIT && last_type != expected_return_type)
                {
                    if (!(last_type == TYPE_INT && expected_return_type == TYPE_DOUBLE))
                    {
                        printError(ERR_SEM_TYPE);
                    }
                }
            }
            return last_type;
            break;

        case TOKEN_UNIT:
        case TOKEN_INT_HOD:
        case TOKEN_DOUBLE_HOD:
        case TOKEN_STRING_HOD:
        case TOKEN_LZAVORKA:
            last_type = parse_value();
            if (current_token.type == TOKEN_EOL)
            {
                match(TOKEN_EOL);
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
    return last_type;
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
            main_flag = true;

            if (pocet_main > 1)
            {
                printError(ERR_SEM_REDEF);
            }

            if (current_token.type == TOKEN_EOL)
            {
                match(TOKEN_EOL);
            }
        }
        else
        {
            printError(ERR_SYN);
        }
    }

    match(TOKEN_DEF);

    char *name = match(TOKEN_ID).string;

    // printf("%s\n", name);

    SymbolData *symbol = symtable_lookup(name);
    if (symbol != NULL)
    {
        printError(ERR_SEM_REDEF);
    }

    match(TOKEN_LZAVORKA);
    /*
        if (main_flag && current_token.type == TOKEN_ID)
        {
            printError(ERR_SEM_ARG);
        }
    */
    int out_param_count = 0;
    DataType *param_types = parse_parameters(&out_param_count);

    match(TOKEN_PZAVORKA);

    DataType return_type = TYPE_UNDEF;
    bool explicit_type = false;

    if (current_token.type == TOKEN_DVOJTEC)
    {
        match(TOKEN_DVOJTEC);
        Token return_type_token = match_type();
        return_type = token_to_dataType(return_type_token);
        explicit_type = true;
    }

    if (main_flag)
    {
        if (out_param_count > 0)
        {
            printError(ERR_SEM_ARG);
        }
        if (explicit_type && return_type != TYPE_UNIT)
        {
            printError(ERR_SEM_TYPE);
        }
    }

    symtable_insert_function(name, return_type, out_param_count, param_types, main_flag);

    main_name = name;

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

    fprintf(stderr, "Konec privniho cteni\n");

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
            match(TOKEN_CARKA);

            if (current_token.type == TOKEN_EOL)
            {
                match(TOKEN_EOL);
            }

            if (current_token.type == TOKEN_PZAVORKA)
            {
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

DataType kontrola_typu(DataType left_type, PrecIndex op, DataType right_type, TokenType op_token)
{
    switch (op)
    {
    case PREC_MUL_DIV:
        if (left_type == TYPE_INT && right_type == TYPE_INT)
        {
            return TYPE_INT;
        }
        else if ((left_type == TYPE_INT && right_type == TYPE_DOUBLE) ||
                 (left_type == TYPE_DOUBLE && right_type == TYPE_INT) ||
                 (left_type == TYPE_DOUBLE && right_type == TYPE_DOUBLE))
        {
            return TYPE_DOUBLE;
        }
        else
        {
            printError(ERR_SEM_TYPE);
        }

        break;
    case PREC_PLUS_MINUS:
        if (left_type == TYPE_STRING && right_type == TYPE_STRING)
        {
            // TODO pro minus asi chyba?
            if (op_token == TOKEN_PLUS)
            {
                return TYPE_STRING;
            }

            printError(ERR_SEM_TYPE);
        }
        else if (left_type == TYPE_INT && right_type == TYPE_INT)
        {
            return TYPE_INT;
        }
        else if ((left_type == TYPE_INT && right_type == TYPE_DOUBLE) ||
                 (left_type == TYPE_DOUBLE && right_type == TYPE_INT) ||
                 (left_type == TYPE_DOUBLE && right_type == TYPE_DOUBLE))
        {
            return TYPE_DOUBLE;
        }
        else
        {
            printError(ERR_SEM_TYPE);
        }

        break;
    case PREC_REL:
        if ((left_type == TYPE_INT && right_type == TYPE_INT) || (left_type == TYPE_INT && right_type == TYPE_DOUBLE) ||
            (left_type == TYPE_DOUBLE && right_type == TYPE_INT) ||
            (left_type == TYPE_DOUBLE && right_type == TYPE_DOUBLE))
        {
            return TYPE_BOOLEAN;
        }
        else if (left_type == TYPE_STRING && right_type == TYPE_STRING)
        {
            return TYPE_BOOLEAN;
        }
        else
        {
            printError(ERR_SEM_TYPE);
        }

        break;
    case PREC_EQUAL:
        if ((left_type == TYPE_INT && right_type == TYPE_DOUBLE) ||
            (left_type == TYPE_DOUBLE && right_type == TYPE_INT))
        {
            return TYPE_BOOLEAN;
        }
        else if (left_type == right_type)
        {
            return TYPE_BOOLEAN;
        }
        else
        {
            printError(ERR_SEM_TYPE);
        }
        break;

    default:
        printError(ERR_SEM_TYPE);
        break;
    }

    return (DataType){0};
}

static DataType expression_to_DataType(Token token)
{
    switch (token.type)
    {
    case TOKEN_INT_HOD:
        return TYPE_INT;
    case TOKEN_DOUBLE_HOD:
        return TYPE_DOUBLE;
    case TOKEN_STRING_HOD:
        return TYPE_STRING;
    case TOKEN_UNIT:
        return TYPE_UNIT;
    case TOKEN_ID:
        SymbolData *sym = symtable_lookup(token.string);
        if (sym == NULL)
        {
            printError(ERR_SEM_UNDEF);
            return TYPE_UNIT;
        }
        return sym->data_type;
    default:
        return TYPE_UNIT;
    }
}

DataType parse_expression()
{
    // bude zpravovavat expresion ve:
    // while, if => boolean
    // val, var (definice) => hodnota, funkce
    // token_id (volani funkce, prirazeni do existujici funkce) => hodnota, funkce
    // TODO zjistit jestli je to volání funkce nebo expression, teoreticky to je
    /*
    fprintf(stderr, "EXPRESION\n");
    while (current_token.type != TOKEN_EOL && current_token.type != TOKEN_DO && current_token.type != TOKEN_THEN &&
           current_token.type != TOKEN_CARKA && current_token.type != TOKEN_PZAVORKA && current_token.type != TOKEN_EOF)
    {
        get_next_token_from_array();
    }

    */

    PrecStack stack;
    stack_init(&stack);

    // Vložení konce stacku
    stack_push(&stack, (StackItem){.sem_type = SEM_TERMINAL, .prec_sym = PREC_STOP, .token_type = current_token.type});

    if (current_token.type == TOKEN_LZAVORKA)
    {
        Token next = (token_index < tokens.count) ? tokens.items[token_index] : (Token){0};
        if (next.type == TOKEN_PZAVORKA)
        {
            match(TOKEN_LZAVORKA);
            match(TOKEN_PZAVORKA);
            return TYPE_UNIT;
        }
    }

    int depth = 0;
    while (true)
    {
        StackItem *top_term = stack_top_terminal(&stack);
        PrecIndex input_prec_index = token_to_index(current_token.type);

        if (current_token.type == TOKEN_PZAVORKA && depth == 0)
        {
            input_prec_index = PREC_STOP;
        }

        // Konec výrazu
        if (top_term->prec_sym == PREC_STOP && input_prec_index == PREC_STOP)
        {
            StackItem result = stack_pop(&stack);
            stack_free(&stack);
            return result.data_type;
        }

        PrecOperation operation = get_prec_operation(top_term->prec_sym, input_prec_index);

        switch (operation)
        {
        case PREC_E:
            stack_push(
                &stack,
                (StackItem){.sem_type = SEM_TERMINAL, .prec_sym = input_prec_index, .token_type = current_token.type});
            get_next_token_from_array();
            depth--;
            break;
        case PREC_S:

            if (current_token.type == TOKEN_ID)
            {
                if (symtable_lookup(current_token.string) == NULL)
                {
                    printError(ERR_SEM_UNDEF);
                }
            }

            if (current_token.type == TOKEN_LZAVORKA)
                depth++;

            stack_vlozeni_zarazky_za_nevyssi_terminal(&stack);

            stack_push(&stack, (StackItem){.sem_type = SEM_TERMINAL,
                                           .prec_sym = input_prec_index,
                                           .data_type = expression_to_DataType(current_token),
                                           .token_type = current_token.type});
            get_next_token_from_array();
            break;

        case PREC_R:

            int count = 0;
            StackItem stack_items[4];
            StackItem top = stack_pop(&stack);
            while (top.sem_type != SEM_STOP)
            {
                if (count >= 3)
                {
                    stack_free(&stack);
                    printError(ERR_SYN);
                }
                stack_items[count] = top;
                count++;

                if (stack.top < 0)
                {
                    stack_free(&stack);
                    printError(ERR_SYN);
                }
                top = stack_pop(&stack);
            }

            DataType result_type = TYPE_UNIT;

            if (count == 1)
            {
                // Pravidlo: E -> i

                if (stack_items[0].sem_type == SEM_TERMINAL && stack_items[0].prec_sym == PREC_ID)
                {
                    result_type = stack_items[0].data_type;
                }
                else
                {
                    stack_free(&stack);
                    printError(ERR_SYN);
                }
            }
            else if (count == 3)
            {
                // Pravidlo: E -> ( E )
                if (stack_items[2].sem_type == SEM_TERMINAL && stack_items[2].token_type == TOKEN_LZAVORKA &&
                    stack_items[1].sem_type == SEM_NONTERMINAL && stack_items[0].sem_type == SEM_TERMINAL &&
                    stack_items[0].token_type == TOKEN_PZAVORKA)
                {
                    result_type = stack_items[1].data_type;
                }
                else if (stack_items[2].sem_type == SEM_NONTERMINAL && stack_items[1].sem_type == SEM_TERMINAL &&
                         stack_items[0].sem_type == SEM_NONTERMINAL)
                {
                    DataType left_type = stack_items[2].data_type;
                    DataType right_type = stack_items[0].data_type;
                    PrecIndex op = stack_items[1].prec_sym;
                    TokenType op_token = stack_items[1].token_type;

                    result_type = kontrola_typu(left_type, op, right_type, op_token);
                }
                else
                {
                    stack_free(&stack);
                    printError(ERR_SYN);
                }
            }
            else
            {
                stack_free(&stack);
                printError(ERR_SYN);
            }

            stack_push(&stack,
                       (StackItem){.sem_type = SEM_NONTERMINAL, .prec_sym = PREC_INVALID, .data_type = result_type});

            break;
        case PREC_ERR:
            stack_free(&stack);
            printError(ERR_SYN);
            break;

        default:
            printError(ERR_SYN);
            break;
        }
    }

    return (DataType){0};
}

void parse_function()
{
    symtable_push_scope();

    if (current_token.type == TOKEN_ZAVINAC)
    {
        match(TOKEN_ZAVINAC);
        if (current_token.type == TOKEN_EOL)
        {
            match(TOKEN_EOL);
        }
        if (current_token.type == TOKEN_ID)
        {
            match(TOKEN_ID);
        }
        if (current_token.type == TOKEN_EOL)
        {
            match(TOKEN_EOL);
        }
    }

    char *function_name = NULL;
    while (current_token.type != TOKEN_LZAVORKA)
    {
        if (current_token.type == TOKEN_ID)
        {
            function_name = current_token.string;
        }
        get_next_token_from_array();
    }

    match(TOKEN_LZAVORKA);

    while (current_token.type != TOKEN_PZAVORKA)
    {
        Token token_name = match(TOKEN_ID);
        match(TOKEN_DVOJTEC);
        Token token_type = match_type();

        // duplicitní parametry
        if (symtable_lookup_current(token_name.string) != NULL)
        {
            printError(ERR_SEM_REDEF);
        }

        symtable_insert_var(token_name.string, token_to_dataType(token_type), false);
        if (current_token.type == TOKEN_CARKA)
        {
            match(TOKEN_CARKA);
        }
    }

    match(TOKEN_PZAVORKA);

    SymbolData *symbol = symtable_lookup(function_name);
    bool explicit = (symbol && symbol->data_type != TYPE_UNDEF);
    DataType expected_return = explicit ? symbol->data_type : TYPE_UNDEF;

    if (current_token.type == TOKEN_DVOJTEC)
    {
        match(TOKEN_DVOJTEC);
        Token return_type_token = match_type();
        expected_return = token_to_dataType(return_type_token);
        explicit = true;
        if (symbol)
        {
            symbol->data_type = expected_return;
        }
    }

    DataType actual_return = {0};
    match(TOKEN_ROVNO);

    if (current_token.type == TOKEN_EOL)
    {
        // Odsazený blok
        match(TOKEN_EOL);

        actual_return = parse_block(explicit ? expected_return : TYPE_ANY, explicit);
    }
    else
    {
        // Jednořádková funkce
        actual_return = parse_value();

        if (current_token.type == TOKEN_EOL)
        {
            match(TOKEN_EOL);
        }
    }

    if (!explicit)
    {
        if (symbol)
        {
            symbol->data_type = actual_return;
        }
        if (symbol && symbol->is_main && actual_return != TYPE_UNIT)
        {
            printError(ERR_SEM_TYPE);
        }
    }
    else if (expected_return != TYPE_UNIT)
    {
        if (actual_return != expected_return)
        {
            if (!(actual_return == TYPE_INT && expected_return == TYPE_DOUBLE))
            {
                printError(ERR_SEM_TYPE);
            }
        }
    }

    symtable_pop_scope();
}
