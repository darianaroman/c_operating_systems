#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <unistd.h>

#define MUTEX 0
#define LIGHT_ON 1
#define LIGHT_OFF 2
#define LET_IN 3

int sem_id;

int no_of_persons;
int no_of_persons_left;
int no_of_persons_inside = 0;

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

int random_number(int lower, int upper)
{
    return (int)(rand() % (upper - lower + 1)) + lower;
}

void *person(void *arg)
{
    int id = *(int *)arg;
    printf("Person %d has just arrived in front of the room's door\n", id);
    P(sem_id, MUTEX);
    if (no_of_persons_inside == 0)
    {
        V(sem_id, LIGHT_ON);
        P(sem_id, LET_IN);
    }
    no_of_persons_inside++;
    printf("person %d entered the room | nr persons inside = %d\n", id, no_of_persons_inside);
    V(sem_id, MUTEX);

    usleep(random_number(1, 100));

    P(sem_id, MUTEX);
    no_of_persons_left--;
    no_of_persons_inside--;
    printf("person %d left the room | nr persons inside = %d\n", id, no_of_persons_inside);
    if (no_of_persons_inside == 0)
    {
        V(sem_id, LIGHT_OFF);
    }
    V(sem_id, MUTEX);

    return NULL;
}

void *light_controller(void *arg)
{
    while (no_of_persons_left > 0)
    {
        P(sem_id, LIGHT_ON);
        printf("Light is ON\n");
        V(sem_id, LET_IN);

        P(sem_id, LIGHT_OFF);
        printf("Light is OFF\n");
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t *person_th;
    int *person_arg;
    pthread_t controller_th;

    if (argc != 2)
    {
        printf("USAGE: %s <no_of_persons>\n", argv[0]);
        exit(1);
    }

    sem_id = semget(IPC_PRIVATE, 4, IPC_CREAT | 0600);
    if (sem_id < 0)
    {
        perror("error! creating the semaphore");
    }

    semctl(sem_id, MUTEX, SETVAL, 1);
    semctl(sem_id, LIGHT_ON, SETVAL, 0);
    semctl(sem_id, LIGHT_OFF, SETVAL, 0);
    semctl(sem_id, LET_IN, SETVAL, 0);

    sscanf(argv[1], "%d", &no_of_persons);
    no_of_persons_left = no_of_persons;

    person_th = (pthread_t *)malloc(no_of_persons * sizeof(pthread_t));
    if (person_th == NULL)
    {
        perror("Cannot allocate memory for threads");
        exit(2);
    }

    person_arg = (int *)malloc(no_of_persons * sizeof(int));
    if (person_arg == NULL)
    {
        perror("Cannot allocate memory for person ids");
        exit(2);
    }

    if (pthread_create(&controller_th, NULL, light_controller, NULL) != 0)
    {
        perror("Cannot create controller threads");
        exit(3);
    }

    for (int i = 0; i < no_of_persons; i++)
    {
        person_arg[i] = i;
        if (pthread_create(&person_th[i], NULL, person, &person_arg[i]) != 0)
        {
            perror("Cannot create person threads");
            exit(3);
        }
    }

    for (int i = 0; i < no_of_persons; i++)
    {
        if (pthread_join(person_th[i], NULL) != 0)
        {
            perror("Cannot join person threads");
            exit(4);
        }
    }

    if (pthread_join(controller_th, NULL) != 0)
    {
        perror("Cannot join controller thread");
        exit(4);
    }

    semctl(sem_id, 0, IPC_RMID, 0);

    return 0;
}