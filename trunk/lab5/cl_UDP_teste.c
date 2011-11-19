#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/times.h>

#define SERVERPORT 9034

#define MAXBUFLEN 500


int main(int argc, char *argv[])
{

    int sockfd;
    int nrecv;
    int port_n;
    
	struct sockaddr_in their_addr; // Armazena a informacao do servidor
	struct hostent *he;

    char bufRcv[MAXBUFLEN];
    socklen_t addr_len;

	if (argc != 3) {
		fprintf(stderr,"usage: talker hostname message, port\n");
		exit(1);
	}

	if ((he=gethostbyname(argv[1])) == NULL) {  // Pega informacao do servidor
		perror("gethostbyname");
		exit(1);
	}

    if ((port_n = atoi(argv[2])) == 0) {  // Pega informacao do servidor
        perror("atoi");
        exit(1);
    }

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;	 // host byte order
	their_addr.sin_port = htons(port_n); // short, network byte order
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct
    addr_len = sizeof(struct sockaddr);

    sendto(sockfd, "a", 1, 0,(struct sockaddr *)&their_addr, sizeof(struct sockaddr));
            
    nrecv = recvfrom(sockfd, bufRcv, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
    bufRcv[nrecv] = '\0';
    printf("%s", bufRcv);

	close(sockfd);

	return 0;
}
