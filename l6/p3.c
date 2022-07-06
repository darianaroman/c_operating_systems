#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    pid_t pid1 = fork();
    printf("pid1 (returned by fork) = %d\n", pid1);
    // pid_t pid2 = 1;
    // pid_t pid3 = 1;
    pid_t pid2;
    pid_t pid3;
    switch (pid1)
    {
    case -1:
        // error case
        perror("Cannot create a new child");
        exit(1);
    case 0:
        // child
        //... does what it is requested ...
        break;
    default:
        // parent
        // ... does what it is requested ...
        waitpid(pid1, NULL, 0);
        pid2 = fork();
        printf("pid2 (returned by fork) = %d\n", pid2);
        waitpid(pid2, NULL, 0);
        break;
    }

    switch (pid2)
    {
    case -1:
        // error case
        perror("Cannot create a new child");
        exit(1);
    case 0:
        // child
        //... does what it is requested ...
        pid3 = fork();
        printf("pid3 (returned by fork) = %d\n", pid3);
        waitpid(pid3, NULL, 0);
        break;
    default:
        // parent
        // ... does what it is requested ...

        break;
    }

    printf("-----\n");
    printf("getpid = %d\n", getpid());
    printf("getppid = %d\n", getppid());
    printf("-----\n");
}