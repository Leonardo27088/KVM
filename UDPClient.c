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
        recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL);
        puts(buffer);
    }

    close(sockfd);

    return 0;
}