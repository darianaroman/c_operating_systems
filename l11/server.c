#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    mkfifo("/home/dariana/fac/lab/l11/pipe_client_write", 0600);
    mkfifo("/home/dariana/fac/lab/l11/pipe_server_write", 0600);
    int fd_client = open("/home/dariana/fac/lab/l11/pipe_client_write", O_RDONLY);
    int fd_server = open("/home/dariana/fac/lab/l11/pipe_server_write", O_WRONLY);
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
    close(fd_client);
    close(fd_server);
    return 0;
}