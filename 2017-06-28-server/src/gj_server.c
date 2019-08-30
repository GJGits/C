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
#include <sys/sendfile.h>

#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include "../../global-headers/gj_server.h"
#include "../../global-headers/gj_tools.h"
#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"

#define MAX_FILENAME_LEN 100

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
    char filename[MAX_FILENAME_LEN] = "../local-storage/";
    if(doTcpReceive(connSock, filename) == 0)
        doTcpSend(connSock, filename);
}

int doTcpReceive(int connSock, char *request) {
    // TODO: INSERIRE LOGICA RECEIVE QUI
    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;

    if(Select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1) {
        
        uint8_t byte = 2;
        Recv(connSock, &byte, 1, 0);

        if(byte == 0) {
            
            uint16_t nameLen = 0;
            Recv(connSock, &nameLen, 2, 0);
            char fileName[nameLen];
            memset(fileName, ' ', nameLen);
            Recv(connSock, fileName, nameLen, 0);
            strcat(request, fileName);
            return 0;
        }

        if(byte == 2) {
            // QUIT ORDINATO
            printf("Server[accepting]: " ANSI_COLOR_GREEN "CONNECTION QUIT" ANSI_COLOR_RESET "\n");
            close(connSock);
            return -1;
        }

        close(connSock);
        printf("Server[request]: " ANSI_COLOR_RED "BAD REQUEST" ANSI_COLOR_RESET "\n");
        return -1;
    }

    close(connSock);
    printf("Server[request]: " ANSI_COLOR_RED "READ TIMEOUT" ANSI_COLOR_RESET "\n");
    return -1;
}

void doTcpSend(int connSock, char *request) {
    // TODO: INSERIRE LOGICA SEND QUI
    printf("Server[request]: " ANSI_COLOR_CYAN "REQUESTED %s" ANSI_COLOR_RESET "\n", request);
    FILE *fp;
    fp = fopen(request, "rb+");
    uint32_t fileInfo[2] = {0,0};
    if(openFile(fp, request, fileInfo) == 0) {
        Send(connSock, "1", 1, 0);
        uint32_t fsizeNetOrder = htonl(fileInfo[0]);
        uint32_t fstampNetOrder = htonl(fileInfo[1]);
        Send(connSock, &fsizeNetOrder, 4, 0);
        Send(connSock, &fstampNetOrder, 4, 0);
        ssize_t bytes_sent = 0;
        while(bytes_sent < fileInfo[0]) {
            bytes_sent += sendfile(connSock, fileno(fp), NULL, fileInfo[0]);
            showProgress((int)bytes_sent, (int)fileInfo[0], "Server[request]: ");
        }
        printf("\n");
        fclose(fp);
        send(connSock, &fileInfo[1], sizeof(uint32_t), 0);
        return;
    }
    Send(connSock, "3", 1, 0);
    close(connSock);
    printf("Server[request]: " ANSI_COLOR_RED "FAILED TO OPEN FILE (%s)" ANSI_COLOR_RESET "\n", request);
}