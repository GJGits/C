#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include<time.h>
#include <inttypes.h>

#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"

char *prog_name; // per evitare errori di compilazione

int main(int argc, char const *argv[])
{
    uint16_t port;
    struct sockaddr_in server_address;
    int sockfd;

    memset(&server_address, 0, sizeof(server_address)); 
    if (!inet_pton(AF_INET, argv[1], &server_address)) {
        printf("Address: " ANSI_COLOR_RED "INVALID ADDRESS" ANSI_COLOR_RESET "\n");
        err_msg("");
    }
    
    printf("Address: " ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n", argv[1]);
    parsePort(argv[2], &port);
    sockfd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    ssize_t bytes_sent = 0;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    uint32_t send_buf = htonl(time(NULL));

    bytes_sent = Sendto(sockfd, &send_buf, sizeof(send_buf), 
        0, (const struct sockaddr*) &server_address, 
        addr_len);
    
    printf("Datagram[sending]: " ANSI_COLOR_GREEN "[%d] (%d bytes)" ANSI_COLOR_RESET "\n", (int) ntohl(send_buf), (int) bytes_sent);
    struct timeval tout;
    tout.tv_sec = 5;
    tout.tv_usec = 0;
    fd_set cset;
    FD_ZERO(&cset); 
    FD_SET(sockfd, &cset);
    int sele = 0;

    if ((sele = Select(FD_SETSIZE, &cset, NULL, NULL, &tout)) > 0) {

        ssize_t bytes_recv = 0;
        uint32_t recv_buffer[2];
        socklen_t addr_len = 0;
        bytes_recv =  Recvfrom(sockfd, 
            &recv_buffer, sizeof(recv_buffer), 
            0, (struct sockaddr *)&server_address, 
            &addr_len);
        if (bytes_recv != 8) {
            printf("Datagram[receiving]: " ANSI_COLOR_RED "INVALID DATAGRAM (%d bytes)\n" ANSI_COLOR_RESET, (int) bytes_recv);
        } else {
            printf("Datagram[receiving]: " ANSI_COLOR_CYAN "[%d][%d] (%d bytes)" ANSI_COLOR_RESET "\n", (int) ntohl(recv_buffer[0]), (int) ntohl(recv_buffer[1]), (int) bytes_recv);
        }
    } else {
            printf("Datagram[receiving]: " ANSI_COLOR_RED "TIME OUT\n" ANSI_COLOR_RESET "\n");
    }

    return 0;
}