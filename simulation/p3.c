#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define PARENT_WRITE "Sol-Do-La-Fa-Me-Do-Re"
#define CHILD_WRITE "Sol-Do-La-Ti-Do-Re-Do"

int main()
{
    pid_t pid;
    int fdToChild[2];
    int fdToParent[2];
    char buffer[100];
    int size = sizeof(PARENT_WRITE) / sizeof(char);

    pipe(fdToChild);
    pipe(fdToParent);

    pid = fork();
    if (pid < 0)
    {
        perror("error! creating child process");
        return 1;
    }
    else if (pid == 0)
    {
        close(fdToChild[1]);
        close(fdToParent[0]);
        read(fdToChild[0], &buffer, size * sizeof(char));
        buffer[size * sizeof(char)] = 0;
        printf("[child] read from pipe: %s\n", buffer);
        
        write(fdToParent[1], CHILD_WRITE, size * sizeof(char));
    }
    else
    {
        close(fdToChild[0]);
        close(fdToParent[1]);
        write(fdToChild[1], &PARENT_WRITE, size * sizeof(char));

        read(fdToParent[0], &buffer, size * sizeof(char));
        buffer[size * sizeof(char)] = 0;
        printf("[parent] read from pipe: %s\n", buffer);
        waitpid(pid, NULL, 0);
    }

    return 0;
}