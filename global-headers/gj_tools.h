#include <stdio.h>
#include <stdlib.h>

void initStr(char* string, int length);
void showProgress(int done, int tot, char * progMsg);
int openFile(FILE *fp, char *fileName, u_int32_t *info);
