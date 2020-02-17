/*
 * TEMPLATE 
 */
#include <stdio.h>
#include <stdlib.h>
#include "../gj_server.h"

char *prog_name;

int main (int argc, char *argv[]) {

	char cwd[100];
	int numOfSlaves;
    
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("App[starting]: " ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", cwd);
    } else {
       printf("App[quitting]: " ANSI_COLOR_RED "UNABLE TO LOCATE WDIR" ANSI_COLOR_RESET "\n");
       return 1;
    }

	// procedo solo se vengono passati esattamente due parametri allo script
 	// 1. il nome dello script (default)
	// 2. la porta
	// 3. il numero di slave
	if (argc == 3) {
		int passiveSocket = startTcpServer(argv[1]);
		numOfSlaves = strtol(argv[2], (char **)NULL, 10); // TODO: implementare vari check
		for (int i = 0; i < numOfSlaves; i++) {
        		runSlave(passiveSocket);
    		}
		 /* Sit back and wait for all child processes to exit */
    		while (waitpid(-1, NULL, 0) > 0);
		close(passiveSocket);
		return 0;
    	}	

   	

	// se arrivo qui ho app crash
	printf("App[quitting]: " ANSI_COLOR_RED "USAGE: <PORT>" ANSI_COLOR_RESET "\n");
	
	return 1;
}
