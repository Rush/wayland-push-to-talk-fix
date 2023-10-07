# wayland-push-to-talk-fix
This fixes the inability to use push to talk in Discord when running Wayland


**NOTE: by default the left Meta (Windows) key is used for push to talk. In order to use a different key, see the configuration section below.**

## Requirements

Nothing special.
- C++ compiler & Make
- libevdev
- libxdo (Debian/Ubuntu: `libxdo-dev`, Fedora/Centos: `libxdo-devel`)

## Approach

Read specific key events via evdev (needs sudo) and then pass them to libxdo to inject key presses to X apps.

# Configuration
The command supports three command line args.
- `-v`: verbose mode, logs all keystrokes
- `-k`: keycode to listen for. [Full list](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h).
- `-n`: keycode to send to discord. [Full list](https://github.com/xkbcommon/libxkbcommon/blob/master/include/xkbcommon/xkbcommon-keysyms.h) (ignore leading `XKB_KEY_`).

# Installation

## Manual run

```
make
sudo ./push-to-talk /dev/input/by-id/<device-id> &
```

## Autostart

First edit the `push-to-talk.desktop` file and replace `/dev/input/by-id/<device-id>` with your device path. Then:
```
make
sudo make install

# to allow you access `/dev/input` devices without root
sudo usermod -aG input <your username>
```
Then just log out and log in. A process named `push-to-talk` should be running (visible in any process monitor).

# License

MIT
