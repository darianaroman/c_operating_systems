#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_TH 100

#define SYNCHRONIZED

#ifdef SYNCHRONIZED
pthread_mutex_t lock;
#endif

long long count = 0;

void *increment(void *arg)
{
    long long aux;

    int th_id = *((int *)arg);

#ifdef SYNCHRONIZED
    // Try getting the lock
    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("Cannot take the lock");
        exit(4);
    }
#endif

    aux = count;
    printf("Thread %d read count having value %lld\n", th_id, aux);
    aux++;
    usleep(10000);
    // sleep(3);
    printf("Thread %d will update count with value %lld\n", th_id, aux);
    count = aux;

#ifdef SYNCHRONIZED
    // Releasing the lock
    if (pthread_mutex_unlock(&lock) != 0)
    {
        perror("Cannot release the lock");
        exit(5);
    }
#endif

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Bad number of parameters\n");
        printf("Usage: %s no_of_threads\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]);

#ifdef SYNCHRONIZED
    // Create the lock to provide mutual exclusion for the concurrent threads
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        perror("Cannot initialize the lock");
        exit(2);
    }
#endif

    int i;
    pthread_t th[MAX_TH];
    int th_args[MAX_TH];

    // Create the N threads
    for (i = 0; i < N; i++)
    {
        th_args[i] = i;
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

#ifdef SYNCHRONIZED
    // Remove the lock
    if (pthread_mutex_destroy(&lock) != 0)
    {
        perror("Cannot destroy the lock");
        exit(6);
    }
#endif

    return 0;
}