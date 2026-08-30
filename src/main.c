#include <stdio.h>

int main(void)
{
    while (1)
    {
        printf("msh> ");
        fflush(stdout);

        char input[1024];

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n");
            break;
        }
    }

    return 0;
}