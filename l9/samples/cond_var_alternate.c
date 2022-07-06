#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_TH 100

pthread_mutex_t lock;
pthread_cond_t cond;
long long count = 0;
int N;
int M;

void* increment(void *arg)
{
    long long aux;
    int th_id = *((int *)arg);

    for (int i = 0; i < M; i++)
    {
        if (pthread_mutex_lock(&lock) != 0)
        {
            perror("error! locking\n");
            exit(1);
        }

        while ((count % N) != th_id)
        {
            if (pthread_cond_wait(&cond, &lock) != 0)
            {
                perror("error! waiting for condition variable\n");
                exit(1);
            }
        }

        aux = count;
        printf("Thread %d : count old value = %lld\n", th_id, aux);
        aux++;
        usleep(100);
        printf("Thread %d : count new value %lld\n", th_id, aux);
        count = aux;

        if(pthread_cond_signal(&cond) != 0)
        {
            perror("error! broadcasting the condition variable\n");
            exit(1);
        }

        if (pthread_mutex_unlock(&lock) != 0)
        {
            perror("error! unlocking\n");
            exit(1);
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("USAGE: %s <number_threads> <number_loops>\n", argv[0]);
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
        if (pthread_create(&threads[i], NULL, increment, &threads_args[i]) != 0)
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