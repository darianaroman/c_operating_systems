#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

int main(int argc, char **argv)
{
    int fd;
    off_t size;
    char *data = NULL;

    if (argc != 2)
    {
        printf("usage: %s <path>\n", argv[0]);
        exit(1);
    }

    sem_t *sem = sem_open("/sem", O_CREAT, 0600, 0);
    if (sem == SEM_FAILED)
    {
        perror("error! creating semaphore");
        exit(1);
    }

    sem_t *sem1;
    sem1 = sem_open("/sem1", O_CREAT, 0600, 0);
    if (sem1 == SEM_FAILED)
    {
        perror("error! creating sem1");
        exit(1);
    }

    sem_t *sem2;
    sem2 = sem_open("/sem2", O_CREAT, 0600, 0);
    if (sem2 == SEM_FAILED)
    {
        perror("error! creating sem2");
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

    sem_post(sem);

    ftruncate(fd, size * sizeof(char));

    data = (char *)mmap(NULL, size * sizeof(char), PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED)
    {
        perror("error! mapping the file\n");
        close(fd);
        exit(1);
    }

    close(fd);

    for (int i = 0; i < size; i++)
    {
        data[i] = data[i] + 1;
        sem_post(sem1);
        sem_wait(sem2);
        sleep(1);
        // usleep(500);
    }

    munmap(data, size * sizeof(char));
    data = NULL;

    shm_unlink(argv[1]);
    sem_close(sem);
    sem_close(sem1);
    sem_close(sem2);
    sem_unlink("/sem1");
    sem_unlink("/sem2");
    sem_unlink("/sem");

    return 0;
}