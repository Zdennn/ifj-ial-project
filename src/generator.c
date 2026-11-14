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
 
void gen_init(FILE *out)
{
    if (out == NULL)
    {
        fprintf(stderr,"generator: gen_init dostal out NULL\n");
        exit(ERR_INTERNAL);
    }

    out_stream = out; 
}

void gen_header(void)
{
    if (out_stream == NULL)
    {
        fprintf(stderr,"generator: gen_init nebyl zavolan\n");
        exit(ERR_INTERNAL);
    }
    
    fprintf(out_stream,".IFJcode26\n");
}
