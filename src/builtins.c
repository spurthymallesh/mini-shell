#define _POSIX_C_SOURCE 200809L

#include "minishell.h"

static void print_shell_path(void);

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

int execute_builtin(char **args)
{
    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
        {
            char *home = getenv("HOME");

            if (home == NULL)
            {
                fprintf(stderr, "cd: HOME not set\n");
                return 1;
            }

            if (chdir(home) != 0)
            {
                perror("cd");
                return 1;
            }
        }
        else if (chdir(args[1]) != 0)
        {
            perror("cd");
            return 1;
        }

        return 0;
    }

    else if (strcmp(args[0], "pwd") == 0)
    {
        char cwd[INPUT_SIZE];

        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            perror("pwd");
            return 1;
        }

        printf("%s\n", cwd);

        return 0;
    }

    else if (strcmp(args[0], "echo") == 0)
    {
        for (int i = 1; args[i] != NULL; i++)
        {
            if (strcmp(args[i], "$?") == 0)
            {
                printf("%d", last_exit_status);
            }
            else if (strcmp(args[i], "$$") == 0)
            {
                printf("%d", getpid());
            }
            else if (strcmp(args[i], "$SHELL") == 0)
            {
                print_shell_path();
            }
            else
            {
                printf("%s", args[i]);
            }

            if (args[i + 1] != NULL)
            {
                printf(" ");
            }
        }

        printf("\n");

        return 0;
    }

    else if (strcmp(args[0], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }

    return 1;
}

static void print_shell_path(void)
{
    char path[INPUT_SIZE];

    ssize_t length = readlink("/proc/self/exe",
                              path,
                              sizeof(path) - 1);

    if (length < 0)
    {
        perror("readlink");
        return;
    }

    path[length] = '\0';

    printf("%s", path);
}