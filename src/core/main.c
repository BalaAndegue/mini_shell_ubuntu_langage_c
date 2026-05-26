#include <stdlib.h>
#include "shell.h"
#include "signals.h"
#include "../../include/minishell.h"

extern char **environ;

int main(void)
{
    signals_init();

    t_shell sh = {
        .prompt      = SHELL_NAME "$ ",
        .last_status = 0,
        .running     = 1,
        .env         = env_copy(environ),
    };

    shell_run(&sh);
    env_free(sh.env);
    return sh.last_status;
}
