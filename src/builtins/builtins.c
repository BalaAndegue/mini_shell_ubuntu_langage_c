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
static int dispatch_true(char **argv, int argc, t_shell *sh);
static int dispatch_false(char **argv, int argc, t_shell *sh);
static int dispatch_type(char **argv, int argc, t_shell *sh);
static int dispatch_help(char **argv, int argc, t_shell *sh);

static const t_builtin_entry builtins_table[] = {
    { "cd",     dispatch_cd     },
    { "exit",   dispatch_exit   },
    { "echo",   dispatch_echo   },
    { "pwd",    dispatch_pwd    },
    { "env",    dispatch_env    },
    { "export", dispatch_export },
    { "unset",  dispatch_unset  },
    { "true",   dispatch_true   },
    { "false",  dispatch_false  },
    { "type",   dispatch_type   },
    { "help",   dispatch_help   },
    { NULL, NULL }
};

static int dispatch_cd(char **argv, int argc, t_shell *sh)
{
    return builtin_cd(argv, argc, sh);
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

static int dispatch_true(char **argv, int argc, t_shell *sh)
{
    return builtin_true(argv, argc, sh);
}

static int dispatch_false(char **argv, int argc, t_shell *sh)
{
    return builtin_false(argv, argc, sh);
}

static int dispatch_type(char **argv, int argc, t_shell *sh)
{
    return builtin_type(argv, argc, sh);
}

static int dispatch_help(char **argv, int argc, t_shell *sh)
{
    return builtin_help(argv, argc, sh);
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

int builtin_cd(char **argv, int argc, t_shell *sh)
{
    const char *target;
    char        oldpwd[4096] = "";

    getcwd(oldpwd, sizeof(oldpwd));

    if (argc < 2 || strcmp(argv[1], "~") == 0) {
        target = env_get(sh->env, "HOME");
        if (!target) target = getenv("HOME");
        if (!target) { fprintf(stderr, "cd: HOME not set\n"); return 1; }
    } else if (strcmp(argv[1], "-") == 0) {
        target = env_get(sh->env, "OLDPWD");
        if (!target) { fprintf(stderr, "cd: OLDPWD not set\n"); return 1; }
        puts(target);
    } else {
        target = argv[1];
    }

    if (chdir(target) != 0) {
        perror("cd");
        return 1;
    }

    char newpwd[4096];
    if (getcwd(newpwd, sizeof(newpwd)) == NULL)
        return 0;

    if (sh->env) {
        env_set(&sh->env, "OLDPWD", oldpwd[0] ? oldpwd : "");
        env_set(&sh->env, "PWD",    newpwd);
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

int builtin_true(char **argv, int argc, t_shell *sh)
{
    (void)argv; (void)argc; (void)sh;
    return 0;
}

int builtin_false(char **argv, int argc, t_shell *sh)
{
    (void)argv; (void)argc; (void)sh;
    return 1;
}

int builtin_type(char **argv, int argc, t_shell *sh)
{
    (void)sh;
    int ret = 0;
    for (int i = 1; i < argc; i++) {
        if (is_builtin(argv[i])) {
            printf("%s is a shell builtin\n", argv[i]);
        } else {
            /* search PATH */
            const char *path_env = getenv("PATH");
            int found = 0;
            if (path_env) {
                char path_copy[4096];
                strncpy(path_copy, path_env, sizeof(path_copy) - 1);
                char *tok = strtok(path_copy, ":");
                while (tok) {
                    char full[4096];
                    snprintf(full, sizeof(full), "%s/%s", tok, argv[i]);
                    if (access(full, X_OK) == 0) {
                        printf("%s is %s\n", argv[i], full);
                        found = 1;
                        break;
                    }
                    tok = strtok(NULL, ":");
                }
            }
            if (!found) {
                fprintf(stderr, "type: %s: not found\n", argv[i]);
                ret = 1;
            }
        }
    }
    return ret;
}

int builtin_help(char **argv, int argc, t_shell *sh)
{
    (void)argv; (void)argc; (void)sh;
    printf("minishell " SHELL_VERSION " — built-in commands:\n");
    printf("  cd [dir|-|~]     change directory\n");
    printf("  echo [-n] [...]  print arguments\n");
    printf("  pwd              print working directory\n");
    printf("  env              print environment\n");
    printf("  export NAME=VAL  set environment variable\n");
    printf("  unset NAME       remove environment variable\n");
    printf("  true             exit 0\n");
    printf("  false            exit 1\n");
    printf("  type NAME        show command type\n");
    printf("  help             this message\n");
    printf("  exit [N]         exit shell with status N\n");
    return 0;
}
