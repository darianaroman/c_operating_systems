// A parent process and a child process, concurrently copies a source file into a destination file
// using the same opened files inherited by the child process from the parent process.By using
// semaphores mutual exclusion is imposed on the two processes that share the space where the
// copy is made.

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/wait.h>

void P(int sem_id, int sem_no)
{
    struct sembuf op = {sem_no, -1, 0};

    semop(sem_id, &op, 1);
}

void V(int sem_id, int sem_no)
{
    struct sembuf op = {sem_no, +1, 0};

    semop(sem_id, &op, 1);
}

void copy(int fdSursa, int fdDest, int semId)

{
    char c;
    int nr, term = 0;
    while (!term)
    {
        P(semId, 0);
        // cerere unica permisiune
        if ((nr = read(fdSursa, &c, 1)) != 1)
        {
            perror("Eroare citire");
            term = 1;
        }

        if (!term && (write(fdDest, &c, nr) != nr))
        {
            perror("Eroare scriere");
            term = 1;
        }

        V(semId, 0);
        // eliberare permisune
    }
}

int main(int argc, char **argv)
{
    int id, pid, fdSursa, fdDest;

    if (argc != 3)
    {
        printf("Utilizare: %s sursa dest\n", argv[0]);

        exit(1);
    }

    id = semget(30000, 1, IPC_CREAT | 0600);

    if (id < 0)
    {
        {
            perror("Eroare creare semafor");
            exit(2);
        }

        if (semctl(id, 0, SETVAL, 1) < 0)
        {
            perror("Eroare setare val. sem.");
            exit(3);
        }
    
        if ((fdSursa = open(argv[1], O_RDONLY)) < 0)
        {
            perror("Eroare deschidere fisier");
            exit(4);
        }

        if ((fdDest = creat(argv[2], 0644)) < 0)
        {
            perror("Eroare creare fisier");
            exit(5);
        }

        pid = fork();

        copy(fdSursa, fdDest, id);

        if (pid)
        {
            waitpid(pid, NULL, 0);
            semctl(id, 0, IPC_RMID, 0);
        }
    }
}