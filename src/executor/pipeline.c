#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include "pipeline.h"
#include "executor.h"
#include "redir.h"
#include "../builtins/builtins.h"
#include "../core/signals.h"

static int cmd_count(t_cmd *cmds)
{
    int n = 0;
    for (t_cmd *c = cmds; c; c = c->next)
        n++;
    return n;
}

/*
 * Execute a pipeline of n commands.
 *
 * For a single command we delegate to execute_cmd() (handles builtins
 * with fd-save/restore and redirections).
 *
 * For n > 1 we:
 *   1. Allocate n-1 pipe(2) fd-pairs.
 *   2. Fork one child per command.  Each child wires its stdin/stdout
 *      to the adjacent pipe ends, then applies any explicit redirections
 *      (which override the pipe fds), and finally execs or runs the
 *      built-in.
 *   3. The parent closes all pipe fds and waits for every child; it
 *      returns the exit status of the LAST command in the pipeline
 *      (POSIX behaviour).
 */
int execute_pipeline(t_cmd *cmds, t_shell *sh)
{
    int n = cmd_count(cmds);

    if (n == 1)
        return execute_cmd(cmds, sh);

    /* allocate pipe pairs: pipes[i] connects command i to command i+1 */
    int (*pipes)[2] = malloc((size_t)(n - 1) * sizeof(*pipes));
    if (!pipes) {
        perror("malloc");
        return 1;
    }
    for (int i = 0; i < n - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            free(pipes);
            return 1;
        }
    }

    pid_t *pids = malloc((size_t)n * sizeof(pid_t));
    if (!pids) {
        perror("malloc");
        for (int i = 0; i < n - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        free(pipes);
        return 1;
    }

    t_cmd *cur = cmds;
    for (int idx = 0; idx < n; idx++, cur = cur->next) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            pids[idx] = -1;
            continue;
        }
        if (pid == 0) {
            signals_child();
            /* wire stdin from previous pipe */
            if (idx > 0) {
                dup2(pipes[idx - 1][0], STDIN_FILENO);
            }
            /* wire stdout to next pipe */
            if (idx < n - 1) {
                dup2(pipes[idx][1], STDOUT_FILENO);
            }
            /* close all pipe ends in the child */
            for (int i = 0; i < n - 1; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            /* explicit redirections override pipe fds */
            if (cur->redirs && apply_redirs(cur->redirs) < 0)
                _exit(1);

            if (is_builtin(cur->argv[0])) {
                int ret = exec_builtin(cur->argv, cur->argc, sh);
                fflush(stdout);
                fflush(stderr);
                _exit(ret);
            }
            execve(cur->argv[0], cur->argv, sh->env);
            execvp(cur->argv[0], cur->argv);
            perror(cur->argv[0]);
            _exit(127);
        }
        pids[idx] = pid;
    }

    /* parent: close all pipe ends */
    for (int i = 0; i < n - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    free(pipes);

    /* wait for all children; keep last exit status */
    int last_status = 0;
    for (int idx = 0; idx < n; idx++) {
        if (pids[idx] < 0)
            continue;
        int status;
        waitpid(pids[idx], &status, 0);
        if (idx == n - 1) {
            if (WIFEXITED(status))
                last_status = WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
                last_status = 128 + WTERMSIG(status);
        }
    }
    free(pids);
    return last_status;
}
