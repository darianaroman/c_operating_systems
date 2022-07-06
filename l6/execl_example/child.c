#include <stdio.h>

int main(int argc, char **argv)
{
    int p;
    for (p = 0; p < argc; p++)
        printf("argv[%d]=%s\n", p, argv[p]);
}