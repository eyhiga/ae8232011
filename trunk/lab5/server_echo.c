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
    char *line;
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

    /*
    if((wsock = fdopen(0, "w")) == NULL)
    {
        perror("wsock");
        exit(1);
    }

    if((rsock = fdopen(0, "r")) == NULL)
    {
        perror("rsock");
        exit(1);
    }*/

    if((sock = fdopen(0, "rw")) == NULL)
    {
        perror("fdopen");
        exit(1);
    }

    if(getpeername(new_fd, (struct sockaddr *)&their_addr, &addr_size) == -1)
    {
        perror("getpeername");
        exit(1);
    }
    setvbuf(wsock, NULL, _IOLBF, 0);
    ctime_r(&now, dt);

    /*while(scanf("%s", line) != EOF)
    {
        printf("%s", line);
    }*/

    while((line = fgets(line, MAXLINE, rsock)) != NULL)
    {
        line[strlen(line)] = '\0';
        fputs(line, wsock);
    }

    /*while ((numbytes = read(new_fd, line, MAXLINE)) > 0) {
        line[numbytes] = '\0';
        printf("Linha Recebida: %s", line);
        write(new_fd, line, numbytes);
    }*/
    close(new_fd);
    fclose(wsock);
    fclose(rsock);

    logging(inet_ntoa(their_addr.sin_addr), dt);
    
    fprintf(stderr, "server: connection from %s closed\n",
            inet_ntoa(their_addr.sin_addr));
    exit(0);

    while(waitpid(-1, NULL, WNOHANG) > 0);

    return 0;

}


