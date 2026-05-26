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

#include "keymap.h"
#include "protocolClient.h"
#include "devices.h"

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

    struct libevdev_uinput* uinput = create_mouse();
    struct libevdev_uinput* uinput_tp = create_touchpad();
    struct libevdev_uinput* uinput_kb = create_keyboard();

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