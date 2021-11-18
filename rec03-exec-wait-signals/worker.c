#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>

#define SHARED_FILE "shared.txt"

void write_to_file(char *msg) {
    int fd = open(SHARED_FILE, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd ==  -1) {
        perror("Failed creating shared file");
        exit(EXIT_FAILURE);
    }

    int bytes_to_write = strlen(msg);
    int bytes_written;
    while ((bytes_written = write(fd, msg, bytes_to_write)) > 0) {
        bytes_to_write -= bytes_written;
        msg += bytes_written;
    }
    if (bytes_written == -1) {
        perror("Failed writing to file");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    
    /*
        Do complex computation here
    */
    sleep(5);
    char *meaning_of_life = "42";

    write_to_file(meaning_of_life);

    if (kill(getppid(), SIGUSR1) == -1) {
        perror("Failed sending SIGUSR1");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}