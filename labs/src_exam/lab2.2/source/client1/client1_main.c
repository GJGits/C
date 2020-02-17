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
	if (argc >= 3) {
		doUdpClient(argv[1], argv[2]);
		return 0;

	}

	printf("Client[error]: " ANSI_COLOR_RED "USAGE: <address> <port> <filename> <file_mod_name>" ANSI_COLOR_RESET);
	return -1;
}
