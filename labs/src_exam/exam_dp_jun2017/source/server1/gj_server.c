
#include "../gj_server.h"

#include <regex.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "../gj_tools.h"

const int REQ_BUF_SIZE = 10000; // da specifica

int startTcpServer(const char *port)
{
    uint16_t i_port;
    int sockfd;

    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    parsePort(port, &i_port);
    bindToAny(sockfd, i_port);
    Listen(sockfd, 516);
    printf("Server[listening]: " ANSI_COLOR_GREEN
           "PORT = %d, BACKLOG = 516" ANSI_COLOR_RESET "\n",
           (int)i_port);

    return sockfd;
}

void runIterativeTcpInstance(int passiveSock, run_params *rp)
{
    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    while (1)
    {
        printf("Server[accepting]: " ANSI_COLOR_YELLOW
               "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n");
        int connSock = Accept(passiveSock, (struct sockaddr *)&cli_addr, &addr_len);
        doTcpJob(connSock, rp);
        close(connSock);
    }
}

void doTcpJob(int connSock, run_params *rp)
{
    // TODO: uscire in caso di errore di una delle due fasi
    printf("Server[connection]:" ANSI_COLOR_GREEN
           "STARTED A NEW ON CONNECTION (PID=%d)" ANSI_COLOR_RESET "\n",
           getpid());
    client_req request = {0, 0, 0, ""};
    uint8_t err = 3;
    doTcpReceive(connSock, &request, rp);
    if (request.status == -1)
    {
        printf("Server[error]: " ANSI_COLOR_RED
               "INVALID REQUEST FROM CLIENT (%d%s)" ANSI_COLOR_RESET "\n",
               request.message, request.filename);
        send(connSock, &err, 1, 0);
        close(connSock);
        return;
    }
    if (request.status == -2)
    {
        printf("Server[error]: " ANSI_COLOR_RED "EXIT FOR TIMEOUT" ANSI_COLOR_RESET
               "\n");
        send(connSock, &err, 1, 0);
        close(connSock);
        return;
    }
    doTcpSend(connSock, &request);
    if (request.status == -3)
    {
        printf("Server[error]: " ANSI_COLOR_RED
               "FILE NOT FOUND: %s" ANSI_COLOR_RESET "\n",
               request.filename);
        send(connSock, &err, 1, 0);
        close(connSock);
        return;
    }
    close(connSock);
}

/**
 * Funzione che ha lo scopo di ricevere una richiesta da un client,
 * validarla ed in seguito:
 *  - memorizzarla in un buffer in caso di richiesta valida
 *  - restituire un codice di errore in caso contrario
 * */

void doTcpReceive(int connSock, client_req *request, run_params *rp)
{
    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;
    if (select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1)
    {
        // TODO: INSERIRE LOGICA RECEIVE QUI
        recv(connSock, &(request->message), 1, 0);
        if (request->message == 0)
        {
            recv(connSock, &(request->fnlen), 2, 0);
            if (request->fnlen > 0)
            {
                recv(connSock, request->filename, request->fnlen, 0);
                request->status = check_rgx("^\\w+[\\.]?\\w+\r\n$", (const char *)request->filename) == 0 ? 0 : -1;
                return;
            }
            request->status = -1;
            return;
        }
    }
    FD_CLR(connSock, &cset); // todo: valutare se necessario
    request->status = -2;    // -2: timeout code
}

void doTcpSend(int connSock, client_req *request)
{
    // TODO: INSERIRE LOGICA SEND QUI
    replace_first(request->filename, 256, '\r', '\0');
    FILE *fp = fopen(request->filename, "rb+");
    request->status = -3;
    if (fp != NULL)
    {
        request->status = 0;
        ssize_t s_stat = 0;
        uint8_t mex = 1;
        s_stat = send(connSock, &mex, 1, 0);
        struct stat stat_buf;
        if (fstat(fileno(fp), &stat_buf) == 0 && s_stat > 0)
        {
            uint32_t snet = htonl(stat_buf.st_size);
            s_stat = send(connSock, &snet, 4, 0);
            send(connSock, &stat_buf.st_mtime, 4, 0);
            ssize_t sent = 0;
            sent += sendfile(connSock, fileno(fp), NULL, (size_t)stat_buf.st_size);
            showProgress((int)sent, (int)stat_buf.st_size, "Server[sending]: ");
            printf("\n");
        }
    }
    fclose(fp);
}