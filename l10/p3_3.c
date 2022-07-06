#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>

#define IS_H 0
#define IS_S 1
#define ACCESS_H 2
#define ACCESS_S 3
#define FORMED_H 4
#define FORMED_S 5
#define ORDER 6

#define MUTEX 7
#define TURNSTILE 8
#define TURNSTILE2 9

#define IS_O 10
#define ACCESS_O 11
#define FORMED_O 12
#define TURNSTILE_O 13
#define TURNSTILE2_O 14

int sem_id;

int countH = 0;
int countO = 0;

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
    printf("atom H %d waiting for S and O\n", id);

    P(sem_id, MUTEX);
    countH++;
    if (countH == 2) {
        V(sem_id, IS_H);

        P(sem_id, TURNSTILE2);
        V(sem_id, TURNSTILE);
    }
    V(sem_id, MUTEX);

    P(sem_id, IS_S);

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

    P(sem_id, FORMED_S);

    V(sem_id, ACCESS_H);
    return NULL;
}

void *atomS(void *arg)
{
    int id = *((int *)arg);
    // printf("created S %d\n", id);

    P(sem_id, ACCESS_S);
    printf("atom S %d waiting for H and O\n", id);

    P(sem_id, IS_H);

    //
    P(sem_id, IS_O);
    //

    V(sem_id, IS_S);
    V(sem_id, IS_S);

    //
    V(sem_id, IS_S);
    V(sem_id, IS_S);
    V(sem_id, IS_S);
    V(sem_id, IS_S);
    //

    P(sem_id, ORDER);
    printf("molecule created S %d\n", id);

    V(sem_id, FORMED_S);
    V(sem_id, FORMED_S);

    //
    V(sem_id, FORMED_S);
    V(sem_id, FORMED_S);
    V(sem_id, FORMED_S);
    V(sem_id, FORMED_S);
    //

    P(sem_id, FORMED_H);

    //
    P(sem_id, FORMED_O);
    //

    V(sem_id, ACCESS_S);
    return NULL;
}

void *atomO(void *arg)
{
    int id = *((int *)arg);
    // printf("created O %d\n", id);

    P(sem_id, ACCESS_O);
    printf("atom O %d waiting for H and S\n", id);

    P(sem_id, MUTEX);
    countO++;
    if (countO == 4)
    {
        V(sem_id, IS_O);

        P(sem_id, TURNSTILE2_O);
        V(sem_id, TURNSTILE_O);
    }
    V(sem_id, MUTEX);

    P(sem_id, IS_S);

    printf("molecule created O %d - ", id);

    P(sem_id, TURNSTILE_O);
    V(sem_id, TURNSTILE_O);

    P(sem_id, MUTEX);
    countO--;
    if (countO == 0)
    {
        V(sem_id, ORDER);
        V(sem_id, FORMED_O);

        P(sem_id, TURNSTILE_O);
        V(sem_id, TURNSTILE2_O);
    }
    V(sem_id, MUTEX);

    P(sem_id, TURNSTILE2_O);
    V(sem_id, TURNSTILE2_O);

    P(sem_id, FORMED_S);

    V(sem_id, ACCESS_O);
    return NULL;
}

int main()
{
    int N = 100;
    pthread_t th_id;
    int th_args[N];

    sem_id = semget(IPC_PRIVATE, 16, IPC_CREAT | 0600);
    if (sem_id < 0)
    {
        perror("error! creating the semaphore");
    }

    semctl(sem_id, IS_H, SETVAL, 0);
    semctl(sem_id, IS_S, SETVAL, 0);
    semctl(sem_id, ACCESS_H, SETVAL, 2);
    semctl(sem_id, ACCESS_S, SETVAL, 1);
    semctl(sem_id, FORMED_H, SETVAL, 0);
    semctl(sem_id, FORMED_S, SETVAL, 0);
    semctl(sem_id, ORDER, SETVAL, 0);
    semctl(sem_id, MUTEX, SETVAL, 1);
    semctl(sem_id, TURNSTILE, SETVAL, 0);
    semctl(sem_id, TURNSTILE2, SETVAL, 1);

    semctl(sem_id, IS_O, SETVAL, 0);
    semctl(sem_id, ACCESS_O, SETVAL, 4);
    semctl(sem_id, FORMED_O, SETVAL, 0);
    semctl(sem_id, TURNSTILE_O, SETVAL, 0);
    semctl(sem_id, TURNSTILE2_O, SETVAL, 1);

    for (int i = 0; i < N; i++)
    {
        th_args[i] = i;
        if (random_number(0, 2) == 0)
        {
            pthread_create(&th_id, NULL, atomS, &th_args[i]);
        }
        else if (random_number(0, 2) == 1)
        {
            pthread_create(&th_id, NULL, atomH, &th_args[i]);
        }
        else
        {
            pthread_create(&th_id, NULL, atomO, &th_args[i]);
        }
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(th_id, NULL);
    }

    semctl(sem_id, 0, IPC_RMID, 0);

    return 0;
}