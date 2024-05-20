#include <blkid/blkid.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>

static int usage(void) {
  fprintf(stderr, "Usage: mountgpt -t <fstype> <device> <mountpoint>\n");
  return EXIT_FAILURE;
}

int main(const int argc, const char* const argv[]) {
  if (argc < 4)
    return usage();

  // Parse command-line arguments
  const char* fstype = NULL;
  const char* device = NULL;
  const char* mountpoint = NULL;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
      fstype = argv[++i];
    } else if (device == NULL) {
      device = argv[i];
    } else if (mountpoint == NULL) {
      mountpoint = argv[i];
    }
  }

  if (fstype == NULL || device == NULL || mountpoint == NULL)
    return usage();

  if (strncmp(device, "PARTUUID=", 9) == 0 ||
      strncmp(device, "PARTLABEL=", 10) == 0) {
    device = blkid_get_devname(NULL, device, NULL);
    if (device == NULL) {
      perror("Error getting device name");
      return EXIT_FAILURE;
    }
  }

  if (mount(device, mountpoint, fstype, 0, NULL) == -1) {
    perror("Error mounting device");
    return EXIT_FAILURE;
  }

  printf("Successfully mounted '%s' '%s' '%s'\n", device, mountpoint, fstype);
  return EXIT_SUCCESS;
}
