#ifndef EXPAND_H
#define EXPAND_H

#include "../../include/minishell.h"

char *expand_token(const char *token, t_shell *sh);
void  expand_argv(char **argv, int argc, t_shell *sh);

#endif /* EXPAND_H */
