#include <stdio.h>
#include <unistd.h>

int main()
{
  pid_t pid = fork();
  if( pid > 0 )
    printf("Luke, I am your father\n");
  else
    printf("Oh NO!\n");
  return 0;
}
