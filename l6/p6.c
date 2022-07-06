#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// the formula I found for the number of processes created looks like this:
// for 
// 1 + sum from k = 0 to steps - 1 ( 2 raised to power k / 2 raised to power round(k / skip) )
// this works for steps != skip
// round() example 3.2 => 3, 4.6 => 4
// for steps = skip the nr of processes created is 2 raised to steps

int main(int argc, char **argv)
{
    int pid, steps, skip;
    int i;

    int aux = 1;
    int status;
    int es = 0;

    int pid_root = getpid();

    if (argc != 3)
    {
        printf("Usage: %s <loop_steps> <skip_step>\n", argv[0]);
        exit(1);
    }
    sscanf(argv[1], "%d", &steps);
    sscanf(argv[2], "%d", &skip);
    for (i = 1; i <= steps; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            // printf("  ");
        }
        else
        {
            if (waitpid(pid, &status, 0) < 0)
            {
                perror("error (waitpid)\n");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status))
            {
                es = WEXITSTATUS(status);
            }
        }
        if (pid == 0 && i % skip == 0)
        {
            break;
        }
    }
    aux += es;
    // printf("getpid -> %d --- es -> %d --- aux = %d\n", getpid(), es, aux);
    if (getpid() == pid_root) {
        printf("number of processes = %d\n", aux);
    }
    exit(aux);
}