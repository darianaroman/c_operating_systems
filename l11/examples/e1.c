#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main()
{
    int fd[2];
    int pid, n;
    char buf[100];

    pipe(fd);
    pid = fork();

    if (pid == 0) // child
    {
        // usleep(100);
        n = read(fd[0], buf, 7);
        buf[n] = 0;
        printf("The child received: %s\n", buf);
        n = write(fd[1], "FIU", 3);
        printf("The child has transmitted: FIU\n");
    }
    else //parent
    {
        n = write(fd[1], "PARENT", 7);
        printf("Parent has sent: PARENT\n");
        n = read(fd[0], buf, 3);
        buf[n] = 0;
        printf("Parent has received: %s\n", buf);
        waitpid(pid, NULL, 0);
    }

    return 0;
}
