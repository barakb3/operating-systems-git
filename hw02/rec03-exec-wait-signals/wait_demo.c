#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHILD_NUM 5
int main()
{
  int   chld_pids[CHILD_NUM];
  int   i = 0;
  int   curr_child = -1;
  int   keep_working = 1;
  int   exit_code = -1;

  char* child_args[] = { "./aux2", "junk", NULL };

  for( i = 0; i < CHILD_NUM; ++i )
    if( 0 == (curr_child = fork()) )
      execvp( child_args[0], child_args );
    else
      chld_pids[i] = curr_child;

  curr_child = waitpid( chld_pids[CHILD_NUM-1],
                        &exit_code,
                        0 );
  printf( "Child %d is gone with status %d\n",
          curr_child, exit_code );

  kill( chld_pids[0], SIGTERM );

  while( keep_working )
    if( -1 != ( curr_child = wait( &exit_code ) ) )
      printf( "Child %d is gone with status %d\n",
              curr_child, exit_code );
    else
      keep_working = 0;
  printf("Done");
}

