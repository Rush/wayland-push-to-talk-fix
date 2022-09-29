#include <stdio.h>
#include <fcntl.h>
#include <libevdev/libevdev.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv)
{
  struct libevdev *dev = NULL;
  if (argc < 2) {
    exit(0);
  }

  int fd = open(argv[1], O_RDONLY | O_NONBLOCK);
  int rc = libevdev_new_from_fd(fd, &dev);
  if (rc < 0)
  {
    fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
    exit(1);
  }
  fprintf(stderr, "Input device name: \"%s\"\n", libevdev_get_name(dev));
  fprintf(stderr, "Input device ID: bus %#x vendor %#x product %#x\n",
          libevdev_get_id_bustype(dev),
          libevdev_get_id_vendor(dev),
          libevdev_get_id_product(dev));

  if (!libevdev_has_event_code(dev, EV_KEY, KEY_LEFTMETA)) {
    printf("This device does not look like a keyboard\n");
    exit(1);
  }

  do {
    struct input_event ev;
    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc == 0) {
      if (ev.type == EV_KEY && ev.code == KEY_LEFTMETA && ev.value != 2) {
        printf("%s Super_L\n", ev.value == 1 ? "keydown" : "keyup");
        fflush(stdout);
      }
    }
  } while (rc == 1 || rc == 0 || rc == -EAGAIN);

  return 0;
}