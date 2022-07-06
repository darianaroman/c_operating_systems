#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>

#define WRITE_FIFO "RESP_PIPE_91150"
#define READ_FIFO "REQ_PIPE_91150"
#define SHM "/GLRjL4Kv"

#define ERROR "ERROR"
#define SUCCESS "SUCCESS"
#define CONNECT "CONNECT"
#define PING "PING"
#define PONG "PONG"
#define CREATE_SHM "CREATE_SHM"
#define WRITE_TO_SHM "WRITE_TO_SHM"
#define MAP_FILE_STRING "MAP_FILE"
#define READ_FROM_FILE_OFFSET "READ_FROM_FILE_OFFSET"
#define READ_FROM_FILE_SECTION "READ_FROM_FILE_SECTION"
#define READ_FROM_LOGICAL_SPACE_OFFSET "READ_FROM_LOGICAL_SPACE_OFFSET"

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

char *createMessage(char *string, int *final_size)
{
    int size = strlen(string);
    char *message = (char *)malloc((size + 1) * sizeof(char));
    message[0] = size;
    for (int i = 1; i <= size; i++)
    {
        message[i] = string[i - 1];
    }
    *final_size = size + 1;
    return message;
}

int main()
{
    int write_fifo;
    int read_fifo;

    // shm data
    unsigned int shm_size = 0;
    int shm_fd;
    char *shared_data = NULL;

    // write to shm data
    unsigned int offset = 0;
    unsigned int value = 0;

    // map file data
    int map_file_fd = 0;
    int map_file_sz = 0;
    unsigned char *data = NULL;

    // read from file offset data
    int read_offset = 0;
    int no_of_bytes = 0;

    // read from file section data
    int no_section = 0;
    int offset_section = 0;
    int no_of_bytes_section = 0;

    int header_size = 0;
    int header_offset = 0;
    int number_sections = 0;
    int offsets_section[100];

    // read from logical space offset
    int logical_offset = 0;
    int logical_no_of_bytes = 0;
    int logical_offsets[100];
    int sizes_section[100];

    char *error = NULL;
    int error_size = 0;
    error = createMessage(ERROR, &error_size);

    char *success = NULL;
    int success_size = 0;
    success = createMessage(SUCCESS, &success_size);

    if (access(WRITE_FIFO, 0) == 0)
    {
        unlink(WRITE_FIFO);
    }

    if (mkfifo(WRITE_FIFO, 0600) != 0)
    {
        perror("ERROR\ncannot create the response pipe");
        return 1;
    }

    read_fifo = open(READ_FIFO, O_RDONLY);
    if (read_fifo == -1)
    {
        unlink(WRITE_FIFO);
        perror("ERROR\ncannot open the request pipe");
        return 1;
    }

    write_fifo = open(WRITE_FIFO, O_WRONLY);
    if (write_fifo == -1)
    {
        unlink(WRITE_FIFO);
        perror("ERROR\ncannot open the response pipe");
        return 1;
    }

    char *connect = NULL;
    int connect_size = 0;
    connect = createMessage(CONNECT, &connect_size);

    if (write(write_fifo, connect, connect_size * sizeof(char)) != connect_size * sizeof(char))
    {
        perror("ERROR\ncannot write into pipe");
        return 1;
    }

    free(connect);

    printf("SUCCESS\n");

    int run = 1;

    while (run)
    {
        char size = 0;
        if (read(read_fifo, &size, sizeof(char)) != sizeof(char))
        {
            perror("ERROR\ncannot read size from pipe");
            return 1;
        }
        int size_int = (int)size;
        char buffer[100];
        if (read(read_fifo, buffer, size_int * sizeof(char)) != size_int * sizeof(char))
        {
            perror("ERROR\ncannot read content from pipe");
            return 1;
        }
        if (strncmp(buffer, PING, 4) == 0)
        {
            char *ping = NULL;
            int ping_size = 0;
            ping = createMessage(PING, &ping_size);

            char *pong = NULL;
            int pong_size = 0;
            pong = createMessage(PONG, &pong_size);

            unsigned int variant = 91150;

            if (write(write_fifo, ping, ping_size * sizeof(char)) != ping_size * sizeof(char))
            {
                perror("ERROR\ncannot write into pipe");
                return 1;
            }
            free(ping);

            if (write(write_fifo, pong, pong_size * sizeof(char)) != pong_size * sizeof(char))
            {
                perror("ERROR\ncannot write into pipe");
                return 1;
            }
            free(pong);

            if (write(write_fifo, &variant, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot write into pipe");
                return 1;
            }
        }
        else if (strncmp(buffer, CREATE_SHM, size_int) == 0)
        {
            char *create_shm;
            int create_shm_size = 0;
            create_shm = createMessage(CREATE_SHM, &create_shm_size);

            if (read(read_fifo, &shm_size, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read shared memory size from pipe");
                return 1;
            }

            shm_fd = shm_open(SHM, O_CREAT | O_RDWR, 0644);
            if (shm_fd < 0)
            {
                perror("ERROR\ncannot create shared memory");
            }
            ftruncate(shm_fd, shm_size);

            shared_data = (char *)mmap(0, shm_size * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

            if (write(write_fifo, create_shm, create_shm_size * sizeof(char)) != create_shm_size * sizeof(char))
            {
                perror("ERROR\ncannot write error into pipe");
                return 1;
            }
            free(create_shm);

            if (shared_data == MAP_FAILED)
            {
                if (write(write_fifo, error, error_size * sizeof(char)) != error_size * sizeof(char))
                {
                    perror("ERROR\ncannot write error into pipe");
                    return 1;
                }
            }
            else
            {
                if (write(write_fifo, success, success_size * sizeof(char)) != success_size * sizeof(char))
                {
                    perror("ERROR\ncannot write success into pipe");
                    return 1;
                }
            }
            close(shm_fd);
        }
        else if (strncmp(buffer, WRITE_TO_SHM, size_int) == 0)
        {
            char *write_to_shm = NULL;
            int write_to_shm_size = 0;
            write_to_shm = createMessage(WRITE_TO_SHM, &write_to_shm_size);

            if (read(read_fifo, &offset, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read offset from pipe");
                return 1;
            }

            if (read(read_fifo, &value, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read value from pipe");
                return 1;
            }

            if (write(write_fifo, write_to_shm, write_to_shm_size * sizeof(char)) != write_to_shm_size * sizeof(char))
            {
                perror("ERROR\ncannot write \"WRITE_TO_SHM\" into pipe");
                return 1;
            }
            free(write_to_shm);

            if (offset < 0 || offset > shm_size - 4)
            {
                if (write(write_fifo, error, error_size * sizeof(char)) != error_size * sizeof(char))
                {
                    perror("ERROR\ncannot write error into pipe");
                    return 1;
                }
            }
            else
            {
                char value_char[4];
                value_char[3] = value >> 24;
                value_char[2] = value >> 16;
                value_char[1] = value >> 8;
                value_char[0] = value;
                for (int i = 0; i < 4; i++)
                {
                    shared_data[offset + i] = value_char[i];
                }
                if (write(write_fifo, success, success_size * sizeof(char)) != success_size * sizeof(char))
                {
                    perror("ERROR\ncannot write success into pipe");
                    return 1;
                }
            }
            // munmap(shared_data, shm_size * sizeof(char));
            // shared_data = NULL;
            // shm_unlink(SHM);
        }
        else if (strncmp(buffer, MAP_FILE_STRING, size_int) == 0)
        {
            char *map_file = NULL;
            int map_file_size = 0;
            map_file = createMessage(MAP_FILE_STRING, &map_file_size);

            char file_name_size_char = 0;
            int file_name_size;
            char file_name[50];

            if (read(read_fifo, &file_name_size_char, sizeof(char)) != sizeof(char))
            {
                perror("ERROR\ncannot read value from pipe");
                return 1;
            }
            file_name_size = (int)file_name_size_char;

            if (read(read_fifo, file_name, file_name_size * sizeof(char)) != file_name_size * sizeof(char))
            {
                perror("ERROR\ncannot read value from pipe");
                return 1;
            }
            file_name[file_name_size] = 0;

            if (write(write_fifo, map_file, map_file_size * sizeof(char)) != map_file_size * sizeof(char))
            {
                perror("ERROR\ncannot write error into pipe");
                return 1;
            }
            free(map_file);

            map_file_fd = open(file_name, O_RDONLY);
            if (map_file_fd == -1)
            {
                if (write(write_fifo, error, error_size * sizeof(char)) != error_size * sizeof(char))
                {
                    perror("ERROR\ncannot write error into pipe");
                    return 1;
                }
            }
            else
            {
                map_file_sz = lseek(map_file_fd, 0, SEEK_END);
                lseek(map_file_fd, 0, SEEK_SET);
                data = (unsigned char *)mmap(NULL, map_file_sz * sizeof(unsigned char), PROT_READ, MAP_SHARED, map_file_fd, 0);
                if (data == MAP_FAILED)
                {
                    if (write(write_fifo, error, error_size * sizeof(char)) != error_size * sizeof(char))
                    {
                        perror("ERROR\ncannot write error into pipe");
                        return 1;
                    }
                }
                else
                {
                    close(map_file_fd);
                    if (write(write_fifo, success, success_size * sizeof(char)) != success_size * sizeof(char))
                    {
                        perror("ERROR\ncannot write success into pipe");
                        return 1;
                    }
                    // munmap(data, map_file_sz * sizeof(char));
                }
            }
        }
        else if (strncmp(buffer, READ_FROM_FILE_OFFSET, size_int) == 0)
        {
            char *read_from_file_offset = NULL;
            int read_from_file_offset_size = 0;
            read_from_file_offset = createMessage(READ_FROM_FILE_OFFSET, &read_from_file_offset_size);

            if (read(read_fifo, &read_offset, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read shared memory size from pipe");
                return 1;
            }

            if (read(read_fifo, &no_of_bytes, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read shared memory size from pipe");
                return 1;
            }

            if (write(write_fifo, read_from_file_offset, read_from_file_offset_size * sizeof(char)) != read_from_file_offset_size * sizeof(char))
            {
                perror("ERROR\ncannot write error into pipe");
                return 1;
            }
            free(read_from_file_offset);

            if (data == NULL ||
                shared_data == NULL ||
                read_offset + no_of_bytes >= map_file_sz)
            {
                if (write(write_fifo, error, error_size * sizeof(char)) != error_size * sizeof(char))
                {
                    perror("ERROR\ncannot write error into pipe");
                    return 1;
                }
            }
            else
            {
                for (int i = 0; i < no_of_bytes; i++)
                {
                    shared_data[i] = data[read_offset + i];
                }
                if (write(write_fifo, success, success_size * sizeof(char)) != success_size * sizeof(char))
                {
                    perror("ERROR\ncannot write success into pipe");
                    return 1;
                }
            }
        }
        else if (strncmp(buffer, READ_FROM_FILE_SECTION, size_int) == 0)
        {
            char *read_from_file_section = NULL;
            int read_from_file_section_size = 0;
            read_from_file_section = createMessage(READ_FROM_FILE_SECTION, &read_from_file_section_size);

            if (read(read_fifo, &no_section, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read section number from pipe");
                return 1;
            }

            if (read(read_fifo, &offset_section, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read offset section from pipe");
                return 1;
            }

            if (read(read_fifo, &no_of_bytes_section, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read no of bytes section from pipe");
                return 1;
            }

            if (write(write_fifo, read_from_file_section, read_from_file_section_size * sizeof(char)) != read_from_file_section_size * sizeof(char))
            {
                perror("ERROR\ncannot write READ_FROM_FILE_SECTION into pipe");
                return 1;
            }
            free(read_from_file_section);

            unsigned char header_size_char[2];
            header_size_char[0] = data[map_file_sz - 4];
            header_size_char[1] = data[map_file_sz - 3];
            header_size_char[2] = 0;
            header_size = unpackValue(header_size_char, 2);

            header_offset = map_file_sz - header_size;

            number_sections = (int)data[header_offset + 4];

            for (int i = 0; i < number_sections; i++)
            {
                unsigned char offset_aux[4];
                offset_aux[0] = data[header_offset + 5 + 15 * i + 7];
                offset_aux[1] = data[header_offset + 5 + 15 * i + 7 + 1];
                offset_aux[2] = data[header_offset + 5 + 15 * i + 7 + 2];
                offset_aux[3] = data[header_offset + 5 + 15 * i + 7 + 3];
                offset_aux[4] = 0;
                offsets_section[i] = unpackValue(offset_aux, 4);
            }

            if (no_section > number_sections)
            {
                if (write(write_fifo, error, error_size * sizeof(char)) != error_size * sizeof(char))
                {
                    perror("ERROR\ncannot write error into pipe");
                    return 1;
                }
            }
            else
            {
                for (int i = 0; i < no_of_bytes_section; i++)
                {
                    shared_data[i] = data[offsets_section[no_section - 1] + offset_section + i];
                }
                if (write(write_fifo, success, success_size * sizeof(char)) != success_size * sizeof(char))
                {
                    perror("ERROR\ncannot write success into pipe");
                    return 1;
                }
            }
        }
        else if (strncmp(buffer, READ_FROM_LOGICAL_SPACE_OFFSET, size_int) == 0)
        {
            char *read_from_logical_space_offset = NULL;
            int read_from_logical_space_offset_size = 0;
            read_from_logical_space_offset = createMessage(READ_FROM_LOGICAL_SPACE_OFFSET, &read_from_logical_space_offset_size);

            if (read(read_fifo, &logical_offset, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read logical offset from pipe");
                return 1;
            }

            if (read(read_fifo, &logical_no_of_bytes, sizeof(unsigned int)) != sizeof(unsigned int))
            {
                perror("ERROR\ncannot read logical no of bytes from pipe");
                return 1;
            }

            if (write(write_fifo, read_from_logical_space_offset, read_from_logical_space_offset_size * sizeof(char)) != read_from_logical_space_offset_size * sizeof(char))
            {
                perror("ERROR\ncannot write READ_FROM_LOGICAL_SPACE_OFFSET into pipe");
                return 1;
            }
            free(read_from_logical_space_offset);

            unsigned char header_size_char[2];
            header_size_char[0] = data[map_file_sz - 4];
            header_size_char[1] = data[map_file_sz - 3];
            header_size_char[2] = 0;
            header_size = unpackValue(header_size_char, 2);

            header_offset = map_file_sz - header_size;

            number_sections = (int)data[header_offset + 4];

            for (int i = 0; i < number_sections; i++)
            {
                unsigned char offset_aux[4];
                offset_aux[0] = data[header_offset + 5 + 15 * i + 7];
                offset_aux[1] = data[header_offset + 5 + 15 * i + 7 + 1];
                offset_aux[2] = data[header_offset + 5 + 15 * i + 7 + 2];
                offset_aux[3] = data[header_offset + 5 + 15 * i + 7 + 3];
                offset_aux[4] = 0;
                offsets_section[i] = unpackValue(offset_aux, 4);

                unsigned char size_aux[4];
                size_aux[0] = data[header_offset + 5 + 15 * i + 11];
                size_aux[1] = data[header_offset + 5 + 15 * i + 11 + 1];
                size_aux[2] = data[header_offset + 5 + 15 * i + 11 + 2];
                size_aux[3] = data[header_offset + 5 + 15 * i + 11 + 3];
                size_aux[4] = 0;
                sizes_section[i] = unpackValue(size_aux, 4);
            }

            logical_offsets[0] = 0;

            for (int i = 1; i < number_sections; i++)
            {
                int aux = (sizes_section[i - 1] / 3072);
                logical_offsets[i] = logical_offsets[i - 1] + aux * 3072;
                if (sizes_section[i - 1] % 3072 > 0)
                {
                    logical_offsets[i] += 3072;
                }
            }

            int aux_logical_offset = 0;
            no_section = 0;
            while (logical_offset > aux_logical_offset && no_section < number_sections)
            {
                no_section++;
                aux_logical_offset = logical_offsets[no_section];
            }
            no_section--;

            int normalized_offset = offsets_section[no_section] + logical_offset - logical_offsets[no_section];

            if (logical_offset - logical_offsets[no_section] >= sizes_section[no_section])
            {
                if (write(write_fifo, error, error_size * sizeof(char)) != error_size * sizeof(char))
                {
                    perror("ERROR\ncannot write error into pipe");
                    return 1;
                }
            }
            else
            {
                for (int i = 0; i < logical_no_of_bytes; i++)
                {
                    shared_data[i] = data[normalized_offset + i];
                }

                if (write(write_fifo, success, success_size * sizeof(char)) != success_size * sizeof(char))
                {
                    perror("ERROR\ncannot write success into pipe");
                    return 1;
                }
            }
        }
        else if (strncmp(buffer, "EXIT", 4) == 0)
        {
            run = 0;
        }
    }

    free(error);
    free(success);

    close(write_fifo);
    close(read_fifo);

    unlink(WRITE_FIFO);

    return 0;
}
