#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) return EXIT_FAILURE;

    int pid_to_kill = atoi(argv[1]);
    if (pid_to_kill == 0) return EXIT_FAILURE;

    if (kill(pid_to_kill, SIGINT) == -1) {
        perror("Failed sending SIGINT");
        return EXIT_FAILURE;
    }
    if (kill(pid_to_kill, SIGKILL) == -1) {
        perror("Failed sending SIGKILL");
        return EXIT_FAILURE;
    }
}