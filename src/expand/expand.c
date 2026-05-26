#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "expand.h"
#include "../../include/minishell.h"

#define OUT_CAP_INIT 256

typedef struct {
    char  *buf;
    size_t len;
    size_t cap;
} t_sbuf;

static int sbuf_push(t_sbuf *s, char c)
{
    if (s->len + 1 >= s->cap) {
        size_t newcap = s->cap * 2;
        char  *tmp    = realloc(s->buf, newcap);
        if (!tmp)
            return -1;
        s->buf = tmp;
        s->cap = newcap;
    }
    s->buf[s->len++] = c;
    return 0;
}

static int sbuf_push_str(t_sbuf *s, const char *str)
{
    while (*str)
        if (sbuf_push(s, *str++) < 0)
            return -1;
    return 0;
}

/* Expand a $NAME or special var; p points just after the '$'.
 * Advances *p past the variable name. Returns a pointer to a static
 * or heap string (caller must NOT free it directly — it's borrowed). */
static const char *expand_var(const char **p, t_shell *sh)
{
    static char numbuf[32];

    if (**p == '?') {
        (*p)++;
        snprintf(numbuf, sizeof(numbuf), "%d", sh->last_status);
        return numbuf;
    }
    if (**p == '$') {
        (*p)++;
        snprintf(numbuf, sizeof(numbuf), "%d", (int)getpid());
        return numbuf;
    }
    if (**p == '!') {
        (*p)++;
        numbuf[0] = '\0';
        return numbuf;
    }

    /* regular variable name: [A-Za-z_][A-Za-z0-9_]* */
    const char *start = *p;
    while (**p && ((**p >= 'A' && **p <= 'Z') ||
                   (**p >= 'a' && **p <= 'z') ||
                   (**p >= '0' && **p <= '9') ||
                   **p == '_'))
        (*p)++;

    if (*p == start)
        return "$";

    size_t  namelen = (size_t)(*p - start);
    char    name[256];
    if (namelen >= sizeof(name))
        namelen = sizeof(name) - 1;
    memcpy(name, start, namelen);
    name[namelen] = '\0';

    const char *val = env_get(sh->env, name);
    return val ? val : "";
}

/* Expand a single token string. Returns a newly allocated string. */
char *expand_token(const char *token, t_shell *sh)
{
    t_sbuf  out = { malloc(OUT_CAP_INIT), 0, OUT_CAP_INIT };
    if (!out.buf)
        return NULL;

    int in_single = 0;
    int in_double = 0;
    const char *p = token;

    while (*p) {
        if (*p == '\'' && !in_double) {
            in_single = !in_single;
            p++;
            continue;
        }
        if (*p == '"' && !in_single) {
            in_double = !in_double;
            p++;
            continue;
        }
        if (*p == '$' && !in_single) {
            p++;
            const char *val = expand_var(&p, sh);
            if (sbuf_push_str(&out, val) < 0) {
                free(out.buf);
                return NULL;
            }
            continue;
        }
        sbuf_push(&out, *p++);
    }
    out.buf[out.len] = '\0';
    return out.buf;
}

/* Expand every token in argv in-place (replaces pointers). */
void expand_argv(char **argv, int argc, t_shell *sh)
{
    for (int i = 0; i < argc; i++) {
        char *expanded = expand_token(argv[i], sh);
        if (expanded)
            argv[i] = expanded;
    }
}
