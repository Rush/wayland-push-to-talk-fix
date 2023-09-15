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
#include <iostream>

/* See https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h */
#define PTT_EV_KEY_CODE KEY_LEFTMETA
/*
 * Full list (Ignore leading XKB_KEY_):
 * https://github.com/xkbcommon/libxkbcommon/blob/master/include/xkbcommon/xkbcommon-keysyms.h
 */
#define PTT_XKEY_EVENT "Super_L"


int get_ev_key_code() {
    if (const char *ev_key_code_env = std::getenv("EV_KEY_CODE"))
        return libevdev_event_code_from_name(EV_KEY, ev_key_code_env);
    return PTT_EV_KEY_CODE;
}

const char *get_xkey_event() {
    if (const char *xkey_event_env = std::getenv("XKEY_EVENT"))
        return xkey_event_env;
    return PTT_XKEY_EVENT;
}

int main(int argc, char **argv) {
    struct libevdev *dev = NULL;
    xdo_t *xdo;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s /dev/input/by-id/<device-name>\n", argv[0]);
        exit(0);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        if (getuid() != 0)
            fprintf(stderr, "Fix permissions to %s or run as root\n", argv[1]);
        exit(1);
    }
    int rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0) {
        fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
        exit(1);
    }
    fprintf(stderr, "Input device name: \"%s\"\n", libevdev_get_name(dev));
    fprintf(stderr, "Input device ID: bus %#x vendor %#x product %#x\n",
            libevdev_get_id_bustype(dev),
            libevdev_get_id_vendor(dev),
            libevdev_get_id_product(dev));


    const int ptt_ev_key_code = get_ev_key_code();
    const char *ptt_xkey_event = get_xkey_event();

    bool skip_event_check = false;
    if (const char *env_skip_event_check = std::getenv("SKIP_EVENT_CHECK")) {
        skip_event_check = strcmp(env_skip_event_check, "1") == 0 || strcmp(env_skip_event_check, "true") == 0;
    }

    if (!skip_event_check && !libevdev_has_event_code(dev, EV_KEY, ptt_ev_key_code)) {
        fprintf(stderr, "This device is not capable of sending this key code\n");
        exit(1);
    }

    xdo = xdo_new(NULL);
    if (xdo == NULL) {
        fprintf(stderr, "Failed to initialize xdo lib\n");
        exit(1);
    }

    do {
        struct input_event ev;

        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc != LIBEVDEV_READ_STATUS_SUCCESS)
            continue;

        if (ev.type == EV_KEY && ev.code == ptt_ev_key_code && ev.value != 2) {
            if (ev.value == 1)
                xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, ptt_xkey_event, 0);
            else
                xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, ptt_xkey_event, 0);
        }
    } while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == -EAGAIN);

    xdo_free(xdo);
    libevdev_free(dev);
    close(fd);

    return 0;
}
