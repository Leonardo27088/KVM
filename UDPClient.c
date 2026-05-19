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
    libevdev_set_name(evdev, "Virtual Mouse");
    libevdev_set_id_vendor(evdev, 0x01);
    libevdev_set_id_product(evdev, 0x01);
    libevdev_set_id_version(evdev, 0x01);
    libevdev_set_id_bustype(evdev, BUS_USB);

    check(libevdev_enable_event_type(evdev, EV_ABS));
    check(libevdev_enable_event_code(evdev, EV_ABS, ABS_X, &abs_x));
    check(libevdev_enable_event_code(evdev, EV_ABS, ABS_Y, &abs_y));
    check(libevdev_enable_event_code(evdev, EV_SYN, SYN_REPORT, 0));

    check(libevdev_enable_event_type(evdev, EV_KEY));
    check(libevdev_enable_event_code(evdev, EV_KEY, BTN_LEFT, NULL));
    check(libevdev_enable_event_code(evdev, EV_KEY, BTN_RIGHT, NULL));
    libevdev_enable_event_code(evdev, EV_KEY, BTN_TOUCH, NULL);

    struct libevdev_uinput* uinput;
    check(libevdev_uinput_create_from_device(evdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uinput));

    while (1) {
        int bytesRead = recvfrom(sockfd, &packet, sizeof(packet), 0, NULL, NULL);

        if (bytesRead == sizeof(packet)) {
            if (packet.type == 1) {
                int targetX = (int)(packet.normX * 1600);
                int targetY = (int)(packet.normY * 900);

                check(libevdev_uinput_write_event(uinput, EV_ABS, ABS_X, targetX));
                check(libevdev_uinput_write_event(uinput, EV_ABS, ABS_Y, targetY));
                check(libevdev_uinput_write_event(uinput, EV_SYN, SYN_REPORT, 0));
            }
        }
    }

    libevdev_uinput_destroy(uinput);
    close(sockfd);


    return 0;
}