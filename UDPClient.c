#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>

#define PORT 27015
#define DEFAULT_BUFLEN 512

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    float normX;
    float normY;
    uint16_t code;
    int32_t value;
} MousePacket;
#pragma pack(pop)

MousePacket packet;

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    uint32_t code;
    int32_t value;
} KeyPacket;
#pragma pack(pop)

static void check(int i) {
    if (i < 0) {
        printf("%s\n", strerror(-i));
        exit(1);
    }
}

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

static int swipe_active = 0;
static int swipe_x1 = 1800, swipe_x2 = 2200;
static int swipe_y  = 2048;
static int swipe_base_x = -1;

static int vk_to_linux(uint32_t vk) {
    switch (vk) {
        case 0x08: return KEY_BACKSPACE;
        case 0x09: return KEY_TAB;
        case 0x0D: return KEY_ENTER;
        case 0x10: return KEY_LEFTSHIFT;
        case 0x11: return KEY_LEFTCTRL;
        case 0x12: return KEY_LEFTALT;
        case 0x13: return KEY_PAUSE;
        case 0x14: return KEY_CAPSLOCK;
        case 0x1B: return KEY_ESC;
        case 0x20: return KEY_SPACE;
        case 0x21: return KEY_PAGEUP;
        case 0x22: return KEY_PAGEDOWN;
        case 0x23: return KEY_END;
        case 0x24: return KEY_HOME;
        case 0x25: return KEY_LEFT;
        case 0x26: return KEY_UP;
        case 0x27: return KEY_RIGHT;
        case 0x28: return KEY_DOWN;
        case 0x2C: return KEY_SYSRQ;
        case 0x2D: return KEY_INSERT;
        case 0x2E: return KEY_DELETE;
        case 0x30: return KEY_0;
        case 0x31: return KEY_1;
        case 0x32: return KEY_2;
        case 0x33: return KEY_3;
        case 0x34: return KEY_4;
        case 0x35: return KEY_5;
        case 0x36: return KEY_6;
        case 0x37: return KEY_7;
        case 0x38: return KEY_8;
        case 0x39: return KEY_9;
        case 0x41: return KEY_A;
        case 0x42: return KEY_B;
        case 0x43: return KEY_C;
        case 0x44: return KEY_D;
        case 0x45: return KEY_E;
        case 0x46: return KEY_F;
        case 0x47: return KEY_G;
        case 0x48: return KEY_H;
        case 0x49: return KEY_I;
        case 0x4A: return KEY_J;
        case 0x4B: return KEY_K;
        case 0x4C: return KEY_L;
        case 0x4D: return KEY_M;
        case 0x4E: return KEY_N;
        case 0x4F: return KEY_O;
        case 0x50: return KEY_P;
        case 0x51: return KEY_Q;
        case 0x52: return KEY_R;
        case 0x53: return KEY_S;
        case 0x54: return KEY_T;
        case 0x55: return KEY_U;
        case 0x56: return KEY_V;
        case 0x57: return KEY_W;
        case 0x58: return KEY_X;
        case 0x59: return KEY_Y;
        case 0x5A: return KEY_Z;
        case 0x5B: return KEY_LEFTMETA;
        case 0x5C: return KEY_RIGHTMETA;
        case 0x60: return KEY_KP0;
        case 0x61: return KEY_KP1;
        case 0x62: return KEY_KP2;
        case 0x63: return KEY_KP3;
        case 0x64: return KEY_KP4;
        case 0x65: return KEY_KP5;
        case 0x66: return KEY_KP6;
        case 0x67: return KEY_KP7;
        case 0x68: return KEY_KP8;
        case 0x69: return KEY_KP9;
        case 0x6A: return KEY_KPASTERISK;
        case 0x6B: return KEY_KPPLUS;
        case 0x6D: return KEY_KPMINUS;
        case 0x6E: return KEY_KPDOT;
        case 0x6F: return KEY_KPSLASH;
        case 0x70: return KEY_F1;
        case 0x71: return KEY_F2;
        case 0x72: return KEY_F3;
        case 0x73: return KEY_F4;
        case 0x74: return KEY_F5;
        case 0x75: return KEY_F6;
        case 0x76: return KEY_F7;
        case 0x77: return KEY_F8;
        case 0x78: return KEY_F9;
        case 0x79: return KEY_F10;
        case 0x7A: return KEY_F11;
        case 0x7B: return KEY_F12;
        case 0x90: return KEY_NUMLOCK;
        case 0x91: return KEY_SCROLLLOCK;
        case 0xA0: return KEY_LEFTSHIFT;
        case 0xA1: return KEY_RIGHTSHIFT;
        case 0xA2: return KEY_LEFTCTRL;
        case 0xA3: return KEY_RIGHTCTRL;
        case 0xA4: return KEY_LEFTALT;
        case 0xA5: return KEY_RIGHTALT;
        case 0xBA: return KEY_SEMICOLON;
        case 0xBB: return KEY_EQUAL;
        case 0xBC: return KEY_COMMA;
        case 0xBD: return KEY_MINUS;
        case 0xBE: return KEY_DOT;
        case 0xBF: return KEY_SLASH;
        case 0xC0: return KEY_GRAVE;
        case 0xDB: return KEY_LEFTBRACE;
        case 0xDC: return KEY_BACKSLASH;
        case 0xDD: return KEY_RIGHTBRACE;
        case 0xDE: return KEY_APOSTROPHE;
        default: return -1;
    }
}

int main() {
    char buffer[DEFAULT_BUFLEN];
    char *message = "Hello server";

    int sockfd, n;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.249");
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect failed\n");
        exit(0);
    }

    sendto(sockfd, message, DEFAULT_BUFLEN, 0, (struct sockaddr*)NULL, sizeof(servaddr));
    printf("Hello Server sent\n");

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

    struct libevdev* evdev_kb = libevdev_new();
    libevdev_set_name(evdev_kb, "KVM Virtual Keyboard");
    libevdev_set_id_bustype(evdev_kb, BUS_USB);

    check(libevdev_enable_event_type(evdev_kb, EV_KEY));

    for (int key = KEY_ESC; key <= KEY_KPDOT; key++) {
        libevdev_enable_event_code(evdev_kb, EV_KEY, key, NULL);
    }

    check(libevdev_enable_event_type(evdev_kb, EV_SYN));

    struct libevdev_uinput* uinput_kb;
    check(libevdev_uinput_create_from_service(evdev_kb, LIBEVDEV_UINPUT_OPEN_MANAGED, &uinput_kb));

    libevdev_free(evdev_kb);

    while (1) {
        int bytesRead = recvfrom(sockfd, &packet, sizeof(packet), 0, NULL, NULL);

        uint8_t buf[32];
        int bytesRead = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
        if (bytesRead > 0) {
            uint8_t type = buf[0];
            if (type == 5) {
                KeyPacket* kpkt = (KeyPacket*)buf;
                int linuxKey = vk_to_linux(kpkt->vkCode);
                if (linuxKey >= 0) {
                    check(libevdev_uinput_write_event(uinput, EV_KEY, linuxKey, kpkt->value));
                    check(libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0));
                }
            } else if (bytesRead == sizeof(MousePacket)) {
                memcpy(&packet, buf, sizeof(packet));
                
                if (packet.type == 1) {
                    int targetX = (int)(packet.normX * 1600);
                    int targetY = (int)(packet.normY * 900);

                    check(libevdev_uinput_write_event(uinput, EV_ABS, ABS_X, targetX));
                    check(libevdev_uinput_write_event(uinput, EV_ABS, ABS_Y, targetY));
                    check(libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0));

                    if (swipe_active) {
                        if (swipe_base_x < 0) swipe_base_x = targetX;

                        int deltaX = targetX - swipe_base_x;
                        int cx1 = 1800 + deltaX * 2;
                        int cx2 = 2200 + deltaX * 2;

                        if (cx1 < 0) cx1 = 0; if (cx1 > 4096) cx1 = 4096;
                        if (cx2 < 0) cx2 = 0; if (cx2 > 4096) cx2 = 4096;

                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 0);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_X, 1600 + deltaX * 2);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 1);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_X, 2048 + deltaX * 2);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 2);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_X, 2496 + deltaX * 2);
                        libevdev_uinput_write_event(uinput_tp, EV_SYN, SYN_REPORT, 0);
                    }
                } else if (packet.type == 2) {
                    check(libevdev_uinput_write_event(uinput, EV_KEY, packet.code, packet.value));
                    check(libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0));

                } else if (packet.type == 3) {
                    check(libevdev_uinput_write_event(uinput, EV_REL, packet.code, packet.value));
                    check(libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0));

                } else if (packet.type == 4) {
                    if (packet.value == 1 && !swipe_active) {
                        swipe_active = 1;
                        swipe_base_x = -1;

                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 0);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_TRACKING_ID, 1);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_X, 1600);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_Y, swipe_y);

                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 1);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_TRACKING_ID, 2);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_X, 2048);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_Y, swipe_y);

                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 2);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_TRACKING_ID, 3);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_X, 2496);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_POSITION_Y, swipe_y);

                        libevdev_uinput_write_event(uinput_tp, EV_KEY, BTN_TOUCH, 1);
                        libevdev_uinput_write_event(uinput_tp, EV_KEY, packet.code, 1);
                        libevdev_uinput_write_event(uinput_tp, EV_SYN, SYN_REPORT, 0);
                    } else if (packet.value == 0 && swipe_active) {
                        swipe_active = 0;
                        swipe_base_x = -1;

                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 0);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_TRACKING_ID, -1);

                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 1);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_TRACKING_ID, -1);

                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_SLOT, 2);
                        libevdev_uinput_write_event(uinput_tp, EV_ABS, ABS_MT_TRACKING_ID, -1);

                        libevdev_uinput_write_event(uinput_tp, EV_KEY, BTN_TOUCH, 0);
                        libevdev_uinput_write_event(uinput_tp, EV_KEY, packet.code, 0);
                        libevdev_uinput_write_event(uinput_tp, EV_SYN, SYN_REPORT, 0);
                    }
                }
            }
        }
    }

    libevdev_uinput_destroy(uinput_tp);
    libevdev_uinput_destroy(uinput);
    close(sockfd);

    return 0;
}