#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
  int   pipefd[2];
  pid_t cpid;
  char  buf;

  if( 2 != argc )
  {
    printf( "Usage: %s <string>\n", argv[0] );
    exit( -1 );
  }

  if( -1 == pipe( pipefd ) )
  {
    perror( "pipe" );
    exit( -1 );
  }

  cpid = fork();
  if( -1 == cpid )
  {
    perror("fork");
    exit( -1 );
  }

  if( 0 == cpid )
  {
    // Child reads from pipe
    // Close unused write end
    close( pipefd[1] );
    while( read( pipefd[0], &buf, 1 ) > 0 )
    {
      printf( "%c", buf );
    }
    puts("");
    close( pipefd[0] );
    exit( 0 );
  }
  else
  {
    // Parent writes argv[1] to pipe
    // Close unused read end
    close( pipefd[0] );

    // Write the data
    write( pipefd[1], argv[1], strlen( argv[1] ) );

    // Reader will see EOF
    close(pipefd[1]);

    // Wait for child
    wait( NULL );
    exit( 0 );
  }
}
