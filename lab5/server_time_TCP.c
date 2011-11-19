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

#define MAXDTSIZE 200  /* Tamanho da string a ser enviada */

void logging(char *address, char *dt){

    FILE *fp;
    char buf[64];
    time_t now;
    time(&now);
    int p = MYPORT;

    sprintf(buf, "%s", ctime(&now));
    buf[strlen(buf)-1] = '\0';
    if((fp=fopen("DT_TCP.LOG", "a")) == 0)
    {
        exit(1);
    }
    fprintf(fp, "%s: server: got connection from %s, PORT: %d\n\n", buf,address,p);
    fclose(fp);

}

int main(int argc, char *argv[]){
    
    int sockfd, new_fd;
    struct sockaddr_in my_addr;    /* informacao de endereco do servidor */
    struct sockaddr_in their_addr; /* informacoes do cliente  */
    unsigned int addr_size;
    int yes=1;
    char dt[MAXDTSIZE];
    time_t now;
    time(&now);

    FILE *wsock;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Inicializacao do socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         /* Ordem dos bytes do host */
    my_addr.sin_port = htons(MYPORT);     /* Ordem dos bytes da rede */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* Preenche com IP do server */
    bzero(&(my_addr.sin_zero), 8);

    /* Seta informacoes do server para o socket */
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    /* Escuta no socket apropriado*/
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    addr_size = sizeof(struct sockaddr_in);

    /* Aceita novas conexoes no socket de envio, atraves dos dados recebidos pelo socket de escuta */
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) == -1) {
        perror("accept");
    }

    if((wsock = fdopen(new_fd, "w")) == NULL){
        perror("wsock");
        exit(1);
    }
    setvbuf(wsock, NULL, _IOLBF, 0);

    ctime_r(&now, dt);

    fputs(dt, wsock);
    close(new_fd);
    fclose(wsock);
    logging(inet_ntoa(their_addr.sin_addr), dt);

    return 0;

}