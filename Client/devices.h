#pragma once

#include <libevdev-1.0/libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>

struct libevdev_uinput* create_mouse();
struct libevdev_uinput* create_touchpad();
struct libebdev_uinput* create_keyboard();