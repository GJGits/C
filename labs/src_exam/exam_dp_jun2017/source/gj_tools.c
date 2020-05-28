#include "gj_tools.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "sockwrap.h"

int str_to_int(char *string)
{
    if (string != NULL)
    {
        if (check_rgx("^[0-9]+$", (const char *)string) == 0)
        {
            return strtol(string, (char **)NULL, 10);
        }
    }
    return -1;
}

void int_to_str(char *string, int num)
{
    sprintf(string, "%d", num);
}

void replace_first(char *string, int len, const char oldc, const char newc)
{
    for (int i = 0; i < len; i++)
    {
        if (string[i] == oldc)
        {
            string[i] = newc;
            return;
        }
    }
}

int check_rgx(const char *pattern, const char *msg)
{
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0)
    {
        return -1;
    }
    int result = regexec(&re, msg, 0, NULL, 0);
    regfree(&re);
    return result;
}

void showProgress(int done, int tot, char *progMsg)
{
    int progress = ((double)done / (double)tot) * 100;
    printf("\r%s " ANSI_COLOR_CYAN "%d bytes (%d%%)" ANSI_COLOR_RESET, progMsg, done, progress);
    fflush(stdout);
}
