#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fs.h"
#include "disk.h"

int main() {
    char* disk_name = "mydisk";
  
    assert(make_fs(disk_name) == 0);

    assert(mount_fs(disk_name) == 0);

    const char* filename = "file1.txt";
    int result = fs_create(filename);
    assert(result == 0);
    assert(fs_open("fileDNE.txt") == -1);
    assert(fs_delete("fileDNE") == -1);

    assert(fs_create("file2") == 0);
    assert(fs_open("file1.txt") == 0);
    assert(fs_open("file1.txt") == 1);

    assert(fs_open("file2") == 2);
    assert(fs_close(2) == 0);
    assert(fs_open("file2") == 2);
    assert(fs_read(2, NULL, 0) == 0);
    assert(fs_delete("file2") == -1);
    assert(fs_close(2) == 0);
    assert(fs_delete("file2") == 0);
    assert(fs_open("file2") == -1);

    assert(umount_fs(disk_name) == 0);
    
    return 0;
}