#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <inttypes.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <sys/socket.h>

#include "sockwrap.h"
#include "errlib.h"

void setAddress(const char *ip, struct sockaddr_in *address);
int connectTcpClient(const char *address, const char *port);
void doClient(int connSock, const char *request);
void clientSend(int connSock, const char *request);
void clientReceive(int connSock, const char *request);
