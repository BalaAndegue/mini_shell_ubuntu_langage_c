#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"

typedef struct {
    const char *name;
    int (*fn)(char **, int, t_shell *);
} t_builtin_entry;

static int dispatch_cd(char **argv, int argc, t_shell *sh);
static int dispatch_exit(char **argv, int argc, t_shell *sh);
static int dispatch_echo(char **argv, int argc, t_shell *sh);
static int dispatch_pwd(char **argv, int argc, t_shell *sh);

static const t_builtin_entry builtins_table[] = {
    { "cd",   dispatch_cd   },
    { "exit", dispatch_exit },
    { "echo", dispatch_echo },
    { "pwd",  dispatch_pwd  },
    { NULL, NULL }
};

static int dispatch_cd(char **argv, int argc, t_shell *sh)
{
    (void)sh;
    return builtin_cd(argv, argc);
}

static int dispatch_exit(char **argv, int argc, t_shell *sh)
{
    return builtin_exit(argv, argc, sh);
}

static int dispatch_echo(char **argv, int argc, t_shell *sh)
{
    (void)sh;
    return builtin_echo(argv, argc);
}

static int dispatch_pwd(char **argv, int argc, t_shell *sh)
{
    (void)sh;
    return builtin_pwd(argv, argc);
}

int is_builtin(const char *cmd)
{
    for (int i = 0; builtins_table[i].name; i++) {
        if (strcmp(builtins_table[i].name, cmd) == 0)
            return 1;
    }
    return 0;
}

int exec_builtin(char **argv, int argc, t_shell *sh)
{
    for (int i = 0; builtins_table[i].name; i++) {
        if (strcmp(builtins_table[i].name, argv[0]) == 0)
            return builtins_table[i].fn(argv, argc, sh);
    }
    return 127;
}

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

int builtin_echo(char **argv, int argc)
{
    int newline = 1;
    int start   = 1;

    if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
        newline = 0;
        start   = 2;
    }
    for (int i = start; i < argc; i++) {
        if (i > start)
            putchar(' ');
        fputs(argv[i], stdout);
    }
    if (newline)
        putchar('\n');
    return 0;
}

int builtin_pwd(char **argv, int argc)
{
    (void)argv;
    (void)argc;
    char buf[4096];
    if (!getcwd(buf, sizeof(buf))) {
        perror("pwd");
        return 1;
    }
    puts(buf);
    return 0;
}
