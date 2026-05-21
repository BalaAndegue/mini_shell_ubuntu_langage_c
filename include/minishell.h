#ifndef MINISHELL_H
#define MINISHELL_H

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>

#define SHELL_NAME     "minishell"
#define SHELL_VERSION  "0.1.0"
#define BUFF_SIZE      4096
#define MAX_ARGS       128

typedef struct s_shell {
    char  *prompt;
    int    last_status;   /* $? — exit status of last command */
    int    running;
} t_shell;

#endif /* MINISHELL_H */
