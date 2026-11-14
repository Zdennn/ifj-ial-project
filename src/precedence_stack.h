/**
 * @file stack.h
 * @brief Definice zásobníku
 *
 * @author ...
 */

#ifndef STACK_H
#define STACK_H

typedef enum
{

} TokenType;

typedef struct
{
    TokenType type;    // Typ tokenu
    char *string;      // Textová hodnota
    int int_val;       // Celočíselná hodnota
    double double_val; // Hodnota desetiného čísla
    int line;          // Číslo řádku
} stack;

// Vytvoří zásobník
void stack_init();
// Vloží na vrchol
void stack_push();
// Odebere a vrátí
void stack_pop();
// Vrátí bez odebrání
void stack_top();
// Vrátí první terminál bez odebrání
void stack_top_terminal();

#endif