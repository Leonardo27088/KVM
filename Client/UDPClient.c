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

    while (1) {
        uint8_t buf[32];
        int bytesRead = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
        if (bytesRead > 0) {
            uint8_t type = buf[0];
            if (type == 5) {
                KeyPacket* kpkt = (KeyPacket*)buf;
                int linuxKey = vk_to_linux(kpkt->code);
                if (linuxKey >= 0) {
                    check(libevdev_uinput_write_event(uinput_kb, EV_KEY, linuxKey, kpkt->value));
                    check(libevdev_uinput_write_event(uinput_kb, EV_SYN, SYN_REPORT, 0));
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