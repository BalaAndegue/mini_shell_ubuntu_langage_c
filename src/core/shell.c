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
#include "../parser/parser.h"
#include "../executor/executor.h"
#include "../expand/expand.h"

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
    int interactive = isatty(STDIN_FILENO);

    while (sh->running) {
        char *line = read_line(sh, interactive);
        if (!line) {
            if (interactive)
                fprintf(stderr, "\nexit\n");
            break;
        }

        t_cmd *cmds = parse_line(line);
        free(line);

        if (!cmds)
            continue;

        /* expand $VAR, $?, $$ — replace each argv[i] in-place (free old) */
        for (t_cmd *c = cmds; c; c = c->next) {
            for (int i = 0; i < c->argc; i++) {
                char *ex = expand_token(c->argv[i], sh);
                if (ex) {
                    free(c->argv[i]);
                    c->argv[i] = ex;
                }
            }
        }

        /* single command (no pipeline yet — pipeline in next commit) */
        sh->last_status = execute_cmd(cmds, sh);

        cmd_free(cmds);
    }
}
