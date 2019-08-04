#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include <time.h> 
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "../headers/request.h"

char *prog_name; // per evitare errori di compilazione
int main(int argc, char const *argv[])
{
    char cwd[100];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("App[starting]: " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", cwd);
   } else {
       printf("App[starting]: " ANSI_COLOR_RED "UNABLE TO LOCATE WDIR" ANSI_COLOR_RESET "\n");
       return 1;
   }
    uint16_t port;
    int sockfd;
    
    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (argc == 2) {

    parsePort(argv[1], &port);
    bindToAny(sockfd, port);
    Listen(sockfd, 516);
    printf("Server[listening]: " ANSI_COLOR_GREEN "PORT = %d, BACKLOG = 516" ANSI_COLOR_RESET "\n", (int) port);

    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    printf("Server[accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n");

    while(1) {
        int connSock = Accept(sockfd, (struct sockaddr *) &cli_addr, &addr_len);
        printf("Server[accepting]: " ANSI_COLOR_CYAN "CONNECTED TO %s" ANSI_COLOR_RESET "\n", inet_ntoa(cli_addr.sin_addr));
        int childpid;
        
        if((childpid = fork()) == 0) {
            // processo figlio
            printf("Server[forking]: " ANSI_COLOR_CYAN "CREATED A NEW PROCESS (%d)" ANSI_COLOR_RESET "\n", getpid());
            close(sockfd);
            processRequest(connSock);
            printf("Server[forking]: "ANSI_COLOR_BLUE "KILLING PID (%d)" ANSI_COLOR_RESET "\n", getpid());
            printf("Server[accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n");
            exit(0);
        }

        close(connSock);
        
    }

    }

    printf("Server[error]: " ANSI_COLOR_RED "USAGE: <PORT>" ANSI_COLOR_RESET "\n");
    Close(sockfd);

    return 0;
}

