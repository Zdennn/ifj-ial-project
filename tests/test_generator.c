/**
 * @file test_generator.c
 * @brief Testy pro generator.c .
 *
 * Testy pro správnost jednotlivých výstupů.
 *
 * @author Trynkl Tadeáš, Javůrek Jonáš, Zdeněk Michalec
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generator.h"

static int check(const char *name, const char *got, const char *expected)
{
    if (name == NULL || got == NULL || expected == NULL)
    {
        fprintf(stderr, "test: check dostal NULL parametr\n");
        return 1;
    }

    if ((strcmp(got, expected)) == 0)
    {
        fprintf(stdout, "PASS: %s\n", name);
        return 0;
    }
    else
    {
        fprintf(stderr, "FAIL:%s\n expected:[%s]\n  got:[%s]\n", name, expected, got);
        return 1;
    }
}

/* Ověří, že výstup obsahuje očekávaný úsek textu. Vrací 0 při shodě, jinak 1. */
static int check_contains(const char *name, const char *got, const char *expected_part)
{
    if (name == NULL || got == NULL || expected_part == NULL)
    {
        fprintf(stderr, "test: check_contains dostal NULL parametr\n");
        return 1;
    }

    if (strstr(got, expected_part) != NULL)
    {
        fprintf(stdout, "PASS: %s\n", name);
        return 0;
    }

    fprintf(stderr, "FAIL: %s\n expected to contain:[%s]\n  got:[%s]\n", name, expected_part, got);
    return 1;
}

static int test_header(void)
{
    char *buf = NULL;
    size_t size = 0;

    FILE *f = open_memstream(&buf, &size);

    if (f == NULL)
    {
        fprintf(stderr, "gen: chyba pri otevirani memstreamu\n");
        return 1;
    }

    gen_init(f);
    gen_header();

    fclose(f);

    int failed = check("gen_header", buf, ".IFJcode26\n");

    free(buf);
    return failed;
}

static char *capture_int(int value)
{
    char *buf = NULL;
    size_t size = 0;

    FILE *f = open_memstream(&buf, &size);
    if (f == NULL)
    {
        return NULL;
    }

    gen_init(f);
    gen_write_int(value);
    fclose(f);

    return buf;
}

static int test_write_int(void)
{
    int failed = 0;
    char *s;

    s = capture_int(5);
    failed += check("int 5", s, "int@5");
    free(s);

    s = capture_int(-5);
    failed += check("int -5", s, "int@-5");
    free(s);

    s = capture_int(0);
    failed += check("int 0", s, "int@0");
    free(s);

    return failed;
}

static char *capture_float(double value)
{
    char *buf = NULL;
    size_t size = 0;

    FILE *f = open_memstream(&buf, &size);

    if (f == NULL)
    {
        return NULL;
    }

    gen_init(f);
    gen_write_float(value);
    fclose(f);

    return buf;
}

static int test_write_float(void)
{
    int failed = 0;
    char *s;

    s = capture_float(1.0);
    failed += check("float 1.0", s, "float@0x1p+0");
    free(s);

    s = capture_float(0.0);
    failed += check("float 0.0", s, "float@0x0p+0");
    free(s);

    s = capture_float(-2.5);
    failed += check("float -2.5", s, "float@-0x1.4p+1");
    free(s);

    s = capture_float(1.1);
    failed += check("float 1.1", s, "float@0x1.199999999999ap+0");
    free(s);

    return failed;
}

static char *capture_string(const char *str)
{
    char *buf = NULL;
    size_t size = 0;

    FILE *f = open_memstream(&buf, &size);
    if (f == NULL)
    {
        return NULL;
    }

    gen_init(f);
    gen_write_string(str);
    fclose(f);

    return buf;
}

static int test_write_string(void)
{
    int failed = 0;
    char *s;

    s = capture_string("ahoj");
    failed += check("string ahoj", s, "string@ahoj");
    free(s);

    s = capture_string("");
    failed += check("string prazdny", s, "string@");
    free(s);

    s = capture_string("a b");
    failed += check("string mezera", s, "string@a\\032b");
    free(s);

    s = capture_string("a\nb");
    failed += check("string novy radek", s, "string@a\\010b");
    free(s);

    s = capture_string("a#b");
    failed += check("string mrizka", s, "string@a\\035b");
    free(s);

    s = capture_string("a\\b");
    failed += check("string zpetne lomitko", s, "string@a\\092b");
    free(s);

    s = capture_string("retezec s lomitkem \\ a\nnovym#radkem");
    failed +=
        check("string priklad ze zadani", s, "string@retezec\\032s\\032lomitkem\\032\\092\\032a\\010novym\\035radkem");
    free(s);

    return failed;
}

static char *capture_instr(const char *opcode, const char *a, const char *b, const char *c)
{
    char *buf = NULL;
    size_t size = 0;

    FILE *f = open_memstream(&buf, &size);
    if (f == NULL)
    {
        return NULL;
    }

    gen_init(f);
    gen_instr(opcode, a, b, c);
    fclose(f);

    return buf;
}

static int test_instr(void)
{
    int failed = 0;
    char *s;

    s = capture_instr("CREATEFRAME", NULL, NULL, NULL);
    failed += check("instr bez operandu", s, "CREATEFRAME\n");
    free(s);

    s = capture_instr("WRITE", "string@ahoj", NULL, NULL);
    failed += check("instr jeden operand", s, "WRITE string@ahoj\n");
    free(s);

    s = capture_instr("MOVE", "LF@x", "int@5", NULL);
    failed += check("instr dva operandy", s, "MOVE LF@x int@5\n");
    free(s);

    s = capture_instr("ADD", "LF@x", "LF@a", "int@5");
    failed += check("instr tri operandy", s, "ADD LF@x LF@a int@5\n");
    free(s);

    return failed;
}

static int test_labels(void)
{
    int failed = 0;
    char *s;
    char name[64];

    /* každé volání vrací jiné číslo */
    int id1 = gen_new_label_id();
    int id2 = gen_new_label_id();
    if (id1 == id2)
    {
        fprintf(stderr, "FAIL: label id se opakuje\n");
        failed++;
    }
    else
    {
        fprintf(stdout, "PASS: label id unikatni\n");
    }

    /* skladani jmena */
    gen_label_name(name, sizeof(name), "while_start", 3);
    failed += check("label jmeno", name, "$while_start_3");

    /* instrukce LABEL */
    char *buf = NULL;
    size_t size = 0;
    FILE *f = open_memstream(&buf, &size);
    if (f == NULL)
    {
        return failed + 1;
    }
    gen_init(f);
    gen_label("$while_start_3");
    fclose(f);
    s = buf;
    failed += check("gen_label", s, "LABEL $while_start_3\n");
    free(s);

    return failed;
}

static char *capture_print(const char *operand)
{
    char *buf = NULL;
    size_t size = 0;

    FILE *f = open_memstream(&buf, &size);
    if (f == NULL)
    {
        return NULL;
    }

    gen_init(f);
    if (operand == NULL)
    {
        gen_print_unit();
    }
    else
    {
        gen_print(operand);
    }
    fclose(f);

    return buf;
}

static int test_print(void)
{
    int failed = 0;
    char *s;

    s = capture_print("int@5");
    failed += check("print int", s, "WRITE int@5\n");
    free(s);

    s = capture_print("string@ahoj");
    failed += check("print string", s, "WRITE string@ahoj\n");
    free(s);

    s = capture_print("LF@x");
    failed += check("print promenna", s, "WRITE LF@x\n");
    free(s);

    s = capture_print(NULL);
    failed += check("print unit", s, "WRITE string@()\n");
    free(s);

    return failed;
}

static FILE *g_file = NULL;
static char *g_buf = NULL;
static size_t g_size = 0;

/* Začne zachytávat výstup generátoru do paměti. */
static void cap_begin(void)
{
    g_buf = NULL;
    g_size = 0;
    g_file = open_memstream(&g_buf, &g_size);
    if (g_file == NULL)
    {
        fprintf(stderr, "test: open_memstream selhal\n");
        exit(1);
    }
    gen_init(g_file);
}

/* Ukončí zachytávání a vrátí text, volající ho musí free(). */
static char *cap_end(void)
{
    fclose(g_file);
    g_file = NULL;
    return g_buf;
}

static int test_functions(void)
{
    int failed = 0;
    char *s;
    char name[GEN_NAME_MAX];

    gen_var(name, sizeof(name), "LF", "x");
    failed += check("gen_var", name, "LF@x");

    gen_func_label(name, sizeof(name), "foo");
    failed += check("gen_func_label", name, "$func_foo");

    cap_begin();
    gen_program_start("main");
    s = cap_end();
    failed += check("program start", s,
                    "DEFVAR GF@$retval\nDEFVAR GF@$tmp1\nDEFVAR GF@$tmp2\nDEFVAR GF@$tmp3\n"
                    "CREATEFRAME\nCALL $func_main\nEXIT int@0\n");
    free(s);

    /* DEFVAR se vypíše před tělo, i když vznikl až po něm */
    cap_begin();
    gen_function_begin("foo");
    gen_defvar_local("x$0");
    gen_instr("MOVE", "LF@x$0", "int@1", NULL);
    gen_defvar_local("y$1");
    gen_function_end();
    s = cap_end();
    failed +=
        check("function begin end", s, "LABEL $func_foo\nPUSHFRAME\nDEFVAR LF@x$0\nDEFVAR LF@y$1\nMOVE LF@x$0 int@1\n");
    free(s);

    /* prázdná funkce */
    cap_begin();
    gen_function_begin("bar");
    gen_function_end();
    s = cap_end();
    failed += check("function prazdna", s, "LABEL $func_bar\nPUSHFRAME\n");
    free(s);

    cap_begin();
    gen_function_return("LF@x");
    s = cap_end();
    failed += check("function return", s, "MOVE GF@$retval LF@x\nPOPFRAME\nRETURN\n");
    free(s);

    cap_begin();
    gen_function_return_unit();
    s = cap_end();
    failed += check("function return unit", s, "MOVE GF@$retval string@()\nPOPFRAME\nRETURN\n");
    free(s);

    cap_begin();
    gen_push("int@5");
    gen_pop("LF@x");
    gen_push_retval();
    s = cap_end();
    failed += check("push pop retval", s, "PUSHS int@5\nPOPS LF@x\nPUSHS GF@$retval\n");
    free(s);

    cap_begin();
    gen_call("foo", 0);
    s = cap_end();
    failed += check("call bez parametru", s, "CREATEFRAME\nCALL $func_foo\n");
    free(s);

    cap_begin();
    gen_call("add", 2);
    s = cap_end();
    failed += check("call se dvema parametry", s,
                    "CREATEFRAME\n"
                    "DEFVAR TF@$p0\n"
                    "DEFVAR TF@$p1\n"
                    "POPS TF@$p1\n"
                    "POPS TF@$p0\n"
                    "CALL $func_add\n");
    free(s);

    /* parametry funkce */
    const char *names[] = {"a$0", "b$1"};
    cap_begin();
    gen_function_begin("add");
    gen_function_params(names, 2);
    gen_function_end();
    s = cap_end();
    failed += check("function params", s,
                    "LABEL $func_add\nPUSHFRAME\n"
                    "DEFVAR LF@a$0\nDEFVAR LF@b$1\n"
                    "MOVE LF@a$0 LF@$p0\nMOVE LF@b$1 LF@$p1\n");
    free(s);

    /* parametry funkce po jednom */
    cap_begin();
    gen_function_begin("f");
    gen_function_param("a$0", 0);
    gen_function_param("b$1", 1);
    gen_function_end();
    s = cap_end();
    failed += check("function param po jednom", s,
                    "LABEL $func_f\nPUSHFRAME\n"
                    "DEFVAR LF@a$0\nDEFVAR LF@b$1\n"
                    "MOVE LF@a$0 LF@$p0\nMOVE LF@b$1 LF@$p1\n");
    free(s);

    return failed;
}

static int check_binary(const char *name, TokenType op, DataType l, DataType r, const char *expected)
{
    cap_begin();
    gen_binary(op, l, r);
    char *s = cap_end();
    int failed = check(name, s, expected);
    free(s);
    return failed;
}

static int test_expressions(void)
{
    int failed = 0;
    char *s;

    /* literály a proměnné */
    cap_begin();
    gen_push_int(5);
    gen_push_float(1.0);
    gen_push_string("a b");
    gen_push_var("x$3");
    s = cap_end();
    failed += check("push literaly", s, "PUSHS int@5\nPUSHS float@0x1p+0\nPUSHS string@a\\032b\nPUSHS LF@x$3\n");
    free(s);

    /* aritmetika nad Int */
    failed += check_binary("int +", TOKEN_PLUS, TYPE_INT, TYPE_INT, "ADDS\n");
    failed += check_binary("int -", TOKEN_MINUS, TYPE_INT, TYPE_INT, "SUBS\n");
    failed += check_binary("int *", TOKEN_NAS, TYPE_INT, TYPE_INT, "MULS\n");
    failed += check_binary("int /", TOKEN_DEL, TYPE_INT, TYPE_INT, "IDIVS\n");

    /* Double */
    failed += check_binary("double /", TOKEN_DEL, TYPE_DOUBLE, TYPE_DOUBLE, "DIVS\n");

    /* konverze Int -> Double */
    failed += check_binary("int + double", TOKEN_PLUS, TYPE_INT, TYPE_DOUBLE,
                           "POPS GF@$tmp1\nINT2FLOATS\nPUSHS GF@$tmp1\nADDS\n");
    failed += check_binary("double * int", TOKEN_NAS, TYPE_DOUBLE, TYPE_INT, "INT2FLOATS\nMULS\n");
    failed += check_binary("int / double", TOKEN_DEL, TYPE_INT, TYPE_DOUBLE,
                           "POPS GF@$tmp1\nINT2FLOATS\nPUSHS GF@$tmp1\nDIVS\n");

    /* konkatenace */
    failed += check_binary("string +", TOKEN_PLUS, TYPE_STRING, TYPE_STRING,
                           "POPS GF@$tmp2\nPOPS GF@$tmp1\nCONCAT GF@$tmp3 GF@$tmp1 GF@$tmp2\nPUSHS GF@$tmp3\n");

    /* relace */
    failed += check_binary("int <", TOKEN_MENSI, TYPE_INT, TYPE_INT, "LTS\n");
    failed += check_binary("int >", TOKEN_VETSI, TYPE_INT, TYPE_INT, "GTS\n");
    failed += check_binary("int <=", TOKEN_MENSI_ROVNO, TYPE_INT, TYPE_INT, "GTS\nNOTS\n");
    failed += check_binary("int >=", TOKEN_VETSI_ROVNO, TYPE_INT, TYPE_INT, "LTS\nNOTS\n");
    failed += check_binary("string ==", TOKEN_STEJNO, TYPE_STRING, TYPE_STRING, "EQS\n");
    failed += check_binary("int !=", TOKEN_NE_ROVNO, TYPE_INT, TYPE_INT, "EQS\nNOTS\n");
    failed += check_binary("int == double", TOKEN_STEJNO, TYPE_INT, TYPE_DOUBLE,
                           "POPS GF@$tmp1\nINT2FLOATS\nPUSHS GF@$tmp1\nEQS\n");

    /* unární mínus */
    cap_begin();
    gen_unary_minus(TYPE_INT);
    s = cap_end();
    failed += check("unary minus int", s, "PUSHS int@-1\nMULS\n");
    free(s);

    cap_begin();
    gen_unary_minus(TYPE_DOUBLE);
    s = cap_end();
    failed += check("unary minus double", s, "PUSHS float@-0x1p+0\nMULS\n");
    free(s);

    /* uložení a výpis ze zásobníku */
    cap_begin();
    gen_store_var("x$3");
    gen_print_stack();
    s = cap_end();
    failed += check("store a print", s, "POPS LF@x$3\nPOPS GF@$tmp1\nWRITE GF@$tmp1\n");
    free(s);

    return failed;
}

static int test_if(void)
{
    int failed = 0;
    char *s;

    cap_begin();
    gen_if_begin(7);
    gen_instr("WRITE", "string@a", NULL, NULL);
    gen_if_else(7);
    gen_instr("WRITE", "string@b", NULL, NULL);
    gen_if_end(7);
    s = cap_end();
    failed += check("if else", s,
                    "POPS GF@$tmp1\n"
                    "JUMPIFEQ $else_7 GF@$tmp1 bool@false\n"
                    "WRITE string@a\n"
                    "JUMP $endif_7\n"
                    "LABEL $else_7\n"
                    "WRITE string@b\n"
                    "LABEL $endif_7\n");
    free(s);

    /* dva různé if mají různá návěští */
    cap_begin();
    gen_if_begin(1);
    gen_if_else(1);
    gen_if_end(1);
    gen_if_begin(2);
    gen_if_else(2);
    gen_if_end(2);
    s = cap_end();
    failed += check("dva if", s,
                    "POPS GF@$tmp1\n"
                    "JUMPIFEQ $else_1 GF@$tmp1 bool@false\n"
                    "JUMP $endif_1\n"
                    "LABEL $else_1\n"
                    "LABEL $endif_1\n"
                    "POPS GF@$tmp1\n"
                    "JUMPIFEQ $else_2 GF@$tmp1 bool@false\n"
                    "JUMP $endif_2\n"
                    "LABEL $else_2\n"
                    "LABEL $endif_2\n");
    free(s);

    return failed;
}

static int test_while(void)
{
    int failed = 0;
    char *s;

    /* pořadí volání odpovídá parseru: begin, (podmínka), cond, (tělo), end */
    cap_begin();
    gen_while_begin(3);
    gen_push_int(1);
    gen_while_cond(3);
    gen_instr("WRITE", "string@a", NULL, NULL);
    gen_while_end(3);
    s = cap_end();
    failed += check("while", s,
                    "LABEL $while_start_3\n"
                    "PUSHS int@1\n"
                    "POPS GF@$tmp1\n"
                    "JUMPIFEQ $while_end_3 GF@$tmp1 bool@false\n"
                    "WRITE string@a\n"
                    "JUMP $while_start_3\n"
                    "LABEL $while_end_3\n");
    free(s);

    /* vnořené while: každé má vlastní návěští */
    cap_begin();
    gen_while_begin(1);
    gen_while_cond(1);
    gen_while_begin(2);
    gen_while_cond(2);
    gen_while_end(2);
    gen_while_end(1);
    s = cap_end();
    failed += check("while vnorene", s,
                    "LABEL $while_start_1\n"
                    "POPS GF@$tmp1\n"
                    "JUMPIFEQ $while_end_1 GF@$tmp1 bool@false\n"
                    "LABEL $while_start_2\n"
                    "POPS GF@$tmp1\n"
                    "JUMPIFEQ $while_end_2 GF@$tmp1 bool@false\n"
                    "JUMP $while_start_2\n"
                    "LABEL $while_end_2\n"
                    "JUMP $while_start_1\n"
                    "LABEL $while_end_1\n");
    free(s);

    return failed;
}

static int test_builtins(void)
{
    int failed = 0;
    char *s;

    cap_begin();
    gen_builtins();
    s = cap_end();

    failed += check_contains("builtin print", s,
                             "LABEL $func_print\n"
                             "PUSHFRAME\n"
                             "WRITE LF@$p0\n"
                             "MOVE GF@$retval string@()\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin readInt", s,
                             "LABEL $func_readInt\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "READ LF@$r int\n"
                             "JUMPIFNEQ $readInt_ok LF@$r nil@nil\n"
                             "MOVE LF@$r int@0\n"
                             "LABEL $readInt_ok\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin readDouble", s,
                             "LABEL $func_readDouble\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "READ LF@$r float\n"
                             "JUMPIFNEQ $readDouble_ok LF@$r nil@nil\n"
                             "MOVE LF@$r float@0x0p+0\n"
                             "LABEL $readDouble_ok\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin readLine", s,
                             "LABEL $func_readLine\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "READ LF@$r string\n"
                             "JUMPIFNEQ $readLine_ok LF@$r nil@nil\n"
                             "MOVE LF@$r string@\n"
                             "LABEL $readLine_ok\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin toInt", s,
                             "LABEL $func_toInt\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "FLOAT2INT LF@$r LF@$p0\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin strLength", s,
                             "LABEL $func_strLength\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "STRLEN LF@$r LF@$p0\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin chr", s,
                             "LABEL $func_chr\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "INT2CHAR LF@$r LF@$p0\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin toStr", s,
                             "LABEL $func_toStr\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "DEFVAR LF@$t\n"
                             "TYPE LF@$t LF@$p0\n"
                             "JUMPIFEQ $toStr_int LF@$t string@int\n"
                             "JUMPIFEQ $toStr_float LF@$t string@float\n"
                             "MOVE LF@$r LF@$p0\n"
                             "JUMP $toStr_end\n"
                             "LABEL $toStr_int\n"
                             "INT2STR LF@$r LF@$p0\n"
                             "JUMP $toStr_end\n"
                             "LABEL $toStr_float\n"
                             "FLOAT2STR LF@$r LF@$p0\n"
                             "LABEL $toStr_end\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin ord", s,
                             "LABEL $func_ord\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "DEFVAR LF@$len\n"
                             "DEFVAR LF@$b\n"
                             "STRLEN LF@$len LF@$p0\n"
                             "LT LF@$b LF@$p1 int@0\n"
                             "JUMPIFEQ $ord_bad LF@$b bool@true\n"
                             "LT LF@$b LF@$p1 LF@$len\n"
                             "JUMPIFEQ $ord_bad LF@$b bool@false\n"
                             "STRI2INT LF@$r LF@$p0 LF@$p1\n"
                             "JUMP $ord_end\n"
                             "LABEL $ord_bad\n"
                             "MOVE LF@$r int@-1\n"
                             "LABEL $ord_end\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n");

    failed += check_contains("builtin subStr", s,
                             "LABEL $func_subStr\n"
                             "PUSHFRAME\n"
                             "DEFVAR LF@$r\n"
                             "DEFVAR LF@$len\n"
                             "DEFVAR LF@$b\n"
                             "DEFVAR LF@$i\n"
                             "DEFVAR LF@$c\n"
                             "STRLEN LF@$len LF@$p0\n"
                             "LT LF@$b LF@$p1 int@0\n"
                             "JUMPIFEQ $subStr_err LF@$b bool@true\n"
                             "LT LF@$b LF@$p2 int@0\n"
                             "JUMPIFEQ $subStr_err LF@$b bool@true\n"
                             "GT LF@$b LF@$p1 LF@$p2\n"
                             "JUMPIFEQ $subStr_err LF@$b bool@true\n"
                             "LT LF@$b LF@$p1 LF@$len\n"
                             "JUMPIFEQ $subStr_err LF@$b bool@false\n"
                             "GT LF@$b LF@$p2 LF@$len\n"
                             "JUMPIFEQ $subStr_err LF@$b bool@true\n"
                             "MOVE LF@$r string@\n"
                             "MOVE LF@$i LF@$p1\n"
                             "LABEL $subStr_loop\n"
                             "LT LF@$b LF@$i LF@$p2\n"
                             "JUMPIFEQ $subStr_done LF@$b bool@false\n"
                             "GETCHAR LF@$c LF@$p0 LF@$i\n"
                             "CONCAT LF@$r LF@$r LF@$c\n"
                             "ADD LF@$i LF@$i int@1\n"
                             "JUMP $subStr_loop\n"
                             "LABEL $subStr_done\n"
                             "MOVE GF@$retval LF@$r\n"
                             "POPFRAME\n"
                             "RETURN\n"
                             "LABEL $subStr_err\n"
                             "EXIT int@40\n");

    free(s);

    return failed;
}

int main(void)
{
    int failed = 0;

    failed += test_header();
    failed += test_write_int();
    failed += test_write_float();
    failed += test_write_string();
    failed += test_instr();
    failed += test_labels();
    failed += test_print();
    failed += test_functions();
    failed += test_expressions();
    failed += test_if();
    failed += test_while();
    failed += test_builtins();

    fprintf(stdout, "Fail %d\n", failed);

    return failed;
}