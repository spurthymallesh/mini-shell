#include "minishell.h"

void signal_handler(int sig_num)
{
    if (sig_num == SIGINT)
    {
        if (foreground_pid == -1)
        {
            printf("\n%s", prompt);
            fflush(stdout);
        }
    }
    else if (sig_num == SIGTSTP)
    {
        if (foreground_pid == -1)
        {
            printf("\n%s", prompt);
            fflush(stdout);
        }
    }
}