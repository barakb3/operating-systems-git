#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFF_SIZE 256

void syscall_error(char *error_msg) {
    //fprintf(stderr, "%s: %s\n",error_msg,  strerror(errno));
    perror(error_msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        syscall_error("Failed opening file");
    }

    char buff[BUFF_SIZE];
    int bytes_read;
    while ((bytes_read = read(fd, buff, BUFF_SIZE)) > 0) {
        if (write(STDOUT_FILENO, buff, bytes_read) == -1) {
            close(fd);
            syscall_error("Failed writing to stdout");
        }
    }
    if (bytes_read == -1) {
        close(fd);
        syscall_error("Failed reading from file");
    }

    close(fd);

    return EXIT_SUCCESS;
}