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

#define SERVERPORT 4950

#define MAXBUFLEN 500

volatile sig_atomic_t keep_going = 1;
int sockfd;


void catch_alarm (int sig)
{
    keep_going = 0;
    signal (sig, catch_alarm);
    close(sockfd);
}

int main(int argc, char *argv[])
{
    float telapsed;
    clock_t start, end;
    struct tms inicio, fim;
	struct sockaddr_in their_addr; // Armazena a informacao do servidor
	struct hostent *he;

    int cont = 1;

	int numBytesSent = 0;
    int numBytesRcv = 0;
    int numBytesRcvAux = 0;
    int contLin = 0;
    char bufRcv[MAXBUFLEN];
    char bufSent[MAXBUFLEN];
    socklen_t addr_len;

	if (argc != 2) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}

	if ((he=gethostbyname(argv[1])) == NULL) {  // Pega informacao do servidor
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

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }
    
    while(fgets(bufSent, MAXBUFLEN, stdin) != NULL){
    }
    rewind(stdin);

    start = times(&inicio); /* Inicio da contagem de tempo */
    signal(SIGALRM, catch_alarm);

    cont = 1;
    keep_going = 1;
    while(cont)
    {
        if(keep_going && (fgets(bufSent, MAXBUFLEN, stdin) != NULL))
        {

            numBytesSent += send(sockfd, bufSent, strlen(bufSent), 0);
            contLin++;
            
            alarm(5);
            numBytesRcvAux = recv(sockfd, bufRcv, MAXBUFLEN-1 , 0);
            if(keep_going){
                bufRcv[numBytesRcvAux] = '\0';
                numBytesRcv += numBytesRcvAux;

                printf("%s", bufRcv);
            }
        }
        else cont = 0;
    }

    send(sockfd, "", 0, 0);

    end = times(&fim);
    telapsed = (float)(end-start) / sysconf(_SC_CLK_TCK); /* termina contagem de tempo */

    fprintf(stderr, "Tempo total: %4.5f s\n", telapsed);
    fprintf(stderr, "Caracteres enviados: %d\n", numBytesSent);
    fprintf(stderr, "Linhas enviadas: %d\n", contLin);
    fprintf(stderr, "Caracteres recebidos: %d\n", numBytesRcv);

    connect(sockfd, AF_UNSPEC, sizeof(struct sockaddr));

	return 0;
}
