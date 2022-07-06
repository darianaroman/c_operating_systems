#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int fd;
    off_t size;
    char *data = NULL;
    int given_offset = 2;

    if (argc != 2)
    {
        printf("usage: %s <path>\n", argv[0]);
        exit(1);
    }

    fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        perror("error! write: aquiring shared memory");
        exit(1);
    }
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);


    ftruncate(fd, size * sizeof(char));

    data = (char *)mmap(NULL, size * sizeof(char), PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED)
    {
        perror("error! mapping the file\n");
        close(fd);
        exit(1);
    }

    // close(fd);

    for (int i = 0; i < size; i++)
    {
        printf("%x\n", data[i]);
    }

    for (int i = given_offset; i < given_offset + 2; i++)
    {
        data[i] = 'x';
    }

    printf("page size = %d\n", getpagesize());
    printf("size of our file = %ld\n", size);

    // data[10000] = 'X';
    // page size = 4096
    // size of our file = 4823
    // => nothing happens

    // page size = 4096
    // size of our file = 4096
    // => Segmentation fault

    // page size = 4096
    // size of our file = 4087
    // => Segmentation fault

    // page size = 4096
    // size of our file = 8192
    // Segmentation fault

    data = (char *)mmap(NULL, 2 * size * sizeof(char), PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED)
    {
        perror("error! remapping the file");
        close(fd);
        exit(1);
    }
    close(fd);

    // data[10000] = 'X';
    // => Bus error

    munmap(data, size * sizeof(char));
    data = NULL;

    shm_unlink(argv[1]);

    return 0;
}