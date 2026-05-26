#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_READLINE
# include <readline/readline.h>
#endif
#include "heredoc.h"
#include "../../include/minishell.h"

/*
 * Read lines from stdin until a line matching `delim` is found.
 * Write all preceding lines into a pipe; return the read-end fd.
 * The caller dups it onto stdin before exec.
 */
int heredoc_collect(const char *delim, t_shell *sh)
{
    (void)sh;
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("heredoc: pipe");
        return -1;
    }

    char  buf[BUFF_SIZE];
    int   interactive = isatty(STDIN_FILENO);

    while (1) {
        if (interactive) {
#ifdef HAVE_READLINE
            char *line = readline("> ");
            if (!line) break;
            int match = (strcmp(line, delim) == 0);
            if (!match) {
                write(pipefd[1], line, strlen(line));
                write(pipefd[1], "\n", 1);
            }
            free(line);
            if (match) break;
#else
            printf("> ");
            fflush(stdout);
            if (!fgets(buf, sizeof(buf), stdin)) break;
            buf[strcspn(buf, "\n")] = '\0';
            if (strcmp(buf, delim) == 0) break;
            write(pipefd[1], buf, strlen(buf));
            write(pipefd[1], "\n", 1);
#endif
        } else {
            if (!fgets(buf, sizeof(buf), stdin)) break;
            buf[strcspn(buf, "\n")] = '\0';
            if (strcmp(buf, delim) == 0) break;
            write(pipefd[1], buf, strlen(buf));
            write(pipefd[1], "\n", 1);
        }
    }
    close(pipefd[1]);
    return pipefd[0];
}
