#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>

#define IS_NA 0
#define IS_CL 1
#define ACCESS_NA 2
#define ACCESS_CL 3
#define FORMED_NA 4
#define FORMED_CL 5
#define ORDER 6

int sem_id;

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

void* atomNa(void* arg)
{
    int id = *((int *)arg);
    printf("created Na %d\n", id);

    P(sem_id, ACCESS_NA);
    printf("atom Na %d waiting for cl\n", id);

    V(sem_id, IS_NA);
    P(sem_id, IS_CL);

    printf("molecule created na %d - ", id);
    V(sem_id, ORDER);

    V(sem_id, FORMED_NA);
    P(sem_id, FORMED_CL);

    V(sem_id, ACCESS_NA);
    return NULL;
}

void* atomCl(void* arg)
{
    int id = *((int *)arg);
    printf("created Cl %d\n", id);

    P(sem_id, ACCESS_CL);
    printf("atom Cl %d waiting for na\n", id);

    V(sem_id, IS_CL);
    P(sem_id, IS_NA);

    P(sem_id, ORDER);
    printf("- molecule created cl %d\n", id);

    V(sem_id, FORMED_CL);
    P(sem_id, FORMED_NA);

    V(sem_id, ACCESS_CL);
    return NULL;
}

int main()
{
    int N = 100;
    pthread_t th_id;
    int th_args[N];

    sem_id = semget(IPC_PRIVATE, 7, IPC_CREAT | 0600);
    if (sem_id < 0)
    {
        perror("error! creating the semaphore");
    }

    semctl(sem_id, IS_NA, SETVAL, 0);
    semctl(sem_id, IS_CL, SETVAL, 0);
    semctl(sem_id, ACCESS_NA, SETVAL, 1);
    semctl(sem_id, ACCESS_CL, SETVAL, 1);
    semctl(sem_id, FORMED_NA, SETVAL, 0);
    semctl(sem_id, FORMED_CL, SETVAL, 0);
    semctl(sem_id, ORDER, SETVAL, 0);

    for (int i = 0; i < N; i++)
    {
        th_args[i] = i;
        if (random_number(0, 1) == 0)
        {
            pthread_create(&th_id, NULL, atomCl, &th_args[i]);
        }
        else
        {
            pthread_create(&th_id, NULL, atomNa, &th_args[i]);
        }
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(th_id, NULL);
    }

    semctl(sem_id, 0, IPC_RMID, 0);

    return 0;
}