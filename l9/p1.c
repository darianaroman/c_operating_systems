#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_TH 100

pthread_mutex_t lock;
pthread_cond_t cond;

typedef struct arg_type
{
    int id;
    int M;
    int N;
} arg_struct;

long long count = 0;
int th_no = 0;

void *increment(void *arg)
{
    long long aux;

    // int th_id = *((int *)arg);
    arg_struct arg_val = *(arg_struct *) arg;

    for (int i = 0; i < arg_val.M; i++)
    {

        // Try getting the lock
        if (pthread_mutex_lock(&lock) != 0)
        {
            perror("Cannot take the lock");
            exit(4);
        }

        while (count % arg_val.N != arg_val.id)
        {
            if (pthread_cond_wait(&cond, &lock) != 0)
            {
                perror("Cannot wait for condition");
                exit(6);
            }
        }

        aux = count;
        printf("Thread %d read count having value %lld\n", arg_val.id, aux);
        aux++;
        usleep(100);
        printf("Thread %d will update count with value %lld\n", arg_val.id, aux);
        count = aux;


        if (pthread_cond_broadcast(&cond) != 0)
        {
            perror("Cannot signal the condition waiters");
            exit(7);
        }

        // Releasing the lock
        if (pthread_mutex_unlock(&lock) != 0)
        {
            perror("Cannot release the lock");
            exit(5);
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Bad number of parameters\n");
        printf("Usage: %s <no_of_threads> <no_of_loops>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    // Create the lock to provide mutual exclusion for the concurrent threads
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
    arg_struct th_args[MAX_TH];

    // Create the N threads
    for (i = 0; i < N; i++)
    {
        th_args[i].id = i;
        th_args[i].M = M;
        th_args[i].N = N;
        if (pthread_create(&th[i], NULL, increment, &th_args[i]) != 0)
        {
            perror("cannot create threads");
            exit(3);
        }
    }

    // Wait for the termination of the N threads created
    for (i = 0; i < N; i++)
    {
        pthread_join(th[i], NULL);
    }

    // Display the final value of count
    printf("At end of program count = %lld\n", count);

    // Remove the lock
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