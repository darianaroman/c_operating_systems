// TO COMPILE: gcc ./p1_1.c -Wall -lrt -pthread -o p1_1 |gcc ./p1_2.c -Wall -lrt -pthread -o p1_2 |gcc ./p1.c -o p1
// TO RUN: ./p1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int n = 100;

    char n_string[10];
    sprintf(n_string, "%d", n);

    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        execlp("/home/dariana/fac/lab/l12/p1_1", "p1_1", "myshm", n_string, NULL);
        perror("error! creating process 1\n");
        exit(1);
    }

    sleep(1); // to let p1_1 create the semaphores before p2_1 tries to open them

    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        execl("/home/dariana/fac/lab/l12/p1_2", "p1_2", "myshm", n_string, NULL);
        perror("error! creating process 2\n");
        exit(1);
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}