/**
 * @file token.h
 * @brief Definice datových typů pro lexikální jednotky (tokeny) - enum.
 * 
 * @author ...
 */

 typedef enum {
    TOKEN_EOF, // Konec souboru
    TOKEN_EOL, // Konec řádku

    TOKEN_ID, // Název proměnné/funkce
    TOKEN_INT,
    TOKEN_DOUBLE,
    TOKEN_STRING,

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