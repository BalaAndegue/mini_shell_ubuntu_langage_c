#ifndef PIPELINE_H
#define PIPELINE_H

#include "../../include/minishell.h"
#include "../../include/cmd.h"

int execute_pipeline(t_cmd *cmds, t_shell *sh);

#endif /* PIPELINE_H */
