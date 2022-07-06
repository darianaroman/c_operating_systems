// Suppose that in a waiting room, to save current during the night time, an optical sensor was
// installed, so that the light was on only when there was somebody in the room. Write the C program,
// using Linux semaphores, the following types of processes:
// a. person, plays the role of a person that wants to enter the room, stays in there for a little
// while and then exits;
// b. controller, that controls the optical sensor. In an infinite loop it detects whether anybody
// entered the room and if so, it turns on the lights;
// when it detects that even the last person has left the room, it turns the lights off.
// Consider the followings :
// don't use the "busy waiting" technique to continuously verify a condition in an loop.
// Instead use operations on semaphores to put a process in a waiting position.
// After everybody left the room, the lights go off. But the next person can enter only after the
// lights turned on.

#include <stdio.h>
#include <stdlib.h>

#include <sys/sem.h>

#include <unistd.h>
#include <sys/wait.h>

#define CAN_TURN_ON 0
#define CAN_TURN_OFF 1
#define MUTEX 2
#define LET_IN 3

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

void person(int id, int sem_id)
{
    V(sem_id, CAN_TURN_ON);
    printf("person %d entered the room\n", id);
    sleep((id + 1) / 10);
    printf("person %d left the room\n", id);
    P(sem_id, CAN_TURN_ON);
    return;
}

void controller(int sem_id)
{
    printf("controller ok\n");
    while(1)
    {
        printf("Light OFF\n");
        P(sem_id, CAN_TURN_ON);
        printf("Light ON\n");
        V(sem_id, CAN_TURN_ON);
        P(sem_id, CAN_TURN_OFF);
    }

    return;
}

int main()
{
    int sem_id;
    int n = 10;

    sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    if (sem_id < 0)
    {
        perror("error! creating semaphores");
        exit(1);
    }

    semctl(sem_id, CAN_TURN_ON, SETVAL, 0);
    semctl(sem_id, CAN_TURN_OFF, SETVAL, 0);
    semctl(sem_id, MUTEX, SETVAL, 1);
    semctl(sem_id, LET_IN, SETVAL, -n);

    pid_t pid = fork();
    if (pid == 0)
    {
        pid_t *person_pids = (pid_t *) malloc (n * sizeof(pid_t));
        for (int i = 0; i < n; i++)
        {
            person_pids[i] = fork();
            if (person_pids[i] == 0)
            {
                person(i, sem_id);
                exit(0);
            }
        }
        for (int i = 0; i < n; i++)
        {
            waitpid(person_pids[i], NULL, 0);
        }
        printf("[child] end\n");
    }
    else
    {
        pid_t aux_pid = fork();
        if (aux_pid == 0)
        {
            controller(sem_id);
            exit(0);
        }
        waitpid(pid, NULL, 0);
        printf("[parent] end\n");
    }
}