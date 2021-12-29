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
    char path[PATH_MAX];
    struct DIR_ENTRY *next;
} DIR_ENTRY;

typedef struct DIR_FIFO_Q
{
    DIR_ENTRY *first;
    DIR_ENTRY *last;
} DIR_FIFO_Q;

DIR_FIFO_Q *initialize_directory_queue(int *status);
int iterate_dir(DIR_FIFO_Q *dir_q, const char *search_term);
DIR_ENTRY *dequeue(DIR_FIFO_Q *dir_q);
int enqueue(DIR_FIFO_Q *dir_q, DIR *dir, char *path);

DIR_FIFO_Q *initialize_directory_queue(int *status)
{
    DIR_FIFO_Q *dir_q = (DIR_FIFO_Q *)malloc(sizeof(DIR_FIFO_Q));

    if (dir_q == NULL)
    {
        *status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the directory queue due to errno: %s\n", strerror(errno));
        return NULL;
    }

    dir_q->first = (DIR_ENTRY *)malloc(sizeof(DIR_ENTRY));
    if (dir_q->first == NULL)
    {
        *status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the directory entry due to errno: %s\n", strerror(errno));
        return NULL;
    }
    dir_q->first->dir = NULL;
    dir_q->first->next = NULL;
    dir_q->last = dir_q->first;
    return dir_q;
}

int iterate_dir(DIR_FIFO_Q *dir_q, const char *search_term)
{
    DIR *dir = dir_q->first->dir;
    struct dirent *curr_entry;
    struct stat curr_statbuf;
    char *curr_name;
    char curr_path[PATH_MAX];
    DIR *new_dir;

    errno = 0;
    while ((curr_entry = readdir(dir)) != NULL)
    {
        curr_name = curr_entry->d_name;

        if (strcmp(curr_name, ".") == 0 || strcmp(curr_name, "..") == 0)
        {
            /* dirent is "." or ".." */
            continue;
        }

        /* modifying the path of the current dirent */
        strcpy(curr_path, dir_q->first->path);
        strcat(curr_path, curr_name);

        /* extracting dirent type by modifing the stat structure to represent the current dirent */
        if (stat(curr_path, &curr_statbuf) != 0)
        {
            printf("the following directory made the problem: %s\n", curr_path);
            printf("File Permissions: \t");
            printf((S_ISDIR(curr_statbuf.st_mode)) ? "d" : "-");
            printf((curr_statbuf.st_mode & S_IRUSR) ? "r" : "-");
            printf((curr_statbuf.st_mode & S_IWUSR) ? "w" : "-");
            printf((curr_statbuf.st_mode & S_IXUSR) ? "x" : "-");
            printf((curr_statbuf.st_mode & S_IRGRP) ? "r" : "-");
            printf((curr_statbuf.st_mode & S_IWGRP) ? "w" : "-");
            printf((curr_statbuf.st_mode & S_IXGRP) ? "x" : "-");
            printf((curr_statbuf.st_mode & S_IROTH) ? "r" : "-");
            printf((curr_statbuf.st_mode & S_IWOTH) ? "w" : "-");
            printf((curr_statbuf.st_mode & S_IXOTH) ? "x" : "-");
            printf("\n\n");
            fprintf(stderr, "Failed when tried to check dir status due to errno: %s\n", strerror(errno));
            return FAILURE;
        }

        if (S_ISDIR(curr_statbuf.st_mode))
        {
            /* dirent is a directory */
            if ((new_dir = opendir(curr_path)) == NULL)
            {
                /* directory can't be searched */
                printf("Directory %s: Permission denied.\n", curr_path);
            }
            else
            {
                /* directory can be searched */
                strcat(curr_path, "/");
                printf("after concatenation: %s\n", curr_path);
                if (enqueue(dir_q, new_dir, curr_path) == FAILURE)
                {
                    /* enqueuing failed */
                    return FAILURE;
                }
            }
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
        fprintf(stderr, "Failed when tried to read dir ( readdir() ) content in some iteration due to errno: %s\n", strerror(errno));
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

int enqueue(DIR_FIFO_Q *dir_q, DIR *dir, char *path)
{
    int status = SUCCESS;
    dir_q->last->next = (DIR_ENTRY *)malloc(sizeof(DIR_ENTRY));
    if (dir_q->last->next == NULL)
    {
        status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the directory entry due to errno: %s\n", strerror(errno));
    }
    dir_q->last = dir_q->last->next;
    dir_q->last->dir = dir;
    strcpy(dir_q->last->path, path);
    dir_q->last->next = NULL;
    return status;
}

int main(int argc, char *argv[])
{
    int status = SUCCESS;
    char *path, *search_term;
    DIR *root;
    DIR_FIFO_Q *dir_q;

    if (argc != 4)
    {
        /* numbar of arguments isn't valid */
        status = FAILURE;
        fprintf(stderr, "Number of arguments isn't valid\n");
        return status;
    }

    path = argv[1];
    search_term = argv[2];
    root = opendir(path);

    if (root == NULL)
    {
        /* root directory can't be searched */
        status = FAILURE;
        printf("Directory %s: Permission denied.\n", path);
    }

    /* initializing the directory queue */
    dir_q = initialize_directory_queue(&status);
    if (dir_q == NULL)
    {
        /* initializing failed */
        return status;
    }

    dir_q->first->dir = root;
    strcpy(dir_q->first->path, path);

    /****** create argv[3] threads ******/

    while (dir_q->first != NULL)
    {
        status = iterate_dir(dir_q, search_term);
        if (status == FAILURE)
        {
            /* Failed when tried to read dir content */
            return status;
        }
        dequeue(dir_q);
        /* free the dequeued entry????????? */
    }
}