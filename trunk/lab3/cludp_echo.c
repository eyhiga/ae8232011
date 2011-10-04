/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT 4950	// the port users will be connecting to

#define MAXDATASIZE 500

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in their_addr; // connector's address information
	struct hostent *he;
	int numBytesSent = 0;
    int contLin = 0;
    char bufRcv[MAXDATASIZE];
    char bufSent[MAXDATASIZE];

	if (argc != 2) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
		perror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;	 // host byte order
	their_addr.sin_port = htons(SERVERPORT); // short, network byte order
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct

    while(fgets(buf, MAXDATASIZE, stdin) != NULL){
    }
    rewind(stdin);
    
    while(fgets(bufSent, MAXDATASIZE, stdin) != NULL)
    {
        numBytesSent += sendto(sockfd, bufSent, strlen(bufSent, 0,(struct sockaddr *)&their_addr, sizeof(struct sockaddr));
        contLin++;
        //printf("%s", bufSent);
        recvfrom(sockfd, bufRcv, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len));
        printf("%s", bufRcv);
    }

    //sendto(sockfd, "", 0, 0,(struct sockaddr *)&their_addr, sizeof(struct sockaddr));

    fprintf(stderr, "Caracteres enviados: %d\n", numBytesSent);
    fprintf(stderr, "Linhas enviadas: %d\n", contLin);
    
    //printf("sent %d bytes to %s\n", numbytes, inet_ntoa(thenumBytesSentir_addr.sin_addr));

	close(sockfd);

	return 0;
}
