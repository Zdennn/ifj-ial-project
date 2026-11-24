/**
 * @file generator.h
 * @brief Rozhraní Generátoru kódu.
 *
 * Implementace překladače imperativního jazyka IFJ26
 *
 * @author ...
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>

#include "symtable.h"
#include "token.h"

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
 * @brief Začátek programu: definice globálních proměnných ($retval, $tmp1–3),
 *        zavolání vstupní funkce a ukončení programu s kódem 0.
 * @param main_name Jméno funkce s @main.
 */
void gen_program_start(const char *main_name);

/**
 * @brief Začátek funkce. Od tohoto volání se výstup ukládá do paměti, dokud nepřijde gen_function_end.
 * @param name Jméno funkce, nesmí být NULL.
 */
void gen_function_begin(const char *name);

/**
 * @brief Konec funkce: vypíše LABEL, PUSHFRAME, všechny DEFVAR a potom tělo funkce.
 */
void gen_function_end(void);

/**
 * @brief Definuje lokální proměnnou funkce. DEFVAR se vypíše na začátek funkce (před tělo),
 *        takže se neopakuje v cyklech. Smí se volat jen uvnitř funkce.
 * @param name Jedinečné jméno proměnné (SymbolData.gen_name).
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
 *        Vygeneruje CREATEFRAME, DEFVAR TF@$p<i>, POPS TF@$p<i> (odzadu) a CALL $func_<name>.
 * @param name  Jméno volané funkce.
 * @param count Počet argumentů.
 */
void gen_call(const char *name, size_t count);

/**
 * @brief Vloží literál typu Int na datový zásobník.
 * @param value Hodnota.
 */
void gen_push_int(int value);

/**
 * @brief Vloží literál typu Double na datový zásobník.
 * @param value Hodnota.
 */
void gen_push_float(double value);

/**
 * @brief Vloží literál typu String na datový zásobník (s escape sekvencemi).
 * @param str Řetězec zakončený nulou, nesmí být NULL.
 */
void gen_push_string(const char *str);

/**
 * @brief Vloží hodnotu lokální proměnné na datový zásobník.
 * @param gen_name Jedinečné jméno proměnné pro generátor (SymbolData.gen_name).
 */
void gen_push_var(const char *gen_name);

/**
 * @brief Provede binární operaci nad dvěma hodnotami na vrcholu zásobníku (levý pod pravým).
 *        Řeší převod Int na Double, celočíselné a desetinné dělení, konkatenaci a relace.
 *        Výsledek (číslo, řetězec nebo Boolean) zůstane na zásobníku.
 * @param op    TOKEN_PLUS, MINUS, NAS, DEL, MENSI, VETSI, MENSI_ROVNO, VETSI_ROVNO, STEJNO, NE_ROVNO.
 * @param left  Typ levého operandu.
 * @param right Typ pravého operandu.
 */
void gen_binary(TokenType op, DataType left, DataType right);

/**
 * Zkratové && a || (BOOLTHEN), první část: volat hned po načtení operátoru,
 * když je levý operand na datovém zásobníku. Vyzvedne ho a při zkratu skočí na výsledek.
 * @param op TOKEN_AND nebo TOKEN_OR.
 */
void gen_logic_begin(TokenType op);

/**
 * Zkratové && a || (BOOLTHEN), druhá část: volat při redukci E op E,
 * když je pravý operand na datovém zásobníku. Vloží výsledek zkratu a koncové návěští.
 * @param op TOKEN_AND nebo TOKEN_OR.
 */
void gen_logic_end(TokenType op);

/**
 * CONTROL: po if použitém jako výraz s větvemi Int a Double převede výsledek na Double, pokud je na zásobníku Int.
 * Typ se zjišťuje za běhu, protože při generování první větve ještě není známý typ druhé.
 * @param id Číslo z gen_new_label_id() pro návěští.
 */
void gen_if_to_double(int id);

/**
 * CONTROL: cyklus for, příprava. Ze zásobníku vyzvedne od, do a případně krok (krok nahoře), uloží je do řídicí
 * proměnné a skrytých proměnných cyklu. Při zadaném kroku za běhu kontroluje, že není nulový (EXIT 40).
 * @param var Jméno řídicí proměnné (už definované přes gen_defvar_local).
 */
void gen_for_init(const char *var, int id, bool has_step);

/** CONTROL: začátek cyklu for a kontrola rozsahu. inclusive je true pro to, false pro until. */
void gen_for_begin(const char *var, int id, bool inclusive);

/** CONTROL: stráž cyklu for. Hodnotu podmínky vyjme ze zásobníku, při nepravdě přeskočí tělo. */
void gen_for_guard(int id);

/** CONTROL: konec cyklu for. Přičte krok, skočí na začátek a vloží návěští konce. */
void gen_for_end(const char *var, int id);

/**
 * @brief Unární mínus nad hodnotou na vrcholu zásobníku.
 * @param type TYPE_INT nebo TYPE_DOUBLE.
 */
void gen_unary_minus(DataType type);

/**
 * @brief Vyjme hodnotu z vrcholu zásobníku a uloží ji do lokální proměnné.
 * @param gen_name Jedinečné jméno proměnné.
 */
void gen_store_var(const char *gen_name);

/**
 * @brief Vyjme hodnotu z vrcholu zásobníku a vypíše ji (print výrazu).
 */
void gen_print_stack(void);

/**
 * @brief Definuje parametry funkce: DEFVAR LF@<jmeno> a MOVE LF@<jmeno> LF@$p<i> (kopie z volání).
 *        Volá se hned po gen_function_begin.
 * @param names Jedinečná jména parametrů pro generátor (SymbolData.gen_name), smí být NULL, když count == 0.
 * @param count Počet parametrů.
 */
void gen_function_params(const char *const *names, size_t count);

/**
 * @brief Definuje jeden parametr funkce: DEFVAR LF@<jmeno> a MOVE LF@<jmeno> LF@$p<index>.
 *        Volá se po gen_function_begin, pro každý parametr v pořadí z definice.
 * @param name  Jedinečné jméno parametru pro generátor (SymbolData.gen_name), nesmí být NULL.
 * @param index Pořadí parametru od nuly.
 */
void gen_function_param(const char *name, size_t index);

/**
 * @brief Začátek if: vyjme podmínku (bool) ze zásobníku a při false skočí na $else_<id>.
 * @param id Unikátní číslo z gen_new_label_id(), stejné pro celý if.
 */
void gen_if_begin(int id);

/**
 * @brief Přechod na větev else: JUMP $endif_<id> a LABEL $else_<id>.
 * @param id Stejné číslo jako u gen_if_begin.
 */
void gen_if_else(int id);

/**
 * @brief Konec if: LABEL $endif_<id>.
 * @param id Stejné číslo jako u gen_if_begin.
 */
void gen_if_end(int id);

/**
 * @brief Začátek while: vypíše LABEL $while_start_<id>. Volá se před vyhodnocením podmínky.
 * @param id Unikátní číslo z gen_new_label_id(), stejné pro celý while.
 */
void gen_while_begin(int id);

/**
 * @brief Po vyhodnocení podmínky: vyjme bool ze zásobníku a při false skočí na $while_end_<id>.
 * @param id Stejné číslo jako u gen_while_begin.
 */
void gen_while_cond(int id);

/**
 * @brief Konec while: JUMP $while_start_<id> a LABEL $while_end_<id>. Volá se za tělem cyklu.
 * @param id Stejné číslo jako u gen_while_begin.
 */
void gen_while_end(int id);

/**
 * @brief Vygeneruje definice všech vestavěných funkcí (print, readLine, readInt, readDouble,
 *        toStr, toInt, strLength, subStr, ord, chr). Volá se jednou, za gen_program_start.
 *        Volají se stejně jako uživatelské funkce přes gen_call.
 */
void gen_builtins(void);

#endif
