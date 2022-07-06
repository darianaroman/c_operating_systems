#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LOOPS 100

char buf[100];

void increment(int pid, long long MAX, int fd[][2])
{
    long long count = 0;
    for (count = 0; count <= MAX; count++)
    {
        printf("Process %d at step %lld\n", pid, count);
        if (pid == 0)
        {
            read(fd[0][0], buf, 1);
            write(fd[1][1], "0", 1);
        }
        if (pid == 1)
        {
            write(fd[0][1], "1", 1);
            read(fd[1][0], buf, 1);
        }
    }
    return;
}

int main()
{
    pid_t pid0, pid1;

    int fd[2][2];

    pipe(fd[0]);
    pipe(fd[1]);

    pid0 = fork();
    if (pid0 == 0)
    {
        close(fd[0][1]);
        close(fd[1][0]);
        increment(0, MAX_LOOPS, fd);
        exit(0);
    }
    pid1 = fork();
    if (pid1 == 0)
    {
        close(fd[0][0]);
        close(fd[1][1]);
        increment(1, MAX_LOOPS, fd);
        exit(0);
    }

    waitpid(pid0, NULL, 0);
    waitpid(pid1, NULL, 0);
}