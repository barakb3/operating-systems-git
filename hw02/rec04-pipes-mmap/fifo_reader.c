#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#define FIFO_NAME "/tmp/osfifo"
#define BUFLEN 64

int main()
{
  int   fd;
  char  buf[BUFLEN+1];
  int   n_curr_read = 0;

  fd = open(FIFO_NAME, O_RDONLY);

  while( 1 )
  {
    n_curr_read = read(fd, buf, BUFLEN);
    if( 0 < n_curr_read )
    {
      buf[n_curr_read] = '\0';
      printf("Reader> %d bytes read\n", n_curr_read);
      printf("Reader> The message: ");
      puts(buf);
    }

    if( 0 >= n_curr_read )
    {
      puts("Reader> Data transfer complete. Leaving.");
      break;
    }

    //--- Persistent "listener"
    //sleep(1);
  }
  close(fd);
  return 0;
}
