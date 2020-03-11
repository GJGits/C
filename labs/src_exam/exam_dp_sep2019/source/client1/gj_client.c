#include "../gj_client.h"
#include "../gj_tools.h"

#include <unistd.h>
#include <sys/sendfile.h>
#include <regex.h>

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
void doClient(int connSock, cli_params *params) {
        
        for (int i = 3 ; i < params->len ; i++ ) {
            client_req req = {1, "GET ", ""};
            strcpy(req.filename, params->requests[i]);
            clientSend(connSock, params, &req);
            clientReceive(connSock, params, &req);
            if (req.status == - 1) {
                printf("Client[receive]:" ANSI_COLOR_RED "ERROR RESPONSE FROM SERVER" ANSI_COLOR_RESET "\n");
            }
        }
}

void clientSend(int connSock, cli_params *params, client_req *req) {
    // TODO: INSERIRE LOGICA SEND QUI
        char end[] = "\r\n";
        send(connSock, req->message, 4, 0);
        send(connSock, req->filename, strlen(req->filename), 0);
        send(connSock, end, 2, 0);
        printf("Client[sending]: " ANSI_COLOR_CYAN "%s%s%s" ANSI_COLOR_RESET, req->message, req->filename, end);
}

void clientReceive(int connSock, cli_params *params, client_req *req) {
    // TODO: INSERIRE LOGICA RECEIVE QUI
    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;
    if(select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1) { 
       recv(connSock, req->message, 5, 0);
       if (strcmp(req->message, "+OK\r\n") == 0) {
           printf("Client[receiving]: " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET , req->message);
           char dim[10] = "";
           for (int i = 0 ; i < 10; i++) {
              char c; 
              recv(connSock, &c, 1, 0); 
              dim[i] = c;
              regex_t re;
              const char pattern[] = "^[0-9]+\r\n$";
              if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) return;
              int status = regexec(&re, dim, 0, NULL, 0);
              regfree(&re);
              if (status == 0) {
                  printf("Client[receiving]: " ANSI_COLOR_CYAN "file lenght -> %s bytes" ANSI_COLOR_RESET "\r", dim);
                  break;
              }
              
           }
        int num = strtol(dim, (char **)NULL, 10); 
        ssize_t read = 0;
        char fbuf[num];
        FILE *fp = fopen(req->filename, "ab+");
        while (read < num) {
            int chunk = (num - read) > 1024 ? 1024 : (num - read); 
            ssize_t t_read = recv(connSock, fbuf, chunk, 0);
            read += t_read;
            fwrite(fbuf, (size_t)t_read, 1, fp);
            memset(fbuf, '\0', num);
        }
        fclose(fp);
        uint32_t time = 0;
        recv(connSock, &time, 4, 0);
        printf("Client[receiving]: " ANSI_COLOR_CYAN "file time -> %d" ANSI_COLOR_RESET "\n", (int)time);
        time = ntohl(time);   
       } else {
           req->status = -1;
           return;
       }
    }
    FD_CLR(connSock, &cset);
}
