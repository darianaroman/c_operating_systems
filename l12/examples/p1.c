#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int shm_fd;
    char *shared_char = NULL;

    shm_fd = shm_open("/myshm", O_CREAT | O_RDWR, 0600);

    if (shm_fd < 0)
    {
        perror("Could not aquire shm");
        return 1;
    }

    ftruncate(shm_fd, sizeof(char));

    shared_char = (char *)mmap(0, sizeof(char), PROT_READ | PROT_WRITE,
                               MAP_SHARED, shm_fd, 0);
    if (shared_char == MAP_FAILED)
    {
        perror("Could not map the shared memory");
        return 1;
    }

    close(shm_fd);

    *shared_char = 'A';
    while (*shared_char == 'A')
    {
        printf("shared_char: %c\n", *shared_char);
        sleep(1);
    }

    printf("shared_char new value: %c\n", *shared_char);

    munmap(shared_char, sizeof(char));
    shared_char = NULL;

    shm_unlink("/myshm");

    return 0;
}