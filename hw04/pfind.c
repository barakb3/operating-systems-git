#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>

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
    int len;
} DIR_FIFO_Q;

typedef struct THREAD_ENTRY
{
    pthread_cond_t *my_condition_variable;
    DIR *dir;
    char path[PATH_MAX];
    struct THREAD_ENTRY *next;
} THREAD_ENTRY;

typedef struct THREAD_FIFO_Q
{
    THREAD_ENTRY *first;
    THREAD_ENTRY *last;
    int len;
} THREAD_FIFO_Q;

DIR_FIFO_Q *initialize_directories_queue();
THREAD_FIFO_Q *initialize_threads_queue();
pthread_t *initialize_threads_id_arr();
void *thread_func(void *thread_param);
void scan_dir(THREAD_ENTRY *my_thread_entry);
DIR_ENTRY *dequeue_dir();
THREAD_ENTRY *dequeue_thread(THREAD_FIFO_Q *thread_q);
void enqueue_dir(DIR *dir, char *path);
void enqueue_thread(THREAD_ENTRY *my_thread_entry);

static int status;
static int done = 0;
static int num_of_threads;
atomic_int threads_initialized = 0;
atomic_int threads_failed = 0;
atomic_int num_of_files_found = 0;

DIR_FIFO_Q *dir_q;
THREAD_FIFO_Q *thread_q;

char *search_term;

pthread_mutex_t thread_initializer;
pthread_mutex_t queues_access;

pthread_cond_t all_initialized;
pthread_cond_t start_work;
pthread_cond_t all_sleep;

DIR_FIFO_Q *initialize_directories_queue()
{
    DIR_FIFO_Q *dir_q = (DIR_FIFO_Q *)malloc(sizeof(DIR_FIFO_Q));

    if (dir_q == NULL)
    {
        status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the directories queue due to errno: %s\n", strerror(errno));
        return NULL;
    }
    
    dir_q->last = dir_q->first;
    return dir_q;
}
THREAD_FIFO_Q *initialize_threads_queue()
{
    THREAD_FIFO_Q *thread_q = (THREAD_FIFO_Q *)malloc(sizeof(THREAD_FIFO_Q));

    if (thread_q == NULL)
    {
        status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the threads queue due to errno: %s\n", strerror(errno));
        return NULL;
    }
    
    thread_q->len = 0;
    return thread_q;
}

pthread_t *initialize_threads_id_arr()
{
    pthread_t *threads_id = (pthread_t *)malloc(num_of_threads * sizeof(pthread_t));
    if (threads_id == NULL)
    {
        status = FAILURE;
        fprintf(stderr, "Failed allocating memory for the threads_id array due to errno: %s\n", strerror(errno));
        return NULL;
    }
    return threads_id;
}

void *thread_func(void *thread_param)
{
    DIR_ENTRY *dir_to_handle;
    pthread_cond_t my_condition_variable;
    THREAD_ENTRY *my_thread_entry = (THREAD_ENTRY *)malloc(sizeof(THREAD_ENTRY));
    if (my_thread_entry == NULL)
    {
        status = FAILURE;
        fprintf(stderr, "Failed allocating memory for thread number %lu due to errno: %s\n", pthread_self(), strerror(errno));
        threads_failed++;
        if (threads_failed == num_of_threads)
        {
            pthread_cond_signal(&all_initialized);
        }
        pthread_exit((void *)FAILURE);
    }

    pthread_cond_init(&my_condition_variable, NULL);
    my_thread_entry->my_condition_variable = &my_condition_variable;

    /* put thread to sleep */
    pthread_mutex_lock(&thread_initializer);
    /* increment the number of threads started by one */
    threads_initialized++;

    if (threads_initialized + threads_failed == num_of_threads)
    {
        /* last thread creation */
        pthread_cond_signal(&all_initialized);
    }
    /* calling cond_wait sleeping the thread and release the mutex so other threads can go to sleep*/
    pthread_cond_wait(&start_work, &thread_initializer);
    /* thread woke up and immediately locked the mutex */
    /* finally release the mutex in order to let other threads wake up */
    pthread_mutex_unlock(&thread_initializer);

    pthread_mutex_lock(&queues_access);
    dir_to_handle = dequeue_dir(dir_q);
    pthread_mutex_unlock(&queues_access);

    if (dir_to_handle != NULL)
    {
        /* the thread that 'caught' the root need to handle it and only after that enter the queue */
        my_thread_entry->dir = dir_to_handle->dir;
        strcpy(my_thread_entry->path, dir_to_handle->path);
        scan_dir(my_thread_entry);
    }

    do
    {
        pthread_mutex_lock(&queues_access);
        /* if didn't find dir to handle in scan dir it goes bacl here and enter the queue */

        my_thread_entry->dir = NULL;
        strcpy(my_thread_entry->path, "\0");
        my_thread_entry->next = NULL;
        enqueue_thread(my_thread_entry);

        if (thread_q->len == threads_initialized)
        {
            pthread_cond_signal(&all_sleep);
        }
        
        pthread_cond_wait(&my_condition_variable, &queues_access);
    
        if (done == 1)
        {
            pthread_mutex_unlock(&queues_access);
            /* destroy thread's condition variable */
            pthread_cond_destroy(&my_condition_variable);
            pthread_exit((void *)SUCCESS);
        }

        pthread_mutex_unlock(&queues_access);

        scan_dir(my_thread_entry);
    } while (1);
}

void scan_dir(THREAD_ENTRY *my_thread_entry)
{
    struct dirent *curr_entry;
    struct stat curr_statbuf;
    char *curr_name;
    char curr_path[PATH_MAX];
    DIR *new_dir;
    THREAD_ENTRY *next_thread_in_queue;
    DIR_ENTRY *next_dir_in_queue;

    errno = 0;
    while ((curr_entry = readdir(my_thread_entry->dir)) != NULL)
    {
        curr_name = curr_entry->d_name;

        if (strcmp(curr_name, ".") == 0 || strcmp(curr_name, "..") == 0)
        {
            /* dirent is "." or ".." */
            continue;
        }

        /* modifying the path of the current dirent */
        strcpy(curr_path, my_thread_entry->path);
        strcat(curr_path, curr_name);

        /* extracting dirent type by modifing the stat structure to represent the current dirent */
        if (stat(curr_path, &curr_statbuf) != 0)
        {
            status = FAILURE;
            fprintf(stderr, "Failed when tried to check dir status due to errno: %s\n", strerror(errno));
            threads_failed++;
            if (threads_failed == num_of_threads)
            {
                pthread_cond_signal(&all_sleep);
            }
            pthread_exit((void *)FAILURE);
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
                pthread_mutex_lock(&queues_access);
                next_thread_in_queue = dequeue_thread(thread_q);
                if (next_thread_in_queue == NULL)
                {
                    enqueue_dir(new_dir, curr_path);
                }
                pthread_mutex_unlock(&queues_access);
                if (next_thread_in_queue != NULL)
                {
                    next_thread_in_queue->dir = new_dir;
                    strcpy(next_thread_in_queue->path, curr_path);
                    pthread_cond_signal(next_thread_in_queue->my_condition_variable);
                }
            }
        }
        else
        {
            /* dirent isn't a directory */
            if (strstr(curr_name, search_term))
            {
                /* the file name contains the search term */
                num_of_files_found++;
                printf("%s\n", curr_path);
            }
        }
    }
    if (errno != 0)
    {
        status = FAILURE;
        fprintf(stderr, "Threads number %lu failed when tried to read dir ( readdir() ) content in some iteration due to errno: %s\n", pthread_self(), strerror(errno));
        threads_failed++;
        if (threads_failed == num_of_threads)
        {
            pthread_cond_signal(&all_sleep);
        }
        pthread_exit((void *)FAILURE);
    }

    /* thread finished scanning some dir and now checks if there are any new directories to work on */
    pthread_mutex_lock(&queues_access);
    next_dir_in_queue = dequeue_dir(dir_q);
    pthread_mutex_unlock(&queues_access);

    if (next_dir_in_queue != NULL)
    {
        /* the thread need to handle the directory (strat scan_dir from the beginning) */
        my_thread_entry->dir = next_dir_in_queue->dir;
        strcpy(my_thread_entry->path, next_dir_in_queue->path);
        scan_dir(my_thread_entry);
    }
    /* otherwise already entered the queue and now goes to sleep */
}

DIR_ENTRY *dequeue_dir()
{
    if (dir_q->first == NULL)
    {
        return NULL;
    }
    DIR_ENTRY *ret = dir_q->first;
    dir_q->first = dir_q->first->next;
    dir_q->len--;
    return ret;
}

THREAD_ENTRY *dequeue_thread(THREAD_FIFO_Q *thread_q)
{
    if (thread_q->first == NULL)
    {
        return NULL;
    }
    THREAD_ENTRY *ret = thread_q->first;
    thread_q->first = thread_q->first->next;
    thread_q->len--;
    if (thread_q->len == 1)
    {
        thread_q->last = thread_q->first;
    }
    /*
    th = thread_q->first;
    printf("thread number %d dequeued the queue: ", ret->debug_number);
    while (th != NULL)
    {
        printf("%d  ", th->debug_number);
        th = th->next;
    }
    printf("\n");
    */
    return ret;
}

void enqueue_dir(DIR *dir, char *path)
{
    if (dir_q->len == 0)
    {
        dir_q->first = (DIR_ENTRY *)malloc(sizeof(DIR_ENTRY));
        if (dir_q->first == NULL)
        {
            status = FAILURE;
            fprintf(stderr, "Failed allocating memory for the directories entry due to errno: %s\n", strerror(errno));
            threads_failed++;
            if (threads_failed == num_of_threads)
            {
                pthread_cond_signal(&all_sleep);
            }
            pthread_exit((void *)FAILURE);
        }
        dir_q->last = dir_q->first;
    }
    else
    {
        dir_q->last->next = (DIR_ENTRY *)malloc(sizeof(DIR_ENTRY));
        if (dir_q->last->next == NULL)
        {
            status = FAILURE;
            fprintf(stderr, "Failed allocating memory for the directory entry due to errno: %s\n", strerror(errno));
            threads_failed++;
            if (threads_failed == num_of_threads)
            {
                pthread_cond_signal(&all_sleep);
            }
            pthread_exit((void *)FAILURE);
        }
        dir_q->last = dir_q->last->next;
    }
    dir_q->last->dir = dir;
    strcpy(dir_q->last->path, path);

    dir_q->len++;
}

void enqueue_thread(THREAD_ENTRY *my_thread_entry)
{
    if (thread_q->len == 0)
    {
        thread_q->first = my_thread_entry;
        thread_q->last = thread_q->first;
    }
    else
    {
        thread_q->last->next = my_thread_entry;
        thread_q->last = thread_q->last->next;
    }
    thread_q->len++;
    /*
    th = thread_q->first;
    printf("thread number %d enqueued to queue: ", my_thread_entry->debug_number);
    while (th != NULL)
    {
        printf("%d  ", th->debug_number);
        th = th->next;
    }
    printf("\n");
    */
}

int main(int argc, char *argv[])
{
    DIR *root;
    pthread_t *threads_id;
    THREAD_ENTRY *th;

    status = SUCCESS;
    if (argc != 4)
    {
        /* numbar of arguments isn't valid */
        status = FAILURE;
        fprintf(stderr, "Number of arguments isn't valid\n");
        exit(status);
    }

    search_term = argv[2];
    num_of_threads = atoi(argv[3]);
    root = opendir(argv[1]);

    if (root == NULL)
    {
        /* root directory can't be searched */
        status = FAILURE;
        printf("Directory %s: Permission denied.\n", argv[1]);
        printf("Done searching, found %d files\n", num_of_files_found);
        exit(status);
    }

    /* initializing the directories queue */
    dir_q = initialize_directories_queue();
    if (dir_q == NULL)
    {
        /* initializing failed */
        exit(status);
    }

    enqueue_dir(root, argv[1]);

    /* initializing the threads_id array */
    threads_id = initialize_threads_id_arr();
    if (threads_id == NULL)
    {
        /* initializing failed */
        exit(status);
    }

    /* initializing the threads queue */
    thread_q = initialize_threads_queue();
    if (thread_q == NULL)
    {
        /* initializing failed */
        exit(status);
    }

    /* initializing mutex and condition variable */
    pthread_mutex_init(&thread_initializer, NULL);
    pthread_cond_init(&all_initialized, NULL);
    pthread_cond_init(&start_work, NULL);

    pthread_mutex_init(&queues_access, NULL);
    pthread_cond_init(&all_sleep, NULL);

    /* creating num_of_threads - 1 (=argv[3] - 1) threads */
    for (int i = 0; i < num_of_threads - 1; i++)
    {
        if (pthread_create(&threads_id[i], NULL, thread_func, NULL) != SUCCESS)
        {
            status = FAILURE;
            fprintf(stderr, "Failed creating thread number %d due to errno: %s\n", i, strerror(errno));
            exit(status);
        }
    }

    /* lock in main before all_initialized condition is met */
    pthread_mutex_lock(&thread_initializer);

    /* now it can create the last thread that will meet the condition all_initialized */
    if (pthread_create(&threads_id[num_of_threads-1], NULL, thread_func, NULL) != 0)
    {
        status = FAILURE;
        fprintf(stderr, "Failed creating thread number %d due to errno: %s\n", num_of_threads, strerror(errno));
        exit(status);
    }

    pthread_cond_wait(&all_initialized, &thread_initializer);
    /* unlocked thread_initializer so last thread created can lock it and signal all_initialized */

    if (threads_failed == num_of_threads)
    {
        return status;
    }

    /* all_initialized sent we can unlock thread_initializer so threads can respond to start_work */
    pthread_mutex_unlock(&thread_initializer);

    /* lock queues before all searching threads to ensure reaching all_sleep condition */
    pthread_mutex_lock(&queues_access);

    /* wake up all threads */
    pthread_cond_broadcast(&start_work);

    /* waiting for all threads to finish their work */
    pthread_cond_wait(&all_sleep, &queues_access);

    if (threads_failed == num_of_threads)
    {
        return status;
    }

    done = 1;

    /* searching threads work is done */
    pthread_mutex_unlock(&queues_access);

    th = thread_q->first;
    /* wake up all threads so they can exit cleanly */
    while (th != NULL)
    {
        pthread_cond_signal(th->my_condition_variable);
        th = th->next;
    }

    /* waiting for all threads to finish their work */
    for (int i = 0; i < num_of_threads; i++)
    {
        if (pthread_join(threads_id[i], NULL) != SUCCESS)
        {
            status = FAILURE;
            fprintf(stderr, "Failed joining thread %d due to errno: %s\n", i, strerror(errno));
            return status;
        }
    }

    pthread_cond_destroy(&all_initialized);
    pthread_cond_destroy(&start_work);
    pthread_mutex_destroy(&thread_initializer);

    pthread_cond_destroy(&all_sleep);
    pthread_mutex_destroy(&queues_access);

    printf("Done searching, found %d files\n", num_of_files_found);
    exit(status);
}