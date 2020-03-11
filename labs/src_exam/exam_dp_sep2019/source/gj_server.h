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
#include "gj_types.h"

int startTcpServer(const char* port);
void runIterativeTcpInstance(int passiveSock, run_params *rp);
void doTcpJob(int connSock, run_params *rp);
void doTcpReceive(int connSock, client_req *request, run_params *rp);
void doTcpSend(int connSock, client_req *request);
int checkRequest(char *request);
int reqCompleted(char *request);
