#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "glob.h"
#include "../../include/cmd.h"

static int has_glob_chars(const char *s)
{
    for (; *s; s++)
        if (*s == '*' || *s == '?' || *s == '[')
            return 1;
    return 0;
}

void expand_globs(t_cmd *cmd)
{
    if (!cmd || !cmd->argv)
        return;

    /* count how many slots we need after expansion */
    int new_count = 0;
    glob_t g = {0};
    int    first = 1;

    for (int i = 0; i < cmd->argc; i++) {
        if (!has_glob_chars(cmd->argv[i])) {
            new_count++;
            continue;
        }
        int flags = (first ? 0 : GLOB_APPEND);
        int rc    = glob(cmd->argv[i], flags | 0, NULL, &g);
        if (rc == GLOB_NOMATCH || rc != 0) {
            /* no match — keep original token */
            new_count++;
        } else {
            new_count += (int)g.gl_pathc - (first ? 0 : (int)(g.gl_pathc - g.gl_pathc));
            /* recalculate: g.gl_pathc is total so far when using GLOB_APPEND */
            first = 0;
        }
    }

    /* rebuild from scratch without GLOB_APPEND complexity */
    char **newargv = malloc((size_t)(cmd->argc + 1) * sizeof(char *) * 64);
    if (!newargv)
        return;

    int out = 0;
    for (int i = 0; i < cmd->argc; i++) {
        if (!has_glob_chars(cmd->argv[i])) {
            newargv[out++] = cmd->argv[i];
            cmd->argv[i]   = NULL; /* transfer ownership */
            continue;
        }
        glob_t gg = {0};
        int rc = glob(cmd->argv[i], 0 | GLOB_NOCHECK, NULL, &gg);
        if (rc == 0) {
            for (size_t j = 0; j < gg.gl_pathc; j++) {
                newargv[out++] = strdup(gg.gl_pathv[j]);
            }
            free(cmd->argv[i]);
            cmd->argv[i] = NULL;
        } else {
            newargv[out++] = cmd->argv[i];
            cmd->argv[i]   = NULL;
        }
        globfree(&gg);
    }
    newargv[out] = NULL;

    free(cmd->argv);
    cmd->argv = newargv;
    cmd->argc = out;
}
