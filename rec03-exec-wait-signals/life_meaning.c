#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>

#define SHARED_FILE "shared.txt"
#define BUFF_SIZE 256

// Global Variables
int keep_waiting = 1;

void read_from_file() {
    int fd = open(SHARED_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Failed opening file");
        exit(EXIT_FAILURE);
    }

    char buff[BUFF_SIZE+1];
    int bytes_read;
    while ((bytes_read = read(fd, buff, BUFF_SIZE)) > 0) {
        buff[bytes_read] = '\0';
        printf("%s", buff);
    }
    if (bytes_read == -1) {
        perror("Failed reading from file");
        exit(EXIT_FAILURE);
    }
}

void my_signal_handler(int signum) {
    keep_waiting = 0;
}

int register_signal_handling() {
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));

    new_action.sa_handler = my_signal_handler;
    
    return sigaction(SIGUSR1, &new_action, NULL);
}

int main(int argc, char *argv[]) {

    register_signal_handling();

    printf("Going to calculate the meaning of life\n");

    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed forking");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child
        char *exec_args[] = { "./worker", NULL };
        execvp(exec_args[0], exec_args);
        perror("Failed executing worker");
        exit(EXIT_FAILURE);
    } else {
        // Parent
        printf("Waiting");
        while(keep_waiting) {
            sleep(1);
            printf(".");
            fflush(stdout);
        }
        printf("\n\n");

        printf("The meaning of life is: ");
        read_from_file();
        printf("\n");
    }

    return EXIT_SUCCESS;
}