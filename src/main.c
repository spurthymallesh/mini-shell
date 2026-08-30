#include "minishell.h"

int main(void)
{
    char input[INPUT_SIZE];
    char *args[MAX_ARGS];

    while (1)
    {
        printf("msh> ");
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
            execute_builtin(args);
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
                execvp(args[0], args);

                perror("execvp");
                exit(EXIT_FAILURE);
            }

            int status;

            if (waitpid(pid, &status, 0) < 0)
            {
                perror("waitpid");
            }
        }
    }

    return 0;
}