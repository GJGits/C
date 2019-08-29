#include "../headers/tools.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

void initStr(char* string, int length) {
    memset(string, ' ', length);
    string[length] = '\0';
}