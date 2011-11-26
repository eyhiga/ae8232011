/*
 ** server.c -- a stream socket server demo
 */

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MYPORT 9450    /* porta usada para a conexao */

#define BACKLOG 10     /* maximo de conexoes que podem ficar na fila  */

#define MAXLINE 500   /* Tamanho da linha recebida */

void logging(char *address, char *dt){

    FILE *fp;
    int p = MYPORT;

    if((fp=fopen("ECHO.LOG", "a")) == 0)
    {
        exit(1);
    }
    fprintf(fp, "%s server: got connection from %s, PORT: %d\n\n", 
            dt ,address,p);
    fclose(fp);

}

int main()
{
    time_t now;
    int new_fd = 0;
    char *line = malloc(sizeof(char) * MAXLINE);
    char dt[MAXLINE];
    //struct sockaddr_in my_addr;    /* informacao de endereco do servidor */
    struct sockaddr_in their_addr; /* informacoes do cliente  */
    unsigned int addr_size;
    //int yes=1;
    FILE *wsock;
    FILE *rsock;
    FILE *sock;
    
    time(&now);

    addr_size = sizeof(struct sockaddr_in);


    if((wsock = fdopen(0, "w")) == NULL)
    {
        perror("wsock");
        exit(1);
    }
    
    if((rsock = fdopen(0, "r")) == NULL)
    {
        perror("wsock");
        exit(1);
    }

    if(getpeername(new_fd, (struct sockaddr *)&their_addr, &addr_size) == -1)
    {
        perror("getpeername");
        exit(1);
    }
    setvbuf(wsock, NULL, _IOLBF, 0);

    ctime_r(&now, dt);

    while(fgets(line, MAXLINE, rsock) != NULL)
    {
       fputs(line, wsock); 
    }

    //fputs(dt, wsock);
    close(new_fd);
    fclose(wsock);
    fclose(rsock);
    logging(inet_ntoa(their_addr.sin_addr), dt);


    return 0;

}


