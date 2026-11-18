/**
 * @file token.h
 * @brief Definice datových typů pro lexikální jednotky (tokeny) - enum.
 *
 * @author ...
 */

#ifndef TOKEN_H
#define TOKEN_H

typedef enum
{
    TOKEN_EOF, // Konec souboru
    TOKEN_EOL, // Konec řádku

    TOKEN_ID, // Název proměnné/funkce
    TOKEN_INT,
    TOKEN_DOUBLE,
    TOKEN_STRING,

    TOKEN_DEF,
    TOKEN_DO,
    TOKEN_WHILE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_VAR,
    TOKEN_VAL,
    TOKEN_IMPORT,
    TOKEN_RETURN,
    TOKEN_THEN,
    TOKEN_UNIT,

    // Tokeny pro hodnoty
    TOKEN_STRING_HOD,
    TOKEN_INT_HOD,
    TOKEN_DOUBLE_HOD,

    TOKEN_ODSAZENI,

    TOKEN_ROVNO,    // =
    TOKEN_PLUS,     // +
    TOKEN_MINUS,    // -
    TOKEN_NAS,      // *
    TOKEN_DEL,      // /
    TOKEN_LZAVORKA, // (
    TOKEN_PZAVORKA, // )
    TOKEN_DVOJTEC,  // :
    TOKEN_TECKA,    // .
    TOKEN_CARKA,    // ,
    TOKEN_STREDNIK, // ;
    TOKEN_ZAVINAC,  // @

    TOKEN_MENSI,       // <
    TOKEN_VETSI,       // >
    TOKEN_MENSI_ROVNO, // <=
    TOKEN_VETSI_ROVNO, // >=
    TOKEN_NE_ROVNO,    // !=
    TOKEN_STEJNO,      // ==


    // BOOLTHEN
    TOKEN_BOOLEAN,
    TOKEN_TRUE_HOD,
    TOKEN_FALSE_HOD,
    TOKEN_NOT, // !
    TOKEN_AND, // &&
    TOKEN_OR,  // ||

    // CONTROL
    TOKEN_FOR,
    TOKEN_LARROW, // <-
} TokenType;

typedef struct
{
    TokenType type;    // Typ tokenu
    char *string;      // Textová hodnota
    int int_val;       // Celočíselná hodnota
    double double_val; // Hodnota desetiného čísla
    int line;          // Číslo řádku
} Token;

#endif