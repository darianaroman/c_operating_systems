// Observations: as the number of threads increases the IO threads starts being unresponsive, it
// responds more and more slowly to the 'x' command as I increase the number of CPU bound threads 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_LENGTH 256

int x_pressed = 0;
struct timeval current_time;

void *execThreadClient(void *arg)
{
    while (1)
    {
        if (x_pressed)
        {
            pthread_exit(0);
        }
    }
    return 0;
}

void *execThreadConnection(void *arg)
{
    int *n = (int *)arg;
    pthread_t t[1000];
    int args[1000];
    for (int i = 0; i < *n; i++)
    {
        args[i] = i;
        if (pthread_create(&t[i], NULL, execThreadClient, &args[i]) != 0)
        {
            printf("ERROR! creating thread\n");
        }
        i++;
        sleep(*n);
    }

    for (int i = 0; i < *n; i++)
    {
        pthread_join(t[i], NULL);
    }

    printf("Connection thread is finished\n");
    gettimeofday(&current_time, NULL);
    printf("end connection: seconds = %ld\nmicro seconds = %ld\n", current_time.tv_sec, current_time.tv_usec);

    return 0;
}

void *execThreadControl(void *arg)
{

    char *buffer = (char *)malloc(sizeof(char));
    printf("$ ");
    while (scanf("%c", buffer) && buffer[0] != 'x')
    {
        if (buffer[0] == '\n')
        {
            printf("$ ");
            gettimeofday(&current_time, NULL);
            printf("enter: seconds = %ld\nmicro seconds = %ld\n", current_time.tv_sec, current_time.tv_usec);
        }
    }
    x_pressed = 1;
    printf("Control thread is finished\n");
    gettimeofday(&current_time, NULL);
    printf("end control: seconds = %ld\nmicro seconds = %ld\n", current_time.tv_sec, current_time.tv_usec);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <number_threads>\n", argv[0]);
        exit(1);
    }
    int n = 0;
    sscanf(argv[1], "%d", &n);
    printf("main thread starts tid=%ld, pid=%d\n", (long int)pthread_self(), (int)getpid());

    pthread_t t_control;
    if (pthread_create(&t_control, NULL, execThreadControl, NULL) != 0)
    {
        printf("ERROR! creating control thread\n");
    }

    pthread_t t_connection;
    if (pthread_create(&t_connection, NULL, execThreadConnection, &n) != 0)
    {
        printf("ERROR! creating connection thread\n");
    }
    pthread_join(t_connection, NULL);
    pthread_join(t_control, NULL);

    printf("Thread main is finished.\n");
    gettimeofday(&current_time, NULL);
    printf("end main: seconds = %ld\nmicro seconds = %ld\n", current_time.tv_sec, current_time.tv_usec);
}