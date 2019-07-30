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

void readAndStore(int connSock);

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

    Connect(sockfd, (const struct sockaddr*)&server_address, sizeof(server_address));

    for(int i = 3; i < argc; i++) {
        char request[6 + strlen(argv[i])];
        memcpy(request, "GET ", 4);
        memcpy(request + 4, argv[i], strlen(argv[i]));
        memcpy(request + 4 + strlen(argv[i]), "\r\n", 2);
        Send(sockfd, request, strlen(request), 0);
        readAndStore(sockfd);
    }

    ssize_t bytes_sent = 0;

    return 0;
}

/**
 * Funziona che legge la risposta del server e nel caso positivo memorizza
 * la risposta nel file system.
 */
void readAndStore(int connSock) {

}

