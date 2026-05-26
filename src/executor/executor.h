#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "../../include/minishell.h"
#include "../../include/cmd.h"

int execute(char **argv, int argc, t_shell *sh);
int execute_cmd(t_cmd *cmd, t_shell *sh);

#endif /* EXECUTOR_H */
