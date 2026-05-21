#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "../lexer/lexer.h"
#include "../executor/executor.h"

#define PROMPT SHELL_NAME "$ "

void shell_run(t_shell *sh)
{
    char  line[BUFF_SIZE];
    char *argv[MAX_ARGS + 1];

    while (sh->running) {
        printf("%s", PROMPT);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\nexit\n");
            break;
        }

        int argc = tokenize(line, argv, MAX_ARGS);
        if (argc == 0)
            continue;

        sh->last_status = execute(argv, argc, sh);
    }
}
