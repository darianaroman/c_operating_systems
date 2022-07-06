#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    char buf[100];
    mkfifo("/home/dariana/fac/lab/l11/pipe_client_write", 0600);
    mkfifo("/home/dariana/fac/lab/l11/pipe_server_write", 0600);
    int fd_client = open("/home/dariana/fac/lab/l11/pipe_client_write", O_WRONLY);
    int fd_server = open("/home/dariana/fac/lab/l11/pipe_server_write", O_RDONLY);
    while (1)
    {
        fgets(buf, 30, stdin);
        buf[strlen(buf) - 1] = 0;
        printf("read in buf: %s\n", buf);
        write(fd_client, buf, 5);
        if (buf[2] == 'x')
        {
            printf("end client\n");
            exit(0);
        }
        read(fd_server, buf, 1);
        buf[1] = 0;
        printf("client: read from pipe %s\n", buf);
    }
    close(fd_client);
    close(fd_server);
    return 0;
}