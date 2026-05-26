#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_READLINE
# include <readline/readline.h>
# include <readline/history.h>
#endif

#include "shell.h"
#include "../lexer/lexer.h"
#include "../executor/executor.h"

static char *read_line(t_shell *sh, int interactive)
{
#ifdef HAVE_READLINE
    if (interactive) {
        char *line = readline(sh->prompt);
        if (line && *line)
            add_history(line);
        return line;
    }
#else
    (void)sh;
#endif
    char *buf = malloc(BUFF_SIZE);
    if (!buf)
        return NULL;
    if (interactive) {
        printf("%s", sh->prompt);
        fflush(stdout);
    }
    if (!fgets(buf, BUFF_SIZE, stdin)) {
        free(buf);
        return NULL;
    }
    buf[strcspn(buf, "\n")] = '\0';
    return buf;
}

void shell_run(t_shell *sh)
{
    char *argv[MAX_ARGS + 1];
    int   interactive = isatty(STDIN_FILENO);

    while (sh->running) {
        char *line = read_line(sh, interactive);
        if (!line) {
            if (interactive)
                fprintf(stderr, "\nexit\n");
            break;
        }

        int argc = tokenize(line, argv, MAX_ARGS);
        if (argc == 0) {
            free(line);
            continue;
        }

        sh->last_status = execute(argv, argc, sh);
        free(line);
    }
}
