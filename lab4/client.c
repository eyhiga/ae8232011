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
#include <sys/wait.h>
#include <sys/times.h>
#include <netinet/in.h>

#define PORT 9034    /* the port client will be connecting to */

#define MAXDATASIZE 500 /* maximo de bytes que podem ser mandados de uma vez */

int main(int argc, char *argv[])
{

    float telapsed;
    clock_t start, end;
    struct tms inicio, fim;
    int sockfd, i;
    int numLinesSent=0;
    int numLinesRcv=0;
    int numBiggestLine=0;
    int numCharsSent=0;
    int numCharsRcv=0;
    int charsSentAux = 0;
    int charsRcvAux = 0;
    int success;
    char *buf = malloc(MAXDATASIZE * sizeof(char));
    char rcv[MAXDATASIZE];
    char* rcvAux = NULL;
    struct hostent *he; /* Extrai informacoes para conexao, como o nome, do servidor */
    struct sockaddr_in their_addr; /* Guarda as informacoes do servidor conectando */

    FILE *rsock, *wsock;

    /* Limpa as strings de envio e recepcao */
    for(i=0;i<MAXDATASIZE;i++)buf[i] = '\0';
    for(i=0;i<MAXDATASIZE;i++)rcv[i] = '\0';

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
    
    /* abre ambos os descritores de arquivo no socket a ser usado para o envio e recebimento */
    rsock = fdopen(sockfd, "r");
    wsock = fdopen(sockfd, "w");
    setvbuf(rsock, NULL, _IOLBF, 0)
    setvbuf(wsock, NULL, _IOLBF, 0)

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

    if(!fork())
    {
        // Processo filho - Envio
        while(fgets(buf, MAXDATASIZE, stdin) != NULL)
        {
            success = fputs(buf, wsock);

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
	
	/* Fechamento do socket */
        shutdown(sockfd, SHUT_WR);
	
	/* Estatisticas de envio do processo filho */
        fprintf(stderr, "Linhas enviadas: %d\n", numLinesSent);
    	fprintf(stderr, "Maior linha: %d\n", numBiggestLine);
    	fprintf(stderr, "Caracteres enviados: %d\n", numCharsSent);
        exit(0);
    }

    /* Processo pai - Laço de recebimento de dados */
    rcvAux = fgets(rcv, MAXDATASIZE, rsock);
    while(rcvAux != NULL)
    {
        charsRcvAux = strlen(rcvAux);
        numCharsRcv += charsRcvAux;
        numLinesRcv++;
        printf("%s", rcv);
        rcvAux = fgets(rcv, MAXDATASIZE, rsock);
    }

    end = times(&fim);
    telapsed = (float)(end-start) / sysconf(_SC_CLK_TCK); /* termina contagem de tempo */
    wait(NULL);
    /* Estatisticas de recebimento do processo pai */
    fprintf(stderr, "Tempo total: %4.1f s\n", telapsed);
    fprintf(stderr, "Linhas recebidas: %d\n", numLinesRcv);
    fprintf(stderr, "Caracteres recebidos: %d\n", numCharsRcv);
    close(sockfd);
    fclose(rsock);

    return 0;

}
