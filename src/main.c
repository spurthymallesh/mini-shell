#include <stdio.h>
#include <string.h>

#define INPUT_SIZE 1024

int main(void)
{
    char input[INPUT_SIZE];

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

        printf("Command entered: %s\n", input);
    }

    return 0;
}