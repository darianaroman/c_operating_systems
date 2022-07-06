#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>

#define MAXSTEPS 10

char name1[] = "THREAD1";
char name2[] = "THREAD2";

void *execThread(void *threadName)
{
    int i;
    int *number = (int *)malloc(sizeof(int));
    char *tmp;

    printf("[Thread %s] Starting. tid=%ld, pid=%d\n", (char *)threadName, (long int)pthread_self(), (int)getpid());

    for (i = 1; i <= MAXSTEPS; i++)
    {
        sleep(1);
        // pthread_yield();
        printf("[Thread %s] Executing step %d\n", (char *)threadName, i);
    }

    tmp = &((char *)threadName)[strlen((char *)threadName) - 1];
    *number = atoi(tmp);

    printf("[Thread %s] Terminating. tid=%ld, pid=%d\n", (char *)threadName, (long int)pthread_self(), (int)getpid());

    return number;
}

int main()
{
    pthread_t t1, t2;
    int *state[2];

    printf("[Thread MAIN] Starting. tid=%ld, pid=%d\n", (long int)pthread_self(), (int)getpid());

    if (pthread_create(&t1, NULL, execThread, name1) != 0)
    {
        perror("Error creating a new thread");
        exit(1);
    }

    if (pthread_create(&t2, NULL, execThread, name2) != 0)
    {
        perror("Error creating a new thread");
        exit(1);
    }

    printf("[Thread MAIN] Created threads [%ld, %ld]\n", (long int)t1, (long int)t2);

    pthread_join(t1, (void **)&state[0]);
    pthread_join(t2, (void **)&state[1]);

    printf("[Thread MAIN] Thread %s[%ld] terminated with state %d\n", name1, (long int)t1, *(state[0]));
    printf("[Thread MAIN] Thread %s[%ld] terminated with state %d\n", name2, (long int)t2, *(state[1]));

    printf("[Thread MAIN] Terminating. tid=%ld, pid=%d\n", (long int)pthread_self(), (int)getpid());
}