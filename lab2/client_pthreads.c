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
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 9034    /* the port client will be connecting to */

#define MAXDATASIZE 500 /* maximo de bytes que podem ser mandados de uma vez */

void *writeProcess(void *writeData);

void *readProcess(void *readData);

typedef struct writeStruct
{
	FILE *wsock;
    int linesSent;
    int charsSent;
    int biggestLineSize;
	int sockfd;
} writeStruct;

typedef struct readStruct
{
	FILE *rsock;
    int linesRead;
    int charsRead;
	int sockfd;
} readStruct;

int main(int argc, char *argv[])
{
    float telapsed;
    clock_t start, end;
    struct tms inicio, fim;
    int sockfd;
	char buf[500];

	writeStruct *writeData;
	readStruct *readData;
	writeData = malloc(sizeof(writeData));
	readData = malloc(sizeof(readData));

	pthread_t write, read;

    struct hostent *he; /* Extrai informacoes para conexao, como o nome, do servidor */
    struct sockaddr_in their_addr; /* Guarda as informacoes do servidor conectando */

    FILE *rsock, *wsock;

    /* Limpa as strings de envio e recepcao */
    //for(i=0;i<MAXDATASIZE;i++)buf[i] = '\0';
    //for(i=0;i<MAXDATASIZE;i++)rcv[i] = '\0';

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }


    if ((he=gethostbyname(argv[1])) == NULL) {  /* Pega informacoes do servidor e guarda em he */
        perror("gethostbyname");
        exit(1);
    }


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { /* cria um novo socket de conexao */
        perror("socket");
        exit(1);
    }

    rsock = fdopen(sockfd, "r");
    wsock = fdopen(sockfd, "w");

	writeData->wsock = wsock;
	readData->rsock = rsock;

	writeData->sockfd = sockfd;
	readData->sockfd = sockfd;

    their_addr.sin_family = AF_INET;       /* Ordem dos bytes do host */
    their_addr.sin_port = htons(PORT);     /* Ordem dos bytes da rede */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
    bzero(&(their_addr.sin_zero), 8);

    /* Conecta usando o socket criado e as informacoes extraidas do servidor */
    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    /* Primeira leitura para guardar arquivo na cache */
    while(fgets(buf, MAXDATASIZE, stdin) != NULL){
    }
    rewind(stdin);	

    start = times(&inicio); /* Inicio da contagem de tempo */

	pthread_create(&write, NULL, writeProcess, (void*)writeData);
	pthread_create(&read, NULL, readProcess, (void*)readData);
	//readProcess((void*)readData);

	pthread_join(write, NULL);
	pthread_join(read, NULL);

    end = times(&fim);
    telapsed = (float)(end-start) / sysconf(_SC_CLK_TCK); /* termina contagem de tempo */

    /* Estatisticas */
    fprintf(stderr, "Tempo total: %4.1f s\n", telapsed);
    fprintf(stderr, "Linhas enviadas: %d\n", writeData->linesSent);
    fprintf(stderr, "Maior linha: %d\n", writeData->biggestLineSize);
    fprintf(stderr, "Caracteres enviados: %d\n", writeData->charsSent);
    fprintf(stderr, "Linhas recebidas: %d\n", readData->linesRead);
    fprintf(stderr, "Caracteres recebidos: %d\n", readData->charsRead);
    close(sockfd);

    return 0;

}

void *writeProcess(void *writeData)
{
	FILE *wsock;
	wsock = ((writeStruct*)writeData)->wsock;
	int sockfd = ((writeStruct*)writeData)->sockfd;
	int numCharsSent = 0;
	int charsSentAux = 0;
	int numBiggestLine = 0;
	int numLinesSent = 0;
	int success = 0;
	char *buf = malloc(sizeof(char) * MAXDATASIZE);

	while(fgets(buf, MAXDATASIZE, stdin) != NULL)
    {
        success = fputs(buf, wsock);
        fflush(wsock);

        if(success > 0)
        {
			charsSentAux = strlen(buf);
            numCharsSent += charsSentAux;
            numLinesSent++;

            if(charsSentAux > numBiggestLine)
            {
                numBiggestLine = charsSentAux;
            }

        }
    }
    fputs(buf, wsock);
	free(buf);
	((writeStruct*)writeData)->linesSent = numLinesSent;
	((writeStruct*)writeData)->charsSent = numCharsSent;
	((writeStruct*)writeData)->biggestLineSize = numBiggestLine;

    shutdown(sockfd, SHUT_WR);
}

void *readProcess(void *readData)
{
	FILE *rsock;
	rsock = ((readStruct*)readData)->rsock;
	char *rcv, *rcvAux;
	rcv = malloc(sizeof(char) * MAXDATASIZE);
	rcvAux = malloc(sizeof(char) * MAXDATASIZE);
	//char *rcv = "Teste";
	int numCharsRcv = 0;
	int numLinesRcv = 0;

	rcvAux = fgets(rcv, MAXDATASIZE, rsock);
    /*while(rcvAux != NULL)
    {
		fflush(rsock);
        numCharsRcv += strlen(rcv);
        numLinesRcv++;
        printf("%s", rcv);
		rcvAux = fgets(rcv, MAXDATASIZE, rsock);
    }*/
	//fgets(rcv, MAXDATASIZE, rsock);
	//printf("%s", rcv);
	((readStruct*)readData)->linesRead = numLinesRcv;
	((readStruct*)readData)->charsRead = numCharsRcv;
	//free(rcv);
}
