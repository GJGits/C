
#include <unistd.h>
#include <sys/sendfile.h>
#include "../gj_tools.h"
#include "../gj_server.h"

const int REQ_BUF_SIZE = 10000; // da specifica

int startUdpServer(const char *port) {
    uint16_t i_port;
    int sockfd;

    sockfd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    parsePort(port, &i_port);
    bindToAny(sockfd, i_port);
    
    return sockfd;
}

int startTcpServer(const char* port) {

    uint16_t i_port;
    int sockfd;

    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    parsePort(port, &i_port);
    bindToAny(sockfd, i_port);
    Listen(sockfd, 516);
    printf("Server[listening]: " ANSI_COLOR_GREEN "PORT = %d, BACKLOG = 516" ANSI_COLOR_RESET "\n", (int) i_port);

    return sockfd;
}

void runUdpServer(int sockfd) {

    while(1) {
       printf("Server[accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A DATAGRAM..." ANSI_COLOR_RESET "\n"); 
       doUdpJob(sockfd);
    }

}

void runIterativeTcpInstance(int passiveSock, const char *string) {

    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    while(1) {
        printf("Server[accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n");
        int connSock = Accept(passiveSock , (struct sockaddr *) &cli_addr, &addr_len);
        doTcpJob(connSock, string);
        close(connSock);
    }

}

void doUdpJob(int sockfd) {
    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    uint32_t buffer;
    doUdpReceive(sockfd, &cli_addr, &addr_len, &buffer);
    doUdpSend(sockfd, &cli_addr, &addr_len, &buffer);
}

void doUdpReceive(int sockfd, struct sockaddr_in *cli_addr, socklen_t *addr_len, u_int32_t *buffer) {
    recvfrom(sockfd, buffer, sizeof(u_int32_t), 
            0, (struct sockaddr *) cli_addr, addr_len);
}

void doUdpSend(int sockfd, struct sockaddr_in *cli_addr, socklen_t *addr_len, u_int32_t *buffer) {
    uint32_t send_buf[2];
    send_buf[0] = *buffer;
    send_buf[1] = htonl(time(NULL));
    sendto(sockfd, send_buf, (2 * sizeof(uint32_t)), 
                0, (struct sockaddr *) cli_addr, *addr_len);
}

void doTcpJob(int connSock, const char *string) {
    //TODO: uscire in caso di errore di una delle due fasi
}

/**
 * Funzione che ha lo scopo di ricevere una richiesta da un client,
 * validarla ed in seguito:
 *  - memorizzarla in un buffer in caso di richiesta valida
 *  - restituire un codice di errore in caso contrario
 * */
 
int doTcpReceive(int connSock, char *request, const char *string) {
    return 1;
}

void doTcpSend(int connSock, char *request) {
    // TODO: INSERIRE LOGICA SEND QUI
}

int reqCompleted(char *request) {
    int len = strlen(request);
    return len > 6 && request[len - 2] == '\r' && request[len - 1] == '\n' ? 0 : -1;
}

int checkRequest(char *request) {
    int len = strlen(request);
    if (len > 6 && request[0] == 'G' && request[1] == 'E' && request[2] == 'T'
        && request[3] == ' ' && request[len - 2] == '\r' && request[len - 1] == '\n') {
            memcpy(request, request + 4, (len - 6)); // estraggo il nome del file dalla richiesta
            request[len - 6] = '\0';
            return access(request, F_OK); // verifico che il file esista nella cartella di lavoro
        }
    return -1;
}
