#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>

int sem_id;
int sem_id1;

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

int th_no = 0;
void limited_area()
{
    P(sem_id1, 0);
    th_no++;
    V(sem_id1, 0);
    usleep(100);
    printf("The no of threads in the limited area is: %d\n", th_no);
    P(sem_id1, 0);
    th_no--;
    V(sem_id1, 0);
}

void *protect_area(void *arg) {
    P(sem_id, 0);
    limited_area();
    V(sem_id, 0);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    int n, m;
    pthread_t *t;
    if (argc != 3)
    {
        printf("Usage: %s <number_threads> <max_number>\n", argv[0]);
        exit(1);
    }
    sscanf(argv[1], "%d", &n);
    sscanf(argv[2], "%d", &m);
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    sem_id1 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (sem_id < 0 || sem_id1 < 0)
    {
        perror("Error creating the semaphore set");
        exit(2);
    }
    // initialize the first (and single) semaphore in the set with 1
    semctl(sem_id, 0, SETVAL, m);
    semctl(sem_id1, 0, SETVAL, 1);

    t = (pthread_t *)malloc(n * sizeof(pthread_t));
    for (int i = 0; i < n; i++)
    {
        pthread_create(&t[i], NULL, protect_area, NULL);
    }
    for (int i = 0; i < n; i++)
    {
        pthread_join(t[i], NULL);
    }
    printf("%d\n", th_no);
    // destroy (remove) the semaphore set
    semctl(sem_id, 0, IPC_RMID, 0);
    semctl(sem_id1, 0, IPC_RMID, 0);
}