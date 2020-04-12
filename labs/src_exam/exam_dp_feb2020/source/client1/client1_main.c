/*
 * TEMPLATE 
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../sockwrap.h"
#include "../errlib.h"
#include "../gj_client.h"

char *prog_name;

int main (int argc, char *argv[])
{	
	if (argc >= 5 && (argc - 3) % 2 == 0) {
		
		int connSock = connectTcpClient(argv[1], argv[2]);
		cli_params params = {argv, argc};
		doClient(connSock, &params);
		close(connSock);
		return 0;

	}

	printf("Client[error]: " ANSI_COLOR_RED "USAGE: <address> <port> <offset> <filename> ... " ANSI_COLOR_RESET);
	return -1;
}
