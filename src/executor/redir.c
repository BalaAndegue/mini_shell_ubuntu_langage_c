#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "redir.h"

int apply_redirs(t_redir *redirs)
{
    for (t_redir *r = redirs; r; r = r->next) {
        int fd = -1;

        switch (r->type) {
        case REDIR_IN:
            fd = open(r->file, O_RDONLY);
            if (fd < 0) { perror(r->file); return -1; }
            if (dup2(fd, STDIN_FILENO) < 0) { perror("dup2"); close(fd); return -1; }
            close(fd);
            break;

        case REDIR_OUT:
            fd = open(r->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror(r->file); return -1; }
            if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2"); close(fd); return -1; }
            close(fd);
            break;

        case REDIR_APPEND:
            fd = open(r->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) { perror(r->file); return -1; }
            if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2"); close(fd); return -1; }
            close(fd);
            break;

        case REDIR_ERR:
            fd = open(r->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror(r->file); return -1; }
            if (dup2(fd, STDERR_FILENO) < 0) { perror("dup2"); close(fd); return -1; }
            close(fd);
            break;

        case REDIR_ERRIN:
            if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) { perror("dup2"); return -1; }
            break;

        case REDIR_HEREDOC:
            /* fd was pre-filled by heredoc_collect() */
            if (r->fd >= 0) {
                if (dup2(r->fd, STDIN_FILENO) < 0) { perror("dup2"); return -1; }
                close(r->fd);
                r->fd = -1;
            }
            break;
        }
    }
    return 0;
}
