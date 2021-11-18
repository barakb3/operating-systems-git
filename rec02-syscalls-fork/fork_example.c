#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int x = 5;

    pid_t pid = fork();
    if (pid > 0) {
        x++;
        fork();
    }

    fork();
    
    printf("%d\n", x);
}