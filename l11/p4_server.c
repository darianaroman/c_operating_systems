#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

int fd_client;
int fd_server;
int n;

void *compute(void *arg)
{
    char buf[100];
    while (1)
    {
        read(fd_client, buf, 5);
        if (buf[2] == 'x')
        {
            printf("end server\n");
            exit(0);
        }
        buf[5] = 0;
        printf("server: read from pipe %s\n", buf);
        int a = buf[0] - 48;
        int b = buf[4] - 48;
        int result;
        if (buf[2] == '+')
        {
            result = a + b;
        }
        else
        {
            result = a - b;
        }
        char res[10];
        sprintf(res, "%d", result);
        printf("server: result = %s\n", res);
        write(fd_server, res, 1);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("server: insert number of clients\n");
        exit(1);
    }
    mkfifo("/home/dariana/fac/lab/l11/pipe_client_write", 0600);
    mkfifo("/home/dariana/fac/lab/l11/pipe_server_write", 0600);
    fd_client = open("/home/dariana/fac/lab/l11/pipe_client_write", O_RDONLY);
    fd_server = open("/home/dariana/fac/lab/l11/pipe_server_write", O_WRONLY);

    n = atoi(argv[1]);

    pthread_t *th_id;
    th_id = (pthread_t *)malloc(n * sizeof(pthread_t));

    for (int i = 0; i < n; i++)
    {
        if(pthread_create(&th_id[i], NULL, compute, NULL) != 0)
        {
            perror("error! creating threads\n");
            exit(1);
        }
    }

    for (int i = 0; i < n; i++)
    {
        if (pthread_join(th_id[i], NULL) != 0)
        {
            perror("error! joining threads\n");
            exit(1);
        }
    }

    close(fd_client);
    close(fd_server);

    free(th_id);
    return 0;
}