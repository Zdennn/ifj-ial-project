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


int main(void)
{
    int failed = 0;

    failed += test_header();
    failed += test_write_int();
    failed += test_write_float();

    fprintf(stdout, "Fail %d\n", failed);

    return failed;
}