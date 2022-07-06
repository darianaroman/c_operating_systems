#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_TH 100

pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_cond_t cond;
pthread_cond_t cond2;

int number_cannibals_boarded = 0;
int number_missionaries_boarded = 0;
int number_cannibals = 0;
int number_missionaries = 0;
int boat_arrived = 0;

int N;

int random_number(int lower, int upper)
{
    return (int)(rand() % (upper - lower + 1)) + lower;
}

void cannibal(int id)
{
    if (number_missionaries)
    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("error! locking\n");
        exit(1);
    }
    number_cannibals++;
    while (number_cannibals_boarded >= number_missionaries_boarded || number_cannibals_boarded + number_missionaries_boarded >= 3 || boat_arrived == 0)
    {
        if (pthread_cond_wait(&cond, &lock) != 0)
        {
            perror("error! waiting for condition variable\n");
            exit(1);
        }
    }
    number_cannibals_boarded++;
    if (pthread_mutex_unlock(&lock) != 0)
    {
        perror("error! unlocking\n");
        exit(1);
    }

    printf("cannibal %d embarked\n", id);

    if (pthread_mutex_lock(&lock2) != 0)
    {
        perror("error! locking\n");
        exit(1);
    }
    while (boat_arrived == 1)
    {
        if (pthread_cond_wait(&cond2, &lock2) != 0)
        {
            perror("error! waiting for condition variable\n");
            exit(1);
        }
    }
    number_cannibals--;
    number_cannibals_boarded--;
    if (pthread_mutex_unlock(&lock2) != 0)
    {
        perror("error! unlocking\n");
        exit(1);
    }
}

void missionary(int id)
{
    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("error! locking\n");
        exit(1);
    }
    number_missionaries++;
    while (number_cannibals_boarded + number_missionaries_boarded >= 3 || boat_arrived == 0)
    {
        if (pthread_cond_wait(&cond, &lock) != 0)
        {
            perror("error! waiting for condition variable\n");
            exit(1);
        }
    }
    number_missionaries_boarded++;
    if (pthread_mutex_unlock(&lock) != 0)
    {
        perror("error! unlocking\n");
        exit(1);
    }

    printf("missionary %d emarked\n", id);

    if (pthread_mutex_lock(&lock2) != 0)
    {
        perror("error! locking\n");
        exit(1);
    }
    while (boat_arrived == 1)
    {
        if (pthread_cond_wait(&cond2, &lock2) != 0)
        {
            perror("error! waiting for condition variable\n");
            exit(1);
        }
    }
    number_missionaries--;
    number_missionaries_boarded--;
    if (pthread_mutex_unlock(&lock2) != 0)
    {
        perror("error! unlocking\n");
        exit(1);
    }
}

void *boat()
{
    while (number_missionaries > 0)
    {
        printf("boat arrived at dock waiting to be boarded\n");
        boat_arrived = 1;
        if (pthread_cond_broadcast(&cond) != 0)
        {
            perror("error! broadcasting the condition variable\n");
            exit(1);
        }
        usleep(1000);
        printf("boat sailed\n");
        boat_arrived = 0;
        if (pthread_cond_broadcast(&cond2) != 0)
        {
            perror("error! broadcasting the condition variable\n");
            exit(1);
        }
        usleep(1000);
    }

    return NULL;
}

void *assign_role(void *arg)
{
    int id = *((int *)arg);
    if (random_number(0, 9) >= 7)
    {
        cannibal(id);
    }
    else
    {
        missionary(id);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("USAGE: %s <number_threads>\n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        perror("error! initializing the lock\n");
        exit(1);
    }

    if (pthread_mutex_init(&lock2, NULL) != 0)
    {
        perror("error! initializing the lock\n");
        exit(1);
    }

    if (pthread_cond_init(&cond, NULL) != 0)
    {
        perror("error! initializing the condition variable\n");
        exit(1);
    }

    if (pthread_cond_init(&cond2, NULL) != 0)
    {
        perror("error! initializing the condition variable\n");
        exit(1);
    }

    pthread_t threads[MAX_TH];
    int threads_args[MAX_TH];
    pthread_t thread_boat;

    for (int i = 0; i < N; i++)
    {
        threads_args[i] = i;
        if (pthread_create(&threads[i], NULL, assign_role, &threads_args[i]) != 0)
        {
            perror("error! creating the threads\n");
            exit(1);
        }
    }

    if (pthread_create(&thread_boat, NULL, boat, NULL) != 0)
    {
        perror("error! creating the boat thread\n");
        exit(1);
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }
    pthread_join(thread_boat, NULL);

    // CLEAN UP:

    if (pthread_mutex_destroy(&lock) != 0)
    {
        perror("error! destroying the lock\n");
        exit(1);
    }

    if (pthread_mutex_destroy(&lock2) != 0)
    {
        perror("error! destroying the lock\n");
        exit(1);
    }

    if (pthread_cond_destroy(&cond) != 0)
    {
        perror("error! destroying the condition variable\n");
        exit(1);
    }

    if (pthread_cond_destroy(&cond2) != 0)
    {
        perror("error! destroying the condition variable\n");
        exit(1);
    }

    return 0;
}