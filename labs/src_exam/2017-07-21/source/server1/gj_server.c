
#include <unistd.h>
#include <sys/sendfile.h>
#include "../gj_tools.h"
#include "../gj_server.h"

const int REQ_BUF_SIZE = 10000; // da specifica

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

void runIterativeTcpInstance(int passiveSock, run_params *rp) {

    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    while(1) {
        printf("Server[accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n");
        int connSock = Accept(passiveSock , (struct sockaddr *) &cli_addr, &addr_len);
        doTcpJob(connSock, rp);
        close(connSock);
    }

}

void doTcpJob(int connSock, run_params *rp) {
    //TODO: uscire in caso di errore di una delle due fasi
    printf("Server[connection]:" ANSI_COLOR_GREEN "STARTED A NEW ON CONNECTION (PID=%d)" ANSI_COLOR_RESET "\n", getpid());
    client_req request = {0, 0, ""};
    doTcpReceive(connSock, &request, rp);
    if (request.status == -1) {
        printf("Server[error]: " ANSI_COLOR_RED "INVALID REQUEST FROM CLIENT" ANSI_COLOR_RESET "\n");
        char err_buff[7] = "-ERR\r\n";
        send(connSock, err_buff, 6, 0);
    }
}

/**
 * Funzione che ha lo scopo di ricevere una richiesta da un client,
 * validarla ed in seguito:
 *  - memorizzarla in un buffer in caso di richiesta valida
 *  - restituire un codice di errore in caso contrario
 * */
 
int doTcpReceive(int connSock, client_req *request, run_params *rp) {
    
    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;
    if(select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1) {
        // TODO: INSERIRE LOGICA RECEIVE QUI
        ssize_t read = 0;
        recv(connSock, &(*request).fsize, 4, 0);
        (*request).fsize = ntohl((*request).fsize);
        ssize_t sent = 0;
        while (read < (*request).fsize) {
            ssize_t read_temp = recv(connSock, (*request).fbuf, 1024, 0);
            read += read_temp;
            showProgress((int)read, (int)(*request).fsize, "Server[receiving]: ");
            printf("\n");
            for (int i = 0; i < read_temp; i++) {
                for (int j = 0; j < strlen((*rp).string); j++) {
                    if ((*request).fbuf[i] == (*rp).string[j]) {
                        (*request).fbuf[i] = '*';
                    }
                }
            }
            sent += send(connSock, (*request).fbuf, read_temp, 0);
            showProgress((int)sent, (int)(*request).fsize, "Server[sending]: ");
            printf("\n");
        }
    }
    FD_CLR(connSock, &cset);
    // esco per timeout
    return 1;
}

void doTcpSend(int connSock, client_req *request) {
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
