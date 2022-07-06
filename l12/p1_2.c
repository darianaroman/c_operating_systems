#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

int main(int argc, char **argv)
{
    int shm_fd;
    off_t size;
    char *data = NULL;

    if (argc != 3)
    {
        printf("usage: %s <shared_memory_name> <size_shared_memory>\n", argv[0]);
        exit(1);
    }
    size = atoi(argv[2]);

    sem_t *sem = sem_open("/sem", O_RDWR);
    if (sem == SEM_FAILED)
    {
        perror("error! opening semaphore");
        exit(1);
    }
    sem_wait(sem);

    sem_t *sem1 = sem_open("/sem1", O_RDWR);
    if (sem1 == SEM_FAILED)
    {
        perror("error! opening sem1");
        exit(1);
    }

    sem_t *sem2 = sem_open("/sem2", O_RDWR);
    if (sem2 == SEM_FAILED)
    {
        perror("error! opening sem2");
        exit(1);
    }

    shm_fd = shm_open(argv[1], O_RDWR, 0600);
    if (shm_fd < 0)
    {
        perror("error! aquiring shared memory\n");
        exit(1);
    }
    // ftruncate(shm_fd, size * sizeof(char)); NO
    data = (char*)mmap(NULL, size * sizeof(char), PROT_READ, MAP_SHARED, shm_fd, 0);

    if (data == MAP_FAILED) {
        perror("error! mapping the file\n");
        close(shm_fd);
        exit(1);
    }
    close(shm_fd);

    for (int i = 0; i < size; i++)
    {
        sem_wait(sem1);
        printf("%c\n", data[i]);
        sem_post(sem2);
    }

    munmap(data, size * sizeof(char));
    data = NULL;

    // shm_unlink(argv[1]); NO
    sem_close(sem);
    sem_close(sem1);
    sem_close(sem2);

    return 0;
}