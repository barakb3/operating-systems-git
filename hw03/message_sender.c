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
    int fd;
    char *endptr;
    unsigned long channel_id = strtol(argv[2], &endptr, 10);
    if (argc != 4)
    {
        /* invalid number of arguments */
        errno = EINVAL;
        fprintf(stderr, "Invalid number of arguments.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((fd = open(argv[1], O_WRONLY)) < 0)
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

    if (write(fd, argv[3], strlen(argv[3])) < 0)
    {
        /* writing failed */
        fprintf(stderr, "Writing failed.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(fd) < 0)
    {
        /* close failed */
        fprintf(stderr, "Close failed.\nerrno: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}