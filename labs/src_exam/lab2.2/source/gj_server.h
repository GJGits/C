#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h> 
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <sys/socket.h>

#include "sockwrap.h"
#include "gj_tools.h"
#include "errlib.h"

int startUdpServer(const char *port);
int startTcpServer(const char* port);
void runIterativeTcpInstance(int passiveSock, const char *string);
void runUdpServer(int sockfd);
void doTcpJob(int connSock, const char *string);
void doUdpJob(int sockfd);
void doUdpReceive(int sockfd, struct sockaddr_in *cli_addr, socklen_t *addr_len, u_int32_t *buffer);
void doUdpSend(int sockfd, struct sockaddr_in *cli_addr, socklen_t *addr_len, u_int32_t *buffer);
int doTcpReceive(int connSock, char *request, const char *string);
void doTcpSend(int connSock, char *request);
int checkRequest(char *request);
int reqCompleted(char *request);
