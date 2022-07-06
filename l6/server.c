#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <string.h>

int main(int argc, char **argv)
{
    int a, b;
    char sign;
    sscanf(argv[1], "%d", &a);
    sscanf(argv[3], "%d", &b);
    strncpy(&sign, argv[2], 1);

    printf("in server %d %c %d\n", a, sign, b);

    if (sign == '+') {
        exit(a + b);
    }
    else if (sign == '-') {
        exit (a - b);
    }
    else {
        exit(0);
    }
}
