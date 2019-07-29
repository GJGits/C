#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <sys/socket.h>
#include "../headers/request.h"

void processRequest(int connSock) {

    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;

    while (select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1) {
        
        char request[30];
        char *fileName;
        int size = strlen(request);
        
        while (size < 2 || (strcmp(request[size - 2], "\r") != 0 && strcmp(request[size - 1], "\n") != 0 )) {
            int amountToRead = size > 30 ? size : 30;
            recv(connSock, request, amountToRead, 0);
        }

        extractFileName(request, fileName);

        if(checkFile(fileName)) {

            char *okMessage = "+OK\r\n";
            uint32_t fileInfo[2];
            getFileInfo(fileName, fileInfo);
            send(connSock, okMessage, 5, 0);
            send(connSock, fileInfo[0], sizeof(uint32_t), 0);

            FILE *fp;
            char *filePath = "./local-storage/";
            strcat(filePath, fileName);
            fp = fopen(filePath, "r");

            if (fp != NULL) {
                char ch;
                while((ch = fgetc(fp)) != EOF) {
                    send(connSock, &ch, 1, 0);
                }
            }

            send(connSock, fileInfo[1], sizeof(uint32_t), 0);

        }

    }
    

}

/** 
 * C substring function definition. Il parametro p
 * rappresenta la posizione iniziale (si conta a partire da 1),
 * il parametro l invece rappresenta la lunghezza della substring
 */
void substring(char s[], char sub[], int p, int l) {
   int c = 0;
   
   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';
}

/**
 * Verifica che la richiesta si formata nella maniera corretta
 * ed estrare il nome del file. Restituisce 0 in caso di successo,
 * -1 in caso di errore.
 */
int extractFileName(char *request, char *fileName) {
    int reqLen = strlen(request);
    if (reqLen > 6) {
        char subGet[4];
        char subEnd[2];
        substring(request, subGet, 1, 4);
        substring(request, subEnd, reqLen - 1, reqLen);
        if (strcmp(subGet, "GET ") == 0 && strcmp(subEnd, "\r\n") == 0) {
            substring(request, fileName, 5, reqLen - 2);
            return 0;
        } else
            return -1;
    } else 
        return -1;
}

/**
 * Verifica che il file sia presente nella cartella utilizzata come
 * local storage. Restituisce 0 nel caso il file esista, -1 viceversa.
 */
int checkFile(char *fileName) {
    // la cartella nella quale viene compilato il sorgente dovrebbe essere
    // la cartella del progetto 
    char *filePath = "./local-storage/";
    strcat(filePath, fileName);
    return access(filePath, F_OK) != -1 ? 0 : 1;
}

/**
 * Restituisce sotto forma di array informazioni sul file richiesto,
 * nello specifico la funzione restituisce la dimensione e il timestamp
 * relativo all'ultima modifica effettuata sul file
 */
void getFileInfo(char *fileName, uint32_t *info) {

    struct stat fstat;
    char *filePath = "./local-storage/";
    strcat(filePath, fileName);
    memset(info, 0, 2);

    if(stat(filePath, &fstat) == 0) {
        info[0] = fstat.st_size;
        info[1] = fstat.st_mtime;
    }
    
    return info;

}