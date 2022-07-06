#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main()
{
    pid_t pid1;
    pid_t pid2;

    pid1 = fork();
    if (pid1 == 0)
    {
        execl("/home/dariana/fac/lab/l11/client", "client", NULL);
        perror("error! client\n");
    }

    pid2 = fork();
    if (pid2 == 0)
    {
        execl("/home/dariana/fac/lab/l11/server", "server", NULL);
        perror("error! server\n");
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}