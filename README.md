# wayland-push-to-talk-fix
This fixes the inability to use push to talk in Discord when running Wayland


**NOTE: by default the left Meta (Windows) key is used for push to talk. In order to use a different key, change values for `PTT_EV_KEY_CODE` and `PTT_XKEY_CODE` in file `push-to-talk.c`.**

## Requirements

Nothing special.
- C++ compiler & Make
- libevdev
- libxdo (Debian/Ubuntu: `libxdo-dev`, Fedora/Centos: `libxdo-devel`)

## Approach

Read specific key events via evdev (needs sudo) and then pass them to libxdo to inject key presses to X apps.

# Installation

Optimally we would install this as a user systemctl service (contributions welcome) but for now you will need to build this and run in the background with the `&` background operator.

```
make
sudo ./push-to-talk /dev/input/by-id/<device-name> &
```

# License

MIT
