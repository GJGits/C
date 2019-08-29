#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include<time.h>
#include <inttypes.h>

#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include "../../global-headers/gj_client.h"
#include "../../global-headers/gj_tools.h"

char *prog_name; // per evitare errori di compilazione

int main(int argc, char const *argv[]) {
    
    checkArgc(argc, "Usage: usage message here...");
    int connSock = connectTcpClient(argv[1],argv[2]);
    doClient(connSock, argv + 3);

    return 0;
}

