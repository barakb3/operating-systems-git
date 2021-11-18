#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFF_SIZE 256

int main(int argc, char *argv[]) {
    int pipefds[2];
    if (pipe(pipefds) == -1) {
        perror("Failed creating pipe");
        exit(EXIT_FAILURE);
    }

    int readerfd = pipefds[0];
    int writerfd = pipefds[1];

    int pid = fork();
    if (pid == -1) {
        perror("Failed forking");
        close(readerfd);
        close(writerfd);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child
        close(readerfd); // close read side

        char *msg = "Hello from child!";
        int msg_len = strlen(msg);
        int bytes_written;
        while ((bytes_written = write(writerfd, msg, msg_len)) > 0) {
            msg += bytes_written;
            msg_len -= bytes_written;
        }
        if (bytes_written == -1) {
            perror("Failed writing to pipe");
            close(writerfd);
            exit(EXIT_FAILURE);
        }

        close(writerfd);
    } else {
        // Parent
        close(writerfd); // close write side

        char buff[BUFF_SIZE+1];
        int bytes_read;
        while ((bytes_read = read(readerfd, buff, BUFF_SIZE)) > 0) {
            buff[bytes_read] = '\0';
            printf("%s", buff);
        }
        if (bytes_read == -1) {
            perror("Failed reading from pipe");
            close(readerfd);
            exit(EXIT_FAILURE);
        }

        printf("\n");
        close(readerfd);
    }
}