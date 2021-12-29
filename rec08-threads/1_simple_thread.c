#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *thread_func(void *thread_param) {
	printf("In thread #%ld\n", pthread_self());
	printf("I received \"%s\" from my caller\n", (char *)thread_param);

	pthread_exit((void *)EXIT_SUCCESS);
	//return (void *)EXIT_SUCCESS;// <- same as this
}

int main(int argc, char *argv[]) {
	printf("In main thread #%lu\n", pthread_self());

	pthread_t thread_id;
	int rc = pthread_create(&thread_id, NULL, thread_func, (void *)"hello");
	if (rc) {
		fprintf(stderr, "Failed creating thread: %s\n", strerror(rc));
		exit(EXIT_FAILURE);
	}

	pthread_exit((void *)EXIT_SUCCESS);
	//return EXIT_SUCCESS; //<- NOT the same as this
}
