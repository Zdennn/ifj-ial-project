/**
 * @file generator.h
 * @brief Rozhraní Generátoru kódu.
 *
 * Implementace překladače imperativního jazyka IFJ26
 *
 * @author Trynkl Tadeáš, Javůrek Jonáš, Zdeněk Michalec
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>

/**
 * @brief Nastaví výstupní proud pro generovaný kód.
 * @param out Cílový proud, nesmí být NULL.
 */
void gen_init(FILE *out);

/**
 * @brief Vypíše úvodní řádek .IFJcode26.
 */
void gen_header(void);

/**
 * @brief Vypíše celočíselný operand ve tvaru int@<hodnota> (bez odřádkování).
 * @param value Hodnota typu Int.
 */
void gen_write_int(int value);

/**
 * @brief Vypíše desetinný operand ve tvaru float@<hodnota> (formát %a, bez odřádkování).
 * @param value Hodnota typu Double.
 */
void gen_write_float(double value);

/**
 * @brief Vypíše řetězcový operand ve tvaru string@<hodnota>.
 *        Znaky s kódem 0-32, 35 a 92 zapisuje jako escape sekvenci \xyz.
 * @param str Řetězec zakončený nulou, nesmí být NULL.
 */
void gen_write_string(const char *str);

/**
 * @brief Vypíše jednu instrukci IFJcode26 ve tvaru "OPCODE a b c\n".
 *
 * Operandy jsou hotový text (např. "LF@x", "int@5", "string@ahoj").
 * Operandy, které instrukce nemá, se předají jako NULL a nevypíšou se.
 * Operandy se řadí zleva, tj. NULL v prostředku (a, NULL, c) není povolen.
 *
 * @param opcode Název instrukce (např. "ADD", "WRITE"), nesmí být NULL.
 * @param a      První operand, nebo NULL.
 * @param b      Druhý operand, nebo NULL (smí být jen pokud je i c NULL, nebo a/b zadány).
 * @param c      Třetí operand, nebo NULL.
 */
void gen_instr(const char *opcode, const char *a, const char *b, const char *c);

/**
 * @brief Vrátí nové unikátní číslo pro návěští (počítadlo roste při každém volání).
 * @return Unikátní nezáporné číslo.
 */
int gen_new_label_id(void);

/**
 * @brief Složí jméno návěští ve tvaru $<prefix>_<id> do zadaného bufferu.
 * @param buf    Cílový buffer, nesmí být NULL.
 * @param size   Velikost bufferu v bajtech.
 * @param prefix Popisná předpona (např. "while_start"), nesmí být NULL.
 * @param id     Číslo z gen_new_label_id().
 */
void gen_label_name(char *buf, size_t size, const char *prefix, int id);

/**
 * @brief Vypíše instrukci LABEL se zadaným jménem.
 * @param name Jméno návěští, nesmí být NULL.
 */
void gen_label(const char *name);

/**
 * @brief Vygeneruje výpis jedné hodnoty (vestavěná funkce print) pro Int, Double nebo String.
 * @param operand Operand instrukce WRITE (např. "int@5", "string@ahoj", "LF@x"), nesmí být NULL.
 */
void gen_print(const char *operand);

/**
 * @brief Vygeneruje výpis hodnoty () typu Unit, tedy řetězce "()".
 */
void gen_print_unit(void);

/* Maximální délka jména operandu/návěští sestavovaného generátorem. */
#define GEN_NAME_MAX 256

/**
 * @brief Složí operand proměnné ve tvaru <frame>@<name> (např. LF@x).
 * @param buf   Cílový buffer, nesmí být NULL.
 * @param size  Velikost bufferu (při oříznutí interní chyba).
 * @param frame Rámec: "GF", "LF" nebo "TF".
 * @param name  Jméno proměnné.
 */
void gen_var(char *buf, size_t size, const char *frame, const char *name);

/**
 * @brief Složí jméno návěští funkce ve tvaru $func_<name>.
 * @param buf  Cílový buffer, nesmí být NULL.
 * @param size Velikost bufferu (při oříznutí interní chyba).
 * @param name Jméno funkce.
 */
void gen_func_label(char *buf, size_t size, const char *name);

/**
 * @brief Začátek programu: DEFVAR GF@$retval a JUMP na vstupní funkci.
 * @param main_name Jméno funkce s @main.
 */
void gen_program_start(const char *main_name);

/**
 * @brief Začátek funkce: LABEL $func_<name> a PUSHFRAME (parametry už jsou v TF).
 * @param name Jméno funkce.
 */
void gen_function_begin(const char *name);

/**
 * @brief Definuje lokální proměnnou: DEFVAR LF@<name>. Jméno musí být unikátní v rámci funkce.
 * @param name Jméno proměnné.
 */
void gen_defvar_local(const char *name);

/**
 * @brief Návrat z funkce: MOVE GF@$retval <value>, POPFRAME, RETURN.
 * @param value Operand návratové hodnoty (např. "LF@x", "int@0").
 */
void gen_function_return(const char *value);

/**
 * @brief Návrat z funkce s hodnotou Unit (string@()).
 */
void gen_function_return_unit(void);

/**
 * @brief Vloží operand na datový zásobník (PUSHS). Používá se pro argumenty.
 * @param operand Operand hodnoty.
 */
void gen_push(const char *operand);

/**
 * @brief Vyjme hodnotu z datového zásobníku do proměnné (POPS).
 * @param var Operand proměnné.
 */
void gen_pop(const char *var);

/**
 * @brief Vloží návratovou hodnotu poslední funkce na datový zásobník (PUSHS GF@$retval).
 */
void gen_push_retval(void);

/**
 * @brief Volání funkce. Argumenty musí být již na datovém zásobníku (poslední nahoře).
 *        Vygeneruje CREATEFRAME, DEFVAR TF@p, POPS TF@p (odzadu) a CALL $func_<name>.
 * @param name   Jméno volané funkce.
 * @param params Jména formálních parametrů v pořadí z definice (smí být NULL, když count == 0).
 * @param count  Počet parametrů.
 */
void gen_call(const char *name, const char *const *params, size_t count);



#endif
