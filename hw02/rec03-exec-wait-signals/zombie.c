#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int pid = fork();
    if (pid == -1) {
        perror("Failed forking");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        printf("I'm child, PID: %d\n", getpid());
    } else {
        // Parent process
        printf("I'm parent, PID: %d\n", getpid());
        getchar();
        //while(1);
    }
}