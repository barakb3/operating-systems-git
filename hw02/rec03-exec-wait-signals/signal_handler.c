#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void my_signal_handler(int signum) {
    printf("Can't stop me!\n");
}

int register_signal_handling() {
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));

    new_action.sa_handler = my_signal_handler;
    
    // Overwrite default behavior for ctrl+c
    return sigaction(SIGINT, &new_action, NULL);
}

int main(int argc, char *argv[]) {

    if (register_signal_handling() == -1) {
        perror("Signal handle registration failed");
        exit(EXIT_FAILURE);
    }

    int i = 1;
    int pid = getpid();
    while(1) {
        printf("Process %d, iteration %d\n", pid, i++);
        sleep(1);
    }

    return EXIT_SUCCESS;
}