#include "../../global-headers/sockwrap.h"
#include "../../global-headers/errlib.h"
#include <time.h> 
#include <string.h>
#include <inttypes.h>
#include <errno.h>

char *prog_name; // per evitare errori di compilazione

int main(int argc, char const *argv[])
{
    uint16_t port;
    int sockfd;
    struct sockaddr_in client_addr;
    
    parsePort(argv[1], &port);
    sockfd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bindToAny(sockfd, port);

    printf("Server[status]: " ANSI_COLOR_GREEN "STARTED" ANSI_COLOR_RESET "\n");

    while (1) {
        
        ssize_t bytes_recv, bytes_sent = 0;
        socklen_t addr_len = sizeof(client_addr);
        uint32_t buffer;
        
        printf("Datagram: " ANSI_COLOR_YELLOW "Waiting for a datagram..." ANSI_COLOR_RESET "\n");
        
        bytes_recv = recvfrom(sockfd, &buffer, sizeof(buffer), 
            0, (struct sockaddr *) &client_addr, &addr_len);

        // procedo solo se ricevo esattamente 4 bytes
        if (bytes_recv == 4) {
            
            printf("Datagram[receiving]: " ANSI_COLOR_CYAN "[%d] (%d bytes)" ANSI_COLOR_RESET "\n", (int) ntohl(buffer), (int) bytes_recv);
            uint32_t resp_buffer[2];
            resp_buffer[0] = buffer;
            resp_buffer[1] = htonl(time(NULL));
            printf("Datagram[client address]: " ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET, inet_ntoa(client_addr.sin_addr));
            
            bytes_sent = sendto(sockfd, &resp_buffer, sizeof(resp_buffer), 
                0, (struct sockaddr *) &client_addr, addr_len);
            
            if (bytes_sent == -1) {
               printf("Server[error]: " ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, 
                    strerror(errno));
            }
            printf("Datagram[sending]: " ANSI_COLOR_GREEN "[%d][%d] (%d bytes)" 
                ANSI_COLOR_RESET "\n", (int) ntohl(resp_buffer[0]), 
                (int) ntohl(resp_buffer[1]), (int)  bytes_sent);
        } else {
            printf("Datagram[error]: " ANSI_COLOR_RED "invalid datagram! (%d bytes)" ANSI_COLOR_RESET "\n", 
                (int) bytes_recv);
        }
    }

    return 0;
}
