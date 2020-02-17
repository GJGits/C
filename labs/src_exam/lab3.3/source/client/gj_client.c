#include "../gj_client.h"
#include "../gj_tools.h"
#include <unistd.h>
#include <rpc/xdr.h>

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

/**
 * Singola interazione client server
 * */
void doClient(int connSock, char *request) {
        clientSend(connSock, request);
        clientReceive(connSock, request);
}

void clientSend(int connSock, char *request) {
    // TODO: INSERIRE LOGICA SEND QUI
    send(connSock, request, 50, 0);
}

void clientReceive(int connSock, char *request) {
    // TODO: INSERIRE LOGICA RECEIVE QUI
    XDR xdrs; /* pointer to XDR stream */
    char response[10];
    u_int result = 0;
    ssize_t rec =  recv(connSock, response, 10, 0);
    printf("recv: %d\n", (int)rec);
    xdrmem_create(&xdrs, response, 10, XDR_DECODE);
    if(!xdr_u_int(&xdrs, &result)) {
        printf("error decoding");
    }
    printf("result: %d\n", result);
    xdr_destroy(&xdrs);
}
