#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"

#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>

#define P6_T5_CAN_START 0
#define P6_T3_CAN_TERMINATE 1

#define P7_LIMIT 0
#define P7_MUTEX 1
#define P7_T12_BARRIER 2
#define P7_BARRIER_44 3

#define P6_T4_CAN_START 0
#define P8_T5_CAN_START 1

int p6_sem_id;
int p7_sem_id;
int p8_sem_id;

int p7_th_remaining = 47;

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

void *p8_th_function(void *arg)
{
    int id = *((int *)arg);
    if (id == 5)
    {
        P(p8_sem_id, P8_T5_CAN_START);
    }
    info(BEGIN, 8, id);

    info(END, 8, id);
    if (id == 4)
    {
        V(p8_sem_id, P6_T4_CAN_START);
    }

    return NULL;
}

void *p7_th_function(void *arg)
{
    int id = *((int *)arg);
    P(p7_sem_id, P7_LIMIT);

    info(BEGIN, 7, id);

    if (id == 12)
    {
        P(p7_sem_id, P7_T12_BARRIER);
    }
    else
    {
        P(p7_sem_id, P7_MUTEX);
        p7_th_remaining--;
        if (p7_th_remaining == 0)
        {
            V(p7_sem_id, P7_T12_BARRIER);
        }
        V(p7_sem_id, P7_MUTEX);

        P(p7_sem_id, P7_BARRIER_44);

        P(p7_sem_id, P7_MUTEX);
        if (p7_th_remaining == 0)
        {
            V(p7_sem_id, P7_BARRIER_44);
        }
        V(p7_sem_id, P7_MUTEX);
    }

    info(END, 7, id);

    if (id == 12)
    {
        V(p7_sem_id, P7_BARRIER_44);
    }

    V(p7_sem_id, P7_LIMIT);

    return NULL;
}

void *p6_th_function(void *arg)
{
    int id = *((int *)arg);
    if (id == 5)
    {
        P(p6_sem_id, P6_T5_CAN_START);
    }
    if (id == 4)
    {
        P(p8_sem_id, P6_T4_CAN_START);
    }
    info(BEGIN, 6, id);
    if (id == 3)
    {
        V(p6_sem_id, P6_T5_CAN_START);
    }

    if (id == 3)
    {
        P(p6_sem_id, P6_T3_CAN_TERMINATE);
    }
    info(END, 6, id);
    if (id == 5)
    {
        V(p6_sem_id, P6_T3_CAN_TERMINATE);
    }
    if (id == 4)
    {
        V(p8_sem_id, P8_T5_CAN_START);
    }
    return NULL;
}

int main()
{
    init();

    info(BEGIN, 1, 0);

    p8_sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    if (p8_sem_id < 0)
    {
        perror("error! creating the semaphore");
    }

    semctl(p8_sem_id, P8_T5_CAN_START, SETVAL, 0);
    semctl(p8_sem_id, P6_T4_CAN_START, SETVAL, 0);

    pid_t pid2;
    if ((pid2 = fork()) == 0)
    {
        info(BEGIN, 2, 0);

        pid_t pid3;
        if ((pid3 = fork()) == 0)
        {
            info(BEGIN, 3, 0);

            pid_t pid5;
            if ((pid5 = fork()) == 0)
            {
                info(BEGIN, 5, 0);

                info(END, 5, 0);
                exit(0);
            }

            waitpid(pid5, NULL, 0);

            info(END, 3, 0);
            exit(0);
        }

        pid_t pid6;
        if ((pid6 = fork()) == 0)
        {
            info(BEGIN, 6, 0);

            p6_sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
            if (p6_sem_id < 0)
            {
                perror("error! creating the semaphore");
            }

            semctl(p6_sem_id, P6_T5_CAN_START, SETVAL, 0);
            semctl(p6_sem_id, P6_T3_CAN_TERMINATE, SETVAL, 0);

            pthread_t p6_th_id[5];
            int p6_th_args[5];
            for (int i = 0; i < 5; i++)
            {
                p6_th_args[i] = i + 1;
                if (pthread_create(&p6_th_id[i], NULL, p6_th_function, &p6_th_args[i]) != 0)
                {
                    perror("error! creating threads in proccess 6\n");
                    exit(1);
                }
            }

            for (int i = 0; i < 5; i++)
            {
                pthread_join(p6_th_id[i], NULL);
            }

            semctl(p6_sem_id, 0, IPC_RMID, 0);

            info(END, 6, 0);
            exit(0);
        }

        pid_t pid9;
        if ((pid9 = fork()) == 0)
        {
            info(BEGIN, 9, 0);

            info(END, 9, 0);
            exit(0);
        }

        waitpid(pid3, NULL, 0);
        waitpid(pid6, NULL, 0);
        waitpid(pid9, NULL, 0);

        info(END, 2, 0);
        exit(0);
    }

    pid_t pid4;
    if ((pid4 = fork()) == 0)
    {
        info(BEGIN, 4, 0);

        info(END, 4, 0);
        exit(0);
    }

    pid_t pid7;
    if ((pid7 = fork()) == 0)
    {
        info(BEGIN, 7, 0);

        p7_sem_id = semget(IPC_PRIVATE, 4, IPC_CREAT | 0600);
        if (p7_sem_id < 0)
        {
            perror("error! creating the semaphore");
        }

        semctl(p7_sem_id, P7_LIMIT, SETVAL, 4);
        semctl(p7_sem_id, P7_MUTEX, SETVAL, 1);
        semctl(p7_sem_id, P7_T12_BARRIER, SETVAL, 0);
        semctl(p7_sem_id, P7_BARRIER_44, SETVAL, 44);

        pthread_t p7_th_id[48];
        int p7_th_args[48];
        for (int i = 0; i < 48; i++)
        {
            p7_th_args[i] = i + 1;
            if (pthread_create(&p7_th_id[i], NULL, p7_th_function, &p7_th_args[i]) != 0)
            {
                perror("error! creating threads in proccess 7\n");
                exit(1);
            }
        }

        for (int i = 0; i < 48; i++)
        {
            pthread_join(p7_th_id[i], NULL);
        }

        semctl(p7_sem_id, 0, IPC_RMID, 0);

        info(END, 7, 0);
        exit(0);
    }

    pid_t pid8;
    if ((pid8 = fork()) == 0)
    {
        info(BEGIN, 8, 0);

        pthread_t p8_th_id[5];
        int p8_th_args[5];
        for (int i = 0; i < 5; i++)
        {
            p8_th_args[i] = i + 1;
            if (pthread_create(&p8_th_id[i], NULL, p8_th_function, &p8_th_args[i]) != 0)
            {
                perror("error! creating threads in proccess 8\n");
                exit(1);
            }
        }

        for (int i = 0; i < 5; i++)
        {
            pthread_join(p8_th_id[i], NULL);
        }

        semctl(p8_sem_id, 0, IPC_RMID, 0);

        info(END, 8, 0);
        exit(0);
    }

    waitpid(pid2, NULL, 0);
    waitpid(pid4, NULL, 0);
    waitpid(pid7, NULL, 0);
    waitpid(pid8, NULL, 0);

    info(END, 1, 0);
    return 0;
}
