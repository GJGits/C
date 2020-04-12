#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

void int_to_str(char *string, int num);
int str_to_int(char *string);
void replace_first(char *string, int len, const char oldc, const char newc);
void showProgress(int done, int tot, char * progMsg);
int check_rgx(const char *pattern, const char *msg);