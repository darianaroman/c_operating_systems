#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("usage: %s <file_name>\n", argv[0]);
        exit(1);
    }

    if (access(argv[1], 0) != 0)
    {
        perror("error! file not found");
        exit(1);
    }

    struct stat fileMetadata;
    
    if (stat(argv[2], &fileMetadata) < 0)
    {
        perror("error! getting info about the file");
        exit(1);
    }

    if (!S_ISDIR(fileMetadata.st_mode))
    {
        perror("error! second arg doesn't correspond to a directory");
        exit(1);
    }

    int fd1[2];
    int fd2[2];

    pipe(fd1);
    pipe(fd2);

    pid_t pid2;
    pid_t pid3;

    pid2 = fork();
    if (pid2 < 0)
    {
        perror("error! creating second process");
        exit(1);
    }

    return 0;
}