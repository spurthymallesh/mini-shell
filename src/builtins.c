#include "minishell.h"

int check_command_type(char *command)
{
    if (command == NULL)
    {
        return NO_COMMAND;
    }

    if (strcmp(command, "cd") == 0 ||
        strcmp(command, "pwd") == 0 ||
        strcmp(command, "echo") == 0 ||
        strcmp(command, "exit") == 0)
    {
        return BUILTIN;
    }

    return EXTERNAL;
}

void execute_builtin(char **args)
{
    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
        {
            char *home = getenv("HOME");

            if (home == NULL)
            {
                fprintf(stderr, "cd: HOME not set\n");
                return;
            }

            if (chdir(home) != 0)
            {
                perror("cd");
            }
        }
        else if (chdir(args[1]) != 0)
        {
            perror("cd");
        }
    }
    else if (strcmp(args[0], "pwd") == 0)
    {
        char cwd[INPUT_SIZE];

        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            perror("pwd");
            return;
        }

        printf("%s\n", cwd);
    }
    else if (strcmp(args[0], "echo") == 0)
    {
        for (int i = 1; args[i] != NULL; i++)
        {
            printf("%s", args[i]);

            if (args[i + 1] != NULL)
            {
                printf(" ");
            }
        }

        printf("\n");
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
}