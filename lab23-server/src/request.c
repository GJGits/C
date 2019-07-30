#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <sys/socket.h>
#include "../headers/request.h"

#define REQUEST_INITIAL_SIZE 30

void processRequest(int connSock) {

    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;

    while (select(FD_SETSIZE, &cset, NULL, NULL, &tval) == 1) {
        
        char *fileName;
        char request[REQUEST_INITIAL_SIZE]; 
        
        readRequest(connSock, request);
        extractFileName(request, fileName);

        if(checkFile(fileName)) {

            free(request);
            char *okMessage = "+OK\r\n";
            send(connSock, okMessage, 5, 0);
            sendFile(connSock, fileName);

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
 * Legge una richiesta e ne estrae il contenuto memorizzandolo
 * nel buffure 'request'. In caso di richiesta letta correttamente
 * restituisce 0, -1 viceversa.
 */
int readRequest(int connSock, char *request) {
    int size = strlen(request);
    int attempts = 10;
    ssize_t readBytes = 0;
    while (size < 2 || (strcmp(request[size - 2], "\r") != 0 && strcmp(request[size - 1], "\n") != 0 ) || attempts == 0) {
        attempts--;
        int amountToRead = size > REQUEST_INITIAL_SIZE ? size : REQUEST_INITIAL_SIZE;
        readBytes = Recv(connSock, request, amountToRead, 0);
        if((strcmp(request[size - 2], "\r") != 0 && strcmp(request[size - 1], "\n") != 0) && size == readBytes) {
            size += 10;
            request = realloc(request, sizeof(char) * size);
        }
    }
}

/**
 * Verifica che la richiesta sia formata nella maniera corretta
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
        info[0] = (uint32_t) htonl(fstat.st_size);
        info[1] = (uint32_t) htonl(fstat.st_mtime);
    }
    
    return info;

}

/**
 * Invia file info e contenuto al client
 */

void sendFile(int connSock, char *fileName) {
    
    uint32_t fileInfo[2];
    getFileInfo(fileName, fileInfo);
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

    fclose(fp);
    send(connSock, fileInfo[1], sizeof(uint32_t), 0);
}