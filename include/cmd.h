#ifndef CMD_H
#define CMD_H

typedef enum e_redir_type {
    REDIR_IN,       /* <  */
    REDIR_OUT,      /* >  */
    REDIR_APPEND,   /* >> */
    REDIR_ERR,      /* 2> */
    REDIR_ERRIN,    /* 2>&1 */
    REDIR_HEREDOC,  /* << */
} t_redir_type;

typedef struct s_redir {
    t_redir_type    type;
    char           *file;
    struct s_redir *next;
} t_redir;

typedef struct s_cmd {
    char           **argv;
    int              argc;
    t_redir         *redirs;
    struct s_cmd    *next;
} t_cmd;

t_cmd  *cmd_new(void);
void    cmd_free(t_cmd *cmd);
t_redir *redir_new(t_redir_type type, char *file);

#endif /* CMD_H */
