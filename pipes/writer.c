#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME "fifo"

int main()
{
    if (access(FIFO_NAME, 0) == 0)
    {
        printf("Deleteing pipe\n");
        unlink(FIFO_NAME);
    }

    printf("Creating pipe\n");
    if (mkfifo(FIFO_NAME, 0600) != 0)
    {
        perror("Cannot create pipe");
        return 1;
    }

    printf("Opening pipe\n");
    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1)
    {
        unlink(FIFO_NAME);
        perror("Cannot open pipe");
        return 1;
    }

    printf("Press any key to write to pipe\n");
    getchar();

    int value = 42;

    printf("Writing value: %d\n", value);
    if (write(fd, &value, sizeof(value)) != sizeof(value))
    {
        perror("Cannot write to pipe");
    }

    printf("Closing pipe\n");
    close(fd);

    printf("Deleting pipe\n");
    unlink(FIFO_NAME);

    return 0;
}