
#include <unistd.h>
#include <sys/sendfile.h>
#include <regex.h>
#include "../gj_tools.h"
#include "../gj_server.h"

const int REQ_BUF_SIZE = 10000; // da specifica

int startTcpServer(const char *port)
{

    uint16_t i_port;
    int sockfd;

    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    parsePort(port, &i_port);
    bindToAny(sockfd, i_port);
    Listen(sockfd, 516);
    printf("Server[listening]: " ANSI_COLOR_GREEN "PORT = %d, BACKLOG = 516" ANSI_COLOR_RESET "\n", (int)i_port);

    return sockfd;
}

void runIterativeTcpInstance(int passiveSock, run_params *rp)
{

    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    printf("Server[accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n");

    while (1)
    {
        FILE *fp = fopen("connections.log", "ab+");
        if (fp != NULL)
        {
            struct timeval tval;
            fd_set cset;
            FD_ZERO(&cset);
            FD_SET(passiveSock, &cset);
            tval.tv_sec = 15;
            tval.tv_usec = 0;
            if (select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1)
            {
                int connSock = Accept(passiveSock, (struct sockaddr *)&cli_addr, &addr_len);
                printf("Server[accepting]: " ANSI_COLOR_CYAN "%s %d" ANSI_COLOR_RESET "\n", inet_ntoa(cli_addr.sin_addr), (int)cli_addr.sin_port);
                fprintf(fp, "Connection accepted from %s %d\n", inet_ntoa(cli_addr.sin_addr), (int)cli_addr.sin_port);
                fflush(fp);
                fclose(fp);

                int childpd;

                if ((childpd = fork()) == 0)
                {
                    printf("Server[forking]: " ANSI_COLOR_CYAN "CREATED A NEW PROCESS (%d)" ANSI_COLOR_RESET "\n", getpid());
                    close(passiveSock);
                    doTcpJob(connSock, rp);
                    printf("Server[forking]: " ANSI_COLOR_MAGENTA "KILLING PID (%d)" ANSI_COLOR_RESET "\n", getpid());
                    printf("Server[accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n");
                    exit(0);
                }

                close(connSock);
                continue;
            }
            FD_CLR(passiveSock, &cset);
            fprintf(fp, "No new connections for %s seconds\n", rp->wait);
            fflush(fp);
            fclose(fp);

        }
    }
}

void doTcpJob(int connSock, run_params *rp)
{
    // TODO: uscire in caso di errore di una delle due fasi
    printf("Server[connection]:" ANSI_COLOR_GREEN
           "STARTED A NEW ON CONNECTION (PID=%d)" ANSI_COLOR_RESET "\n",
           getpid());
    client_req request = {0, "", ""};
    doTcpReceive(connSock, &request, rp);
    if (request.status == -1)
    {
        printf("Server[error]: " ANSI_COLOR_RED
               "INVALID REQUEST FROM CLIENT (%s%s)" ANSI_COLOR_RESET "\n",
               request.message, request.filename);
        send(connSock, "-ERR\r\n", 6, 0);
        close(connSock);
        return;
    }
    if (request.status == -2)
    {
        printf("Server[error]: " ANSI_COLOR_RED "EXIT FOR TIMEOUT" ANSI_COLOR_RESET
               "\n");
        send(connSock, "-ERR\r\n", 6, 0);
        close(connSock);
        return;
    }
    doTcpSend(connSock, &request);
    if (request.status == -3)
    {
        printf("Server[error]: " ANSI_COLOR_RED
               "FILE NOT FOUND: %s" ANSI_COLOR_RESET "\n",
               request.filename);
        send(connSock, "-ERR\r\n", 6, 0);
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
        recv(connSock, request->message, 4, 0);
        int status = check_rgx("^GET $", (const char *)request->message);
        if (status == 0)
        {
            recv(connSock, request->filename, 256, 0);
            int status2 = check_rgx("\\w+[\\.]?\\w+\r\n$", (const char *)request->filename);
            if (status2 == 0)
            {
                request->status = 0;
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
        s_stat = send(connSock, "+OK\r\n", 5, 0);
        struct stat stat_buf;
        if (fstat(fileno(fp), &stat_buf) == 0 && s_stat > 0)
        {
            char str_val[15]; // 12 + 3 (up to 1 TB)
            sprintf(str_val, "%d", (int)stat_buf.st_size);
            strcat(str_val, "\r\n");
            s_stat = send(connSock, str_val, strlen(str_val), 0);
            ssize_t sent = 0;
            sent += sendfile(connSock, fileno(fp), NULL, (size_t)stat_buf.st_size);
            showProgress((int)sent, (int)stat_buf.st_size, "Server[sending]: ");
            printf("\n");
            send(connSock, &stat_buf.st_mtime, 4, 0);
        }
    }
    fclose(fp);
}