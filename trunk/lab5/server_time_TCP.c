/*
 ** Servidor que devolve a data e hora corrente para o cliente, e salva tambem a data e hora e o endereco e porta
 ** conectada pelo cliente em um log
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

#define MYPORT 9451    /* porta usada para a conexao */

#define BACKLOG 10     /* maximo de conexoes que podem ficar na fila  */

#define MAXBUFLEN 100

#define MAXDTSIZE 200  /* Tamanho da string a ser enviada */

void logging(char *address, char *dt){

    FILE *fp;

    if((fp=fopen("DT_TCP.LOG", "a")) == 0)
    {
        exit(1);
    }
    fprintf(fp, "%s server: got connection from %s\n\n", dt ,address);
    fclose(fp);

}

int main(int argc, char *argv[]){

    int new_fd = 0;
    //struct sockaddr_in my_addr;    /* informacao de endereco do servidor */
    struct sockaddr_in their_addr; /* informacoes do cliente  */
    unsigned int addr_size;
    char dt[MAXDTSIZE];
    char buf[MAXBUFLEN];
    time_t now;
    time(&now);

    
    addr_size = sizeof(struct sockaddr_in);
    FILE *wsock;

    if((wsock = fdopen(0, "w")) == NULL){
        perror("wsock");
        exit(1);
    }
    
    if(getpeername(new_fd, (struct sockaddr *)&their_addr, &addr_size) == -1){
        perror("getpeername");
        exit(1);
    }
    setvbuf(wsock, NULL, _IOLBF, 0);

    ctime_r(&now, dt);

    fputs(dt, wsock);
    close(new_fd);
    fclose(wsock);
    sprintf(buf, "%s:%d", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));
    logging(buf, dt);

    return 0;

}
