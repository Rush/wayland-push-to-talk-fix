#include <stdio.h>
#include <fcntl.h>
#include <libevdev/libevdev.h>
extern "C" {
  #include <xdo.h>
}
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


int main(int argc, char **argv)
{
  struct libevdev *dev = NULL;
  xdo_t *xdo;

  bool verbose = false;
  const char* keycode = "KEY_LEFTMETA";
  const char *keyname = "Super_L";

  int opt;
  while ((opt = getopt(argc, argv, "vk:n:")) != -1) {
    switch (opt) {
      case 'v':
        verbose = true;
        break;
      case 'k':
        keycode = optarg;
        break;
      case 'n':
        /*
         * Full list (Ignore leading XKB_KEY_):
         * https://github.com/xkbcommon/libxkbcommon/blob/master/include/xkbcommon/xkbcommon-keysyms.h
         */
        keyname = optarg;
        break;
      default:
        fprintf(stderr, "Usage: %s [-v] [-k keycode] [-n keyname] /dev/input/by-id/<device-name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Usage: %s [-v] [-k keycode] [-n keyname] /dev/input/by-id/<device-name>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int fd = open(argv[optind], O_RDONLY);
  if (fd < 0) {
    perror("Failed to open device");
    if (getuid() != 0)
      fprintf(stderr, "Fix permissions to %s or run as root\n", argv[1]);
    exit(1);
  }
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

  int ev_keycode = libevdev_event_code_from_name(EV_KEY, keycode);
  if (ev_keycode < 0) {
    fprintf(stderr, "Key code not found\n");
    fprintf(stderr, "see https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h\n");
    exit(1);
  }

  if (!libevdev_has_event_code(dev, EV_KEY, ev_keycode)) {
    fprintf(stderr, "This device is not capable of sending this key code\n");
    exit(1);
  }

  xdo = xdo_new(NULL);
  if (xdo == NULL) {
    fprintf(stderr, "Failed to initialize xdo lib\n");
    exit(1);
  }

  if (verbose) {
    fprintf(stderr, "Listening for code %s, sending %s\n", libevdev_event_code_get_name(EV_KEY, ev_keycode), keyname);
  }

  do {
    struct input_event ev;

    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc != LIBEVDEV_READ_STATUS_SUCCESS)
      continue;

    if (ev.type == EV_KEY && ev.code == ev_keycode && ev.value != 2) {
      if (verbose)
        fprintf(stderr, "key %s\n", ev.value ? "up" : "down");
      if (ev.value == 1)
        xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, keyname, 0);
      else
        xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, keyname, 0);
    }
  } while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == -EAGAIN);

  xdo_free(xdo);
  libevdev_free(dev);
  close(fd);

  return 0;
}