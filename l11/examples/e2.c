#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main()
{
    int fdToLeft[2], fdToRight[2];
    int pid, n;
    char buf[100];

    pipe(fdToLeft);
    pipe(fdToRight);

    pid = fork();

    if (pid == 0) // child
    {
        close(fdToLeft[0]);
        close(fdToRight[1]);

        n = read(fdToRight[0], buf, 7);
        buf[n] = 0;
        printf("The child received: %s\n", buf);
        n = write(fdToLeft[1], "CHILD", 5);
        printf("The child has transmitted: CHILD\n");
    }
    else // parent
    {
        close(fdToLeft[1]);
        close(fdToRight[0]);

        n = write(fdToRight[1], "PARENT", 7);
        printf("Parent has sent: PARENT\n");
        n = read(fdToLeft[0], buf, 5);
        buf[n] = 0;
        printf("Parent has received: %s\n", buf);
        waitpid(pid, NULL, 0);
    }

    return 0;
}
