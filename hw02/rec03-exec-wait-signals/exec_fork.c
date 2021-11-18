#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define AUX_PATH "./aux"

int main(int argc, char *argv[]) {
    printf("Hello from exec_fork, PID: %d, PPID: %d\n", getpid(), getppid());

    int pid = fork();
    if (pid == -1) {
        perror("Failed forking");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        //sleep(1);
        char *exec_args[] = { AUX_PATH, NULL };
        if (execvp(AUX_PATH, exec_args) == -1) {
            perror("Failed executing aux");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        sleep(1);
        printf("Child %d executed, I'm done\n", pid);
        exit(EXIT_SUCCESS);
    }

    printf("Do you see me?\n");
}