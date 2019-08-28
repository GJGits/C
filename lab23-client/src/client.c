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
#include "../headers/gj_client.h"

#define FILE_PATH "../local-storage/"
#define SEND_BUF_SIZE 1024

char *prog_name; // per evitare errori di compilazione

void readAndStore(int connSock, const char *fileName);

int main(int argc, char const *argv[])
{
    int sockfd;

    checkArgc(argc, "Usage: <server address> <server port> <fileName1> ... <fileNameN>\n");
    sockfd = connectTcpClient(argv[1],argv[2]);

    for(int i = 3; i < argc; i++) {
        int reqSize = 6 + strlen(argv[i]);
        char fileName[strlen(FILE_PATH) + reqSize - 5];
        memset(fileName, ' ', strlen(FILE_PATH) + reqSize - 5);
        memcpy(fileName, FILE_PATH, strlen(FILE_PATH));
        memcpy(fileName + strlen(FILE_PATH), argv[i], reqSize - 6);
        fileName[strlen(FILE_PATH) + reqSize - 6] = '\0';
        char request[reqSize];
        memset(request, ' ', reqSize);
        request[reqSize] = '\0';
        memcpy(request, "GET ", 4);
        memcpy(request + 4, argv[i], reqSize - 6);
        memcpy(request + 4 + reqSize - 6, "\r\n", 2);
        printf("Request[sending]: " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET, request);
        Send(sockfd, request, reqSize, 0);
        readAndStore(sockfd, argv[i]);
    }




    return 0;
}


/**
 * Funzione che legge la risposta del server e nel caso positivo memorizza
 * la risposta nel file system.
 */
void readAndStore(int connSock, const char *fileName) {

    // read response
    char response[6];
    memset(response, '\0', 6);
    response[5] = '\0';
    ssize_t bytes_read = Recv(connSock, response, 5, 0);

    if(strcmp(response, "+OK\r\n") == 0) {
        printf("Request[receiving]: " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET, response);
        uint32_t fileSize = 0;
        bytes_read = Recv(connSock, &fileSize, sizeof(fileSize), 0);
        fileSize = ntohl(fileSize);
        printf("Request[receiving]: " ANSI_COLOR_CYAN "FILE SIZE (%d bytes)" ANSI_COLOR_RESET "\n", fileSize);
        if(fileSize > 0) {
            bytes_read = 0;
            FILE *fptr;
            fptr = fopen(fileName,"ab+");
            while (bytes_read < fileSize) {
                int bufSize = (fileSize - bytes_read) >= SEND_BUF_SIZE ? SEND_BUF_SIZE : (fileSize - bytes_read);
                char sendBuf[bufSize];
                bytes_read += Recv(connSock, sendBuf, bufSize, 0);
                int progress = ((double) bytes_read / (double) fileSize) * 100;
                printf("\rRequest[receiving]: " ANSI_COLOR_CYAN "%d bytes (%d%%)" ANSI_COLOR_RESET, (int) bytes_read, progress);
                fflush(stdout);
                fwrite(sendBuf, bufSize, 1, fptr);
            }
            printf("\nRequest[writing]: " ANSI_COLOR_CYAN "WROTE (%d bytes)" ANSI_COLOR_RESET "\n", (int) bytes_read);
            fclose(fptr);
            uint32_t timestamp = 0;
            bytes_read = Recv(connSock, &timestamp, sizeof(timestamp), 0);
            timestamp = ntohl(timestamp);
            printf("Request[receiving]: " ANSI_COLOR_CYAN "TIMESTAMP (%d)" ANSI_COLOR_RESET "\n", (int)timestamp);
        }
    } else {
        printf("%s\n", response);
    }

}

