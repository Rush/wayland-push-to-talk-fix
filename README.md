# wayland-push-to-talk-fix
This fixes the inability to use push to talk in Discord when running Wayland


* NOTE: by default the left Meta (Windows) key is used for push to talk. In order to use a different key, change the `push-to-talk.sh` and `push-to-talk.cpp` files accordingly.*

## Requirements

Nothing special.
- C++ compiler & Makefile
- libevdev
- `xdotool` shell tool
- bash

## Approach

Read specific key events via evdev (needs sudo) and then pass them to xdotool to inject key presses to all X apps.

# Installation

Optimally we would install this as a user systemctl service (contributions welcome) but for now you will need to build this and run in the background with the `&` background operator.

```
make
./push-to-talk.sh &
```

# License

MIT
