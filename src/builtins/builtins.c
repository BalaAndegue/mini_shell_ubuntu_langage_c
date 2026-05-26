#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "../../include/minishell.h"

typedef struct {
    const char *name;
    int (*fn)(char **, int, t_shell *);
} t_builtin_entry;

static int dispatch_cd(char **argv, int argc, t_shell *sh);
static int dispatch_exit(char **argv, int argc, t_shell *sh);
static int dispatch_echo(char **argv, int argc, t_shell *sh);
static int dispatch_pwd(char **argv, int argc, t_shell *sh);
static int dispatch_env(char **argv, int argc, t_shell *sh);
static int dispatch_export(char **argv, int argc, t_shell *sh);
static int dispatch_unset(char **argv, int argc, t_shell *sh);

static const t_builtin_entry builtins_table[] = {
    { "cd",     dispatch_cd     },
    { "exit",   dispatch_exit   },
    { "echo",   dispatch_echo   },
    { "pwd",    dispatch_pwd    },
    { "env",    dispatch_env    },
    { "export", dispatch_export },
    { "unset",  dispatch_unset  },
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

static int dispatch_env(char **argv, int argc, t_shell *sh)
{
    return builtin_env(argv, argc, sh);
}

static int dispatch_export(char **argv, int argc, t_shell *sh)
{
    return builtin_export(argv, argc, sh);
}

static int dispatch_unset(char **argv, int argc, t_shell *sh)
{
    return builtin_unset(argv, argc, sh);
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

int builtin_env(char **argv, int argc, t_shell *sh)
{
    (void)argv;
    (void)argc;
    if (!sh->env)
        return 0;
    for (int i = 0; sh->env[i]; i++)
        puts(sh->env[i]);
    return 0;
}

int builtin_export(char **argv, int argc, t_shell *sh)
{
    if (argc < 2) {
        return builtin_env(argv, argc, sh);
    }
    for (int i = 1; i < argc; i++) {
        char *eq = strchr(argv[i], '=');
        if (!eq) {
            /* export VAR without value: make it visible but unchanged */
            continue;
        }
        char name[256];
        size_t nlen = (size_t)(eq - argv[i]);
        if (nlen >= sizeof(name)) {
            fprintf(stderr, "export: name too long\n");
            return 1;
        }
        memcpy(name, argv[i], nlen);
        name[nlen] = '\0';
        if (env_set(&sh->env, name, eq + 1) != 0) {
            perror("export");
            return 1;
        }
    }
    return 0;
}

int builtin_unset(char **argv, int argc, t_shell *sh)
{
    for (int i = 1; i < argc; i++)
        env_unset(&sh->env, argv[i]);
    return 0;
}
