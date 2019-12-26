#include <stdio.h>
#include <stdlib.h>

void initStr(char* string, int length);
void int_to_str(char *string, int num);
int str_to_int(char *string);
void showProgress(int done, int tot, char * progMsg);
int openFile(FILE *fp, char *fileName, u_int32_t *info);
