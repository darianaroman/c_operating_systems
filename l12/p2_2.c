#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int fd;
    off_t size, i;
    char *data = NULL;

    int count = 0;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1)
    {
        perror("Could not open input file");
        return 1;
    }
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    data = (char *)mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (data == (void *)-1)
    {
        perror("Could not map file");
        close(fd);
        return 1;
    }

    for (i = 0; i < size; i++)
    {
        if (data[i] != 'a' && data[i] != 'e' && data[i] != 'i' && data[i] != 'o' && data[i] != 'u')
        {
            data[count] = data[i];
            count++;
        }
    }

    for (i = count; i < size; i++)
    {
        data[i] = '.';
    }

    munmap(data, size);
    close(fd);

    return 0;
}