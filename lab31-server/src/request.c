#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h> 
#include <sys/socket.h>
#include <sys/sendfile.h>
#include "../headers/request.h"

#define REQUEST_INITIAL_SIZE 30
#define FILE_PATH "../local-storage/"

void processRequest(int connSock) {

    struct timeval tval;
    fd_set cset;
    FD_ZERO(&cset);
    FD_SET(connSock, &cset);
    tval.tv_sec = 15;
    tval.tv_usec = 0;
    int n;

    while ((n = Select(FD_SETSIZE, &cset, NULL, NULL, &tval)) == 1) {
        
        char *request = (char *)calloc(REQUEST_INITIAL_SIZE, sizeof(char)); 
        char *errorMessage = "-ERR\r\n";
        
        if (readRequest(connSock, request) == 0) {

            ssize_t fileLen = strlen(FILE_PATH) + strlen(request) - 5;
            char *fileName = (char *) calloc(fileLen, sizeof(char));
            memcpy(fileName, FILE_PATH, strlen(FILE_PATH));
            if(extractFileName(request, fileName) == 0) {

                if(checkFile(fileName) == 0) {

                    printf("Server[sending]: " ANSI_COLOR_CYAN "SENDING OF %s (0%%)" ANSI_COLOR_RESET, fileName);
                    free(request);
                    char *okMessage = "+OK\r\n";
                    send(connSock, okMessage, 5, 0);
                    sendFile(connSock, fileName);
                    printf("\nServer[sending]: " ANSI_COLOR_GREEN "SENT %s" ANSI_COLOR_RESET "\n", fileName);
                    free(fileName);

                } else {
                    printf("Server[error]: " ANSI_COLOR_RED "FILE %s DOES NOT EXIST!" ANSI_COLOR_RESET "\n", fileName);
                    free(request);
                    free(fileName);
                    if(send(connSock, errorMessage, strlen(errorMessage), MSG_NOSIGNAL) == -1)
                        n = -1;
                }
                    

            } else {
                printf("Server[error]: " ANSI_COLOR_RED "CANNOT EXTRACT FILE NAME FROM REQUEST (%s)" ANSI_COLOR_RESET "\n", request);
                free(request);
                if(send(connSock, errorMessage, strlen(errorMessage), MSG_NOSIGNAL) == -1)
                    n = -1;
            }
                

        } else  {
            printf("Server[error]: " ANSI_COLOR_RED "WRONG REQUEST" ANSI_COLOR_RESET "\n", request);
            free(request);
            if(send(connSock, errorMessage, strlen(errorMessage), MSG_NOSIGNAL) == -1)
                return;
        }
            

    }

    close(connSock);
    

}

/**
 * Verifica che nella richiesta siano presenti i
 * caretteri di ritorno che contraddistinguono il
 * termina della richiesta stessa. Ritorna 0 se
 * i caratteri sono presenti, -1 viceversa.
 */
int endReached(char *request) {
    int reqLen = strlen(request);
    if(reqLen > 2) {
        return strcmp(request + (reqLen - 2), "\r\n") == 0 ? 0 : -1;
    } else 
        return -1;
}

/**
 * Legge una richiesta e ne estrae il contenuto memorizzandolo
 * nel buffure 'request'. In caso di richiesta letta correttamente
 * restituisce 0, -1 viceversa.
 */
int readRequest(int connSock, char *request) {
    int size = REQUEST_INITIAL_SIZE;
    int attempts = 3;
    ssize_t readBytes = 0;
    while (endReached(request) != 0 && attempts > 0) {
        readBytes = recv(connSock, request, size - abs((int)readBytes), 0);
        attempts = readBytes <= 0 ? (attempts - 1) : attempts;
        if(endReached(request) == -1 && size == readBytes) {
            size += 10;
            request = realloc(request, sizeof(char) * size);
        }
    }

    return endReached(request);
}

/**
 * Verifica che la richiesta sia formata nella maniera corretta
 * ed estrare il nome del file. Restituisce 0 in caso di successo,
 * -1 in caso di errore.
 */
int extractFileName(char *request, char *fileName) {
    ssize_t reqLen = strlen(request);
    if (reqLen > 6) {
        char *subGet = (char *) calloc(5, sizeof(char));
        memcpy(subGet, request, 4);
        subGet[4] = '\0';
        if (strcmp(subGet, "GET ") == 0 && endReached(request) == 0) {
            memcpy(fileName + strlen(FILE_PATH), request + 4, reqLen - 6);
            fileName[strlen(FILE_PATH) + reqLen - 6] = '\0';
            free(subGet);
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
    return access(fileName, F_OK) != -1 ? 0 : 1;
}

/**
 * Restituisce sotto forma di array informazioni sul file richiesto,
 * nello specifico la funzione restituisce la dimensione e il timestamp
 * relativo all'ultima modifica effettuata sul file
 */
void getFileInfo(char *fileName, uint32_t *info) {

    struct stat fstat;
    
    if(stat(fileName, &fstat) == 0) {
        info[0] = (uint32_t) htonl(fstat.st_size);
        info[1] = (uint32_t) htonl(fstat.st_mtime);
    }
    
}

/**
 * Invia file info e contenuto al client
 */

void sendFile(int connSock, char *fileName) {
    
    uint32_t fileInfo[2];
    getFileInfo(fileName, fileInfo);
    send(connSock, &fileInfo[0], sizeof(uint32_t), 0);
    FILE *fp;
    fp = fopen(fileName, "rb+");

    if (fp != NULL) {
        ssize_t bytes_sent = 0;
        while (bytes_sent < fileInfo[0]) {
          bytes_sent += sendfile(connSock, fileno(fp), NULL, ntohl(fileInfo[0]));  
          int progress = ((double) bytes_sent / (double) ntohl(fileInfo[0])) * 100;
          printf("\rServer[sending]: " ANSI_COLOR_CYAN "SENDING OF %s, %d [%d%%]" ANSI_COLOR_RESET, fileName, (int) bytes_sent ,progress);
          fflush(stdout);  
        }
        
    } else 
        printf("Server[error]: " ANSI_COLOR_RED "FAILED TO OPEN %s" ANSI_COLOR_RESET "\n", fileName);

    fclose(fp);
    send(connSock, &fileInfo[1], sizeof(uint32_t), 0);
}