#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LOOPS 100

char buf[100];

void increment(int pid, long long MAX, int **fd, int n)
{
    long long count = 0;
    for (count = 0; count <= MAX; count++)
    {
        read(fd[(pid + n - 1) % n][0], buf, 1);
        printf("Process %d at step %lld\n", pid, count);
        write(fd[pid][1], "x", 1);
    }
    return;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <number_processes>\n", argv[0]);
        exit(1);
    }
    int n = atoi(argv[1]);
    printf("n = %d\n", n);

    int **fd;
    fd = (int **)malloc(n * sizeof(int *));
    pid_t *pid;
    pid = (pid_t *)malloc(n * sizeof(pid_t));

    for (int i = 0; i < n; i++)
    {
        fd[i] = (int *)malloc(2 * sizeof(int));
        pipe(fd[i]);
    }

    for (int i = 0; i < n; i++)
    {
        pid[i] = fork();
        if (pid[i] < 0)
        {
            perror("error! creating child\n");
            exit(1);
        }

        if (pid[i] == 0)
        {
            for (int j = 1; j < n; j++)
            {
                close(fd[(i + j) % n][1]);
            }
            for (int j = 0; j < n - 1; j++)
            {
                close(fd[(i + j) % n][0]);
            }
            usleep(100);

            increment(i, MAX_LOOPS, fd, n);
            exit(0);
        }
    }

    write(fd[n - 1][1], "x", 1);

    for (int i = 0; i < n; i++)
    {
        waitpid(pid[i], NULL, 0);
    }

    for (int i = 0; i < n; i++)
    {
        free(fd[i]);
    }
    free(pid);
    free(fd);
    return 0;
}