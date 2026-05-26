#ifndef MINISHELL_H
#define MINISHELL_H

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>

#define SHELL_NAME     "minishell"
#define SHELL_VERSION  "0.1.0"
#define BUFF_SIZE      4096
#define MAX_ARGS       128

typedef struct s_shell {
    char  *prompt;        /* static fallback prompt string */
    char  *ps1;           /* PS1 format string (NULL → use default) */
    int    last_status;
    int    running;
    char **env;           /* NULL-terminated copy of the environment */
} t_shell;

char *prompt_build(t_shell *sh);

/* env helpers */
char  **env_copy(char **envp);
void    env_free(char **env);
char   *env_get(char **env, const char *name);
int     env_set(char ***env, const char *name, const char *value);
int     env_unset(char ***env, const char *name);

#endif /* MINISHELL_H */
