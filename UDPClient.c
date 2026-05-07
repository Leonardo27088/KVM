#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 27015
#define DEFAULT_BUFLEN 512

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    float normX;
    float normY;
    uint8_t button;
} MousePacket;
#pragma pack(pop)

MousePacket packet;

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

    while (1) {
        int bytesRead = recvfrom(sockfd, &packet, sizeof(packet), 0, NULL, NULL);

        if (bytesRead == sizeof(packet)) {
            if (packet.type == 1) {
                int targetX = (int)(packet.normX * 1600);
                int targetY = (int)(packet.normY * 900);

                printf("Move to: %d, %d\n", targetX, targetY);
            }
        }
        // recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL);
        // puts(buffer);
    }

    close(sockfd);

    return 0;
}