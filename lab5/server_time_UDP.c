/*
 ** Servidor que devolve a data e hora corrente para o cliente, e salva tambem a data e hora e o endereco e porta
 ** conectada pelo cliente em um log
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
#include <sys/wait.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define MYPORT 9034    /* porta usada para a conexao */

#define BACKLOG 10     /* maximo de conexoes que podem ficar na fila  */

#define MAXBUFLEN 100

#define MAXDTSIZE 200  /* Tamanho da string a ser enviada */

void logging(char *address, char *dt){

    FILE *fp;
    int p = MYPORT;

    if((fp=fopen("DT_UDP.LOG", "a")) == 0)
    {
        exit(1);
    }
    fprintf(fp, "%s server: got connection from %s, PORT: %d\n\n", dt,address,p);
    fclose(fp);

}

int main(int argc, char *argv[]){
    
    int sockfd;
    struct sockaddr_in my_addr;    /* informacao de endereco do servidor */
    struct sockaddr_in their_addr; /* informacoes do cliente  */
    socklen_t addr_len;
    char dt[MAXDTSIZE];
    char aux[MAXBUFLEN];
    time_t now;
    time(&now);
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Inicializacao do socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         /* Ordem dos bytes do host */
    my_addr.sin_port = htons(MYPORT);     /* Ordem dos bytes da rede */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* Preenche com IP do server */
    memset(&(my_addr.sin_zero), '\0', 8);

    /* Seta informacoes do server para o socket */
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    addr_len = sizeof(struct sockaddr);

    recvfrom(sockfd, aux, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
    
    ctime_r(&now, dt);
    
    sendto(sockfd, dt, strlen(dt), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));

    
    close(sockfd);
    logging(inet_ntoa(their_addr.sin_addr), dt);

    return 0;

}