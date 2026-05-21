#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "builtins.h"

int builtin_cd(char **argv, int argc)
{
    if (argc < 2) {
        fprintf(stderr, "cd: missing operand\n");
        return 1;
    }
    if (chdir(argv[1]) != 0) {
        perror("cd");
        return 1;
    }
    return 0;
}

int builtin_exit(char **argv, int argc, t_shell *sh)
{
    int code = sh->last_status;
    if (argc >= 2)
        code = atoi(argv[1]);
    fprintf(stderr, "exit\n");
    exit(code);
}
