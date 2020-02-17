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

int startTcpServer(const char* port);
void runIterativeTcpInstance(int passiveSock);
void runConcurrentTcp(int passiveSock);
void runSlave(int passiveSock);
void doTcpJob(int connSock);
int doTcpReceive(int connSock, char *request);
void doTcpSend(int connSock, char *request);
int checkRequest(char *request);
int reqCompleted(char *request);
