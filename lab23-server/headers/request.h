#include <inttypes.h>

void processRequest(int connSock);
int extractFileName(char *request, char *fileName);
int checkFile(char *fileName);
void getFileInfo(char *fileName, uint32_t *info);
void substring(char s[], char sub[], int p, int l);