#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <number_clients>\n", argv[0]);
        exit(1);
    }
    pid_t pid2;

    int n = atoi(argv[1]);
    pid_t *pid_client;

    pid_client = (pid_t *)malloc(n * sizeof(pid_t));

    for (int i = 0; i < n; i++)
    {
        pid_client[i] = fork();
        if (pid_client[i] == 0)
        {
            execl("/home/dariana/fac/lab/l11/client", "client", NULL);
            perror("error! client\n");
        }
    }

    char n_string[10];
    sprintf(n_string, "%d", n);

    pid2 = fork();
    if (pid2 == 0)
    {
        execl("/home/dariana/fac/lab/l11/p4_server", "p4_server", n_string,  NULL);
        perror("error! server\n");
    }


    for (int i = 0; i < n; i++)
    {
        waitpid(pid_client[i], NULL, 0);
    }
    waitpid(pid2, NULL, 0);
    free(pid_client);
    return 0;
}