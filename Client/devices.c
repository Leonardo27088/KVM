#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/input-event-codes.h>

#include "devices.h"

void check(int i) {
    if (i < 0) {
        printf("%s\n", strerror(-i));
        exit(1);
    }
}

struct libevdev_uinput* create_mouse() {
    struct input_absinfo abs_x = {
        .value = 0,
        .minimum = 0,
        .maximum = 1600,
        .fuzz = 0,
        .flat = 0,
        .resolution = 1
    };

    struct input_absinfo abs_y = {
        .value = 0,
        .minimum = 0,
        .maximum = 900,
        .fuzz = 0,
        .flat = 0,
        .resolution = 1
    };

    struct libevdev* evdev = libevdev_new();
    libevdev_set_name(evdev, "KVM Virtual Mouse");
    libevdev_set_id_vendor(evdev, 0x01);
    libevdev_set_id_product(evdev, 0x01);
    libevdev_set_id_version(evdev, 0x01);
    libevdev_set_id_bustype(evdev, BUS_USB);

    check(libevdev_enable_property(evdev, INPUT_PROP_POINTER));

    check(libevdev_enable_event_type(evdev, EV_ABS));
    check(libevdev_enable_event_code(evdev, EV_ABS, ABS_X, &abs_x));
    check(libevdev_enable_event_code(evdev, EV_ABS, ABS_Y, &abs_y));

    check(libevdev_enable_event_type(evdev, EV_KEY));
    check(libevdev_enable_event_code(evdev, EV_KEY, BTN_LEFT, NULL));
    check(libevdev_enable_event_code(evdev, EV_KEY, BTN_RIGHT, NULL));
    check(libevdev_enable_event_code(evdev, EV_KEY, BTN_MIDDLE, NULL));
    check(libevdev_enable_event_code(evdev, EV_KEY, BTN_TOUCH, NULL));

    check(libevdev_enable_event_type(evdev, EV_REL));
    check(libevdev_enable_event_code(evdev, EV_REL, REL_WHEEL, NULL));

    check(libevdev_enable_event_type(evdev, EV_SYN));
    check(libevdev_enable_event_code(evdev, EV_SYN, SYN_REPORT, 0));

    struct libevdev_uinput* uinput;
    check(libevdev_uinput_create_from_device(evdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uinput));

    libevdev_free(evdev);
    return uinput;
}

struct libevdev_uinput* create_touchpad() {
    struct input_absinfo mt_slot  = {
        .minimum = 0,
        .maximum = 4 
    };

    struct input_absinfo mt_pos_x = {
        .minimum = 0,
        .maximum = 4096,
        .resolution = 40 
    };

    struct input_absinfo mt_pos_y = {
        .minimum = 0,
        .maximum = 4096,
        .resolution = 40 
    };

    struct libevdev* evdev_tp = libevdev_new();
    libevdev_set_name(evdev_tp, "KVM Virtual Touchpad");
    libevdev_set_id_bustype(evdev_tp, BUS_USB);

    check(libevdev_enable_property(evdev_tp, INPUT_PROP_POINTER));
    check(libevdev_enable_property(evdev_tp, INPUT_PROP_BUTTONPAD));

    check(libevdev_enable_event_type(evdev_tp, EV_ABS));
    check(libevdev_enable_event_code(evdev_tp, EV_ABS, ABS_MT_SLOT, &mt_slot));
    check(libevdev_enable_event_code(evdev_tp, EV_ABS, ABS_MT_TRACKING_ID, &mt_slot));
    check(libevdev_enable_event_code(evdev_tp, EV_ABS, ABS_MT_POSITION_X, &mt_pos_x));
    check(libevdev_enable_event_code(evdev_tp, EV_ABS, ABS_MT_POSITION_Y, &mt_pos_y));
    check(libevdev_enable_event_code(evdev_tp, EV_ABS, ABS_X, &mt_pos_x));
    check(libevdev_enable_event_code(evdev_tp, EV_ABS, ABS_Y, &mt_pos_y));

    check(libevdev_enable_event_type(evdev_tp, EV_KEY));
    check(libevdev_enable_event_code(evdev_tp, EV_KEY, BTN_TOUCH, NULL));
    check(libevdev_enable_event_code(evdev_tp, EV_KEY, BTN_TOOL_FINGER, NULL));
    check(libevdev_enable_event_code(evdev_tp, EV_KEY, BTN_TOOL_TRIPLETAP, NULL));

    check(libevdev_enable_event_type(evdev_tp, EV_SYN));

    struct libevdev_uinput* uinput_tp;
    check(libevdev_uinput_create_from_device(evdev_tp, LIBEVDEV_UINPUT_OPEN_MANAGED, &uinput_tp));

    libevdev_free(evdev_tp);
    return uinput_tp;
}

struct libevdev_uinput* create_keyboard() {
    struct libevdev* evdev_kb = libevdev_new();
    libevdev_set_name(evdev_kb, "KVM Virtual Keyboard");
    libevdev_set_id_bustype(evdev_kb, BUS_USB);

    check(libevdev_enable_event_type(evdev_kb, EV_KEY));

    for (int key = KEY_ESC; key <= KEY_KPDOT; key++) {
        libevdev_enable_event_code(evdev_kb, EV_KEY, key, NULL);
    }

    check(libevdev_enable_event_code(evdev_kb, EV_KEY, KEY_LEFTMETA, NULL));
    check(libevdev_enable_event_code(evdev_kb, EV_KEY, KEY_LEFT, NULL));
    check(libevdev_enable_event_code(evdev_kb, EV_KEY, KEY_RIGHT, NULL));
    check(libevdev_enable_event_code(evdev_kb, EV_KEY, KEY_UP, NULL));
    check(libevdev_enable_event_code(evdev_kb, EV_KEY, KEY_DOWN, NULL));
    check(libevdev_enable_event_code(evdev_kb, EV_KEY, KEY_RIGHTALT, NULL));

    check(libevdev_enable_event_type(evdev_kb, EV_SYN));

    struct libevdev_uinput* uinput_kb;
    check(libevdev_uinput_create_from_device(evdev_kb, LIBEVDEV_UINPUT_OPEN_MANAGED, &uinput_kb));

    libevdev_free(evdev_kb);
    return uinput_kb;
}