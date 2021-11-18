#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> // for wait macros etc
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv)
{
	pid_t pid1 = fork();
	pid_t pid2 = fork();
	
	while (1);
	// how many processes?
	// run in the background (add & at the end)
	// and use "ps" for the answer
}
