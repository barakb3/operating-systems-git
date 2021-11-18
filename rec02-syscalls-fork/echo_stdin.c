#include <unistd.h>
#include <stdio.h>

#define BUF_SIZE 256

int main()
{
  char    buf[BUF_SIZE+1];
  ssize_t bytes_read     = -1;
  ssize_t bytes_written  = -1;
  int     exit_status    =  0;

  bytes_read = read(0, buf, BUF_SIZE);
  if(bytes_read > 0 && bytes_read <= BUF_SIZE)
  {
    buf[bytes_read] = '\0';
    bytes_written = write(1, buf, bytes_read);
    if(bytes_written != bytes_read)
    {
      printf("Bytes read %d, bytes written %d\n",
             bytes_read, bytes_written);
      exit_status = 1;
    }
  }
  else
  {
    printf("Bytes read %d\n", bytes_read);
    exit_status = 2;
  }
  return exit_status;
}
