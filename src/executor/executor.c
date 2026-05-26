#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "executor.h"
#include "redir.h"
#include "../builtins/builtins.h"
#include "../core/signals.h"

static int wait_child(pid_t pid)
{
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return 1;
    }
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);
    return 1;
}

static int run_external(char **argv, int argc, t_shell *sh, t_redir *redirs)
{
    (void)argc;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        signals_child();
        if (redirs && apply_redirs(redirs) < 0)
            exit(1);
        execve(argv[0], argv, sh->env);
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(127);
    }
    return wait_child(pid);
}

int execute(char **argv, int argc, t_shell *sh)
{
    if (argc == 0 || argv[0] == NULL)
        return 0;

    if (is_builtin(argv[0]))
        return exec_builtin(argv, argc, sh);

    return run_external(argv, argc, sh, NULL);
}

int execute_cmd(t_cmd *cmd, t_shell *sh)
{
    if (!cmd || cmd->argc == 0 || !cmd->argv[0])
        return 0;

    if (is_builtin(cmd->argv[0])) {
        /* builtins: apply redirs in the current process, restore after */
        int saved_in  = dup(STDIN_FILENO);
        int saved_out = dup(STDOUT_FILENO);
        int saved_err = dup(STDERR_FILENO);

        int ret = 0;
        if (cmd->redirs)
            ret = apply_redirs(cmd->redirs);
        if (ret == 0)
            ret = exec_builtin(cmd->argv, cmd->argc, sh);

        dup2(saved_in,  STDIN_FILENO);
        dup2(saved_out, STDOUT_FILENO);
        dup2(saved_err, STDERR_FILENO);
        close(saved_in); close(saved_out); close(saved_err);
        return ret;
    }

    return run_external(cmd->argv, cmd->argc, sh, cmd->redirs);
}
