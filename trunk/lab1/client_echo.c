/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 3490    /* the port client will be connecting to */

#define MAXDATASIZE 100 /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    int sockfd;  
	int numLinesSent=0;
	int numLinesRcv=0;
	int numBiggestLine=0;
	int numCharsSent=0;
	int numCharsRcv=0;
	int charsSentAux = 0;
	int charsRcvAux = 0;
    char buf[MAXDATASIZE];
	char rcv[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  /* get the host info */
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;         /* host byte order */
    their_addr.sin_port = htons(PORT);     /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
    bzero(&(their_addr.sin_zero), 8);        /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    /* First read to store data in cache. */
    while(fgets(buf, MAXDATASIZE, stdin) != NULL){
    }
    rewind(stdin);

    while(fgets(buf, MAXDATASIZE, stdin) != NULL)
    {
        charsSentAux = write(sockfd, buf, strlen(buf));
		if(charsSentAux > 0) {
			numLinesSent++;
			numCharsSent += charsSentAux;
			if(charsSentAux > numBiggestLine) {
				numBiggestLine = charsSentAux;
			}
		}


        charsRcvAux = read(sockfd, rcv, MAXDATASIZE);
		if(charsRcvAux > 0)
		{
			numLinesRcv++;
			numCharsRcv += charsRcvAux;
		}
        //printf("%s", rcv);
    }

	fprintf(stderr, "Linhas enviadas: %d\n", numLinesSent);
	fprintf(stderr, "Maior linha: %d\n", numBiggestLine);
	fprintf(stderr, "Caracteres enviados: %d\n", numCharsSent);
	fprintf(stderr, "Linhas recebidas: %d\n", numLinesRcv);
	fprintf(stderr, "Caracteres recebidos: %d\n", numCharsRcv);
    close(sockfd);

    return 0;
}
