/*
 * TEMPLATE 
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../../../commons/sockwrap.h"
#include "../../../commons/errlib.h"
#include "../../../commons/gj_client.h"

char *prog_name;

int main (int argc, char *argv[])
{	
	if (argc >= 4) {
		
		//struct sockaddr_in *address;
		//setAddress(argv[1], address);
		int connSock = connectTcpClient(argv[1], argv[2]);
		for (int i = 3; i < argc; i++) {
			doClient(connSock, argv[i]);
		}

		close(connSock);
		return 0;

	}

	printf("Client[error]: " ANSI_COLOR_RED "USAGE: <address> <port> <...files>" ANSI_COLOR_RESET);
	return -1;
}
