#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_LENGTH 256

typedef struct arg_type {
    int n;
    int id;
    int times;
}arg;

int q_pressed = 0;

void *execThread(void *arg_thread)
{
    arg *arg_s = (arg *)arg_thread;
    int i = arg_s->id;
    int times = arg_s->times;
    int n = arg_s->n;
    double *result = (double *) malloc(sizeof(double));
    *result = 1;
    for (int j = 0; j <= times; j++) {
        // printf("%e.\n", (double)(*result) * (n * j + i + 1));
        *result = (double) (*result) * (double)(n * j + i + 1);
    }
    // printf("Thread %d is finished. times = %d n = %d\n", i, times, n);
    return result;
}

int main(int argc, char **argv)
{

    if (argc == 3)
    {
        printf("main thread starts tid=%ld, pid=%d\n", (long int)pthread_self(), (int)getpid());

        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        printf("seconds : %ld\nmicro seconds : %ld\n", current_time.tv_sec, current_time.tv_usec);

        int n = 0;
        sscanf(argv[1], "%d", &n);
        int m = 0;
        sscanf(argv[2], "%d", &m);

        printf("m = %d, n = %d\n", m, n);

        double result = 1;
        int quotient = m / n;
        int remainder = m % n;

        pthread_t t[n];
        arg args[n];
        for (int i = 0; i < n; i++)
        {
            args[i].id = i;
            args[i].n = n;
            if (i+1 <= remainder) {
                args[i].times = quotient;
            }
            else {
                args[i].times = quotient - 1;
            }
            if (pthread_create(&t[i], NULL, execThread, &args[i]) != 0)
            {
                printf("ERROR! creating thread\n");
            }
        }

        double *aux_result;
        for (int i = 0; i < n; i++)
        {
            pthread_join(t[i], (void**)&aux_result);
            result *= *aux_result;
        }

        printf("Thread main is finished.\n");
        gettimeofday(&current_time, NULL);
        printf("seconds : %ld\nmicro seconds : %ld\n", current_time.tv_sec, current_time.tv_usec);
        printf("Result = %e\n", result);
    }
    else
    {
        printf("Usage: %s <number_threads> <max_number>\n", argv[0]);
        exit(1);
    }
}