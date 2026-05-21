#include <string.h>
#include "lexer.h"

/*
 * Splits `line` into whitespace-delimited tokens.
 * Writes pointers into argv (must hold max_args+1 slots).
 * Returns the number of tokens found.
 * argv[n] is always set to NULL (execvp-ready).
 */
int tokenize(char *line, char **argv, int max_args)
{
    int argc = 0;
    char *tok = strtok(line, " \t\n");

    while (tok != NULL && argc < max_args) {
        argv[argc++] = tok;
        tok = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    return argc;
}
