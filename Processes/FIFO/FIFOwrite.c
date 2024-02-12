#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_CHAR_LEN 100 

int main (int argc, char** argv) {
	if( mkfifo("myfifo", 0777) == -1) {
		if(errno != EEXIST) {
		printf("cannot create fifo file");
		return 1;
		}
	}
	
	char arr1[MAX_CHAR_LEN], arr2[MAX_CHAR_LEN];

	while(1) {
		int fd;
		fd = open("myfifo", O_RDONLY);
		read(fd, arr1, MAX_CHAR_LEN);

		printf("User1: %s\n", arr1);
		close(fd);

		fd = open("myfifo", O_WRONLY);
		fgets(arr2, MAX_CHAR_LEN, stdin);
		write(fd, arr2, MAX_CHAR_LEN);
		close(fd);
	}		
	return 0;
}

