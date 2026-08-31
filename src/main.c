#include "minishell.h"

int last_exit_status = 0;
char prompt[INPUT_SIZE] = "msh> ";
pid_t foreground_pid = -1;

int main(void)
{
    char input[INPUT_SIZE];
    char *args[MAX_ARGS];

    struct sigaction sa;

sa.sa_handler = signal_handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;

sigaction(SIGINT, &sa, NULL);
sigaction(SIGTSTP, &sa, NULL);

    while (1)
    {
        printf("%s", prompt);
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0)
        {
            continue;
        }

        if (strncmp(input, "PS1=", 4) == 0)
{
    handle_prompt_assignment(input);
    last_exit_status = 0;
    continue;
}

/* tokenize command */

        int argc = 0;
        char *token = strtok(input, " ");

        while (token != NULL && argc < MAX_ARGS - 1)
        {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }

        args[argc] = NULL;

        int command_type = check_command_type(args[0]);

       if (command_type == BUILTIN)
{
    last_exit_status = execute_builtin(args);
}
        else if (command_type == EXTERNAL)
        {
            pid_t pid = fork();

            if (pid < 0)
            {
                perror("fork");
                continue;
            }

           if (pid == 0)
{
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    execvp(args[0], args);

    perror("execvp");
    exit(EXIT_FAILURE);
}
foreground_pid = pid;

int status;

if (waitpid(pid, &status, WUNTRACED) < 0)
{
    perror("waitpid");
    last_exit_status = 1;
}
else if (WIFEXITED(status))
{
    last_exit_status = WEXITSTATUS(status);
}
else if (WIFSIGNALED(status))
{
    last_exit_status = 128 + WTERMSIG(status);
}
else if (WIFSTOPPED(status))
{
    printf("\n[%d] Stopped\n", pid);
    fflush(stdout);
}

foreground_pid = -1;
        }
    }

    return 0;
}