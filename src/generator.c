/**
 * @file generator.c
 * @brief Implementace generátoru kódu.
 *
 * Generuje cílový kód IFJcode26 
 * (instrukce, konstanty, návěští, funkce a výrazy na datovém zásobníku)
 *
 * @author Tadeáš Trynkl, Jonáš Javůrek, Zdeněk Michalec
 */

#include "generator.h"
#include "error.h"

#include <stdlib.h>
#include <stdbool.h>

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
        fprintf(stderr,"gen: gen_write_string má prazdný str\n");
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
    gen_instr("DEFVAR", "GF@$tmp1", NULL, NULL);
    gen_instr("DEFVAR", "GF@$tmp2", NULL, NULL);
    gen_instr("DEFVAR", "GF@$tmp3", NULL, NULL);
    gen_instr("JUMP", label, NULL, NULL);
}

/* Stav rozpracované funkce: tělo a DEFVAR se drží zvlášť v paměti. */
static FILE *final_out = NULL;
static FILE *body_stream = NULL;
static FILE *defs_stream = NULL;
static char *body_buf = NULL;
static char *defs_buf = NULL;
static size_t body_size = 0;
static size_t defs_size = 0;
static char current_label[GEN_NAME_MAX];

void gen_function_begin(const char *name)
{
    require_stream();

    if (name == NULL)
    {
        internal_error("gen_function_begin dostal NULL");
    }
    if (body_stream != NULL)
    {
        internal_error("gen_function_begin: predchozi funkce nebyla ukoncena");
    }

    gen_func_label(current_label, sizeof(current_label), name);

    body_stream = open_memstream(&body_buf, &body_size);
    defs_stream = open_memstream(&defs_buf, &defs_size);
    if (body_stream == NULL || defs_stream == NULL)
    {
        internal_error("gen_function_begin: open_memstream selhal");
    }

    final_out = out_stream;
    out_stream = body_stream;
}

void gen_defvar_local(const char *name)
{
    if (defs_stream == NULL)
    {
        internal_error("gen_defvar_local mimo funkci");
    }

    char var[GEN_NAME_MAX];
    gen_var(var, sizeof(var), "LF", name);
    fprintf(defs_stream, "DEFVAR %s\n", var);
}

void gen_function_end(void)
{
    if (body_stream == NULL)
    {
        internal_error("gen_function_end mimo funkci");
    }

    fclose(body_stream);
    fclose(defs_stream);
    body_stream = NULL;
    defs_stream = NULL;

    out_stream = final_out;

    gen_label(current_label);
    gen_instr("PUSHFRAME", NULL, NULL, NULL);
    fputs(defs_buf, out_stream);
    fputs(body_buf, out_stream);

    free(defs_buf);
    free(body_buf);
    defs_buf = NULL;
    body_buf = NULL;
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

void gen_call(const char *name, size_t count)
{
    if (name == NULL)
    {
        internal_error("gen_call dostal NULL");
    }

    char var[GEN_NAME_MAX];
    char label[GEN_NAME_MAX];

    gen_instr("CREATEFRAME", NULL, NULL, NULL);

    for (size_t i = 0; i < count; i++)
    {
        snprintf(var, sizeof(var), "TF@$p%zu", i);
        gen_instr("DEFVAR", var, NULL, NULL);
    }

    /* Argumenty byly pushnuty zleva doprava, takže poslední je na vrcholu. */
    for (size_t i = count; i > 0; i--)
    {
        snprintf(var, sizeof(var), "TF@$p%zu", i - 1);
        gen_instr("POPS", var, NULL, NULL);
    }

    gen_func_label(label, sizeof(label), name);
    gen_instr("CALL", label, NULL, NULL);
}

void gen_push_int(int value)
{
    require_stream();
    fputs("PUSHS ", out_stream);
    gen_write_int(value);
    fputc('\n', out_stream);
}

void gen_push_float(double value)
{
    require_stream();
    fputs("PUSHS ", out_stream);
    gen_write_float(value);
    fputc('\n', out_stream);
}

void gen_push_string(const char *str)
{
    require_stream();
    fputs("PUSHS ", out_stream);
    gen_write_string(str);
    fputc('\n', out_stream);
}

void gen_push_var(const char *gen_name)
{
    if (gen_name == NULL)
    {
        internal_error("gen_push_var dostal NULL");
    }

    char var[GEN_NAME_MAX];
    gen_var(var, sizeof(var), "LF", gen_name);
    gen_push(var);
}

static bool is_numeric(DataType t)
{
    return t == TYPE_INT || t == TYPE_DOUBLE;
}

/* Sjednotí typy číselných operandů: Int se převede na Double, pokud je druhý Double. */
static void promote(DataType left, DataType right)
{
    if (left == TYPE_INT && right == TYPE_DOUBLE)
    {
        /* levý operand leží pod pravým, takže pravý dočasně vyjmeme */
        gen_instr("POPS", "GF@$tmp1", NULL, NULL);
        gen_instr("INT2FLOATS", NULL, NULL, NULL);
        gen_instr("PUSHS", "GF@$tmp1", NULL, NULL);
    }
    else if (left == TYPE_DOUBLE && right == TYPE_INT)
    {
        gen_instr("INT2FLOATS", NULL, NULL, NULL);
    }
}

static const char *arith_instr(TokenType op, bool ints)
{
    switch (op)
    {
    case TOKEN_PLUS:
        return "ADDS";
    case TOKEN_MINUS:
        return "SUBS";
    case TOKEN_NAS:
        return "MULS";
    default:
        return ints ? "IDIVS" : "DIVS";
    }
}

void gen_binary(TokenType op, DataType left, DataType right)
{
    require_stream();

    bool numeric = is_numeric(left) && is_numeric(right);

    /* Konkatenace řetězců: CONCAT nemá zásobníkovou verzi. */
    if (op == TOKEN_PLUS && left == TYPE_STRING && right == TYPE_STRING)
    {
        gen_instr("POPS", "GF@$tmp2", NULL, NULL);
        gen_instr("POPS", "GF@$tmp1", NULL, NULL);
        gen_instr("CONCAT", "GF@$tmp3", "GF@$tmp1", "GF@$tmp2");
        gen_instr("PUSHS", "GF@$tmp3", NULL, NULL);
        return;
    }

    /* Aritmetika */
    if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_NAS || op == TOKEN_DEL)
    {
        if (!numeric)
        {
            internal_error("gen_binary: aritmetika nad necislem");
        }
        promote(left, right);
        bool ints = (left == TYPE_INT && right == TYPE_INT);
        gen_instr(arith_instr(op, ints), NULL, NULL, NULL);
        return;
    }

    /* Relace */
    if (!numeric && left != right)
    {
        internal_error("gen_binary: porovnani ruznych typu");
    }
    if (numeric)
    {
        promote(left, right);
    }

    switch (op)
    {
    case TOKEN_MENSI:
        gen_instr("LTS", NULL, NULL, NULL);
        break;
    case TOKEN_VETSI:
        gen_instr("GTS", NULL, NULL, NULL);
        break;
    case TOKEN_MENSI_ROVNO: /* a <= b je not(a > b) */
        gen_instr("GTS", NULL, NULL, NULL);
        gen_instr("NOTS", NULL, NULL, NULL);
        break;
    case TOKEN_VETSI_ROVNO: /* a >= b je not(a < b) */
        gen_instr("LTS", NULL, NULL, NULL);
        gen_instr("NOTS", NULL, NULL, NULL);
        break;
    case TOKEN_STEJNO:
        gen_instr("EQS", NULL, NULL, NULL);
        break;
    case TOKEN_NE_ROVNO:
        gen_instr("EQS", NULL, NULL, NULL);
        gen_instr("NOTS", NULL, NULL, NULL);
        break;
    default:
        internal_error("gen_binary: neznamy operator");
    }
}

void gen_unary_minus(DataType type)
{
    /* Násobení -1 zachová i záporné nuly u desetinných čísel. */
    if (type == TYPE_INT)
    {
        gen_push_int(-1);
    }
    else if (type == TYPE_DOUBLE)
    {
        gen_push_float(-1.0);
    }
    else
    {
        internal_error("gen_unary_minus: nepodporovany typ");
    }
    gen_instr("MULS", NULL, NULL, NULL);
}

void gen_store_var(const char *gen_name)
{
    if (gen_name == NULL)
    {
        internal_error("gen_store_var dostal NULL");
    }

    char var[GEN_NAME_MAX];
    gen_var(var, sizeof(var), "LF", gen_name);
    gen_pop(var);
}

void gen_print_stack(void)
{
    gen_instr("POPS", "GF@$tmp1", NULL, NULL);
    gen_instr("WRITE", "GF@$tmp1", NULL, NULL);
}