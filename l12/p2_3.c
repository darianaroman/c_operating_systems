// TO COMPILE: gcc ./p2_3_write.c -Wall -lrt -pthread -o p2_3_write |gcc ./p2_3_read.c -Wall -lrt -pthread -o p2_3_read |gcc ./p2_3.c -o p2_3
// TO RUN: ./p2_3 <path>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <path>\n", argv[0]);
        exit(1);
    }

    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        execlp("/home/dariana/fac/lab/l12/p2_3_write", "p2_3_write", argv[1], NULL);
        perror("error! creating process 1\n");
        exit(1);
    }

    sleep(1); // to let p2_3_write create semaphores before p2_3_read tries to open them

    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        execl("/home/dariana/fac/lab/l12/p2_3_read", "p2_3_read", argv[1], NULL);
        perror("error! creating process 2\n");
        exit(1);
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}