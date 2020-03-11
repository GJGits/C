
#include <unistd.h>
#include <sys/sendfile.h>
#include "../gj_tools.h"
#include "../gj_server.h"

const int REQ_BUF_SIZE = 262; // 6 (GET \r\n) + 256 (Max filename len in linux)

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

void runIterativeTcpInstance(int passiveSock) {

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
    printf("Server[connection]:" ANSI_COLOR_GREEN "STARTED A NEW ON CONNECTION (PID=%d)" ANSI_COLOR_RESET "\n", getpid());
    char *request = (char *) calloc(REQ_BUF_SIZE, sizeof(char)); 
    int status = 0;
    while((status = doTcpReceive(connSock, request)) == 0) {
        doTcpSend(connSock, request);
        free(request);
        request = (char *) calloc(REQ_BUF_SIZE, sizeof(char));
    }
    if (status == -1) {
        printf("Server[error]: " ANSI_COLOR_RED "INVALID REQUEST FROM CLIENT: %s" ANSI_COLOR_RESET "\n", request);
        char err_buff[7] = "-ERR\r\n";
        send(connSock, err_buff, 6, 0);
    }
    close(connSock);
    free(request);
}

/**
 * Funzione che ha lo scopo di ricevere una richiesta da un client,
 * validarla ed in seguito:
 *  - memorizzarla in un buffer in caso di richiesta valida
 *  - restituire un codice di errore in caso contrario
 * */
 
int doTcpReceive(int connSock, char *request) {
    
    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;
    if(select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1) {
        // TODO: INSERIRE LOGICA RECEIVE QUI
        ssize_t read = 0;
        while (reqCompleted(request) == -1) {
         ssize_t received = Recv(connSock, request, REQ_BUF_SIZE, 0); 
         read += received;
         if (read == 0)
            return 1; // se ricevo 0 il client ha chiuso la connessione oppure non ci sono più bytes da leggere
        }
        printf("Server[receive]: " ANSI_COLOR_CYAN "RECEIVED %s" ANSI_COLOR_RESET "\n", request);
        return checkRequest(request);
    }
    FD_CLR(connSock, &cset);
    // esco per timeout
    return 1;
}

void doTcpSend(int connSock, char *request) {
    // TODO: INSERIRE LOGICA SEND QUI

    
    int i = 0;
     char offset[10];
    while(request[i] != ' ') {
        offset[i] = request[i];
        i++;
    }
    uint32_t off = str_to_int(offset);
    uint32_t off_net = htonl(off);
    // 2. send file size
    FILE *fp = fopen(request + i + 1, "rb+");

    // qui il file dovrebbe esistere, ma potrebbe essere inacessibile o
    // l'apertura potrebbe fallire
    if (fp == NULL) {
        printf("SERVER[READING]" ANSI_COLOR_RED "CANNOT OPEN FILE %s " ANSI_COLOR_RESET "\n", request);
        char err_buff[7] = "-ERR\r\n";
        send(connSock, err_buff, 6, 0);
        return;
    }

    struct stat stat_buf;
    
    if (fstat(fileno(fp), &stat_buf) == 0) {
        
        long f_size = stat_buf.st_size;
        long f_time = stat_buf.st_mtime;
        uint32_t net_f_size = htonl(f_size);
        if (off > f_size) {
             printf("SERVER[READING]" ANSI_COLOR_RED "OFFSET TOO BIG " ANSI_COLOR_RESET "\n");
            char err_buff[7] = "-ERR\r\n";
            send(connSock, err_buff, 6, 0);
            return;
        }
        
        // 1. send ok message
        char ok_msg[6] = "+OK\r\n";
        send(connSock, ok_msg, 5, 0);
        send(connSock, &net_f_size, 4, 0);
        send(connSock, &off, 4, 0);
        fseek(fp, off, SEEK_SET);
        // 3. send file content
        ssize_t sent = 0;
        while (sent < f_size) {
            sent += sendfile(connSock, fileno(fp), NULL, f_size - off);
            showProgress((int)sent, (int)f_size, "Server[sending]: ");
            printf("\n");
        }

        fclose(fp);
        // 4. send file timestamp
        uint32_t net_f_time = htonl(f_time);
        send(connSock, &net_f_time, 4, 0); 

    } else {
        char err_buff[7] = "-ERR\r\n";
        send(connSock, err_buff, 6, 0);
        return;
    }


}

int reqCompleted(char *request) {
    int len = strlen(request);
    return len > 6 && request[len - 2] == '\r' && request[len - 1] == '\n' ? 0 : -1;
}

int checkRequest(char *request) {
    int len = strlen(request);
    if (len > 6 && request[0] == 'G' && request[1] == 'E' && request[2] == 'T'
        && request[3] == ' ' && request[len - 2] == '\r' && request[len - 1] == '\n') {
            char offset[10];
            int i = 4;
            // leggo offset
            while (request[i] != ' ') {
                offset[i - 4] = request[i];
                i++;
            }
            while (request[i] != '\r') {
                i++;
                if (request[i] == '\n') {
                    request[i] = '\0';
                    break;
                }
            }
            int req_len = strlen(request);
            offset[i - 4] = '\0';
            int file_off = strlen(offset);
            printf("request: %s\n", request);
            memcpy(request, request + 4, req_len - 4);
            request[req_len - 4] = '\0';
            printf("request: %s\n", request);
            int acc = access(request + strlen(offset) + 1, F_OK);
            printf("acc: %d\n", acc);
            return  acc;// verifico che il file esista nella cartella di lavoro
        }
    return -1;
}