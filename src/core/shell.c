#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shell.h"
#include "../lexer/lexer.h"
#include "../executor/executor.h"

#define PROMPT SHELL_NAME "$ "

void shell_run(t_shell *sh)
{
    char  line[BUFF_SIZE];
    char *argv[MAX_ARGS + 1];
    int   interactive = isatty(STDIN_FILENO);

    while (sh->running) {
        if (interactive) {
            printf("%s", PROMPT);
            fflush(stdout);
        }

        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (interactive)
                fprintf(stderr, "\nexit\n");
            break;
        }

        int argc = tokenize(line, argv, MAX_ARGS);
        if (argc == 0)
            continue;

        sh->last_status = execute(argv, argc, sh);
    }
}
