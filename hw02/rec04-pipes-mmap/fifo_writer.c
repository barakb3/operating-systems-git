#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFLEN    1024
#define FIFO_NAME "/tmp/osfifo"

int main(int argc, char** argv)
{
  int   fd;
  char  buf[BUFLEN+1];
  int   n_chars_written = 0;
  int   n_chars_to_write = -1;
  int   n_times_to_repeat = 1;
  char* str_message = argv[1];

  if( argc > 2 )
    n_times_to_repeat = atoi( argv[2] );

  fd = open( FIFO_NAME, O_WRONLY );
  sprintf( buf, "%d says- %s", getpid(), str_message );
  n_chars_to_write = strlen( buf );
  for( int i = 0; i < n_times_to_repeat; ++i )
  {
    n_chars_written = write(fd, buf, n_chars_to_write);
    printf("Writer> %d bytes written through pipe\n",
            n_chars_written);
  }
  close(fd);
  return 0;
}
