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
	if (argc >= 2) {
		char i[10];
		char j[10];
		printf( "Inserisci primo valore:");
		scanf("%s", i);
		printf( "Inserisci secondo valore:");
		scanf("%s", j);
		int connSock = connectTcpClient(argv[1], argv[2]);
		char request[50];
		sprintf(request, "%s %s\r\n", i, j);
		doClient(connSock, request);
		close(connSock);
		return 0;

	}

	printf("Client[error]: " ANSI_COLOR_RED "USAGE: <address> <port> <...files>" ANSI_COLOR_RESET);
	return -1;
}
