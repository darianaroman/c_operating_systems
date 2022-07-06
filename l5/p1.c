#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_LEN 512
#define MAX_PATH_LEN_DOUBLE 1024

#define __DEBUG

#ifdef __DEBUG
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

//If the found element corresponds to a file, calls another function
//"int copy_file(char *src, char *dst)" that copies the found file in
//another directory, under the same name, but ended with ".n" extension
//(n is the n-th occurrence of that name). The destination directory
//should be also specified as a command line parameter. Adapt the
//functions required at points 1 and 2 to also consider this new parameter.
int copy_file(char *src, char *dst, int *n, char *file_name)
{
    int from, to, nr, nw;
    char buf[MAX_PATH_LEN];

    printf("in copy-file: src name = %s == ", src);
    printf("nr = %d\n", *n);
    if ( (from = open(src, O_RDONLY)) < 0 ) {
        ERR_MSG("ERROR! copy: opening src file");
        exit(5);
    }

    snprintf(buf, MAX_PATH_LEN, "%s/%s.%d", dst, file_name, *n);
    printf(" === dest name: %s\n", buf);

    if ( (to = creat(buf, 0666)) < 0 ) {
        ERR_MSG("ERROR! copy: creating the file");
        exit(6);
    }

    while ( (nr = read(from, buf, sizeof(buf))) != 0 ) {
        if (nr < 0) {
            ERR_MSG("ERROR! copy: reading");
            exit(7);
        }

        if ( (nw = write(to, &buf, nr)) < 0 ) {
            ERR_MSG("ERROR! copy: writing");
            exit(8);
        }
    }
    close(from);
    close(to);
    return 1;
}

//If the found path corresponds to a directory, calls another
//"int copy_dir(char *src, char *dst)" function that copies
//that directory in another directory, under the same name,
//but ended with ".n" extension (n is the n-th occurrence of
//that name). The destination directory is the same one as that
//at point 5
int copy_dir(char *src, char *dst, int *n, char *dir_name)
{
    struct stat st;
    char buf[MAX_PATH_LEN];

    printf("in copy-file: src name = %s == ", src);
    printf("nr = %d\n", *n);

    snprintf(buf, MAX_PATH_LEN, "%s/%s.%d", dst, dir_name, *n);

    if ( stat(buf, &st) == -1 ) {
        mkdir(buf, 0700);
    }

    DIR *dir;
    struct dirent *dir_entry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    dir = opendir(buf);
    if (dir == 0)
    {
        printf("ERROR! copy_dir: (opening the directory)");
        exit(4);
    }

    while ((dir_entry = readdir(dir)) != 0)
    {
        snprintf(name, MAX_PATH_LEN_DOUBLE, "%s/%s", buf, dir_entry->d_name);
        lstat(name, &inode);

        if (S_ISDIR(inode.st_mode) && strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)
        {
            copy_dir(name, buf, n, dir_entry->d_name);
        }
        else if (S_ISREG(inode.st_mode))
        {
            copy_file(name, buf, n, dir_entry->d_name);
        }
    }
    closedir(dir);
    return 1;
}

// Calls a function "int search_dir(char *dir_name, char *searched_name)" to check if there is
//an element with the searched name in the given directory (not entering subdirectories).
//If such an element exists, then its absolute path and type (file, directory, symbolic link)
//must be displayed on the screen.
int search_dir(char *dir_name, char *searched_name, char *dst_directory, int *n)
{
    DIR *dir;
    struct dirent *dir_entry;
    struct stat inode;
    char name[MAX_PATH_LEN];
    int found = 0;

    dir = opendir(dir_name);
    if (dir == 0) {
        printf("ERROR! (opening the directory)");
        exit(4);
    }

    while ( (dir_entry = readdir(dir)) != 0 ) {

        if (!strcmp(searched_name, dir_entry->d_name)) {
            (*n)++;
            found = 1;
            snprintf(name, MAX_PATH_LEN, "%s/%s", dir_name, dir_entry->d_name);
            lstat(name, &inode);

            if (S_ISDIR(inode.st_mode)) {
                printf("dir ");
                copy_dir(name, dst_directory, n, dir_entry->d_name);
            }
            else if (S_ISREG(inode.st_mode)) {
                printf("file ");
                copy_file(name, dst_directory, n, dir_entry->d_name);
            }
            else if (S_ISLNK(inode.st_mode)) {
                printf("lnk ");
            }
            printf("%s\n", name);
        }
    }
    closedir(dir);
    return found;
}

//Calls a recursive function "int search_tree(char *dir_name, char *searched_name)"
//to search for a given name in the entire tree having the given directory as root.
//The function search_tree should use function search_dir.
int search_tree(char *dir_name, char *searched_name, char *dst_directory, int *n)
{
    DIR *dir;
    struct dirent *dir_entry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    dir = opendir(dir_name);
    if (dir == 0) {
        printf("ERROR! (opening directory in tree");
        exit(5);
    }

    search_dir(dir_name, searched_name, dst_directory, n);
    while ( (dir_entry = readdir(dir)) != 0 ) {
        snprintf(name, MAX_PATH_LEN, "%s/%s", dir_name, dir_entry->d_name);
        lstat(name, &inode);

        if (S_ISDIR(inode.st_mode)) {
            if (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
                search_tree(name, searched_name, dst_directory, n);
            }
        }
    }
    closedir(dir);
    return 1;
}

int main(int argc, char **argv)
{
    int n = 0;
    if (argc != 4) {
        printf("USAGE: %s <starting_directory> <file_name> <dst_directory>\n", argv[0]);
        exit(1);
    }

    struct stat fileMetadata;
    if (stat(argv[1], &fileMetadata) < 0) {
        printf("ERROR! (getting info about the file)");
        exit(2);
    }

    if (S_ISDIR(fileMetadata.st_mode)) {
        printf("tree search:\n");
        search_tree(argv[1], argv[2], argv[3], &n);
    }
    else {
        printf("%s is not a directory!\n", argv[1]);
        exit(3);
    }

    return 0;
}
