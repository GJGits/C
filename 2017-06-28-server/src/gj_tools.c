#include "../../global-headers/gj_tools.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "../../global-headers/sockwrap.h"

void initStr(char* string, int length) {
    memset(string, ' ', length);
    string[length] = '\0';
}

void showProgress(int done, int tot, char * progMsg) {
    int progress = ((double) done / (double) tot) * 100;
    printf("\r%s " ANSI_COLOR_CYAN "%d bytes (%d%%)" ANSI_COLOR_RESET, progMsg, done, progress);
    fflush(stdout);
}

/**
 * Restituisce sotto forma di array informazioni sul file richiesto,
 * nello specifico la funzione restituisce la dimensione e il timestamp
 * relativo all'ultima modifica effettuata sul file
 */
int openFile(FILE *fp, char *fileName, uint32_t *info) {

    if(fp != NULL) {

        struct stat fstat;
    
        if(stat(fileName, &fstat) == 0) {
            info[0] = (uint32_t) fstat.st_size;
            info[1] = (uint32_t) fstat.st_mtime;
            return 0;
        }

    }

    return -1;

    
}
