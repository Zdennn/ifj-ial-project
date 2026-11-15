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
    failed += check("program start", s, "DEFVAR GF@$retval\nJUMP $func_main\n");
    free(s);

    cap_begin();
    gen_function_begin("foo");
    s = cap_end();
    failed += check("function begin", s, "LABEL $func_foo\nPUSHFRAME\n");
    free(s);

    cap_begin();
    gen_defvar_local("x");
    s = cap_end();
    failed += check("defvar local", s, "DEFVAR LF@x\n");
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
    gen_call("foo", NULL, 0);
    s = cap_end();
    failed += check("call bez parametru", s, "CREATEFRAME\nCALL $func_foo\n");
    free(s);

    const char *params[] = {"a", "b"};
    cap_begin();
    gen_call("add", params, 2);
    s = cap_end();
    failed += check("call se dvema parametry", s,
                    "CREATEFRAME\n"
                    "DEFVAR TF@a\n"
                    "DEFVAR TF@b\n"
                    "POPS TF@b\n"
                    "POPS TF@a\n"
                    "CALL $func_add\n");
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


    fprintf(stdout, "Fail %d\n", failed);

    return failed;
}