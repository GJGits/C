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

    int numOfRequests = sizeof(requests) / sizeof(char);

    for(int i = 0; i < numOfRequests; i++) {
        printf("Client[request]: " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", requests[i]);
        clientSend(connSock, requests[i]);
        clientReceive(connSock, requests[i]);
    }

}

void clientSend(int connSock, const char *request) {
    // TODO: INSERIRE LOGICA SEND QUI
}

void clientReceive(int connSock, const char *request) {
    // TODO: INSERIRE LOGICA RECEIVE QUI
}
