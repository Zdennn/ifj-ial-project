/**
 * @file main.c
 * @brief Vstupni bod aplikace.
 *
 * @author ...
 */

#include "error.h"
#include "generator.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    gen_init(stdout);
    int result = parse();
    if (result != ERR_OK)
    {
        printError(result);
    }

    return 0;
}