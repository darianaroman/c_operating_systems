#include <stdio.h>
#include <unistd.h>

int main()
{
    int pid;
    pid = fork();
    if (pid > 0)
    {
        // parent doing something
    }
    else
    {
        // child loading and executing a new code
        execl("./child", "child", "p1", "10", NULL);
        perror(" execl has not succeded ");
    }
}