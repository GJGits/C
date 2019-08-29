#include "../../global-headers/gj_tools.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
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
