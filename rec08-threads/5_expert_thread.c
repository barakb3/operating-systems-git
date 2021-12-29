#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THREAD_COUNT 10000

static int x = 0;

void *thread_func(void *thread_param) {
    // WARNING: THIS EXAMPLE IS TRIGGERING UNDEFINED BEHAVIOR
    // Having multiple accesses to the same non atomic variable at the same time where
    // at least one of them is a write is undefined.
    //
    // This example is here to show the complications of sharing data between threads.
    // DO NOT USE THIS IN PRODUCTION.
    x += 1;
	return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread_ids[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        int rc = pthread_create(&thread_ids[i], NULL, thread_func, NULL);
        if (rc) {
            fprintf(stderr, "Failed creating thread: %s\n", strerror(rc));
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    printf("x=%d\n", x);

    return EXIT_SUCCESS;
}
