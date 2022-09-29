#!/bin/bash
exec ./push-to-talk /dev/input/event2 | while read line; do xdotool $line;done
