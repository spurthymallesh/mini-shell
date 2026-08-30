#ifndef MINISHELL_H
#define MINISHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_SIZE 1024
#define MAX_ARGS 64

#define BUILTIN 1
#define EXTERNAL 2
#define NO_COMMAND 3

int check_command_type(char *command);
void execute_builtin(char **args);
void execute_external(char **args);

#endif