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

#include <sys/types.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MYPORT 9452    /* porta usada para a conexao */

#define BACKLOG 10     /* maximo de conexoes que podem ficar na fila  */

#define MAXBUFLEN 100

#define MAXDTSIZE 200  /* Tamanho da string a ser enviada */

void logging(char *address, char *dt){

    FILE *fp;

    if((fp=fopen("DT_UDP.LOG", "a")) == 0)
    {
        exit(1);
    }
    fprintf(fp, "%s server: got connection from %s\n\n", dt,address);
    fclose(fp);

}

int main(int argc, char *argv[]){
    
    int sockfd = 0;
    struct sockaddr_in their_addr; /* informacoes do cliente  */
    socklen_t addr_len;
    char dt[MAXDTSIZE];
    char aux[MAXBUFLEN];
    time_t now;
    time(&now);
    FILE *wsock;
    FILE *f = fopen("teste", "w");
    char buf[MAXBUFLEN];
    
    addr_len = sizeof(struct sockaddr);

    if((wsock = fdopen(0, "w")) == NULL)
    {
        perror("fdopen");
        exit(1);
    }

    setvbuf(wsock, NULL, _IOLBF, 0);

    recvfrom(sockfd, aux, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
    
    ctime_r(&now, dt);
    
    sendto(sockfd, dt, strlen(dt), 0, (struct sockaddr *)&their_addr, 
            sizeof(struct sockaddr));

    
    close(sockfd);
    fclose(wsock);
    sprintf(buf, "%s:%d", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));
    logging(buf, dt);

    return 0;

}
