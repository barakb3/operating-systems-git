#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#define SUCCESS 0
#define FAILURE 1

typedef struct DIR_ENTRY
{
    DIR *dir;
    struct DIR_ENTRY *next;
} DIR_ENTRY;

typedef struct DIR_FIFO_Q
{
    DIR_ENTRY *first;
    DIR_ENTRY *last;
} DIR_FIFO_Q;

DIR_FIFO_Q *initialize_directory_queue(int *status);
int iterate_dir(DIR_FIFO_Q *dir_q, char *path, const char *search_term);
DIR_ENTRY *dequeue(DIR_FIFO_Q *dir_q);
int enqueue(DIR_FIFO_Q *dir_q, DIR *dir);

DIR_FIFO_Q *initialize_directory_queue(int *status)
{
    DIR_FIFO_Q *dir_q = (DIR_FIFO_Q *)malloc(sizeof(DIR_FIFO_Q));

    if (dir_q == NULL)
    {
        *status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the directory queue due to errno: %s", strerror(errno));
        return NULL;
    }

    dir_q->first = (DIR_ENTRY *)malloc(sizeof(DIR_ENTRY));
    if (dir_q->first == NULL)
    {
        *status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the directory entry due to errno: %s", strerror(errno));
        return NULL;
    }
    dir_q->first->dir = NULL;
    dir_q->first->next = NULL;
    dir_q->last = dir_q->first;
    return dir_q;
}

int iterate_dir(DIR_FIFO_Q *dir_q, char *path, const char *search_term)
{
    DIR_ENTRY *first = dir_q->first;
    struct dirent *curr_entry;
    struct stat *curr_stat;
    char *curr_name;
    char curr_path[PATH_MAX];
    DIR *new_dir;

    errno = 0;
    while ((curr_entry = readdir(first->dir)) != NULL)
    {
        curr_name = curr_entry->d_name;

        if (strcmp(curr_name, ".") == 0 || strcmp(curr_name, "..") == 0)
        {
            /* dirent is "." or ".." */
            continue;
        }

        /* extracting dirent type by modifing the stat structure to represent the current dirent */
        strcpy(curr_path, path);
        strcat(curr_path, curr_name);
        if (stat(curr_path, curr_stat) != 0)
        {
            fprintf(stderr, "Failed when tried to check dir status due to errno: %s", strerror(errno));
            return 1;
        }

        if (S_ISDIR(curr_stat->st_mode))
        {
            /* dirent is a directory */
            if ((new_dir = opendir(curr_path)) == NULL)
            {
                /* directory can't be searched */
                printf("Directory %s: Permission denied.\n", curr_path);
            }
            enqueue(dir_q, new_dir);
        }
        else
        {
            /* dirent isn't a directory */
            if (strstr(curr_name, search_term))
            {
                /* the file name contains the search term */
                printf("%s\n", curr_path);
            }
        }
    }
    if (errno != 0)
    {
        fprintf(stderr, "Failed when tried to read dir content due to errno: %s", strerror(errno));
        return FAILURE;
    }
    return SUCCESS;
}

DIR_ENTRY *dequeue(DIR_FIFO_Q *dir_q)
{
    DIR_ENTRY *ret = dir_q->first;
    dir_q->first = dir_q->first->next;
    return ret;
}

int enqueue(DIR_FIFO_Q *dir_q, DIR *dir)
{
    int status = SUCCESS;
    dir_q->last->next = (DIR_ENTRY *)malloc(sizeof(DIR_ENTRY));
    if (dir_q->last->next == NULL)
    {
        status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the directory entry due to errno: %s", strerror(errno));
    }
    dir_q->last = dir_q->last->next;
    dir_q->last->dir = dir;
    dir_q->last->next = NULL;
    return status;
}

int main(int argc, char *argv[])
{
    int status = SUCCESS;
    char *path;
    const char *search_term;

    /* initializing the directory queue */
    DIR_FIFO_Q *dir_q = initialize_directory_queue(&status);
    if (dir_q == NULL)
    {
        /* initializing failed */
        return status;
    }
    
    if (argc != 4)
    {
        /* numbar of arguments isn't valid */
        status = FAILURE;
        fprintf(stderr, "Number of arguments isn't valid");
        return status;
    }

    path = argv[1];
    search_term = argv[2];

    if ((dir_q->first->dir = opendir(argv[2])) == NULL)
    {
        /* root directory can't be searched */
        status = FAILURE;
        fprintf(stderr, "Root directory can't be searched, failed due to errno: %s", strerror(errno));
        return status;
    }

    /* create argv[3] threads */

    while (dir_q->first != NULL)
    {
        if ((status = iterate_dir(dir_q, path, search_term)) == 1)
        {
            /* Failed when tried to read dir content */
            return status;
        }
        dequeue(dir_q);
    }
}