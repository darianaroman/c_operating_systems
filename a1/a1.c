#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_PATH_LEN 512

int unpackValue(unsigned char *string, int size)
{
    int value = 0;
    for (int i = size - 1; i >= 0; i--)
    {
        value = value << 8;
        value = value + string[i];
    }
    return value;
}

// returns 1 if the file is of SF format
int parse(char *path, int display, int section, int *get_offset, int *get_size, int *get_nr_sections)
{
    int fd;
    int ok = 0;
    if ((fd = open(path, O_RDONLY)) < 0)
    {
        printf("ERROR! parse:(opening the file)\n");
    }
    lseek(fd, -2, SEEK_END);
    char *magic = (char *)malloc(3 * sizeof(char));
    if (read(fd, magic, 2) <= 0)
    {
        printf("ERROR! parse:(reading magic)\n");
    }
    magic[2] = 0;
    if (strcmp(magic, "JL") == 0)
    {
        // magic OK
        lseek(fd, -4, SEEK_END);
        unsigned char *h_size = (unsigned char *)malloc(3 * sizeof(unsigned char));
        if (read(fd, h_size, 2) <= 0)
        {
            printf("ERROR! parse:(reading header size)\n");
        }
        h_size[2]= 0;
        // unsigned int h_size_u = h_size[1];
        // h_size_u = (h_size_u << 8) + h_size[0];
        // int h_size_int = (int)h_size_u;
        int h_size_int = unpackValue(h_size, 2);

        lseek(fd, -h_size_int, SEEK_END);
        unsigned char *version = (unsigned char *)malloc(5 * sizeof(unsigned char));
        if (read(fd, version, 4) <= 0)
        {
            printf("ERROR! parse: reading version\n");
        }
        version[4] = 0;
        int version_int = 0;
        // for (int j = 3; j >= 0; j--)
        // {
        //     version_int = version_int << 8;
        //     version_int = version_int + version[j];
        // }
        version_int = unpackValue(version, 4);
        if (version_int >= 29 && version_int <= 144)
        {
            unsigned char *nr_sect = (unsigned char *)malloc(2 * sizeof(unsigned char));
            if (read(fd, nr_sect, 1) <= 0)
            {
                printf("ERROR! parse: reading nr sections\n");
            }
            nr_sect[1] = 0;
            int nr_sect_int = (int)*nr_sect;
            if (nr_sect_int >= 6 && nr_sect_int <= 19)
            {
                unsigned char **name = (unsigned char **)malloc(nr_sect_int * sizeof(unsigned char *));
                unsigned char *type = (unsigned char *)malloc(nr_sect_int * sizeof(unsigned char));
                unsigned char **offset = (unsigned char **)malloc(nr_sect_int * sizeof(unsigned char *));
                unsigned char **size = (unsigned char **)malloc(nr_sect_int * sizeof(unsigned char *));
                ok = 1;
                for (int i = 0; i < nr_sect_int; i++)
                {
                    name[i] = (unsigned char *)malloc(7 * sizeof(unsigned char));
                    offset[i] = (unsigned char *)malloc(5 * sizeof(unsigned char));
                    size[i] = (unsigned char *)malloc(5 * sizeof(unsigned char));
                    read(fd, name[i], 6);
                    name[i][6] = 0;
                    read(fd, &type[i], 1);
                    read(fd, offset[i], 4);
                    offset[i][4] = 4;
                    read(fd, size[i], 4);
                    size[i][4] = 0;
                    int type_int = (int)type[i];
                    if (type_int != 31 && type_int != 59)
                    {
                        ok = 0;
                    }
                }
                if (ok)
                {
                    unsigned int size_i[nr_sect_int];
                    unsigned int offset_i[nr_sect_int];
                    for (int i = 0; i < nr_sect_int; i++)
                    {
                        size_i[i] = 0;
                        offset_i[i] = 0;
                        // for (int j = 3; j >= 0; j--)
                        // {
                        //     size_i[i] = size_i[i] << 8;
                        //     size_i[i] = size_i[i] + size[i][j];
                        //     offset_i[i] = offset_i[i] << 8;
                        //     offset_i[i] = offset_i[i] + offset[i][j];
                        // }
                        size_i[i] = unpackValue(size[i], 4);
                        offset_i[i] = unpackValue(offset[i], 4);
                    }
                    // getting the offset for section
                    if (section != -1)
                    {
                        if (get_offset != NULL && get_size != NULL)
                        {
                            *get_offset = (int)offset_i[section - 1];
                            *get_size = (int)size_i[section - 1];
                            *get_nr_sections = nr_sect_int;
                        }
                        if (section == 0 && get_nr_sections != NULL)
                        {
                            int count_sections = 0;
                            char **get_section = (char **)malloc(nr_sect_int * sizeof(char *));
                            for (int i = 0; i < nr_sect_int; i++)
                            {
                                int count_lines = 0;
                                get_section[i] = (char *)malloc(size_i[i] * sizeof(char));
                                //
                                if (read(fd, get_section[i], size_i[i]) > 0)
                                {
                                    get_section[i][size_i[i]] = 0;
                                    char *get_line = NULL;
                                    char *delim = "\n";
                                    get_line = strtok(get_section[i], delim);
                                    count_lines++;
                                    while (get_line != NULL)
                                    {
                                        get_line = strtok(NULL, delim);
                                        count_lines++;
                                    }
                                }
                                //
                                free(get_section[i]);
                                if (count_lines == 14)
                                {
                                    count_sections++;
                                }
                            }
                            free(get_section);
                            *get_nr_sections = count_sections;
                        }
                    }
                    if (display)
                    {
                        printf("SUCCESS\n");
                        printf("version=%d\n", version_int);
                        printf("nr_sections=%d\n", nr_sect_int);
                        for (int i = 0; i < nr_sect_int; i++)
                        {
                            int type_i = (int)type[i];
                            printf("section%d: %s %d %u\n", i + 1, name[i], type_i, size_i[i]);
                        }
                    }
                }
                else if (display)
                {
                    printf("ERROR\nwrong sect_types\n");
                }

                for (int i = 0; i < nr_sect_int; i++)
                {
                    free(name[i]);
                    free(size[i]);
                    free(offset[i]);
                }

                free(name);
                free(type);
                free(offset);
                free(size);
            }
            else if (display)
            {
                printf("ERROR\nwrong sect_nr\n");
            }
            free(nr_sect);
        }
        else if (display)
        {
            printf("ERROR\nwrong version\n");
        }
        free(version);
        free(h_size);
    }
    else if (display)
    {
        printf("ERROR\nwrong magic\n");
    }
    free(magic);
    close(fd);
    return ok;
}

void extract(char *path, int section, int line)
{
    int fd = 0;
    int offset = 0;
    int size = 0;
    int nr_sections = 0;
    if (parse(path, 0, section, &offset, &size, &nr_sections))
    {
        if (section <= nr_sections) {
            if ((fd = open(path, O_RDONLY)) < 0)
            {
                printf("ERROR! extract: opening the file\n");
                exit(1);
            }
            char *get_section = (char *)malloc(size * sizeof(char));
            if (lseek(fd, offset, SEEK_SET) != offset)
            {
                printf("ERROR! extract: lseek\n");
                exit(1);
            }
            if (read(fd, get_section, size) > 0)
            {
                get_section[size] = 0;
                char *reversed_section = (char *)malloc(size * sizeof(char));
                for (int i = 0; i < size; i++)
                {
                    reversed_section[i] = get_section[size - i - 1];
                }
                reversed_section[size] = 0;
                char *get_line = NULL;
                char *delim = "\n";
                get_line = strtok(reversed_section, delim);
                for (int i = 0; i < line - 1; i++)
                {
                    get_line = strtok(NULL, delim);
                }
                if (get_line != NULL)
                {
                    printf("SUCCESS\n%s", get_line);
                }
                else
                {
                    printf("ERROR\ninvalid line\n");
                }
                free(get_section);
                free(reversed_section);
            }
        }
        else
        {
            printf("ERROR\ninvalid section");
        }
    }
    else
    {
        printf("ERROR\ninvalid file");
    }
    close(fd);
}

int list(char *path, int recursive, int option, int size, char *str)
{
    struct stat path_stat;
    if (stat(path, &path_stat) == -1)
    {
        return 0;
    }

    DIR *dir;
    struct dirent *dir_entry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    dir = opendir(path);

    while ((dir_entry = readdir(dir)) != 0)
    {
        snprintf(name, MAX_PATH_LEN, "%s/%s", path, dir_entry->d_name);
        if (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)
        {
            lstat(name, &inode);
            if (!option || (option == 1 && inode.st_size > size) || (option == 2 && !strncmp(dir_entry->d_name, str, strlen(str))))
            {
                printf("%s\n", name);
            }
            else if (option == 3)
            {
                if (S_ISREG(inode.st_mode))
                {
                    int nr_sections = 0;
                    parse(name, 0, 0, NULL, NULL, &nr_sections);
                    if (nr_sections >= 6)
                    {
                        printf("%s\n", name);
                    }
                }
            }

            if (recursive)
            {
                if (S_ISDIR(inode.st_mode))
                {
                    list(name, recursive, option, size, str);
                }
            }
        }
    }
    closedir(dir);
    return 1;
}

int findall(char *path)
{
    struct stat dir_stat;
    if (stat(path, &dir_stat) == 0)
    {
        printf("SUCCESS\n");
        list(path, 1, 3, 0, NULL);
    }
    else
    {
        printf("ERROR\ninvalid directory path\n");
    }
    return 0;
}

int main(int argc, char **argv)
{
    char *path = NULL;
    if (argc >= 2)
    {
        // command variant
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("91150\n");
        }
        // command list
        else if (strcmp(argv[1], "list") == 0)
        {
            char *str = NULL;
            char *size_char = NULL;
            int size = -1;
            int recursive = 0;
            int option = 0;
            for (int i = 2; i < argc; i++)
            {
                if (strncmp(argv[i], "path=", 5) == 0)
                {
                    path = strtok(argv[i], "=");
                    path = strtok(NULL, " ");
                }
                else if (strcmp(argv[i], "recursive") == 0)
                {
                    recursive = 1;
                }
                else if (strncmp(argv[i], "size_greater=", 13) == 0)
                {
                    size_char = strtok(argv[i], "=");
                    size_char = strtok(NULL, " ");
                    sscanf(size_char, "%d", &size);
                    option = 1;
                }
                else if (strncmp(argv[i], "name_starts_with=", 17) == 0)
                {
                    str = strtok(argv[i], "=");
                    str = strtok(NULL, " ");
                    option = 2;
                }
            }
            if (list(path, recursive, option, size, str))
            {
                printf("SUCCESS\n");
            }
            else
            {
                printf("ERROR!\ninvalid directory path\n");
            }
        }
        // command parse
        else if (strcmp(argv[1], "parse") == 0)
        {
            if (strncmp(argv[2], "path=", 5) == 0)
            {
                path = strtok(argv[2], "=");
                path = strtok(NULL, " ");
                parse(path, 1, -1, NULL, NULL, NULL);
            }
        }
        // comand extract
        else if (strcmp(argv[1], "extract") == 0)
        {
            char *section = NULL;
            char *line = NULL;
            char *path = NULL;
            int section_int = -1;
            int line_int = -1;
            for (int i = 2; i < argc; i++)
            {
                if (strncmp(argv[i], "path=", 5) == 0)
                {
                    path = strtok(argv[i], "=");
                    path = strtok(NULL, " ");
                }
                else if (strncmp(argv[i], "section=", 8) == 0)
                {
                    section = strtok(argv[i], "=");
                    section = strtok(NULL, " ");
                    sscanf(section, "%d", &section_int);
                }
                else if (strncmp(argv[i], "line=", 5) == 0)
                {
                    line = strtok(argv[i], "=");
                    line = strtok(NULL, " ");
                    sscanf(line, "%d", &line_int);
                }
            }
            extract(path, section_int, line_int);
        }
        // command findall
        else if (strcmp(argv[1], "findall") == 0)
        {
            if (strncmp(argv[2], "path=", 5) == 0)
            {
                path = strtok(argv[2], "=");
                path = strtok(NULL, " ");
                findall(path);
            }
        }
    }
    else
    {
        fprintf(stderr, "USAGE: %s [OPTIONS] [PARAMETERS]\n", argv[0]);
        exit(1);
    }

    return 0;
}
