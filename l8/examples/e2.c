// Synchronizes the execution of processes of type producer and consumer that are generated
// continuously and affect a circular message buffer, in a way that the FIFO communication
// principles are met. Unread messages cannot be overwritten and a message cannot be read
// more than once.

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define N 100
#define MUTEX 0
#define SPATII 1
#define MESAJE 2

int *buffer, *prodMsg, *consMsg;

void P(int semId, int semNr)
{
    struct sembuf op = {semNr, -1, 0};
    semop(semId, &op, 1);
}

void V(int semId, int semNr)
{
    struct sembuf op = {semNr, +1, 0};
    semop(semId, &op, 1);
}

void producator(int idProd, int msg, int semId)
{
    P(semId, SPATII);
    P(semId, MUTEX);
    buffer[*prodMsg] = msg;
    *prodMsg = (*prodMsg + 1) % N;
    printf("Prod.%d msg: %d\n", idProd, msg);
    V(semId, MUTEX);
    V(semId, MESAJE);
}

void consumator(int idCons, int *msg, int semId)
{
    P(semId, MESAJE);
    P(semId, MUTEX);
    *msg = buffer[*consMsg];
    *consMsg = (*consMsg + 1) % N;
    printf("Cons.%d msg: %d\n", idCons, *msg);
    V(semId, MUTEX);
    V(semId, SPATII);
}

int main(int argc, char **argv)
{
    int semId, shmId, i, pid, msg;
    shmId = semget(IPC_PRIVATE, (N + 2) * sizeof(int), IPC_CREAT | 0600);

    if (shmId < 0)
    {
        perror("Eroare creare shm");
        exit(2);
    }

    buffer = (int *)shmat(shmId, 0, 0);
    prodMsg = &buffer[N];
    consMsg = &buffer[N + 1];
    semId = semget(IPC_PRIVATE, 3, IPC_CREAT | 0600);
    if (semId < 0)
    {
        perror("Eroare creare sem");
        exit(2);
    }

    semctl(semId, MUTEX, SETVAL, 1);
    // lacat
    semctl(semId, SPATII, SETVAL, N);
    // pt. prod.
    semctl(semId, MESAJE, SETVAL, 0);
    // pt. cons.
    if ((pid = fork()) == 0) // fiu creeaza prod.
    {
        for (i = 1; i <= 10 * N; i++)
            if (fork() == 0)
            {
                producator(i, i, semId);
                exit(0);
            }
    }
    else // parinte creeaza cons.
    {
        for (i = 1; i <= 10 * N; i++)
            if (fork() == 0)
            {
                consumator(i, &msg, semId);
                exit(0);
            }   
    }
    return 0;
}