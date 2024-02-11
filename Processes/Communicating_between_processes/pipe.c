#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	int fd[2];	//[0] is the read, [1] is the write
	if( pipe(fd) == -1 ) {
		perror("ERROR:");
		exit(1);
	}
	
	int cpid = fork();

	if( cpid == 0 ) {
		close(fd[0]);
		int x = 0;
		printf("Input a number: ");
		scanf("%d", &x);
		write(fd[1], &x, sizeof(int));
		close(fd[1]);
		exit(0);
	} else {
		close(fd[1]);
		int y = 0;
		read(fd[0], &y, sizeof(int));
		close(fd[0]);
		printf("Got from child %d", y);
	}

	exit(0);
}
