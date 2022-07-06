#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <pthread.h>

#define MAX_LENGTH 256

int q_pressed = 0;

void *execThread(void *arg)
{
    int *v = (int*)arg;
    while(1) {
        // printf("%d\n", *v);
        if (q_pressed)
        {
            printf("Thread %d is finished.\n", *v);
            pthread_exit(0);
        }
    }

    return 0;
}

int main(int argc, char **argv) {

    if (argc == 2) {
        int n = 0;
        sscanf(argv[1], "%d", &n);
        printf("main thread starts tid=%ld, pid=%d\n", (long int)pthread_self(), (int)getpid());

        pthread_t t[n];
        int args[n];
        for (int i = 0; i < n; i++)
        {
            args[i] = i;
            if (pthread_create(&t[i], NULL, execThread, &args[i]) != 0)
            {
                printf("ERROR! creating thread\n");
            }
        }

        char *buffer = (char *)malloc(sizeof(char));
        printf("$ ");
        while (scanf("%c", buffer) && buffer[0] != 'q')
        {
            // printf("$ ");
        }
        q_pressed = 1;

        for (int i = 0; i < n; i++)
        {
            pthread_join(t[i], NULL);
        }

        printf("Thread main is finished.\n");
    }
    else {
        printf("Usage: %s <number_threads>\n", argv[0]);
        exit(1);
    }
   
}