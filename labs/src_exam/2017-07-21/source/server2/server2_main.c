#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../errlib.h"
#include "../sockwrap.h"
#include "../myFunc.h"
#include "../myInclude.h"

#define LISTENQUEUE 15
#define MAXBUFL 255

#define MSG_OK "+OK\r\n"
#define MSG_ERR "-ERR\r\n"
#define MSG_GET "GET"
#define MSG_QUIT "QUIT"

/* Function definition */
ssize_t writeN(int s, void *ptr, size_t nbytes);
ssize_t readN(int s, void *ptr, size_t len);
int sendError(int socket);
int processFileRequests (int socket, char *stringReceived);
int freeZombie();
void killChild();
static void sigHandler (int signal);


/* Global Variables */
char *prog_name;
int childForked=0;
int childPid=0;

int main (int argc, char *argv[]) {

	int socket, newConnection, err=0, childToFork;
	short port;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=3)
		err_quit ("usage: %s <port> <string>\n", prog_name);
		
	port = atoi(argv[1]);
        childToFork = 3;
        

    if (signal(SIGUSR1, sigHandler) == SIG_ERR) {

    	printf ("(%s) - ERROR - Can't handle signals\n", prog_name);

        return 1;
    }
    
	socket = myTcpServerStartup(port, AF_INET);

	while (1) {
		printf ("(%s) waiting for connections ...\n", prog_name);

		int retry = 0;
		do {
			newConnection = accept (socket, (SA*) &cliaddr, &cliaddrlen);
			if (newConnection<0) {
				if (INTERRUPTED_BY_SIGNAL ||
				    errno == EPROTO || errno == ECONNABORTED ||
				    errno == EMFILE || errno == ENFILE ||
				    errno == ENOBUFS || errno == ENOMEM	) {
					retry = 1;
					err_ret ("(%s) error - accept() failed", prog_name);
				} else {
					err_ret ("(%s) error - accept() failed", prog_name);
					return 1;
				}
			} else {
				printf("(%s) - new connection from client %s:%u\n", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
				retry = 0;
			}
		} while (retry);

		if( (childPid = fork()) < 0){
			printf ("(%s) - fork() failed!\n", prog_name);
			continue;

		} else if (childPid > 0) {
			/* Parent Process */

                        childForked++;

			close(newConnection); // Close the new socket, it's handled by child process
			printf ("(%s) - Child %d handle the request\n", prog_name, childPid);

            // Check if I forked more than childToFork, if so wait at least one returns

            if(childForked == childToFork) {

                if (freeZombie() == 1) {

                    if (close(socket) == -1) {

                        printf ("(%s) --- ERROR - close() failed\n", prog_name);
                        return 1;
                    }
                }

                if ( waitpid(-1, NULL, 0) == -1) {

                    printf ("(%s) --- ERROR - waitpid() failed\n", prog_name);

                    return 1;
                }
            }

		} else {
			
			/* Child Process */
			close(socket);
	                err = processFileRequests(newConnection, argv[2]);
			printf ("(%s) - CHILD: connection closed by %s\n", prog_name, (err==0)?"client":"server");

                        killChild();

		}
	}
}

int freeZombie() {

    pid_t childpid;

    childpid = waitpid(-1, NULL, WNOHANG);

    while (childpid > 0) {
        /* Esempio:
         * 1) client A: apre connessione
         * 2) client A: chiude connessione -> figlio A manda segnale e diventa zombie
         * 3) client B: apre connessione
         * 4) client C: apre connessione
         * 5) client D: apre connessione [3 figli]
         * 6) waitpid: libera il figlio A zombie
         */

        printf ("(%s) - Child %d was a zombie [%d child]\n", prog_name, childpid, childForked);

        childpid = waitpid(-1, NULL, WNOHANG);
    }

    if (childpid == -1) {

    	printf ("(%s) --- ERROR - waitpid() failed\n", prog_name);

        return 1;
    }

    // childpid = 0
    printf ("(%s) - No child zombie remained\n", prog_name);
    return 0;
}

void killChild() {

    pid_t ppid;
      
    ppid = getppid();

    if (kill(ppid, SIGUSR1) == -1) {

    	printf ("(%s) --- ERROR - kill() failed\n", prog_name);

        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

static void sigHandler (int signal) {

  pid_t childpid;
  
  if (signal == SIGUSR1) {

    childForked--;

    printf ("(%s) - Signal received, child terminated. [%d child]\n", prog_name, childForked);

    if (childForked == 0)
      return;
    
    childpid = waitpid(-1, NULL, WNOHANG);
    while (childpid > 0) {

      /* Esempio:
       * 1) client A: apre connessione
       * 2) client B: apre connessione
       * 3) client B: chiude connessione -> figlio B manda segnale e diventa zombie
       * 4) client C: apre connessione
       * 5) client C: chiude connessione -> figlio C manda segnale e diventa zombie
       * 6) waitpid: libera il figlio B zombie
       */

        printf ("(%s) - Child %d was a zombie [%d child]\n", prog_name, childpid, childForked);

        childpid = waitpid(-1, NULL, WNOHANG);
    }

    if (childpid == -1)
    	printf ("(%s) --- ERROR - waitpid() failed\n", prog_name);

    else // childpid = 0
    	printf ("(%s) - No child zombie remained\n", prog_name);
  }

}

/*
* Return values:
*
*  > 0   = # characters read
*
*  -1  = an error occured
*/
ssize_t writeN(int s, void *ptr, size_t nbytes){

	size_t nleft;
	ssize_t nwritten;

	for (nleft=nbytes; nleft>0;) {
		nwritten = send(s, ptr, nleft, MSG_NOSIGNAL);
		if (nwritten == -1) /* Error */
			return(nwritten);
		else {
			nleft -= nwritten;
			ptr += nwritten;
		}
	}
	
	return (nbytes - nleft);
}

/*
* Return values:
*
*  > 0 = # characters read
*
*  0   = orderly shutdown (Connection closed by peer)
*
*  -1  = an error occured
*/
ssize_t readN(int s, void *ptr, size_t len){

	ssize_t nread;
	size_t nleft;
	for (nleft=len; nleft>0;){

		nread = recv(s, ptr, nleft, 0);

		if (nread > 0){
			nleft -= nread;
			ptr += nread;
		}
		else if (nread == 0) /* Connection closed by peer */
			break;
		else
			return (nread);
	}
	
	return (len - nleft);
}

int sendError(int socket){

	if(writeN(socket, MSG_ERR, strlen(MSG_ERR)) == -1)
		return 0;

	return 1; // Server will close the connection
}

int processFileRequests (int socket, char *stringReceived) {

	char fileBuffer[CHUNK], ch;

	ssize_t nread, leftBytes, byte_to_read;
	ssize_t fileNameLength;
	uint32_t fileSize;

        int n;
	struct timeval tval;
	fd_set cset;
	FILE *fd;
	struct stat fileStat;

	while (1) {

		printf("(%s) --- Waiting for file request...\n",prog_name);

                nread = readN(socket, &fileSize, sizeof(uint32_t));
                
                printf("(%s) --- File dimension received: %d\n", prog_name, (int)ntohl(fileSize));
                   
		leftBytes = ntohl(fileSize);

		byte_to_read = CHUNK;
                writeN(socket, &fileSize, sizeof(uint32_t));		                    
		while( leftBytes > 0 ) {

                        if (leftBytes < CHUNK)
                            byte_to_read = leftBytes;
                        
                        /*
                        
                                RECEIVEING THE FILE 1460 bytes at time or fileSize at time
                        */

                /*
                 * 
                 * PREPARING SELECT()
                 * 
                 */
                        FD_ZERO(&cset);
                        FD_SET(socket, &cset);
                        tval.tv_sec = 3;
                        tval.tv_usec = 0;


                        if ((n = select(socket+1, &cset, NULL, NULL, &tval)) == -1)
		                        printf("(%s) select() failed\n", prog_name);

                        if (n > 0) {

                                /*
                                * 
                                * SOCKET SELECTED
                                * 
                                */


                                nread = readN(socket, fileBuffer, byte_to_read);



                                if(nread <= 0) {

                                        return 1;
                                }


                                /*
                                TRANSFORM THE BYTE RECEIVED

                                */
                                for(int i=0; i<byte_to_read; i++){

                                        for(int k=0; k<strlen(stringReceived); k++){

                                                if(strncmp((fileBuffer+i), (stringReceived+k), sizeof(char)) == 0 ){

                                                        *(fileBuffer+i) = '*';           
                                                }
                                        

                                }
                                }
                                       
                                /*
                                        SEND THE MODIFIED FILE BACK
                                */ 
                                writeN(socket, (void *)fileBuffer, byte_to_read);
                                leftBytes -= nread;
			}else {

                                /*
                                * 
                                * SELECT TIMEOUT
                                * 
                                */
                                printf("(%s) No response after %d seconds!\n", prog_name, 3); // tval is reset by select

                                return 0;
		        }
                }
                   return 0;
	}
}
