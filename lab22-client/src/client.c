#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include<time.h>

#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"

char *prog_name; // per evitare errori di compilazione

int main(int argc, char const *argv[])
{
    // clear console (Linux only)
    printf("\e[1;1H\e[2J");
    uint16_t port;
    // client_address richiesto solo per configurazione docker
    struct sockaddr_in server_address, client_address; 
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

    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = htonl(INADDR_ANY);
    client_address.sin_port = htons(3000);

    if (bind(sockfd, (struct sockaddr *) &client_address, sizeof(client_address)) < 0) {
        perror("bind");
        exit(1);
    }

    ssize_t bytes_sent = 0;
    ssize_t bytes_recv = 0;
    socklen_t addr_len = 0;
    uint32_t send_buf = htonl(time(NULL));

    Sendto(sockfd, &send_buf, sizeof(send_buf), 
        MSG_CONFIRM, (const struct sockaddr*) &server_address, 
        sizeof(server_address));
    
    printf("Datagram[sending]: " ANSI_COLOR_GREEN "%hu" ANSI_COLOR_RESET "\n", send_buf);
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
            MSG_WAITALL, (struct sockaddr *)&server_address, 
            &addr_len);

        if (bytes_recv != 64) {
            printf("Datagram[receiving]: " ANSI_COLOR_RED "INVALID DATAGRAM\n" ANSI_COLOR_RESET "\n");
        } else {
            printf("Datagram[receiving]: " ANSI_COLOR_GREEN "%d %d\n" ANSI_COLOR_RESET "\n", 
                recv_buffer[0], recv_buffer[1]);
        }
    } else {
            printf("Datagram[receiving]: " ANSI_COLOR_RED "TIME OUT\n" ANSI_COLOR_RESET "\n");
    }

    return 0;
}