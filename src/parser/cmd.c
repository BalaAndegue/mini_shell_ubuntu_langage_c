#include <stdlib.h>
#include <string.h>
#include "../../include/cmd.h"

t_cmd *cmd_new(void)
{
    t_cmd *cmd = calloc(1, sizeof(*cmd));
    return cmd;
}

t_redir *redir_new(t_redir_type type, char *file)
{
    t_redir *r = calloc(1, sizeof(*r));
    if (!r)
        return NULL;
    r->type = type;
    r->file = file;
    r->fd   = -1;
    return r;
}

static void redir_free(t_redir *r)
{
    while (r) {
        t_redir *next = r->next;
        free(r->file);
        free(r);
        r = next;
    }
}

void cmd_free(t_cmd *cmd)
{
    while (cmd) {
        t_cmd *next = cmd->next;
        redir_free(cmd->redirs);
        if (cmd->argv) {
            for (int i = 0; i < cmd->argc; i++)
                free(cmd->argv[i]);
            free(cmd->argv);
        }
        free(cmd);
        cmd = next;
    }
}
