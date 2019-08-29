#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include <time.h> 
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include "../../global-headers/gj_server.h"
#include "../../global-headers/gj_tools.h"

char *prog_name; // per evitare errori di compilazione

int main(int argc, char const *argv[]) {
    
    checkArgc(argc, "Usage: <port>");
    int passiveSock = startTcpServer(INADDR_ANY, argv[1]);
    runTcpInstance(passiveSock);
    
    close(passiveSock);	

    return 0;
}

