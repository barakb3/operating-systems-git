#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> // for wait macros etc
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv)
{
  int f = fork();
  if( f > 0 )
  {
    //Parent goes here
    printf("parent pid=%d, child pid=%d\n", getpid(), f);
    return 0;
  }
  // Son
  while( 1 )
  {
    sleep(4);
    printf( "Son pid = %d, parent = %d\n", getpid(), getppid());
  }
}
