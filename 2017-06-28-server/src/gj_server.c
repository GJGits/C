#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> 
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <sys/socket.h>

#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include "../../global-headers/gj_server.h"
#include "../../global-headers/gj_tools.h"
#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"

int startTcpServer(in_addr_t addr, const char* port) {

    uint16_t i_port;
    int sockfd;

    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    parsePort(port, &i_port);
    bindToAny(sockfd, i_port);
    Listen(sockfd, 516);
    printf("Server[listening]: " ANSI_COLOR_GREEN "PORT = %d, BACKLOG = 516" ANSI_COLOR_RESET "\n", (int) i_port);

    return sockfd;
}

void runTcpInstance(int passiveSock) {

    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    while(1) {
        printf("Server[accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n");
        int connSock = Accept(passiveSock , (struct sockaddr *) &cli_addr, &addr_len);
        doTcpJob(connSock);
        close(connSock);
    }

}

void doTcpJob(int connSock) {
    //TODO: uscire in caso di errore di una delle due fasi
    if(doTcpReceive(connSock) == 0)
        doTcpSend(connSock);
}

int doTcpReceive(int connSock) {
    // TODO: INSERIRE LOGICA RECEIVE QUI
    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;

    if(Select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1) {
        
        return 0;
    }

    return -1;
}

void doTcpSend(int connSock) {
    // TODO: INSERIRE LOGICA SEND QUI
}