#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME "fifo"

int main()
{
    printf("Opening pipe\n");
    int fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1)
    {
        unlink(FIFO_NAME);
        perror("Cannot open pipe");
        return 1;
    }

    printf("Reading from pipe\n");
    int value = 0;
    if (read(fd, &value, sizeof(value)) != sizeof(value))
    {
        perror("Cannot read from pipe");
        close(fd);
        return 1;
    }

    printf("The read value is %d\n", value);

    printf("Closing pipe\n");
    close(fd);
    
    return 0;
}