#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THREAD_COUNT 10

void *thread_func(void *thread_param) {
    long i = (long)thread_param;
    printf("I'm thread number %ld\n", i);
	return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread_ids[THREAD_COUNT];
    for (size_t i = 0; i < THREAD_COUNT; i++) {
        int rc = pthread_create(&thread_ids[i], NULL, thread_func, (void *)i);
        if (rc) {
            fprintf(stderr, "Failed creating thread: %s\n", strerror(rc));
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < THREAD_COUNT; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    return EXIT_SUCCESS;
}
