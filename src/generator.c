/**
 * @file generator.c
 * @brief Implementace generátoru kódu.
 *
 * TODO - Popisek
 *
 * @author Tadeáš Trynkl, Jonáš Javůrek, Zdeněk Michalec
 */

#include "generator.h"
#include <stdlib.h>
#include "error.h"

static FILE *out_stream = NULL;

static void require_stream(void)
{
    if (out_stream == NULL)
    {
        fprintf(stderr,"gen: out_stream NULL\n");
        exit(ERR_INTERNAL);
    }
    
}

void gen_init(FILE *out)
{
    if (out == NULL)
    {
        fprintf(stderr,"gen: gen_init dostal NULL\n");
        exit(ERR_INTERNAL);
    }

    out_stream = out; 
}

void gen_header(void)
{
    require_stream();   
    fprintf(out_stream,".IFJcode26\n");
}

void gen_write_int(int value)
{
    require_stream();
    fprintf(out_stream, "int@%d", value);
}

void gen_write_float(double value)
{
    require_stream();
    fprintf(out_stream,"float@%a", value);
}

void gen_write_string(const char *str)
{
    require_stream();

    if (str == NULL)
    {
        fprintf(stderr,"gen: gen_write_string má prazdný str");
        exit(ERR_INTERNAL);
    }

    fprintf(out_stream, "string@");

    for (const unsigned char *p = (const unsigned char *)str; *p != '\0'; p++)
    {
        if (*p <= 32 || *p == 35 || *p == 92)
        {
            fprintf(out_stream, "\\%03d", *p);
        }else
        {
            fputc(*p, out_stream);
        }   
    }   
}

void gen_instr(const char *opcode, const char *a, const char *b, const char *c)
{
    require_stream();

    if (opcode == NULL)
    {
        fprintf(stderr, "gen: gen_instr dostal opcode NULL\n");
        exit(ERR_INTERNAL);
    }

    fprintf(out_stream, "%s", opcode);

    if (a != NULL)
    {
        fprintf(out_stream, " %s", a);
    }
    if (b != NULL)
    {
        fprintf(out_stream, " %s", b);
    }
    if (c != NULL)
    {
        fprintf(out_stream, " %s", c);
    }

    fputc('\n', out_stream);
}

static int label_counter = 0;

int gen_new_label_id(void)
{
    return label_counter++;
}

void gen_label_name(char *buf, size_t size, const char *prefix, int id)
{
    if (buf == NULL || prefix == NULL)
    {
        fprintf(stderr, "gen: gen_label_name dostal NULL za buf nebo prefix\n");
        exit(ERR_INTERNAL);
    }

    int n = snprintf(buf, size, "$%s_%d", prefix, id);
    if (n < 0 || (size_t)n >= size)
    {
        fprintf(stderr, "gen: gen_label_name, buffer je maly\n");
        exit(ERR_INTERNAL);
    }
}

void gen_label(const char *name)
{
    if (name == NULL)
    {
        fprintf(stderr, "gen: gen_label dostal NULL\n");
        exit(ERR_INTERNAL);
    }

    gen_instr("LABEL", name, NULL, NULL);
}

void gen_print(const char *operand)
{
    if (operand == NULL)
    {
        fprintf(stderr, "gen: gen_print dostal NULL\n");
        exit(ERR_INTERNAL);
    }

    gen_instr("WRITE", operand, NULL, NULL);
}

void gen_print_unit(void)
{
    gen_instr("WRITE", "string@()", NULL, NULL);
}

/* Ukončí překladač s interní chybou a hláškou na stderr. */
static void internal_error(const char *msg)
{
    fprintf(stderr, "gen: %s\n", msg);
    exit(ERR_INTERNAL);
}

void gen_var(char *buf, size_t size, const char *frame, const char *name)
{
    if (buf == NULL || frame == NULL || name == NULL)
    {
        internal_error("gen_var dostal NULL");
    }

    int n = snprintf(buf, size, "%s@%s", frame, name);
    if (n < 0 || (size_t)n >= size)
    {
        internal_error("gen_var, buffer je maly");
    }
}

void gen_func_label(char *buf, size_t size, const char *name)
{
    if (buf == NULL || name == NULL)
    {
        internal_error("gen_func_label dostal NULL");
    }

    int n = snprintf(buf, size, "$func_%s", name);
    if (n < 0 || (size_t)n >= size)
    {
        internal_error("gen_func_label, buffer je maly");
    }
}

void gen_program_start(const char *main_name)
{
    if (main_name == NULL)
    {
        internal_error("gen_program_start dostal NULL");
    }

    char label[GEN_NAME_MAX];
    gen_func_label(label, sizeof(label), main_name);

    gen_instr("DEFVAR", "GF@$retval", NULL, NULL);
    gen_instr("JUMP", label, NULL, NULL);
}

void gen_function_begin(const char *name)
{
    if (name == NULL)
    {
        internal_error("gen_function_begin dostal NULL");
    }

    char label[GEN_NAME_MAX];
    gen_func_label(label, sizeof(label), name);

    gen_instr("LABEL", label, NULL, NULL);
    gen_instr("PUSHFRAME", NULL, NULL, NULL);
}

void gen_defvar_local(const char *name)
{
    char var[GEN_NAME_MAX];
    gen_var(var, sizeof(var), "LF", name);
    gen_instr("DEFVAR", var, NULL, NULL);
}

void gen_function_return(const char *value)
{
    if (value == NULL)
    {
        internal_error("gen_function_return dostal NULL");
    }

    gen_instr("MOVE", "GF@$retval", value, NULL);
    gen_instr("POPFRAME", NULL, NULL, NULL);
    gen_instr("RETURN", NULL, NULL, NULL);
}

void gen_function_return_unit(void)
{
    gen_function_return("string@()");
}

void gen_push(const char *operand)
{
    if (operand == NULL)
    {
        internal_error("gen_push dostal NULL");
    }

    gen_instr("PUSHS", operand, NULL, NULL);
}

void gen_pop(const char *var)
{
    if (var == NULL)
    {
        internal_error("gen_pop dostal NULL");
    }

    gen_instr("POPS", var, NULL, NULL);
}

void gen_push_retval(void)
{
    gen_instr("PUSHS", "GF@$retval", NULL, NULL);
}

void gen_call(const char *name, const char *const *params, size_t count)
{
    if (name == NULL || (params == NULL && count > 0))
    {
        internal_error("gen_call dostal NULL");
    }

    char var[GEN_NAME_MAX];
    char label[GEN_NAME_MAX];

    gen_instr("CREATEFRAME", NULL, NULL, NULL);

    for (size_t i = 0; i < count; i++)
    {
        gen_var(var, sizeof(var), "TF", params[i]);
        gen_instr("DEFVAR", var, NULL, NULL);
    }

    /* Argumenty byly pushnuty zleva doprava, takže poslední je na vrcholu. */
    for (size_t i = count; i > 0; i--)
    {
        gen_var(var, sizeof(var), "TF", params[i - 1]);
        gen_instr("POPS", var, NULL, NULL);
    }

    gen_func_label(label, sizeof(label), name);
    gen_instr("CALL", label, NULL, NULL);
}