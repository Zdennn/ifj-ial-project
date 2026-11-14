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
