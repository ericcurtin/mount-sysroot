#include <blkid/blkid.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>

#define autofree __attribute__((cleanup(cleanup_free)))
#define autofclose __attribute__((cleanup(cleanup_fclose)))

static inline void cleanup_free(void* p) {
  free(*(void**)p);
}

static inline void cleanup_fclose(FILE** stream) {
  if (*stream)
    fclose(*stream);
}

static char* read_proc_cmdline(void) {
  autofclose FILE* f = fopen("/proc/cmdline", "r");
  if (!f) {
    perror("Failed to open /proc/cmdline");
    return NULL;
  }

  char* cmdline = NULL;
  size_t len;

  /* Note that /proc/cmdline will not end in a newline, so getline
   * will fail unelss we provide a length.
   */
  if (getline(&cmdline, &len, f) < 0) {
    perror("Failed to read /proc/cmdline");
    return NULL;
  }

  /* ... but the length will be the size of the malloc buffer, not
   * strlen().  Fix that.
   */
  len = strlen(cmdline);
  if (cmdline[len - 1] == '\n')
    cmdline[len - 1] = '\0';

  return cmdline;
}

static char* find_proc_cmdline_key(const char* cmdline, const char* key) {
  const size_t key_len = strlen(key);
  for (const char* iter = cmdline; iter;) {
    const char* next = strchr(iter, ' ');
    if (strncmp(iter, key, key_len) == 0 && iter[key_len] == '=') {
      const char* start = iter + key_len + 1;
      if (next)
        return strndup(start, next - start);

      return strdup(start);
    }

    if (next)
      next += strspn(next, " ");

    iter = next;
  }

  return NULL;
}

int main(void) {
  const autofree char* cmdline = read_proc_cmdline();
  if (cmdline == NULL)
    return EXIT_FAILURE;

  autofree char* device = find_proc_cmdline_key(cmdline, "mount-sysroot.root");
  if (device == NULL)
    device = find_proc_cmdline_key(cmdline, "root");

  if (device == NULL) {
    perror("No mount-sysroot.root= or root= karg present in /proc/cmdline");
    return EXIT_FAILURE;
  }

  autofree char* fstype =
      find_proc_cmdline_key(cmdline, "mount-sysroot.rootfstype");
  if (fstype == NULL)
    fstype = find_proc_cmdline_key(fstype, "rootfstype");

  if (strncmp(device, "PARTUUID=", 9) == 0 ||
      strncmp(device, "PARTLABEL=", 10) == 0) {
    device = blkid_get_devname(NULL, device, NULL);
    if (device == NULL) {
      perror("Error getting device name");
      return EXIT_FAILURE;
    }
  }

  if (mount(device, "/sysroot", fstype, 0, NULL) == -1) {
    perror("Error mounting device");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
