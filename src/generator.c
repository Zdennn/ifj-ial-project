/**
 * @file generator.c
 * @brief Implementace generátoru kódu.
 *
 * TODO - Popisek
 *
 * @author Tadeáš Trynkl, Jonáš Javůrek, Zdeněk Michalec
 */

 #include "generator.h"

 static FILE *out_stream = NULL;
 
void gen_init(FILE *out)
{
    out_stream = out; 
}

void gen_header(void)
{
    fprintf(out_stream,".IFJcode26\n");
}
