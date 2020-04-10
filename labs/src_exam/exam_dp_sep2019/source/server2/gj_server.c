
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
                fwrite("Connection accepted from ", 25, 1, fp);
                fwrite(inet_ntoa(cli_addr.sin_addr), strlen(inet_ntoa(cli_addr.sin_addr)), 1, fp);
                fwrite(" ", 1, 1, fp);
                char c_port[6] = "";
                sprintf(c_port, "%d", (int)cli_addr.sin_port);
                fwrite(c_port, 5, 1, fp);
                fwrite("\n", 1, 1, fp);
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
            fclose(fp);

        }
    }
}

void doTcpJob(int connSock, run_params *rp)
{
    //TODO: uscire in caso di errore di una delle due fasi
    client_req request = {0, "", ""};
    doTcpReceive(connSock, &request, rp);
    if (request.status == -1)
    {
        printf("Server[error]: " ANSI_COLOR_RED "INVALID REQUEST FROM CLIENT (%s%s)" ANSI_COLOR_RESET "\n", request.message, request.filename);
        char err_buff[7] = "-ERR\r\n";
        send(connSock, err_buff, 6, 0);
        close(connSock);
        return;
    }
    if (request.status == -2)
    {
        printf("Server[error]: " ANSI_COLOR_RED "EXIT FOR TIMEOUT" ANSI_COLOR_RESET "\n");
        char err_buff[7] = "-ERR\r\n";
        send(connSock, err_buff, 6, 0);
        close(connSock);
        FILE *fp = fopen("connections.log", "ab+");
        if (fp != NULL)
        {
            char message[50] = "";
            sprintf(message, "No new connections for %s seconds", rp->wait);
            fwrite(message, strlen(message), 1, fp);
            fclose(fp);
        }
        return;
    }
    doTcpSend(connSock, &request);
    if (request.status == -3)
    {
        printf("Server[error]: " ANSI_COLOR_RED "FILE NOT FOUND: %s" ANSI_COLOR_RESET "\n", request.filename);
        char err_buff[7] = "-ERR\r\n";
        send(connSock, err_buff, 6, 0);
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
    tval.tv_sec = strtol(rp->wait, (char **)NULL, 10);
    tval.tv_usec = 0;
    if (select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1)
    {
        // TODO: INSERIRE LOGICA RECEIVE QUI
        recv(connSock, request->message, 4, 0);
        regex_t re;
        const char pattern[] = "^GET $";
        if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0)
        {
            request->status = -1;
            return;
        }
        int status = regexec(&re, request->message, 0, NULL, 0);
        regfree(&re);
        if (status == 0)
        {
            recv(connSock, request->filename, 256, 0);
            regex_t re;
            const char pattern2[] = "^\\w+[\\.]?\\w+\r\n$";
            if (regcomp(&re, pattern2, REG_EXTENDED | REG_NOSUB) != 0)
            {
                request->status = -1;
                return;
            }
            int status2 = regexec(&re, request->filename, 0, NULL, 0);
            regfree(&re);
            if (status2 != 0)
            {
                request->status = -1;
                return;
            }
            return; // caso positivo, tutto ok
        }
        else
        {
            request->status = -1;
            return;
        }
    }
    FD_CLR(connSock, &cset);
    // esco per timeout
    request->status = -2;
    return;
}

void doTcpSend(int connSock, client_req *request)
{
    // TODO: INSERIRE LOGICA SEND QUI
    int pos = 0;
    for (int i = 0; i < strlen(request->filename); i++)
    {
        if (request->filename[i] == '\r')
        {
            pos = i;
            break;
        }
    }
    memset((request->filename) + pos, '\0', 1);
    FILE *fp = fopen(request->filename, "rb+");
    if (fp != NULL)
    {
        ssize_t s_stat = 0;
        const char ok[] = "+OK\r\n";
        s_stat = send(connSock, ok, 5, 0);
        struct stat stat_buf;
        if (fstat(fileno(fp), &stat_buf) == 0 && s_stat > 0)
        {
            long size = stat_buf.st_size;
            u_int32_t time = stat_buf.st_mtime;
            char str_val[10];
            const char end[] = "\r\n";
            sprintf(str_val, "%d", (int)size);
            strcat(str_val, end);
            s_stat = send(connSock, str_val, strlen(str_val), 0);
            ssize_t sent = 0;
            sent += sendfile(connSock, fileno(fp), NULL, (size_t)size);
            showProgress((int)sent, (int)size, "Server[sending]: ");
            printf("\n");
            send(connSock, &time, 4, 0);
        }
    }
    else
    {
        request->status = -3;
    }
    fclose(fp);
}

int reqCompleted(char *request)
{
    int len = strlen(request);
    return len > 6 && request[len - 2] == '\r' && request[len - 1] == '\n' ? 0 : -1;
}

int checkRequest(char *request)
{
    int len = strlen(request);
    if (len > 6 && request[0] == 'G' && request[1] == 'E' && request[2] == 'T' && request[3] == ' ' && request[len - 2] == '\r' && request[len - 1] == '\n')
    {
        memcpy(request, request + 4, (len - 6)); // estraggo il nome del file dalla richiesta
        request[len - 6] = '\0';
        return access(request, F_OK); // verifico che il file esista nella cartella di lavoro
    }
    return -1;
}
