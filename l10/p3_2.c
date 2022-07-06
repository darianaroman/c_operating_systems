#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>

#define IS_H 0
#define IS_O 1
#define ACCESS_H 2
#define ACCESS_O 3
#define FORMED_H 4
#define FORMED_O 5
#define ORDER 6
#define MUTEX 7
#define TURNSTILE 8
#define TURNSTILE2 9

int sem_id;

int countH = 0;

int random_number(int lower, int upper)
{
    return (int)(rand() % (upper - lower + 1)) + lower;
}

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

void *atomH(void *arg)
{
    int id = *((int *)arg);
    // printf("created H %d\n", id);

    P(sem_id, ACCESS_H);
    printf("atom H %d waiting for O\n", id);

    P(sem_id, MUTEX);
    countH++;
    if (countH == 2) {
        V(sem_id, IS_H);

        P(sem_id, TURNSTILE2);
        V(sem_id, TURNSTILE);
    }
    V(sem_id, MUTEX);

    P(sem_id, IS_O);

    printf("molecule created H %d - ", id);

    P(sem_id, TURNSTILE);
    V(sem_id, TURNSTILE);

    P(sem_id, MUTEX);
    countH--;
    if (countH == 0)
    {
        V(sem_id, ORDER);
        V(sem_id, FORMED_H);

        P(sem_id, TURNSTILE);
        V(sem_id, TURNSTILE2);
    }
    V(sem_id, MUTEX);

    P(sem_id, TURNSTILE2);
    V(sem_id, TURNSTILE2);

    P(sem_id, FORMED_O);

    V(sem_id, ACCESS_H);
    return NULL;
}

void *atomO(void *arg)
{
    int id = *((int *)arg);
    // printf("created O %d\n", id);

    P(sem_id, ACCESS_O);
    printf("atom O %d waiting for H\n", id);

    P(sem_id, IS_H);
    V(sem_id, IS_O);
    V(sem_id, IS_O);

    P(sem_id, ORDER);
    printf("- molecule created O %d\n", id);

    V(sem_id, FORMED_O);
    V(sem_id, FORMED_O);
    P(sem_id, FORMED_H);

    V(sem_id, ACCESS_O);
    return NULL;
}

int main()
{
    int N = 100;
    pthread_t th_id;
    int th_args[N];

    sem_id = semget(IPC_PRIVATE, 10, IPC_CREAT | 0600);
    if (sem_id < 0)
    {
        perror("error! creating the semaphore");
    }

    semctl(sem_id, IS_H, SETVAL, 0);
    semctl(sem_id, IS_O, SETVAL, 0);
    semctl(sem_id, ACCESS_H, SETVAL, 2);
    semctl(sem_id, ACCESS_O, SETVAL, 1);
    semctl(sem_id, FORMED_H, SETVAL, 0);
    semctl(sem_id, FORMED_O, SETVAL, 0);
    semctl(sem_id, ORDER, SETVAL, 0);
    semctl(sem_id, MUTEX, SETVAL, 1);
    semctl(sem_id, TURNSTILE, SETVAL, 0);
    semctl(sem_id, TURNSTILE2, SETVAL, 1);

    for (int i = 0; i < N; i++)
    {
        th_args[i] = i;
        if (random_number(0, 1) == 0)
        {
            pthread_create(&th_id, NULL, atomO, &th_args[i]);
        }
        else
        {
            pthread_create(&th_id, NULL, atomH, &th_args[i]);
        }
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(th_id, NULL);
    }

    semctl(sem_id, 0, IPC_RMID, 0);

    return 0;
}