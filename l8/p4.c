#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>

int n = 100;

int sem;

int turn = 0;

void P(int sem_id, int sem_no)
{
    struct sembuf op = {sem_no, -1, 0};

    semop(sem_id, &op, 1);
}

void V(int sem_id, int sem_no)
{
    struct sembuf op = {sem_no, +1, 0};

    semop(sem_id, &op, 1);
}

long long count = 0;
void increment(int th_id)
{
    int i = 0;
    while (i < 100)
    {
        long long aux;
        P(sem, th_id);
        aux = count;
        aux++;
        usleep(100);
        printf("count=%lld from thread %d\n", count, th_id);
        count = aux;
        if (th_id < n - 1)
        {
            V(sem, th_id + 1);
        }
        else
        {
            V(sem, 0);
        }
        i++;
    }
}

void *protect_thread(void *arg)
{
    int *th_id = (int *)arg;
    increment(*th_id);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    int *ids;
    pthread_t *t;
    if (argc != 1)
    {
        printf("Usage: %s\n", argv[0]);
        exit(1);
    }
    sem = semget(IPC_PRIVATE, n, IPC_CREAT | 0600);

    if (sem < 0)
    {
        perror("Error creating the semaphore set");
        exit(2);
    }
    // initialize the first (and single) semaphore in the set with 1

    semctl(sem, 0, SETVAL, 1);
    for (int i = 1; i < n; i++)
    {
        semctl(sem, i, SETVAL, 0);
    }

    t = (pthread_t *)malloc(n * sizeof(pthread_t));
    ids = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
    {
        ids[i] = i;
        pthread_create(&t[i], NULL, protect_thread, &ids[i]);
    }
    for (int i = 0; i < n; i++)
    {
        pthread_join(t[i], NULL);
    }
    printf("%lld\n", count);
    // destroy (remove) the semaphore set
    semctl(sem, 0, IPC_RMID, 0);
    for (int i = 1; i < n; i++)
    {
        semctl(sem, i, IPC_RMID, 0);
    }
}