#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>

int sem_id;

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
void* increment(void *arg)
{
    long long aux;
    P(sem_id, 0);
    aux = count;
    aux++;
    // usleep(100);
    count = aux;
    V(sem_id, 0);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    int n;
    pthread_t *t;
    if (argc != 2) {
        printf("Usage: %s <number_threads>\n", argv[0]);
        exit(1);
    }
    sscanf(argv[1], "%d", &n);
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (sem_id < 0)
    {
        perror("Error creating the semaphore set");
        exit(2);
    }
    // Initialize the first (and single) semaphore in the set with 1
    semctl(sem_id, 0, SETVAL, 1);
    t = (pthread_t *) malloc (n * sizeof(pthread_t));
    for (int i = 0; i < n; i++) {
        pthread_create(&t[i], NULL, increment, NULL);
    }
    for (int i = 0; i < n; i++)
    {
        pthread_join(t[i], NULL);
    }
    printf("%lld\n", count);
    // destroy (remove) the semaphore set
    semctl(sem_id, 0, IPC_RMID, 0);
}