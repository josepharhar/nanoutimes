#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: cinc <filepath>\n");
    return 1;
  }

  char* filepath = argv[1];
  struct stat stat_struct;
  memset(&stat_struct, 0, sizeof(struct stat));
  if (stat(filepath, &stat_struct)) {
    printf("stat() failed, strerror: %s\n", strerror(errno));
    return 1;
  }

  struct timeval times[2];
  memset(times, 0, sizeof(struct timeval) * 2);
  times[0].tv_sec = stat_struct.st_atimespec.tv_sec;
  times[0].tv_nsec = stat_struct.st_atimespec.tv_nsec;
  times[1].tv_sec = stat_struct.st_mtimespec.tv_sec;
  times[1].tv_nsec = stat_struct.st_mtimespec.tv_nsec + 1;
  if (utimes(filepath, times)) {
    printf("utimes() failed, strerror: %s\n", strerror(errno));
    return 1;
  }

  printf("successfully incremented mtime\n");
  printf("from: %d.%d\n",
      stat_struct.st_mtimespec.tv_sec,
      stat_struct.st_mtimespec.tv_nsec);
  printf("  to: %d.%d\n", times[1].tv_sec, times[1].tv_nsec);

  return 0;
}
