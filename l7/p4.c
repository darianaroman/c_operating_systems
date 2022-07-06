#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <pthread.h>

#define MAX_LENGTH 256

int x_pressed = 0;

void *execThreadClient(void *arg)
{
    int *id = (int *) arg;
    printf("Client thread id = %d Starting...\n", *id);
    sleep(*id);
    printf("Client thread id = %d Ending...\n", *id);
    if (x_pressed) {
        pthread_exit(0);
    }
    return 0;
}

void *execThreadConnection(void *arg)
{
    int *n = (int *) arg;
    pthread_t t[1000];
    int args[1000];
    int i = 0;
    int number_threads = 0;
    while(i < 1000 && !x_pressed) {
        args[i] = i;
        if (pthread_create(&t[i], NULL, execThreadClient, &args[i]) != 0)
        {
            printf("ERROR! creating thread\n");
        }
        i++;
        sleep(*n);
    }

    number_threads = i + 1;

    for (int i = 0; i < number_threads; i++)
    {
        pthread_join(t[i], NULL);
    }

    printf("Connection thread is finished\n");
    return 0;
}

void *execThreadControll(void *arg)
{
    char *buffer = (char *)malloc(sizeof(char));
    printf("$ ");
    while (scanf("%c", buffer) && buffer[0] != 'x')
    {
        if (buffer[0] == '\n')
        {
            printf("$ ");
        }
    }
    x_pressed = 1;
    printf("Controll thread is finished\n");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <number_seconds>\n", argv[0]);
        exit(1);
    }
    int n = 0;
    sscanf(argv[1], "%d", &n);
    printf("main thread starts tid=%ld, pid=%d\n", (long int)pthread_self(), (int)getpid());

    pthread_t t_controll;
    if (pthread_create(&t_controll, NULL, execThreadControll, NULL) != 0)
    {
        printf("ERROR! creating controll thread\n");
    }

    pthread_t t_connection;
    if (pthread_create(&t_connection, NULL, execThreadConnection, &n) != 0)
    {
        printf("ERROR! creating connection thread\n");
    }
    pthread_join(t_connection, NULL);
    pthread_join(t_controll, NULL);

    printf("Thread main is finished.\n");
}