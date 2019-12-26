#include "gj_tools.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "sockwrap.h"

int str_to_int(char *string) {
    return atoi(string);
}

void int_to_str(char *string, int num) {
    sprintf(string, "%d", num);
}

void initStr(char* string, int length) {
    memset(string, '\0', length);
    string[length] = '\0';
}

void showProgress(int done, int tot, char * progMsg) {
    int progress = ((double) done / (double) tot) * 100;
    printf("\r%s " ANSI_COLOR_CYAN "%d bytes (%d%%)" ANSI_COLOR_RESET, progMsg, done, progress);
    fflush(stdout);
}
