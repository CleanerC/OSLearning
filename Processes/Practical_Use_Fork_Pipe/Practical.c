#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv) {
	int arr[] = { 1, 2, 3, 4, 1, 2};
	int start = 0, end = 0, arraySize = sizeof(arr) / sizeof(int);
	int fd[2];
	pipe(fd);

	int id = fork();
	if(id == 0) {
		start = 0;
		end = start + arraySize / 2;
	} else {
		start = arraySize / 2;
		end = arraySize;
	}

	int sum = 0;
	int ii = 0;

	for(ii = start; ii < end; ii++) {
		sum += arr[ii];
	}

	if(id == 0) {
		close(fd[0]);
		write(fd[1], &sum, sizeof(int));
		close(fd[1]);
	}
	
	int fromChild = 0;
	close(fd[1]);
       	read(fd[0], &fromChild, sizeof(int));	
	close(fd[0]);
	if(id > 0) {
		sum += fromChild;
		printf("the sum of the array is %d\n", sum);
	}
	return 0;
}
