#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_TH 100

pthread_mutex_t lock;
pthread_cond_t cond;

pthread_mutex_t car_lock;
pthread_cond_t car_cond;

typedef struct arg_type
{
    int id;
    int N;
} arg_struct;

int lights_turn = 0;

void *traffic_light(void *arg)
{
    int id = *(int *)arg;
    while (1)
    {
        if (pthread_mutex_lock(&lock) != 0)
        {
            perror("Cannot take the lock");
            exit(1);
        }

        while (id != lights_turn)
        {
            if (pthread_cond_wait(&cond, &lock) != 0)
            {
                perror("Cannot wait for condition");
                exit(6);
            }
        }

        printf("traffic light %d GREEN\n", id);

        if (pthread_cond_broadcast(&car_cond) != 0)
        {
            perror("Cannot signal the condition waiters");
            exit(1);
        }

        sleep(5);
        printf("traffic light %d RED\n", id);

        lights_turn++;
        lights_turn %= 2;

        if (pthread_cond_broadcast(&cond) != 0)
        {
            perror("Cannot signal the condition waiters");
            exit(1);
        }

        if (pthread_mutex_unlock(&lock) != 0)
        {
            perror("Cannot release the lock");
            exit(2);
        }
    }
    return NULL;
}

void *car(void *arg)
{
    arg_struct arg_val = *(arg_struct *)arg;
    int id = arg_val.id;
    int direction = arg_val.id % 2;

    if (pthread_mutex_lock(&car_lock) != 0)
    {
        perror("can't lock car_lock mutex");
        exit(1);
    }

    printf("car %d arrived at intersection, direction = %d\n", id, direction);
    while (lights_turn != direction)
    {
        if (pthread_cond_wait(&car_cond, &car_lock) != 0)
        {
            perror("can't wait for car_cond");
            exit(1);
        }
    }

    sleep(1);
    printf("car %d crossing the intersection, direction = %d\n", id, direction);
    sleep(1);
    printf("car %d leaving the intersection, direction = %d\n", id, direction);

    if (pthread_mutex_unlock(&car_lock) != 0)
    {
        perror("can't unlock car_lock lock");
        exit(1);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Bad number of parameters\n");
        printf("Usage: %s <cars>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]);

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        perror("Cannot initialize the lock");
        exit(2);
    }

    if (pthread_cond_init(&cond, NULL) != 0)
    {
        perror("Cannot initialize the condition variable");
        exit(3);
    }

    int i;
    pthread_t th[MAX_TH];
    arg_struct car_args[MAX_TH];

    for (i = 0; i < N; i++)
    {
        car_args[i].id = i;
        car_args[i].N = N;
        if (pthread_create(&th[i], NULL, car, &car_args[i]) != 0)
        {
            perror("cannot create threads");
            exit(3);
        }
    }

    pthread_t lights_th[2];
    int lights_ids[2];

    for (i = 0; i < 2; i++)
    {
        lights_ids[i] = i;
        if (pthread_create(&lights_th[i], NULL, traffic_light, &lights_ids[i]) != 0)
        {
            perror("cannot create traffic lights");
            exit(3);
        }
    }

    for (i = 0; i < N; i++)
    {
        pthread_join(th[i], NULL);
    }

    pthread_join(lights_th[0], NULL);
    pthread_join(lights_th[1], NULL);

    if (pthread_mutex_destroy(&lock) != 0)
    {
        perror("Cannot destroy the lock");
        exit(6);
    }

    if (pthread_cond_destroy(&cond) != 0)
    {
        perror("Cannot destroy the condition variable");
        exit(9);
    }

    return 0;
}