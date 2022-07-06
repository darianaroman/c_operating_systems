#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

int main(int argc, char **argv)
{
    char cmd[30];
    char *pos;
    char cmd1[30];
    char cmd2[30];

    int fd[2];

    int pid1;
    int pid2;

    while (1)
    {
        printf("> ");
        fgets(cmd, 30, stdin);
        if (strncmp(cmd, "exit", 4) == 0)
        {
            exit(0);
        }
        cmd[strlen(cmd) - 1] = 0;
        pos = index(cmd, '|');
        if (pos == NULL || pos == cmd)
        {
            printf("Introduce two commands, separated by |\n");
            continue;
        }


        if (*(pos - 1) == ' ')
        {
            strncpy(cmd1, cmd, pos - cmd - 1);
            cmd1[pos - cmd - 1] = 0;
        }
        else
        {
            strncpy(cmd1, cmd, pos - cmd);
            cmd1[pos - cmd] = 0;
        }

        printf("cmd1 = %s\n", cmd1);

        char *cmd_end = &cmd[strlen(cmd) - 1];

        if (*(pos + 1) == ' ')
        {
            strncpy(cmd2, pos + 2, cmd_end - (pos));
            cmd2[cmd_end - (pos)] = 0;
        }
        else
        {
            strncpy(cmd2, pos + 1, cmd_end - pos);
            cmd2[cmd_end - pos] = 0;
        }

        printf("cmd2 = %s\n", cmd2);

        pipe(fd);

        pid1 = fork();
        if (pid1 < 0)
        {
            perror("error! creating the first child\n");
            exit(1);
        }

        if (pid1 == 0)
        {
            close(fd[0]);
            dup2(fd[1], 1); // redirect stdout
            close(fd[1]);

            execlp(cmd1, cmd1, NULL);
            perror("error! executing command 1\n");
            exit(1);
        }

        pid2 = fork();
        if (pid2 < 0)
        {
            perror("error! creating the second child\n");
            exit(1);
        }

        if (pid2 == 0)
        {
            close(fd[1]);
            dup2(fd[0], 0); // redirect stdin
            close(fd[0]);

            execlp(cmd2, cmd2, NULL);
            perror("error! executing command 2\n");
            exit(1);
        }

        close(fd[0]);
        close(fd[1]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    }

    return 0;
}
