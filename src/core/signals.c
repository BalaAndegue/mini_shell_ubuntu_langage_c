#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "signals.h"

/* SIGINT handler for the interactive prompt: print a newline and redraw.
 * We do NOT exit — Ctrl-C cancels the current line, not the shell. */
static void handle_sigint(int sig)
{
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
    /* readline's rl_on_new_line() + rl_redisplay() would be ideal but
     * require readline — we keep this handler signal-safe (async-safe). */
}

/* Install shell-level signal handlers. */
void signals_init(void)
{
    struct sigaction sa_int  = {0};
    struct sigaction sa_quit = {0};

    sa_int.sa_handler  = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags    = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    /* SIGQUIT: ignore at shell level (Ctrl-\ should only kill a child) */
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/* Restore default handlers in child processes so they die on Ctrl-C/\. */
void signals_child(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
}
