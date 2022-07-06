#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define __DEBUG

#ifdef __DEBUG // for debugging also displays the file, function and line in source code
               // where the error was found
void debug_info(const char *file, const char *function, const int line)
{
    fprintf(stderr, "DEBUG. ERROR PLACE: File=\"%s\", Function=\"%s\", Line=\"%d\"\n", file, function, line);
}

#define ERR_MSG(DBG_MSG)                              \
    {                                                 \
        perror(DBG_MSG);                              \
        debug_info(__FILE__, __FUNCTION__, __LINE__); \
    }

#else // with no __DEBUG just displays the error message

#define ERR_MSG(DBG_MSG) \
    {                    \
        perror(DBG_MSG); \
    }

#endif

// if char_buffer[100], line = char_buffer, max_length <= 100
// line_length <= actual size of the line
int get_line(int fd, char *line, int line_no, int max_length, int *line_length)
{
    int bytesRead;
    int countLines = 0;

    char *buf = (char *)malloc(max_length * sizeof(char));
    char **lines = (char **)malloc(max_length * max_length * sizeof(char));
    int countChars = 0;

    lines[countLines] = (char *)malloc(max_length * sizeof(char));
    while ((bytesRead = read(fd, buf, 1)) != 0)
    {
        if (bytesRead < 0)
        {
            ERR_MSG("ERROR (reading from inside the file)");
            exit(3);
        }
        lines[countLines][countChars] = buf[0];
        countChars++;

        if (buf[0] == '\n')
        {
            lines[countLines][countChars] = 0;
            if (countLines == line_no) {
                *line_length = countChars + 1;
            }
            countLines++;
            lines[countLines] = (char *)malloc(max_length * sizeof(char));
            countChars = 0;
        }
    }

    strcpy(line, lines[line_no]);
    free(lines);
    free(buf);
    return 1;
}

int reverse_lines(int fd, int max_length, int fd_output) {
    int bytesRead;
    int countLines = 0;

    char *buf = (char *)malloc(max_length * sizeof(char));
    char **lines = (char **)malloc(max_length * max_length * sizeof(char));
    int countChars = 0;

    lines[countLines] = (char *)malloc(max_length * sizeof(char));
    while ((bytesRead = read(fd, buf, 1)) != 0)
    {
        if (bytesRead < 0)
        {
            ERR_MSG("ERROR (reading from inside the file)");
            exit(5);
        }
        lines[countLines][countChars] = buf[0];
        countChars++;

        if (buf[0] == '\n')
        {
            lines[countLines][countChars] = 0;
            countLines++;
            lines[countLines] = (char *)malloc(max_length * sizeof(char));
            countChars = 0;
        }
    }

    for (int i = countLines - 1; i >= 0; i--)
    {
        if (write(fd_output, lines[i], sizeof(lines[i])) < 0) {
            ERR_MSG("ERROR! (writing the file");
            exit(6);
        }
    }
    
    free(lines);
    free(buf);

    return 1;
}

int main(int argc, char **argv)
{
    int fd;
    int fd_output;
    int max_length = 100;
    int line_length;
    char *line = (char*) malloc (max_length * sizeof(char));
    int line_no;

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <file_name> <line_no> <file_name_destinantion>\n", argv[0]);
        exit(1);
    }

    if ((fd = open(argv[1], O_RDONLY)) < 0)
    {
        ERR_MSG("ERROR (opening the file)");
        exit(2);
    }

    sscanf(argv[2], "%d", &line_no);
    lseek(fd, 0, SEEK_SET);
    if (get_line(fd, line, line_no, max_length, &line_length)) {
        printf("line = %s\n", line);
        printf("line length = %d\n", line_length);
    }

    if ((fd_output = open(argv[3], O_WRONLY)) < 0)
    {
        ERR_MSG("ERROR (opening the destination file)");
        exit(4);
    }
    lseek(fd, 0, SEEK_SET);
    reverse_lines(fd, max_length, fd_output);

    return 0;
}