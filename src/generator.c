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

int gen_new_label_id(void)
{
    static int label_counter = 0;
    label_counter++;

    return label_counter;
}

void gen_label_name(char *buf, size_t size, const char *prefix, int id)
{
    if (buf == NULL || prefix == NULL)
    {
        fprintf(stderr,"gen: gen_label_name dostal NULL za buf nebo prefix");
        exit(ERR_INTERNAL);
    }else
    {
        snprintf(buf, size, "$%s_%d", prefix, id);
    }    
}

void gen_label(const char *name)
{
    if (name == NULL)
    {
        fprintf(stderr,"gen: gen_label dostal NULL");
        exit(ERR_INTERNAL);
    }
    gen_instr("LABEL", name, NULL, NULL);
}