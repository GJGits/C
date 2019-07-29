#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include <time.h> 
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include "../headers/request.h";

char *prog_name; // per evitare errori di compilazione
int main(int argc, char const *argv[])
{
    uint16_t port;
    int sockfd;
    struct sockaddr_in client_addr;
    
    parsePort(argv[1], &port);
    sockfd = (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bindToAny(sockfd, port);
    Listen(sockfd, 516);

    struct sockaddr_in cli_addr;
    socklen_t addr_len;

    while(1) {
        int connSock = Accept(sockfd, (struct sockaddr *) &cli_addr, &addr_len);
        processRequest(connSock);
    }

    Close(sockfd);

    return 0;
}

