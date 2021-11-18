#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv)
{
  while(1)
  {
    printf("Name: %s, PID: %d\n", argv[0], getpid());
    sleep(4);
  }
}
