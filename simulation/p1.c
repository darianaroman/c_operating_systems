#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main(int argc, char **argv)
{
    int child_pid;
    child_pid = fork();
    if (child_pid < 0)
    {
        perror("error! creating child process");
        exit(1);
    }
    else if (child_pid == 0)
    {
        printf("[child] my pid is %d and my parent's pid is %d\n", getpid(), getppid());
        execl("ls", "ls", argv[1], NULL);
        perror("error! loading and executing in child process");
        exit(50);
    }
    else
    {
        printf("[parent] my pid is %d and my child's pid is %d\n", getpid(), child_pid);
        int status;
        waitpid(child_pid, &status, 0);
        int es = WEXITSTATUS(status);
        printf("[parent] child exit status = %d\n", es);
    }
    return 0;
}