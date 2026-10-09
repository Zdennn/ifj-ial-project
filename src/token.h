/**
 * @file token.h
 * @brief Definice datových typů pro lexikální jednotky (tokeny) - enum.
 * 
 * @author ...
 */

#ifndef TOKEN_H
#define TOKEN_H

 typedef enum {
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

    TOKEN_ROVNO, // = 
    TOKEN_PLUS, // + 
    TOKEN_MINUS, // - 
    TOKEN_MUL, // * 
    TOKEN_DIV, // / 
    TOKEN_LZAVORKA, // ( 
    TOKEN_PZAVORKA, // ) 
    TOKEN_COLON, // : 

    TOKEN_LESS, // <
    TOKEN_GREATER, // >
    TOKEN_LESS_EQ, // <=
    TOKEN_GREATER_EQ, // >=
    TOKEN_NOT_EQ, // !=
    TOKEN_EQ, // ==

} TokenType;

typedef struct {
    TokenType type;     // Typ tokenu
    char* string;       // Textová hodnota
    int int_val;        // Celočíselná hodnota
    double double_val;  // Hodnota desetiného čísla
} Token;

#endif