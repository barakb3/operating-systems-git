#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *thread_func(void *thread_param) {
	long *y = (long *)thread_param;
	printf("In thread #%ld\n", pthread_self());
	printf("I received %ld from my caller\n", *y);

	*y += 1;
	// Treat the return value as if it was a pointer
	return (void *)*y;
}

int main(int argc, char *argv[]) {
	printf("In main thread #%lu\n", pthread_self());

	pthread_t thread_id;
	long *x = malloc(sizeof(long));
	*x = 5;

	int rc = pthread_create(&thread_id, NULL, thread_func, (void *)x);
	if (rc) {
		fprintf(stderr, "Failed creating thread: %s\n", strerror(rc));
		free(x);
		exit(EXIT_FAILURE);
	}

	long thread_return;
	pthread_join(thread_id, (void **)&thread_return);

	printf("Thread %lu finished and returned %ld\n", thread_id, thread_return);
	printf("Original value was %ld\n", *x);

	free(x);

	return EXIT_SUCCESS;
}
