#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>

#define MAXSTEPS 10

char name[] = "THREAD1";

void *execThread(void *threadName)
{
    int i;
    int *number = (int *)malloc(sizeof(int));
    char *tmp;

    printf("[Thread %s] Starting. tid= %ld, pid=%d\n", (char *)threadName, (long int)pthread_self(), (int)getpid());

    for (i = 1; i <= MAXSTEPS; i++)
    {
        sleep(1);
        printf("[Thread %s] Executing step %d.\n", (char *)threadName, i);
    }

    tmp = &((char *)threadName)[strlen((char *)threadName) - 1];
    *number = atoi(tmp);

    printf("[Thread %s] Terminating. tid= %ld, pid=%d\n", (char *)threadName, (long int)pthread_self(), (int)getpid());

    return number;
}

int main()
{

    pthread_t t;
    int *state;

    printf("[Thread MAIN] Starting. tid=%ld, pid=%d\n", (long int)pthread_self(), (int)getpid());

    if (pthread_create(&t, NULL, execThread, name) != 0)
    {
        perror("Error creating a new thread");
        exit(1);
    }

    printf("[Thread MAIN] Created a new thread [%ld]\n", (long int)t);

    printf("[Thread MAIN] Waiting the created thread [%ld]\n", (long int)t);

    pthread_join(t, (void **)&state);

    printf("[Thread MAIN] Thread %s[%ld] terminated with state %d\n", name, (long int)t, *(state));
    return 0;
}
