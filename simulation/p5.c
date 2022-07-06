#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_NR_TH 1000

int N;

void *th_function(void *arg)
{
    int id = *((int *)arg);
    int *sum = (int *)malloc(sizeof(int));
    *sum = 0;
    printf("thread id = %d\n", id);
    for (int i = 0; i < N; i++)
    {
        *sum += id + i * N;
    }
    return sum;
}

int main(int argc, char **argv)
{
    N = atoi(argv[1]);

    pthread_t th_id[MAX_NR_TH];
    int th_args[MAX_NR_TH];
    for (int i = 0; i < N; i++)
    {
        th_args[i] = i + 1;
        if (pthread_create(&th_id[i], NULL, th_function, &th_args[i]) != 0)
        {
            perror("error! creating threads\n");
            exit(1);
        }
    }

    int sum = 0;
    void *aux;
    for (int i = 0; i < N; i++)
    {
        pthread_join(th_id[i], &aux);
        sum += *((int *)aux);
    }

    printf("sum = %d\n", sum);
    return 0;
}