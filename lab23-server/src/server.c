#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include <time.h> 
#include <string.h>
#include <inttypes.h>
#include <errno.h>

char *prog_name; // per evitare errori di compilazione
int main(int argc, char const *argv[])
{
    uint16_t port;
    int sockfd;
    struct sockaddr_in client_addr;
    
    parsePort(argv[1], &port);

    return 0;
}

