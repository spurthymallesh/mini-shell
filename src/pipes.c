#include "minishell.h"
#include <errno.h>

int execute_pipeline(char *input)
{
    char *commands[MAX_ARGS];
    char *args[MAX_ARGS];
    char *token;
    int command_count = 0;
    int i;
    int pipefd[2];
    int previous_fd = -1;
    pid_t pids[MAX_ARGS];
    int status;

    /*
     * Split the complete command line using '|'.
     */
    token = strtok(input, "|");

    while (token != NULL && command_count < MAX_ARGS - 1)
    {
        commands[command_count++] = token;
        token = strtok(NULL, "|");
    }

    commands[command_count] = NULL;

    /*
     * Create processes for every command.
     */
    for (i = 0; i < command_count; i++)
    {
        /*
         * Create pipe unless this is the last command.
         */
        if (i < command_count - 1)
        {
            if (pipe(pipefd) < 0)
            {
                perror("pipe");
                return 1;
            }
        }

        /*
         * Tokenize individual command.
         */
        int argc = 0;

        token = strtok(commands[i], " \t");

        while (token != NULL && argc < MAX_ARGS - 1)
        {
            args[argc++] = token;
            token = strtok(NULL, " \t");
        }

        args[argc] = NULL;

        /*
         * Ignore empty commands.
         */
        if (args[0] == NULL)
        {
            continue;
        }

        /*
         * Create child.
         */
        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork");
            return 1;
        }

        if (pids[i] == 0)
        {
           
            if (previous_fd != -1)
            {
                if (dup2(previous_fd, STDIN_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            
            if (i < command_count - 1)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            
            if (previous_fd != -1)
            {
                close(previous_fd);
            }

            if (i < command_count - 1)
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            execvp(args[0], args);

            perror("execvp");
            exit(EXIT_FAILURE);
        }

        /*
         * Parent process.
         */

        if (previous_fd != -1)
        {
            close(previous_fd);
        }

       
        if (i < command_count - 1)
        {
            close(pipefd[1]);
            previous_fd = pipefd[0];
        }
    }

    /*
     * Close final pipe descriptor.
     */
    if (previous_fd != -1)
    {
        close(previous_fd);
    }

    /*
     * Wait for all pipeline processes.
     */
    for (i = 0; i < command_count; i++)
    {
        if (waitpid(pids[i], &status, 0) < 0)
        {
            if (errno == EINTR)
            {
                i--;
                continue;
            }

            perror("waitpid");
            return 1;
        }
    }

    if (WIFEXITED(status))
    {
        last_exit_status = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        last_exit_status = 128 + WTERMSIG(status);
    }

    return last_exit_status;
}