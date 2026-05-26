#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "../../include/minishell.h"

/* -------------------------------------------------------------------------
 * Token array helpers
 * ---------------------------------------------------------------------- */

typedef struct {
    char **tokens;
    int    count;
    int    cap;
} t_tokarr;

static int tokarr_push(t_tokarr *a, char *tok)
{
    if (a->count >= a->cap) {
        int    newcap = a->cap ? a->cap * 2 : 16;
        char **tmp    = realloc(a->tokens, (size_t)newcap * sizeof(char *));
        if (!tmp)
            return -1;
        a->tokens = tmp;
        a->cap    = newcap;
    }
    a->tokens[a->count++] = tok;
    return 0;
}

/* -------------------------------------------------------------------------
 * Redirection detection
 * ---------------------------------------------------------------------- */

static int is_redir_op(const char *tok, t_redir_type *out)
{
    if (strcmp(tok, "<")    == 0) { *out = REDIR_IN;     return 1; }
    if (strcmp(tok, ">")    == 0) { *out = REDIR_OUT;    return 1; }
    if (strcmp(tok, ">>")   == 0) { *out = REDIR_APPEND; return 1; }
    if (strcmp(tok, "2>")   == 0) { *out = REDIR_ERR;    return 1; }
    if (strcmp(tok, "2>&1") == 0) { *out = REDIR_ERRIN;  return 1; }
    if (strcmp(tok, "<<")   == 0) { *out = REDIR_HEREDOC;return 1; }
    return 0;
}

/* -------------------------------------------------------------------------
 * Lexer: split line into raw tokens respecting '' and ""
 * ---------------------------------------------------------------------- */

static char *extract_token(const char **p)
{
    const char *s   = *p;
    char        buf[BUFF_SIZE];
    int         len = 0;

    while (*s && *s != ' ' && *s != '\t') {
        if (*s == '\'') {
            s++;
            while (*s && *s != '\'')
                buf[len++] = *s++;
            if (*s)
                s++;
        } else if (*s == '"') {
            s++;
            while (*s && *s != '"')
                buf[len++] = *s++;
            if (*s)
                s++;
        } else {
            buf[len++] = *s++;
        }
    }
    *p = s;
    if (len == 0)
        return NULL;
    buf[len] = '\0';
    return strdup(buf);
}

static char **lex(char *line, int *ntok)
{
    t_tokarr arr = {0};
    const char *p = line;

    while (*p) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (!*p)
            break;

        /* check for 2>&1 and >> before single-char ops */
        if (p[0] == '2' && p[1] == '>' && p[2] == '&' && p[3] == '1') {
            tokarr_push(&arr, strdup("2>&1"));
            p += 4;
        } else if (p[0] == '2' && p[1] == '>') {
            tokarr_push(&arr, strdup("2>"));
            p += 2;
        } else if (p[0] == '>' && p[1] == '>') {
            tokarr_push(&arr, strdup(">>"));
            p += 2;
        } else if (*p == '>' || *p == '<' || *p == '|') {
            char op[3] = { *p, '\0', '\0' };
            if (*p == '<' && p[1] == '<') { op[1] = '<'; p++; }
            tokarr_push(&arr, strdup(op));
            p++;
        } else {
            char *tok = extract_token(&p);
            if (tok)
                tokarr_push(&arr, tok);
        }
    }
    *ntok = arr.count;
    return arr.tokens;
}

/* -------------------------------------------------------------------------
 * Build t_cmd list from token array
 * ---------------------------------------------------------------------- */

static t_cmd *build_cmds(char **toks, int ntok)
{
    t_cmd    *head    = NULL;
    t_cmd    *cur     = NULL;
    t_tokarr  argv    = {0};

    for (int i = 0; i <= ntok; i++) {
        int at_pipe = (i < ntok && strcmp(toks[i], "|") == 0);
        int at_end  = (i == ntok);

        if (at_end || at_pipe) {
            t_cmd *cmd = cmd_new();
            if (!cmd)
                goto fail;

            /* finalise argv */
            tokarr_push(&argv, NULL);
            cmd->argv = argv.tokens;
            cmd->argc = argv.count - 1;
            argv = (t_tokarr){0};

            if (!head)
                head = cmd;
            if (cur)
                cur->next = cmd;
            cur = cmd;
            continue;
        }

        t_redir_type rtype;
        if (is_redir_op(toks[i], &rtype)) {
            if (i + 1 >= ntok) {
                fprintf(stderr, "minishell: parse error near '%s'\n", toks[i]);
                goto fail;
            }
            t_redir *r = redir_new(rtype, toks[i + 1]);
            if (!r)
                goto fail;
            /* attach redir to current (or next) cmd — we'll attach lazily */
            /* store in a temporary: flush pending argv first */
            /* simplification: attach to cur after it's created            */
            /* we do a two-pass — push redir marker tokens, resolve later  */
            /* Actually: build cur eagerly up to this point */
            if (cur && r) {
                t_redir **tail = &cur->redirs;
                while (*tail)
                    tail = &(*tail)->next;
                *tail = r;
            } else {
                /* no cmd yet: defer — create empty cmd */
                t_cmd *cmd = cmd_new();
                if (!cmd) { free(r); goto fail; }
                cmd->redirs = r;
                if (!head) head = cmd;
                if (cur) cur->next = cmd;
                cur = cmd;
            }
            i++; /* consume filename token */
        } else {
            tokarr_push(&argv, toks[i]);
        }
    }
    return head;

fail:
    cmd_free(head);
    free(argv.tokens);
    return NULL;
}

/* -------------------------------------------------------------------------
 * Public entry
 * ---------------------------------------------------------------------- */

t_cmd *parse_line(char *line)
{
    int    ntok  = 0;
    char **toks  = lex(line, &ntok);
    if (!toks || ntok == 0) {
        free(toks);
        return NULL;
    }

    t_cmd *cmds = build_cmds(toks, ntok);

    for (int i = 0; i < ntok; i++)
        free(toks[i]);
    free(toks);

    return cmds;
}
