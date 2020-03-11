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
	// 3. numero secondi di wait
	if (argc == 3) {
		int passiveSocket = startTcpServer(argv[1]);
		run_params run = {argv[2]};
		runIterativeTcpInstance(passiveSocket, &run);
		close(passiveSocket);
		return 0;
	}

	// se arrivo qui ho app crash
	printf("App[quitting]: " ANSI_COLOR_RED "USAGE: <PORT> <SECONDS>" ANSI_COLOR_RESET "\n");
	
	return 1;
}
