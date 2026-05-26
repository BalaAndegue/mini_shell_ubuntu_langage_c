#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/minishell.h"

/*
 * Supported PS1 escape sequences:
 *   \u  — username (from $USER)
 *   \h  — short hostname (up to first '.')
 *   \w  — working directory ($HOME replaced with ~)
 *   \$  — '#' if root, '$' otherwise
 *   \\  — literal backslash
 */
char *prompt_build(t_shell *sh)
{
    const char *fmt = sh->ps1
        ? sh->ps1
        : (env_get(sh->env, "PS1") ? env_get(sh->env, "PS1") : "\\u:\\w\\$ ");

    char        buf[512];
    size_t      out = 0;
    char        cwd[256];
    const char *user = env_get(sh->env, "USER");
    const char *home = env_get(sh->env, "HOME");

    if (!getcwd(cwd, sizeof(cwd)))
        cwd[0] = '\0';

    /* replace $HOME with ~ in cwd */
    char cwd_display[256];
    if (home && strncmp(cwd, home, strlen(home)) == 0 && cwd[strlen(home)] == '/')
        snprintf(cwd_display, sizeof(cwd_display), "~%s", cwd + strlen(home));
    else if (home && strcmp(cwd, home) == 0)
        snprintf(cwd_display, sizeof(cwd_display), "~");
    else
        snprintf(cwd_display, sizeof(cwd_display), "%s", cwd);

    for (const char *p = fmt; *p && out < sizeof(buf) - 2; p++) {
        if (*p == '\\' && *(p + 1)) {
            p++;
            switch (*p) {
            case 'u':
                if (user) {
                    size_t l = strlen(user);
                    if (out + l < sizeof(buf) - 2) {
                        memcpy(buf + out, user, l);
                        out += l;
                    }
                }
                break;
            case 'h': {
                char hn[128] = "localhost";
                gethostname(hn, sizeof(hn));
                char *dot = strchr(hn, '.');
                if (dot) *dot = '\0';
                size_t l = strlen(hn);
                if (out + l < sizeof(buf) - 2) {
                    memcpy(buf + out, hn, l);
                    out += l;
                }
                break;
            }
            case 'w': {
                size_t l = strlen(cwd_display);
                if (out + l < sizeof(buf) - 2) {
                    memcpy(buf + out, cwd_display, l);
                    out += l;
                }
                break;
            }
            case '$':
                buf[out++] = (getuid() == 0) ? '#' : '$';
                break;
            case '\\':
                buf[out++] = '\\';
                break;
            default:
                buf[out++] = '\\';
                buf[out++] = *p;
            }
        } else {
            buf[out++] = *p;
        }
    }
    buf[out] = '\0';
    return strdup(buf);
}
