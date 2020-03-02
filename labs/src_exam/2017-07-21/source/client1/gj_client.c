#include "../gj_client.h"
#include "../gj_tools.h"
#include <unistd.h>
#include <sys/sendfile.h>

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
void doClient(int connSock, const char *request, const char *file_mod_name) {
        pid_t child_pid;
        FILE *fp = fopen(request, "rb+");
        if (fp == NULL) {
            printf("CLIENT[READING]" ANSI_COLOR_RED "CANNOT OPEN FILE %s " ANSI_COLOR_RESET "\n", request);
            close(connSock);
            return;
        }
        struct stat stat_buf;
        long f_size = 0;
        if (fstat(fileno(fp), &stat_buf) == 0) { 
            f_size = stat_buf.st_size;
        }
        if ((child_pid = fork()) < 0) { 
            // error on forking
            printf("CLIENT[FORKING]" ANSI_COLOR_RED "CANNOT FORK PROCESS " ANSI_COLOR_RESET "\n");
            close(connSock);
            return;
        } else if (child_pid > 0) {
            // parent process
            clientReceive(connSock, file_mod_name, f_size);
            waitpid(child_pid, NULL, 0);
        } else {
            // child process
            clientSend(connSock, request);
        }
        
        
        fclose(fp);
        close(connSock);
}

void clientSend(int connSock, const char *request) {
    // TODO: INSERIRE LOGICA SEND QUI
    FILE *fp = fopen(request, "rb+");

    // qui il file dovrebbe esistere, ma potrebbe essere inacessibile o
    // l'apertura potrebbe fallire
    if (fp == NULL) {
        printf("CLIENT[READING]" ANSI_COLOR_RED "CANNOT OPEN FILE %s " ANSI_COLOR_RESET "\n", request);
        //todo: close socket?
        return;
    }

    struct stat stat_buf;
    
    if (fstat(fileno(fp), &stat_buf) == 0) { 
        long f_size = stat_buf.st_size;
        uint32_t net_f_size = htonl(f_size);
        send(connSock, &net_f_size, 4, 0);
        ssize_t sent = 0;
        sent += sendfile(connSock, fileno(fp), NULL, f_size);
        showProgress((int)sent, (int)f_size, "Client[sending]: ");
        printf("\n");
        fclose(fp);
    }
}

void clientReceive(int connSock, const char *request, long f_size) {
    // TODO: INSERIRE LOGICA RECEIVE QUI
    printf("invoco receive");
    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;
    if(select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1) { 
       char buf[f_size];
       ssize_t read = 0;
       FILE *fp = fopen(request, "ab+");
       while (read < f_size) {
           ssize_t r_temp = recv(connSock, buf, f_size, 0);
           read += r_temp;
           showProgress((int)read, (int)f_size, "Client[receiving]: ");
           printf("\n");
       }
       printf("buf: %s\n", buf);
       fwrite(buf, f_size, 1, fp);
       fclose(fp);
    }
    FD_CLR(connSock, &cset);
}
