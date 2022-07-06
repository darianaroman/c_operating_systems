#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <string.h>

#define MAX_LINE 128

int main(int argc, char **argv)
{
    int childPid;

    printf("[Parent] PID=%d ParentPID=%d\n", getpid(), getppid());

    // Create a new child
    childPid = fork();

    // Check if successfully created the child
    if (childPid < 0)
    { // if not, terminate the parent too
        perror("Error creating new process");
        exit(1);
    }

    // Here we separare between the code executed by the parent and child
    if (childPid > 0)
    { // parent
        printf("[Parent] ChildPID=%d\n", childPid);

        waitpid(childPid, NULL, 0);

        printf("[Parent] More commands: (enter \"quit\" to leave)\n");

        char buffer[MAX_LINE];
        pid_t pid;
        int status;

        printf("$");
        while (scanf("%s", buffer) && strncmp(buffer, "quit", 4) != 0)
        {
            if (buffer[strlen(buffer) - 1] == '\n')
                buffer[strlen(buffer) - 1] = 0;

            if ((pid = fork()) < 0)
            {
                printf("ERROR! (forking)");
            }
            else if (pid == 0)
            {
                execlp(buffer, buffer, NULL);
                printf("ERROR! couldn’t execute: %s", buffer);
                exit(127);
            }

            if (waitpid(pid, &status, 0) < 0)
                printf("ERROR! (waitpid)");
            printf("$");
        }

        printf("[Parent] Terminate the execution\n");
    }
    else
    { // child
        printf("[Child] PID=%d ParentPID=%d\n", getpid(), getppid());
        printf("[Child] Load and execute a new code. The actual one is overwritten and lost. There is no return from a successfull exec!!!\n");

        execlp(argv[1], argv[1], argv[2], NULL);

        // Putting code here makes sense only when execlp does not execute successfully
        perror("Error executing exec");
        printf("[Child] Terminate the execution\n");
        exit(1);
    }
}
