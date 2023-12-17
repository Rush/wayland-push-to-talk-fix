#include <libevdev/libevdev.h>
#include <xdo.h>

#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  const char *keycode = "KEY_LEFTMETA";
  const char *keyname = "Super_L";
  struct libevdev *dev = NULL;
  unsigned char verbose = 0;
  struct input_event ev;
  int ev_keycode;
  int button = 0;
  xdo_t *xdo;
  int opt;
  int fd;
  int rc;

  while ((opt = getopt(argc, argv, "vk:n:")) != -1) {
    switch (opt) {
      case 'v':
        verbose = 1;
        break;
      case 'k':
        keycode = optarg;
        break;
      case 'n':
        if (optarg && strlen(optarg) >= 5 && !strncmp(optarg, "MOUSE", 5)) {
          button = strtol((optarg + 5), NULL, 10);

          if (errno) {
            perror("strtol");
            exit(EXIT_FAILURE);
          }
        }

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

  fd = open(argv[optind], O_RDONLY);
  if (fd < 0) {
    perror("Failed to open device");
    if (getuid() != 0)
      fprintf(stderr, "Fix permissions to %s or run as root\n", argv[1]);
    exit(1);
  }

  rc = libevdev_new_from_fd(fd, &dev);
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

  ev_keycode = libevdev_event_code_from_name(EV_KEY, keycode);
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
  if (!xdo) {
    fprintf(stderr, "Failed to initialize xdo lib\n");
    exit(1);
  }

  if (verbose) {
    fprintf(stderr, "Listening for code %s, sending %s\n", libevdev_event_code_get_name(EV_KEY, ev_keycode), keyname);
  }

  do {
    rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc != LIBEVDEV_READ_STATUS_SUCCESS)
      continue;

    if (ev.type == EV_KEY && ev.code == ev_keycode && ev.value != 2) {
      if (verbose)
        fprintf(stderr, "key %s\n", ev.value ? "up" : "down");

      if (ev.value == 1) {
        if (!button)
          xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, keyname, 0);
        else
          xdo_mouse_down(xdo, CURRENTWINDOW, button);
      } else {
        if (!button)
          xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, keyname, 0);
        else
          xdo_mouse_up(xdo, CURRENTWINDOW, button);
      }
    }
  } while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == -EAGAIN);

  xdo_free(xdo);
  libevdev_free(dev);
  close(fd);

  return 0;
}
