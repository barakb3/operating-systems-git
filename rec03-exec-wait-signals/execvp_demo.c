#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv )
{
  char* exec_args[] = { "./aux2", "some", "thing", NULL };
  printf("Inside: %s, PID: %d\n", argv[0], getpid());
  execvp(exec_args[0], exec_args);
  puts("Fantastic!");
  return 0;
}
