#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int unpackValue(char *string, int size)
{
    int value = 0;
    for (int i = size - 1; i >= 0; i--)
    {
        value = value << 8;
        value = value + string[i];
    }
    return value;
}

int main(int argc, char **argv)
{
    int fd;
    off_t size;
    char *data = NULL;

    if (argc != 2)
    {
        printf("usage: %s <path_of_file>\n", argv[1]);
        exit(1);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("error! opening the file");
        exit(1);
    }

    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    if (size % 16 != 0)
    {
        size = size - (size % 16);
        ftruncate(fd, size);
    }

    printf("the size of the given file is %ld\n", size);

    int second_quarter_offset = size / 4;

    lseek(fd, second_quarter_offset, SEEK_SET);

    char little_buff;
    for (int i = 0; i < size / 4; i++)
    {
        if (read(fd, &little_buff, sizeof(char)) != sizeof(char))
        {
            perror("error! reading from second quarter");
        }
        printf("%c", little_buff);
    }
    printf("\n");

    int last_quarter_offset = 3 * (size / 4);

    data = (char*)mmap(NULL, size / 4, PROT_READ, MAP_PRIVATE, fd, last_quarter_offset);

    if (data == MAP_FAILED)
    {
        perror("error! mapping the file");
        close(fd);
        exit(1);
    }

    close(fd);

    int sum = 0;
    char data_aux[4];
    for (int i = 0; i < size / 4; i = i + 4)
    {
        data_aux[0] = data[i];
        data_aux[1] = data[i + 1];
        data_aux[2] = data[i + 2];
        data_aux[3] = data[i + 3];
        sum += unpackValue(data_aux, 4);
    }

    printf("sum is %d\n", sum);

    return 0;
}