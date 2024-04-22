#include "fs.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#define BYTES_KB 1024
#define BYTES_MB (1024 * BYTES_KB)

int main() {
  const char *disk_name = "test_fs";
  const char *file_name = "test_file";
  char *read_buf = calloc(1, BYTES_MB);
  int fd;
  int bytes_written;

  srand(time(NULL));
  char *buf = calloc(1, BYTES_MB);
  for (int i = 0; i < BYTES_MB; i++) {
    buf[i] = 'A' + rand() % 26;
  }

  remove(disk_name); // remove disk if it exists
  assert(make_fs(disk_name) == 0);
  assert(mount_fs(disk_name) == 0);

  // 14.1) [EXTRA CREDIT] Write a 30 MiB file
  assert(fs_create(file_name) == 0);
  fd = fs_open(file_name);
  assert(fd >= 0);
  for(int ii = 0; ii < 30; ii++) {
    bytes_written = fs_write(fd, buf, BYTES_MB);
    assert(bytes_written == BYTES_MB);
  }

  assert(fs_get_filesize(fd) == 30 * BYTES_MB);
  assert(fs_lseek(fd, 0) == 0);
  memset(read_buf, 0, 30 * BYTES_MB);
  assert(fs_close(fd) == 0);
  assert(fs_delete(file_name) == 0);

}
