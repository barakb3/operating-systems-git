#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// print first 10 characters of a text file
#define BUFSIZE 10
int main(int argc, char** argv)
{
  // assert first argument filename
  assert ( argc == 2 );

  int fd = open(argv[1], O_RDWR);

  if( fd < 0 )
  {
    printf( "Error opening file: %s\n", strerror( errno ) );
    return errno;
  }
  else
  {
    printf( "File is opened. Descriptor %d\n", fd);
  }

  // read first 10 characters
  char buf[BUFSIZE+1];
  ssize_t len = read(fd, buf, BUFSIZE);
  if( len < 0 )
  {
    printf("Error reading from file: %s\n", strerror(errno));
    return -1;
  }
  buf[len] = '\0'; // string closer

  // we assume this is indeed a text file with readable characters...
  printf("First %d characters are: '%s'\n",len, buf);

  close(fd); // close file
}

