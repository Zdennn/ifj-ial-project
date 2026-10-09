/**
 * @file scanner.c
 * @brief Implementace lexikálního analyzátoru.
 * 
 * Čte vstup po znacích ze stdin a převádí je na tokeny, které předává syntaktickému analyzátoru (parser).
 * 
 * @author ...
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "error.h"
#include "token.h"
#include "scanner.h"

int get_next_token(Token *token){
    int c;
    
    while ((c = getchar()) != EOF){

        if(c == ' ')
            continue;
        if(c == '\n'){
            token->type = TOKEN_EOL;
            return ERR_OK;
        }

        // Tokeny pro identifikátory nebo while, if...
        if (isalpha(c))
        {
            
        }
        // Tokeny pro čísla
        if (isdigit(c))
        {
            
        }
        


        // Tokeny pro operace
        if(c == '='){
            token->type = TOKEN_ROVNO;
            return ERR_OK;
        }
        if(c == '+'){
            token->type = TOKEN_PLUS;
            return ERR_OK;
        }
        if(c == '-'){
            token->type = TOKEN_MINUS;
            return ERR_OK;
        }
        if(c == '*'){
            token->type = TOKEN_MUL;
            return ERR_OK;
        }
        if(c == '/'){
            token->type = TOKEN_DIV;
            return ERR_OK;
        }
        if(c == '('){
            token->type = TOKEN_LZAVORKA;
            return ERR_OK;
        }
        if(c == ')'){
            token->type = TOKEN_PZAVORKA;
            return ERR_OK;
        }
        if(c == ':'){
            token->type = TOKEN_COLON;
            return ERR_OK;
        }

        // Tokeny pro porovnávání 
        // TODO dodělat pro <=, >=, ==, !=
        if(c == '<'){
            token->type = TOKEN_LESS;
            return ERR_OK;
        }
        if(c == '>'){
            token->type = TOKEN_GREATER;
            return ERR_OK;
        }
        
        return ERR_LEX;
    }
    // Token konce souboru
    token->type= TOKEN_EOF;
    return ERR_OK;
}