/*
 * TEMPLATE 
 */
#include <stdio.h>
#include <stdlib.h>
#include "../gj_server.h"

char *prog_name;

int main (int argc, char *argv[]) {

	// procedo solo se vengono passati esattamente due parametri allo script
 	// 1. il nome dello script (default)
	// 2. la porta
	if (argc == 2) {
		int sockfd = startUdpServer(argv[1]);
		runUdpServer(sockfd);
		return 0;
	}

	// se arrivo qui ho app crash
	printf("App[quitting]: " ANSI_COLOR_RED "USAGE: <PORT>" ANSI_COLOR_RESET "\n");
	
	return 1;
}
