#include "minishell.h"
#include <errno.h>

extern volatile sig_atomic_t child_event_count;
extern pid_t completed_pids[MAX_COMPLETED_JOBS];
extern int completed_status[MAX_COMPLETED_JOBS];

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
    else if (sig_num == SIGCHLD)
    {
        int status;
        pid_t pid;

        while (1)
        {
            pid = waitpid(-1, &status, WNOHANG);

            if (pid <= 0)
            {
                break;
            }

            if (child_event_count < MAX_COMPLETED_JOBS)
            {
                completed_pids[child_event_count] = pid;
                completed_status[child_event_count] = status;
                child_event_count++;
            }
        }
    }
}