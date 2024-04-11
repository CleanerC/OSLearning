#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"

// int size = 40 << 20; //40 MiB
int size = 1 << 20; //1 MiB
// int size = 10; char* hello = "1234567890";
// int size = 8000;

int main() {
    char* disk_name = "mydisk";
  
    assert(make_fs(disk_name) == 0);

    assert(mount_fs(disk_name) == 0);

    const char* filename = "file1.txt";
    int result = fs_create(filename);
    assert(result == 0);

    assert(fs_open("file1.txt") == 0);
    assert(fs_open("file1.txt") == 1);
    assert(fs_close(1) == 0);
    char* hello = malloc(size);
    for(int ii = 0; ii < size; ii++) { hello[ii] = '1';}
    assert(fs_write(0, hello, size) == size);
    assert(fs_lseek(0, size) == 0);
    assert(fs_write(0, hello, size) == size);
    
    char* recieve = malloc(size);
    assert(fs_lseek(0, size) == 0);
    assert(fs_read(0, recieve, size) == size);
    for(int ii = 0; ii < size; ii++) { assert( recieve[ii] == hello[ii] ); }

    assert(fs_lseek(0, 0) == 0);
    assert(fs_read(0, recieve, size) == size);
    for(int ii = 0; ii < size; ii++) { assert( recieve[ii] == hello[ii] ); }
    
    assert(umount_fs(disk_name) == 0);
    
    free(hello);
    free(recieve);
    return 0;
}