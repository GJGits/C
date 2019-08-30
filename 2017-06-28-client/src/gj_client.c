#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <inttypes.h>

#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include "../../global-headers/gj_client.h"
#include "../../global-headers/gj_tools.h"

#define READ_BUF_SIZE 1024

/**
 * Setta l'ip nella struttura adibita a 
 * conservare le credenziali del server. Genera un messaggio 
 * d'errore in caso l'indirizzo non sia valido
 */
void setAddress(const char *ip, struct sockaddr_in *address) {
    
    struct hostent *he;
    struct in_addr **addr_list;

    if ((he = gethostbyname(ip)) != NULL) {
        addr_list = (struct in_addr **) he->h_addr_list;
        // Return the first one
        if (addr_list[0] != NULL) {
            int length = strlen(inet_ntoa(*addr_list[0]));
            char ip_addr[length];
            memset(ip_addr,0,length);
            strcpy(ip_addr, inet_ntoa(*addr_list[0]));
            if(!inet_pton(AF_INET, ip_addr, address)) {
                printf("Server[address]: " ANSI_COLOR_RED "INVALID ADDRESS (%s)" ANSI_COLOR_RESET "\n", ip_addr);
                err_quit("");
            }
            return;
        }
    }
    
    if(!inet_pton(AF_INET, ip, address)) {
        printf("Server[address]: " ANSI_COLOR_RED "INVALID ADDRESS (%s)" ANSI_COLOR_RESET "\n", ip);
        err_quit("");
    }

}

/**
 * Crea una connessione verso il server e
 * restituisce il socket connesso
 * */
 
int connectTcpClient(const char *address, const char *char_port) {
    
    uint16_t port;
    struct sockaddr_in server_address;
    int sockfd;

    memset(&server_address, 0, sizeof(server_address)); 
    setAddress(address, &server_address);
    printf("Server[address]: " ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n", address);

    parsePort(char_port, &port);
    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    Connect(sockfd, (const struct sockaddr*) &server_address, sizeof(server_address));

    return sockfd;

}

void doClient(int connSock, const char **requests) {

    // valgrind immette 7 parametri in più 
    int numOfRequests = (sizeof(requests) / sizeof(char)) - 7;
    for(int i = 0; i < numOfRequests; i++) {
        printf("Client[request]: " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", requests[i]);
        clientSend(connSock, requests[i]);
        clientReceive(connSock, requests[i]);
    }

}

void clientSend(int connSock, const char *request) {
    // TODO: INSERIRE LOGICA SEND QUI
    uint8_t byte = 0;
    Send(connSock, &byte, 1, 0); // -1: SEND GET MESSAGE
    u_int16_t fileNameLen = htons((u_int16_t)strlen(request)); 
    Send(connSock, &fileNameLen, 2, 0); // -2: SEND FILE NAME LENGTH
    char fileName[fileNameLen];
    memset(fileName, ' ', fileNameLen);
    strcpy(fileName, request);
    Send(connSock, fileName, fileNameLen, 0); // -3: SEND FILE NAME
}

void clientReceive(int connSock, const char *request) {
    // TODO: INSERIRE LOGICA RECEIVE QUI
    char respStatus;
    Recv(connSock, &respStatus, 1, 0);
    
    if(respStatus == '1') {
        
        u_int32_t fileSize = 0;
        Recv(connSock, &fileSize, 4, 0);
        fileSize = ntohl(fileSize);
        ssize_t bytesRead = 0;
        FILE *fp;
        fp = fopen(request, "ab+");
        
        while(bytesRead < fileSize) {
            ssize_t remaining = fileSize - bytesRead;
            ssize_t toRead = (remaining < READ_BUF_SIZE) ? remaining : READ_BUF_SIZE; 
            char readBuf[toRead];
            bytesRead += Recv(connSock, readBuf, toRead, 0);
            showProgress((int) bytesRead, (int) fileSize, "Request[receiving]: ");
            fwrite(readBuf, toRead, 1, fp);
        }
        
        printf("\nRequest[writing]: " ANSI_COLOR_CYAN "WROTE (%d bytes)" ANSI_COLOR_RESET "\n", (int) bytesRead);
        fclose(fp);

        uint32_t timestamp = 0;
        bytesRead += Recv(connSock, &timestamp, 4, 0);
        timestamp = ntohl(timestamp);
        printf("Request[receiving]: " ANSI_COLOR_CYAN "TIMESTAMP (%d)" ANSI_COLOR_RESET "\n", (int)timestamp);

    } else {
        printf("Client[request]: " ANSI_COLOR_RED "ERR" ANSI_COLOR_RESET "\n");
    }
}
