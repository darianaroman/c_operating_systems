#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <string.h>

#define MAX_LINE 128

int main(int argc, char **argv)
{
    printf("More commands: (enter \"#\" to leave)\n");

    char *buffer = (char *)malloc(MAX_LINE * sizeof(char));
    char *sign = (char *)malloc(sizeof(char));
    char *a = (char *)malloc(sizeof(char));
    char *b = (char *)malloc(sizeof(char));

    printf("> ");
    while (sign[0] != '#')
    {
        scanf("%s", buffer);
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = 0;
        }
        strncpy(a, buffer, 1);
        a[1] = 0;

        scanf("%s", buffer);
        if (buffer[strlen(buffer) - 1] == '\n')
        {
            buffer[strlen(buffer) - 1] = 0;
        }
        strncpy(sign, buffer, 1);
        sign[1] = 0;

        scanf("%s", buffer);
        if (buffer[strlen(buffer) - 1] == '\n')
        {
            buffer[strlen(buffer) - 1] = 0;
        }
        strncpy(b, buffer, 1);
        b[1] = 0;

        printf("in client: %s %s %s\n", a, sign, b);

        if (sign[0] != '#')
        {
            int pid;
            int status;
            pid = fork();
            if (pid > 0)
            {
                // parent doing something
                if (waitpid(pid, &status, 0) < 0)
                {
                    perror("error (waitpid)\n");
                    exit(EXIT_FAILURE);
                }

                if (WIFEXITED(status))
                {
                    int es = WEXITSTATUS(status);
                    printf("(in parent) result is: %d\n", es);
                }
            }
            else
            {
                // child loading and executing a new code
                execl("./server", "server", a, sign, b, NULL);
                perror("execl has not succeded");
            }
        }
    }
}
