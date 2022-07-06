#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define N 10

int sem_id;

// decrement by 1 the semaphore sem_no in the semaphore set sem_id
//    - ask for a permission, i.e. wait until a permission become available
void P(int sem_id, int sem_no)
{
    struct sembuf op = {sem_no, -1, 0};

    semop(sem_id, &op, 1);
}

// increment by 1 the semaphore sem_no in the semaphore set sem_id
//   - release a previously obtained persmission
void V(int sem_id, int sem_no)
{
    struct sembuf op = {sem_no, +1, 0};

    semop(sem_id, &op, 1);
}

// function executed by the N concurrent threads
// maximum 3 threads are allowed to be simultaneously in theri critical regions, i.e. the room
// this means that we cannot see more than 3 consecutive messages announcing the entrance, not followed by a leaving message
void *three_in_a_room(void *arg)
{
    int th_id = *((int *)arg);

    printf("Thread %d is trying entering the room, but will wait of not enough place there.\n", th_id);

    P(sem_id, 0);

    printf("Thread %d has entered the room and will stay in for about 2 seconds.\n", th_id);
    sleep(2);
    printf("Thread %d is leaving the room.\n", th_id);

    V(sem_id, 0);

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t t[N];
    int th_arg[N];
    int i;

    // create a private (unnamed) semaphore set of just 1 semaphore
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (sem_id < 0)
    {
        perror("Error creating the semaphore set");
        exit(2);
    }

    // Initialize the first (and single) semaphore in the set with 3
    semctl(sem_id, 0, SETVAL, 3);

    // create N competing threads
    for (i = 0; i < N; i++)
    {
        th_arg[i] = i;
        pthread_create(&t[i], NULL, three_in_a_room, &th_arg[i]);
    }

    // wait for the created threads to terminate
    for (i = 0; i < N; i++)
    {
        pthread_join(t[i], NULL);
    }

    // destroy (remove) the semaphore set
    semctl(sem_id, 0, IPC_RMID, 0);
}
