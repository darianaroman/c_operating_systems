#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <unistd.h>

#define MAX_TH 1000
#define MAX 10

pthread_mutex_t lock;
pthread_cond_t cond;

int passing_direction = 0;
int N;
int number_cars_bridge[1];

typedef struct car_args_type
{
    int car_id;
    int car_direction;
} car_args;

int random_number(int lower, int upper)
{
    return (int)(rand() % (upper - lower + 1)) + lower;
}

void enter_bridge(int id, int direction)
{
    pthread_mutex_lock(&lock);
    while (passing_direction != direction || number_cars_bridge[1 - direction] > 0 || number_cars_bridge[0] + number_cars_bridge[1] >= MAX)
    {
        pthread_cond_wait(&cond, &lock);
    }
    number_cars_bridge[direction]++;
    printf("car %d entered bridge - direction %d - number cars on bridge = %d\n", id, direction, number_cars_bridge[direction]);
    pthread_mutex_unlock(&lock);
    return;
}

void pass_bridge(int id, int direction)
{
    printf("car %d crossing\n", id);
    sleep(1);
    return;
}

void exit_bridge(int id, int direction)
{
    pthread_mutex_lock(&lock);
    number_cars_bridge[direction]--;
    printf("car %d exited the bridge - direction %d - number cars on bridge = %d\n", id, direction, number_cars_bridge[direction]);
    pthread_mutex_unlock(&lock);
}

void *car(void *arg_param)
{
    car_args *arg = (car_args*)arg_param;

    enter_bridge(arg->car_id, arg->car_direction);
    pass_bridge(arg->car_id, arg->car_direction);
    exit_bridge(arg->car_id, arg->car_direction);

    return NULL;
}

void change_passing_direction()
{
    passing_direction = 1 - passing_direction;
    printf("new passing direction = %d\n", passing_direction);
    pthread_cond_broadcast(&cond);
    return;
}

void *trafic_controller(void *arg)
{
    while (1)
    {
        sleep(2);
        change_passing_direction();
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("USAGE: %s <number_cars>\n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);

    number_cars_bridge[0] = 0;
    number_cars_bridge[1] = 0;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_t thread_controller;
    if (pthread_create(&thread_controller, NULL, trafic_controller, NULL) != 0)
    {
        perror("error! creating the controller thread\n");
        exit(1);
    }

    pthread_t th[MAX_TH];
    car_args car_args_value[MAX_TH];

    for (int i = 0; i < N; i++)
    {
        car_args_value[i].car_id = i;
        car_args_value[i].car_direction = random_number(0, 1);
        if (pthread_create(&th[i], NULL, car, &car_args_value[i]) != 0)
        {
            perror("cannot create threads");
            exit(1);
        }
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(th[i], NULL);
    }
    pthread_join(thread_controller, NULL);

    // CLEAN UP:

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}