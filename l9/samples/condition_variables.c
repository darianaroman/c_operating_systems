#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_TH 100

#define SYNCHRONIZED

#ifdef SYNCHRONIZED
pthread_mutex_t lock;
pthread_cond_t cond;
#endif

int N, M;

int th_no = 0;
void *limited_area(void *arg)
{
    int th_id = *((int *)arg);

    // ENTRANCE of the CRITICAL REGION
    // Here is where the thread must check (ask the OS) if it is safe to enter or must wait

    printf("Thread %d is trying to enter its critical region\n", th_id);

#ifdef SYNCHRONIZED
    // Check if entrance in the limited area is allowed
    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("Cannot take the lock");
        exit(4);
    }

    while (th_no >= M)
    { // if already M inside, the wait
        if (pthread_cond_wait(&cond, &lock) != 0)
        {
            perror("Cannot wait for condition");
            exit(6);
        }
    }
#endif

    th_no++;

#ifdef SYNCHRONIZED
    if (pthread_mutex_unlock(&lock) != 0)
    {
        perror("Cannot release the lock");
        exit(5);
    }
#endif

    // BEGINNING of the CRITICAL REGION

    // A thread must be running here only when it is safe,
    //   i.e. there should be so many threads simultaneously inside their critical region
    //   as it is safe for the specific application
    //   e.g. for our applications, thNo should not be more than M at any time
    printf("Thread %d is running inside its critical region\n", th_id);
    printf("----- The no of threads in the limited area is: %d (there should not be more than %d)\n", th_no, M);
    usleep(1000);

    // END of the CRITICAL REGION

    // EXIT of the CRITICAL REGION
    // Here is where the thread must announce (the OS) that it leaves its critical region

    printf("Thread %d is exiting its critical region\n", th_id);

#ifdef SYNCHRONIZED
    // Check if exit from the limited area is allowed
    if (pthread_mutex_lock(&lock) != 0)
    {
        perror("Cannot take the lock");
        exit(4);
    }
#endif

    th_no--;

#ifdef SYNCHRONIZED
    // signal that a new place is available
    if (pthread_cond_signal(&cond) != 0)
    {
        perror("Cannot signal the condition waiters");
        exit(7);
    }

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
    if (argc != 3)
    {
        printf("Bad number of parameters\n");
        printf("Usage: %s <no_of_threads> <max_no_if_threads_in_limited_area>\n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);
    M = atoi(argv[2]);

#ifdef SYNCHRONIZED
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
#endif

    int i;
    pthread_t th[MAX_TH];
    int th_args[MAX_TH];

    // Create the N threads
    for (i = 0; i < N; i++)
    {
        th_args[i] = i;
        if (pthread_create(&th[i], NULL, limited_area, &th_args[i]) != 0)
        {
            perror("cannot create threads");
            exit(4);
        }
    }

    // Wait for the termination of the N threads created
    for (i = 0; i < N; i++)
    {
        pthread_join(th[i], NULL);
    }

#ifdef SYNCHRONIZED
    // Remove the lock
    if (pthread_mutex_destroy(&lock) != 0)
    {
        perror("Cannot destroy the lock");
        exit(8);
    }

    if (pthread_cond_destroy(&cond) != 0)
    {
        perror("Cannot destroy the condition variable");
        exit(9);
    }
#endif

    return 0;
}