#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "executor.h"
#include "../builtins/builtins.h"

static int run_external(char **argv, int argc)
{
    (void)argc;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(127);
    }
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

int execute(char **argv, int argc, t_shell *sh)
{
    if (argc == 0 || argv[0] == NULL)
        return 0;

    if (is_builtin(argv[0]))
        return exec_builtin(argv, argc, sh);

    return run_external(argv, argc);
}
