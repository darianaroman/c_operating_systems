#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>

#define MUTEX 0
#define TOKENS 1

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

int sem_id;

int no_cars;

void *car_parking(void *arg)
{
    P(sem_id, TOKENS);
    P(sem_id, MUTEX);
    no_cars++;
    printf("No of cars in the park: %d\n", no_cars);
    V(sem_id, MUTEX);
    usleep(2000);
    P(sem_id, MUTEX);
    no_cars--;
    V(sem_id, MUTEX);
    V(sem_id, TOKENS);
    return NULL;
}

int main()
{
    int N = 1000;

    sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    if (sem_id < 0)
    {
        perror("error! creating the semaphore set");
        exit(1);
    }
    semctl(sem_id, MUTEX, SETVAL, 1);
    semctl(sem_id, TOKENS, SETVAL, 50);

    pthread_t th_id[N];
    for (int i = 0; i < N; i++)
    {
        if (pthread_create(&th_id[i], NULL, car_parking, NULL) != 0)
        {
            perror("error! creating threads");
            exit(1);
        }
        usleep(5); // maybe 5 ms
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(th_id[i], NULL);
    }

    semctl(sem_id, 0, IPC_RMID, 0);

    return 0;
}