#ifndef HEREDOC_H
#define HEREDOC_H

#include "../../include/cmd.h"
#include "../../include/minishell.h"

/*
 * For each REDIR_HEREDOC in cmd->redirs, read lines from stdin until
 * the delimiter is encountered, write them into a pipe, and replace the
 * redir's type with REDIR_IN and file with the read-end fd path
 * (stored as a special sentinel handled by apply_redirs).
 *
 * Returns the read-end fd (>=0) or -1 on error.
 * The caller is responsible for closing the fd.
 */
int heredoc_collect(const char *delim, t_shell *sh);

#endif /* HEREDOC_H */
