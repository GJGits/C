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

int main(int argc, char *argv[])
{
	if (argc >= 4)
	{

		int connSock = connectTcpClient(argv[1], argv[2]);
		if (connSock != -1)
		{
			for (int i = 3; i < argc; i++)
			{
				doClient(connSock, argv[i]);
			}
			close(connSock);
			return 0;
		}
		printf("Client[error]: " ANSI_COLOR_RED "CONNECTION TO SERVER FAILED" ANSI_COLOR_RESET "\n");
		return -1;
	}

	printf("Client[error]: " ANSI_COLOR_RED "USAGE: <address> <port> <...files>" ANSI_COLOR_RESET);
	return -1;
}
