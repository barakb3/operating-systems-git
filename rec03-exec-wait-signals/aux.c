#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("Hello from aux, PID: %d, PPID: %d\n", getpid(), getppid());
}