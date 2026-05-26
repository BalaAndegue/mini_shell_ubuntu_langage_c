#ifndef GLOB_H
#define GLOB_H

#include "../../include/cmd.h"

/*
 * Expand glob patterns in cmd->argv.
 * Tokens containing *, ?, or [...] are expanded via glob(3).
 * If a pattern matches nothing it is kept as-is (bash behaviour).
 * The function replaces cmd->argv with a new heap-allocated array.
 */
void expand_globs(t_cmd *cmd);

#endif /* GLOB_H */
