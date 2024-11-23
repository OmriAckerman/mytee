#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


#define PROGRAM_NAME "tee";

#define HELP_OPTION "-h"
#define HELP_EX_OPTION "--help"

#define VERSION_OPTION "-v"
#define VERSION_EX_OPTION "--version"

#define APPEND_OPTION "-a"
#define APPEND_EX_OPTION "--append"

static bool append_flag = false;

int process_args(int argc, char* argv[], int** opened_fds, int* number_of_files)
{
    int arg_idx = 1;

    // parse options
    while (arg_idx < argc)
    {
        if (0 == strcmp(argv[arg_idx], HELP_OPTION) || 0 == strcmp(argv[arg_idx], HELP_EX_OPTION)) {
            printf("I am helping not you!!!\n");
            exit(EXIT_SUCCESS);
        } else if (0 == strcmp(argv[arg_idx], VERSION_OPTION) || 0 == strcmp(argv[arg_idx], VERSION_EX_OPTION)) {
            printf("I am the latest version...\n");
            exit(EXIT_SUCCESS);
        } else if (0 == strcmp(argv[arg_idx], APPEND_OPTION) || 0 == strcmp(argv[arg_idx], APPEND_EX_OPTION)) {
            append_flag = true;
        } else {
            // Done parsing options, this and following args should be files.
            break;
        }

        arg_idx++;
    }

    // Now open file fds
    int* fds = malloc(sizeof(int) * (*number_of_files));
    *opened_fds = fds;  
    *number_of_files = argc - arg_idx;
    
    int fd_idx = 0;
    int flags = append_flag ? (O_CREAT | O_WRONLY | O_APPEND) : (O_CREAT | O_WRONLY | O_TRUNC);

    while (arg_idx < argc)
    {
        int fd = open(argv[arg_idx], flags, 0644);
        if (fd == -1) {
            perror("open");
            return -1;
        }

        fds[fd_idx] = fd;

        arg_idx++;
        fd_idx++;
    }

    return 1;
}

void close_files(int* opened_fds, int number_of_files)
{
    int fd_idx;
    for(fd_idx = 0; fd_idx < number_of_files; fd_idx++)
    {
        close(opened_fds[fd_idx]);
    }

    free(opened_fds);
}

ssize_t mytee(int* files, int number_of_files)
{
    size_t buffer_size = 100;
    size_t bytes_read = 0;
    char* buffer = malloc(buffer_size);

    if (NULL == buffer) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int c = 0;

    while (c != EOF) {
        while((c = getchar()) != EOF)
        {
            if(bytes_read + 1 >= buffer_size)
            {
                buffer = realloc(buffer, buffer_size * 2);
                if (NULL == buffer) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
                buffer_size *= 2;
            }
            
            buffer[bytes_read++] = (char)c;
            
            if ('\n' == c) {
                break;
            } 
        }
     
        buffer[bytes_read] = '\0';

        write(STDOUT_FILENO, buffer, bytes_read);

        for (int fd_idx = 0; fd_idx < number_of_files; fd_idx++)
        {
            write(files[fd_idx], buffer, bytes_read);
        }

        bytes_read = 0;
    }

    free(buffer);
    return bytes_read;
}

int main(int argc, char* argv[])
{
    char* buffer = NULL;
    int* files = NULL;
    int number_of_files = 0;

    if (-1 != process_args(argc, argv, &files, &number_of_files))
    {
        mytee(files, number_of_files);
    }

    close_files(files, number_of_files);

    return EXIT_SUCCESS;
}