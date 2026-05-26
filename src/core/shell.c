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
#include "../executor/pipeline.h"
#include "../executor/heredoc.h"
#include "../expand/expand.h"
#include "../expand/glob.h"

static void history_load(void)
{
#ifdef HAVE_READLINE
    const char *home = getenv("HOME");
    if (!home) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/.minishell_history", home);
    read_history(path);
#endif
}

static void history_save(void)
{
#ifdef HAVE_READLINE
    const char *home = getenv("HOME");
    if (!home) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/.minishell_history", home);
    write_history(path);
#endif
}

static char *read_line(t_shell *sh, int interactive)
{
    char *prompt = interactive ? prompt_build(sh) : NULL;

#ifdef HAVE_READLINE
    if (interactive) {
        char *line = readline(prompt ? prompt : sh->prompt);
        free(prompt);
        if (line && *line)
            add_history(line);
        return line;
    }
#endif
    char *buf = malloc(BUFF_SIZE);
    if (!buf) { free(prompt); return NULL; }
    if (interactive) {
        printf("%s", prompt ? prompt : sh->prompt);
        fflush(stdout);
        free(prompt);
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

    if (interactive)
        history_load();

    while (sh->running) {
        char *line = read_line(sh, interactive);
        if (!line) {
            if (interactive) {
                fprintf(stderr, "\nexit\n");
                history_save();
            }
            break;
        }

        /* split on ';' outside quotes, run each segment as a pipeline */
        char *p   = line;
        char *seg = line;

        while (1) {
            /* advance past quoted regions so we don't split on ; inside quotes */
            if (*p == '\'' || *p == '"') {
                char q = *p++;
                while (*p && *p != q) p++;
                if (*p) p++;
                continue;
            }
            if (*p == ';' || *p == '\0') {
                int at_end = (*p == '\0');
                *p = '\0';      /* NUL-terminate this segment */

                /* skip leading whitespace */
                while (*seg == ' ' || *seg == '\t') seg++;

                if (*seg != '\0') {
                    t_cmd *cmds = parse_line(seg);
                    if (cmds) {
                        /* pre-collect heredocs before forking */
                        for (t_cmd *c = cmds; c; c = c->next) {
                            for (t_redir *r = c->redirs; r; r = r->next) {
                                if (r->type == REDIR_HEREDOC && r->fd < 0)
                                    r->fd = heredoc_collect(r->file, sh);
                            }
                        }
                        for (t_cmd *c = cmds; c; c = c->next) {
                            for (int i = 0; i < c->argc; i++) {
                                char *ex = expand_token(c->argv[i], sh);
                                if (ex) { free(c->argv[i]); c->argv[i] = ex; }
                            }
                            expand_globs(c);
                        }
                        sh->last_status = execute_pipeline(cmds, sh);
                        cmd_free(cmds);
                    }
                }
                if (at_end)
                    break;
                seg = ++p;
            } else {
                p++;
            }
        }
        free(line);
    }
}
