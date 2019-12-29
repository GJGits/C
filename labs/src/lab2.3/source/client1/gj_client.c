#include "../../../commons/gj_client.h"
#include "../../../commons/gj_tools.h"
#include <unistd.h>

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
void doClient(int connSock, const char *request) {

        clientSend(connSock, request);
        clientReceive(connSock, request);
}

void clientSend(int connSock, const char *request) {
    // TODO: INSERIRE LOGICA SEND QUI
    // 1. Creo richiesta
    char richiesta[4 + strlen(request) + 3];
    memset(richiesta, '\0', 4 + strlen(request) + 3);
    memcpy(richiesta, "GET ", 4);
    memcpy(richiesta + 4, request, strlen(request));
    memcpy(richiesta + 4 + strlen(request), "\r\n", 2);
   
    // 2. Invio richiesta
     Send(connSock, richiesta, (4 + strlen(request) + 2),  0);
     printf("Client[request]: " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", richiesta);
}

void clientReceive(int connSock, const char *request) {
    // TODO: INSERIRE LOGICA RECEIVE QUI

       // 1. leggo tipo di risposta e se ok procedo
       char resp_type[6] = " " ;
       Recv(connSock, resp_type, 6, 0);
       printf("Response: %s\n", resp_type);
       if (strcmp(resp_type, "+OK\r\n") == 0) {
          // 2. leggo lunghezza file
          printf("Client[receive]: " ANSI_COLOR_CYAN "%s +OK" ANSI_COLOR_RESET "\n", request);
          uint32_t f_size;
          Recv(connSock, &f_size, 4, 0);
          f_size = ntohl(f_size);
          printf("Client[receive]: " ANSI_COLOR_CYAN "%s %d bytes" ANSI_COLOR_RESET "\n", request, (int) f_size);
          // 3. creo il file
          FILE *fp = fopen(request, "ab"); 
          if (fp != NULL) {
              ssize_t read = 0;
              while (read < f_size) {
                ssize_t r_size = (f_size - read) > 1000 ? 1000 : (f_size - read);
                char r_buf[r_size];
                read += Recv(connSock, r_buf, r_size, 0);
                showProgress((int) read, (int)f_size, "Client[writing]: ");
                fwrite(r_buf, r_size, 1, fp); 
              }
              printf("\n");
              fclose(fp);
              // 4. leggo timestamp
              uint32_t timestamp;
              Recv(connSock, &timestamp, 4, 0);
              timestamp = ntohl(timestamp);
              printf("Client[receive]: " ANSI_COLOR_CYAN "TIMESTAMP: %d" ANSI_COLOR_RESET "\n", (int)timestamp);
          } else {
            printf("Client[error]: " ANSI_COLOR_RED "ERRORE NELLA CREAZIONE DEL FILE " ANSI_COLOR_RESET "\n");
            return;
          }
       } else {
           printf("Client[request]: " ANSI_COLOR_RED "ERROR RESPONSE FROM SERVER" ANSI_COLOR_RESET "\n");
       }
       
}
