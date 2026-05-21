#include <stdlib.h>
#include "shell.h"

int main(void)
{
    t_shell sh = {
        .prompt      = SHELL_NAME "$ ",
        .last_status = 0,
        .running     = 1,
    };

    shell_run(&sh);
    return sh.last_status;
}
