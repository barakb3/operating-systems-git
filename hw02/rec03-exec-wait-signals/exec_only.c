#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define AUX_PATH "./aux"

int main(int argc, char *argv[]) {
    printf("Hello from exec_only, PID: %d, PPID: %d\n", getpid(), getppid());

    char *exec_args[] = { AUX_PATH, NULL };
    if (execvp(AUX_PATH, exec_args) == -1) {
        perror("Failed executing aux");
        exit(EXIT_FAILURE);
    }

    printf("Do you see me?\n");
}