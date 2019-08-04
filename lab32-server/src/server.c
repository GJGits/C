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
#include <sys/wait.h>
#include "../headers/request.h"

#define MAX_SLAVE 10

char *prog_name; // per evitare errori di compilazione

void runSlave(int passiveSocket, struct sockaddr *cli_addr, socklen_t *addr_len);

int main(int argc, char const *argv[])
{
    uint16_t port;
    int sockfd;
    int numOfSlaves;
    
    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (argc == 3) {
    
    numOfSlaves = strtol(argv[2], (char **)NULL, 10); // TODO: implementare vari check
    parsePort(argv[1], &port);
    bindToAny(sockfd, port);
    Listen(sockfd, 516);
    printf("Server[listening]: " ANSI_COLOR_GREEN "PORT = %d, BACKLOG = 516" ANSI_COLOR_RESET "\n", (int) port);

    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    for (int i = 0; i < numOfSlaves; i++) {
        runSlave(sockfd,(struct sockaddr *) &cli_addr, &addr_len);
    }

    }

    /* Sit back and wait for all child processes to exit */
    while (waitpid(-1, NULL, 0) > 0);

    close(sockfd);
    printf("Server[error]: " ANSI_COLOR_RED "USAGE: <PORT> <POOL_SIZE>" ANSI_COLOR_RESET "\n");

    return 0;
}

void runSlave(int passiveSocket, struct sockaddr *cli_addr, socklen_t *addr_len) {

    int childpid;

    if((childpid = fork()) == 0) { 
        // nuovo processo slave
        int slaveNum = getpid();
        printf("Server[forking]: " ANSI_COLOR_CYAN "CREATED SLAVE (%d)" ANSI_COLOR_RESET "\n", slaveNum);
        while(1) {
            printf("Slave[%d][accepting]: " ANSI_COLOR_YELLOW "WAITING FOR A CONNECTION..." ANSI_COLOR_RESET "\n", slaveNum);
            int connSock = Accept(passiveSocket, (struct sockaddr *) cli_addr, addr_len);
            processRequest(connSock, slaveNum);
            close(connSock);
        }

        Close(passiveSocket);
    }

}

