#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../include/minishell.h"

static int env_count(char **env)
{
    int n = 0;
    while (env && env[n])
        n++;
    return n;
}

char **env_copy(char **envp)
{
    int    n   = env_count(envp);
    char **cpy = malloc((size_t)(n + 1) * sizeof(char *));
    if (!cpy)
        return NULL;
    for (int i = 0; i < n; i++) {
        cpy[i] = strdup(envp[i]);
        if (!cpy[i]) {
            for (int j = 0; j < i; j++)
                free(cpy[j]);
            free(cpy);
            return NULL;
        }
    }
    cpy[n] = NULL;
    return cpy;
}

void env_free(char **env)
{
    if (!env)
        return;
    for (int i = 0; env[i]; i++)
        free(env[i]);
    free(env);
}

char *env_get(char **env, const char *name)
{
    if (!env || !name)
        return NULL;
    size_t len = strlen(name);
    for (int i = 0; env[i]; i++) {
        if (strncmp(env[i], name, len) == 0 && env[i][len] == '=')
            return env[i] + len + 1;
    }
    return NULL;
}

int env_set(char ***envp, const char *name, const char *value)
{
    char  *entry;
    size_t len = strlen(name) + strlen(value) + 2;

    entry = malloc(len);
    if (!entry)
        return -1;
    snprintf(entry, len, "%s=%s", name, value);

    char **env    = *envp;
    size_t namelen = strlen(name);

    for (int i = 0; env[i]; i++) {
        if (strncmp(env[i], name, namelen) == 0 && env[i][namelen] == '=') {
            free(env[i]);
            env[i] = entry;
            return 0;
        }
    }

    int    n      = env_count(env);
    char **newenv = realloc(env, (size_t)(n + 2) * sizeof(char *));
    if (!newenv) {
        free(entry);
        return -1;
    }
    newenv[n]     = entry;
    newenv[n + 1] = NULL;
    *envp = newenv;
    return 0;
}

int env_unset(char ***envp, const char *name)
{
    char  **env    = *envp;
    size_t  namelen = strlen(name);

    for (int i = 0; env[i]; i++) {
        if (strncmp(env[i], name, namelen) == 0 && env[i][namelen] == '=') {
            free(env[i]);
            while (env[i + 1]) {
                env[i] = env[i + 1];
                i++;
            }
            env[i] = NULL;
            return 0;
        }
    }
    return 0;
}
