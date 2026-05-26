#ifndef BUILTINS_H
#define BUILTINS_H

#include "../../include/minishell.h"

int builtin_cd(char **argv, int argc);
int builtin_exit(char **argv, int argc, t_shell *sh);
int builtin_echo(char **argv, int argc);
int builtin_pwd(char **argv, int argc);

int is_builtin(const char *cmd);
int exec_builtin(char **argv, int argc, t_shell *sh);

#endif /* BUILTINS_H */
