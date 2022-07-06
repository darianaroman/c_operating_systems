#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_TH 100

pthread_mutex_t lock;
pthread_cond_t cond;

int N, M;
int thread_number = 0;

void* limited_area(void *arg)
{
    int thread_id = *((int*) arg);
    printf("thread with id %d is trying to enter its critical region\n", thread_id);

    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("error! locking\n");
        exit(1);
    }

    while (thread_number >= M)
    {
        if (pthread_cond_wait(&cond, &lock) != 0)
        {
            perror("error! condvariable wait\n");
            exit(1);
        }
    }

    thread_number++;

    if (pthread_mutex_unlock(&lock) != 0)
    {
        perror("error! unlocking\n");
        exit(1);
    }


    printf("thread with id %d in its critical region - number threads = %d\n", thread_id, thread_number);
    usleep(1000);

    printf("thread with id %d exiting its critical region\n", thread_id);


    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("error! locking\n");
        exit(1);
    }

    thread_number--;

    if (pthread_cond_signal(&cond) != 0)
    {
        perror("error! signaling condition variable\n");
        exit(1);
    }

    if (pthread_mutex_unlock(&lock) != 0)
    {
        perror("error! unlocking\n");
        exit(1);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("USAGE: %s <number_threads> <max_nr_threads_in_limited_area>\n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);
    M = atoi(argv[2]);

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        perror("error! initializing the lock\n");
        exit(1);
    }

    if (pthread_cond_init(&cond, NULL) != 0)
    {
        perror("error! initializing the condition variable\n");
        exit(1);
    }

    pthread_t threads[MAX_TH];
    int threads_args[MAX_TH];

    for (int i = 0; i < N; i++)
    {
        threads_args[i] = i;
        if (pthread_create(&threads[i], NULL, limited_area, &threads_args[i]) != 0)
        {
            perror("error! creating the threads\n");
            exit(1);
        }
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // CLEAN UP:

    if (pthread_mutex_destroy(&lock) != 0)
    {
        perror("error! destroying the lock\n");
        exit(1);
    }

    if (pthread_cond_destroy(&cond) != 0)
    {
        perror("error! destroying the condition variable\n");
        exit(1);
    }

    return 0;
}