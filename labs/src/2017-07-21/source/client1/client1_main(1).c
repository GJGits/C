#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../errlib.h"
#include "../sockwrap.h"
#include "../myFunc.h"
#include "../myInclude.h"

#define MAXBUFL 255
#define MAXFILENAME 249 //255 - (\r\n - GET0x20)
#define TIMEOUT 3

#define MAX_UINT16T 0xffff

#define MSG_GET "GET "
#define MSG_QUIT "QUIT\r\n"
#define MSG_OK "+OK"
#define MSG_ERR "-ERR"

char *prog_name;
ssize_t writeN(int s, const void *ptr, size_t nbytes);
ssize_t readN(int s, void *ptr, size_t len);
int read_response(int s, char *buffer, size_t length);
int requestFile(int socket, char *inputFile);
static int ip_version(const char *src);
void receiveFile(int socket, char *outpuFile);

int s; // socket shared
char *outputFile;

int main(int argc, char *argv[]){

	int err, protocol_family, ipVersion;
	int childPid=0;
	
	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=5)
		err_quit ("usage: %s <Server_Address> <port> <input_file> <output_file>\n", prog_name);
		
		
        /* Check if file exist and it's readable */
        if( access( argv[3], R_OK ) == -1 )
		err_quit ("INPUT FILE DOESN'T EXIST OR IT'S NOT READABLE'", prog_name);
	
	outputFile = argv[4];
	
	ipVersion = ip_version(argv[1]);
	if(ipVersion == 4)
	    protocol_family = PF_INET;
	else if (ipVersion == 6){
	    protocol_family = PF_INET6;
	    printf("IPV6");
	    }
	else
	    err_quit("(%s) - ERROR ADDDRESS WRONG!", prog_name);
	    
	    
	    
	s = tcp_connect(argv[1], argv[2], protocol_family);	    
	    
        if( (childPid = fork()) < 0){
	        printf ("(%s) - fork() failed!\n", prog_name);
	        exit(1);

        } else if (childPid > 0) {
	        /* Parent Process */

	        printf ("(%s) - Child %d handle the request\n", prog_name, childPid);
                err = requestFile(s, argv[3]);	
	        
	        if ( waitpid(childPid, NULL, 0) == -1) {

                    printf ("(%s) --- ERROR - waitpid() failed\n", prog_name);

                    return 1;
                }
	
        } else {
	
	        /* Child Process */
	        
	       
	        receiveFile(s, outputFile);
	        printf ("(%s) - CHILD: connection closed by %s\n", prog_name, (err==0)?"client":"server");


        }

	printf ("(%s) - connection closed by %s\n", prog_name, (err==0)?"client":"server");

	if (close(s) == -1)
		printf("(%s) Error caused by close(): %s\n", prog_name, strerror(errno));
	
	printf("===========================================================\n");

	return 0;

}

/*
* Return values:
*
*  > 0   = # characters read
*
*  -1  = an error occured
*/
ssize_t writeN(int s, const void *ptr, size_t nbytes) {

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
ssize_t readN(int s, void *ptr, size_t len) {

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

/*
 * READING RESPONSE
 */
int read_response(int s, char *buffer, size_t length){

	ssize_t nread;

	/* Do not use Readline as it exits on errors
	(e.g., clients resetting the connection due to close() on sockets with data yet to read).
	servers must be robust and never close */
	/* Do not use Readline_unbuffered because if it fails, it exits! */
	nread = readline_unbuffered (s, buffer, length);
	if (nread <= 0) {
		printf("(%s) - Read error/Connection closed\n",prog_name);

		return 1;
	}

	/* append the string terminator after CR-LF that is, \r\n (0x0d,0x0a) */
	buffer[nread]='\0';
	printf("(%s) --- Result: '%s'\n",prog_name, buffer);

	return 0;

}

void receiveFile(int socket, char *outputFile){


	char fileBuffer[CHUNK], ch; /* +1 to make room for \0 */

	ssize_t nread, leftBytes, byte_to_read;
	uint32_t fileSize;
	FILE *fd;
	struct stat fileStat;
        
        printf("(%s)CHILD--- Waiting for file transformed...\n",prog_name);

        nread = readN(socket, &fileSize, sizeof(uint32_t));

	printf("(%s) --- Received file size '%d'\n", prog_name, (int)ntohl(fileSize));

        leftBytes = ntohl(fileSize);

        byte_to_read = CHUNK;

        if( (fd = fopen(outputFile, "wb")) != NULL) {

                printf("(%s) CHILD--- File created!\n", prog_name);

                byte_to_read = CHUNK;
                            
	        while( leftBytes > 0 ) {

                        if (leftBytes < CHUNK)
                            byte_to_read = leftBytes;
    
		        nread = readN(socket, fileBuffer, byte_to_read);
		        if(nread <= 0) {

			        printf("(%s) CHILD--- File '%s' closed even if it's not complete!\n", prog_name, outputFile);
			        fclose(fd);

			        return;
		        }

		        fwrite(fileBuffer, byte_to_read, 1, fd);
		        leftBytes -= nread;
	        }

        } else {
	        printf("(%s) CHILD--- Error cannot open/create the file\n", prog_name);
	        return;
        }

        fclose(fd);
        printf("(%s) CHILD--- File '%s' received!\n", prog_name, outputFile);
        
        return;

}

int requestFile(int socket, char *inputFile) {

	char fileBuffer[CHUNK];
	size_t leftBytes, byte_to_read;
        ssize_t nread;
        uint32_t fileSize;
	FILE *fd;
	struct stat fileStat;
	
	/* Get message line from the user */
        printf("(%s) --- I'm going to send this file: %s'\n", prog_name, inputFile);

	/*
	 * ********************************************************************************
	 *                              PROCESS THE REQUESTS                              *
	 * ********************************************************************************
	 */

	/* Get file stat */
	if( stat(inputFile, &fileStat) < 0 )
		return 1;
	
	fileSize = htonl((uint32_t)fileStat.st_size);


        if(writeN(socket, &fileSize, sizeof(uint32_t)) == -1)
	        return 0;

	printf("(%s) --- SENT file size '%d'\n", prog_name, ntohl(fileSize));
	printf("(%s) --- SENDING file '%s'\n", prog_name, inputFile);

        leftBytes = fileSize;

            if( (fd = fopen (inputFile, "rb")) != NULL) {

                printf("(%s) --- File opened!\n", prog_name);
                
	            memset(fileBuffer, 0, CHUNK);
	            
	            leftBytes = ntohl(fileSize);
	            
            	if (leftBytes < CHUNK)
                    byte_to_read = leftBytes;
                else
       	            byte_to_read = CHUNK;
	                
	            while (fread(fileBuffer, byte_to_read, 1, fd) == 1) {

		            if(writeN(socket, fileBuffer, byte_to_read) == -1) {

			            printf("(%s) - FAILURE file not sent!\n", prog_name);
			            fclose(fd);

			            return 0;
		            }
		            
		            leftBytes -= byte_to_read;
               	            memset(fileBuffer, 0, CHUNK);
               	            
                            if (leftBytes < CHUNK)
                                byte_to_read = leftBytes;
	            }
	            
	            if(leftBytes == 0)
    	                printf("(%s) --- File sent correctly!\n", prog_name);
    	            else
       	                printf("(%s) - ERROR File partially sent!\n", prog_name);

	            fclose(fd);

            } else {
					printf("(%s) - Error cannot open the file\n", prog_name);
					return 0;
				}
		printf("(%s) --- File '%s' SENT!\n", prog_name, inputFile);
	return 0;
}

static int
ip_version(const char *src) {
    char buf[16];
    if (inet_pton(AF_INET, src, buf)) {
        return 4;
    } else if (inet_pton(AF_INET6, src, buf)) {
        return 6;
    }
    return -1;
    
    /*
    OR MAYBE
    
        struct addrinfo hint, *res = NULL;
        int ret;

        memset(&hint, '\0', sizeof hint);

        hint.ai_family = PF_UNSPEC;
        hint.ai_flags = AI_NUMERICHOST;

        ret = getaddrinfo(argv[1], NULL, &hint, &res);
        if (ret) {
            puts("Invalid address");
            puts(gai_strerror(ret));
            return 1;
        }
        if(res->ai_family == AF_INET) {
            printf("%s is an ipv4 address\n",argv[1]);
        } else if (res->ai_family == AF_INET6) {
            printf("%s is an ipv6 address\n",argv[1]);
        } else {
            printf("%s is an is unknown address format %d\n",argv[1],res->ai_family);
        }

       freeaddrinfo(res);
       return 0;

     */
}

