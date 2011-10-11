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
#include <sys/times.h>

#define SERVERPORT 4950	// the port users will be connecting to

#define MAXBUFLEN 500

void catch_alarm (int sig)
{
    keep_going = 0;
    signal (sig, catch_alarm);
}

int main(int argc, char *argv[])
{
    float telapsed;
    clock_t start, end;
    struct tms inicio, fim;
	int sockfd;
	struct sockaddr_in their_addr; // connector's address information
	struct hostent *he;

	int numBytesSent = 0;
    int numBytesRcv = 0;
    int numBytesRcvAux = 0;
    int contLin = 0;
    char bufRcv[MAXBUFLEN];
    char bufSent[MAXBUFLEN];
    socklen_t addr_len;

    volatile sig_atomic_t keep_going = 1;

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

    addr_len = sizeof(struct sockaddr);

    while(fgets(bufSent, MAXBUFLEN, stdin) != NULL){
    }
    rewind(stdin);

    start = times(&inicio); /* Inicio da contagem de tempo */

    int cont=1;
    signal(SIGALRM, catch_alarm);
    alarm (2);
    while((fgets(bufSent, MAXBUFLEN, stdin) != NULL) && (keep_going))
    {
        numBytesSent += sendto(sockfd, bufSent, strlen(bufSent), 0,(struct sockaddr *)&their_addr, sizeof(struct sockaddr));
        contLin++;
        numBytesRcvAux = recvfrom(sockfd, bufRcv, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
        bufRcv[numBytesRcvAux] = '\0';
        //printf("%d;\n", cont++);
        numBytesRcv += numBytesRcvAux;
        printf("%s", bufRcv);
    }

    /*if(!keep_going)
    {

    }*/

    sendto(sockfd, "", 0, 0,(struct sockaddr *)&their_addr, sizeof(struct sockaddr));

    end = times(&fim);
    telapsed = (float)(end-start) / sysconf(_SC_CLK_TCK); /* termina contagem de tempo */

    fprintf(stderr, "Tempo total: %4.1f s\n", telapsed);
    fprintf(stderr, "Caracteres enviados: %d\n", numBytesSent);
    fprintf(stderr, "Linhas enviadas: %d\n", contLin);
    fprintf(stderr, "Caracteres recebidos: %d\n", numBytesRcv);

    //printf("sent %d bytes to %s\n", numbytes, inet_ntoa(thenumBytesSentir_addr.sin_addr));

	close(sockfd);

	return 0;
}
