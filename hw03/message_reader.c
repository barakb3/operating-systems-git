#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "message_slot.h"

int main(int argc, char **argv)
{
    int fd, bytes_read;
    char *endptr;
    unsigned long channel_id= strtol(argv[2], &endptr, 10);
    unsigned char buf[BUF_LEN];
    if (argc != 3)
    {
        /* invalid number of arguments */
        errno = EINVAL;
        fprintf(stderr, "Invalid number of arguments.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if ((fd = open(argv[1], O_RDONLY)) < 0)
    {
        /* open failed */
        fprintf(stderr, "Open file failed.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0)
    {
        /* set channel failed */
        fprintf(stderr, "Set channel failed.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((bytes_read = read(fd, buf, BUF_LEN)) < 0)
    {
        /* reading failed */
        fprintf(stderr, "Reading failed.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(fd) < 0)
    {
        /* close failed */
        fprintf(stderr, "Close failed.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (write(1, buf, bytes_read) < 0)
    {
        /* writing to stdout failed */
        fprintf(stderr, "Writing to STDOUT failed.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}