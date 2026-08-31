#include "minishell.h"

void handle_prompt_assignment(char *input)
{
    if (strncmp(input, "PS1=", 4) != 0)
    {
        return;
    }

    char *new_prompt = input + 4;

    if (*new_prompt == '\0')
    {
        strcpy(prompt, "msh> ");
        return;
    }

    snprintf(prompt, INPUT_SIZE, "%s> ", new_prompt);
}