/*
** server.c -- a stream socket server demo
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

#define MYPORT 9034    /* porta usada para a conexao */

#define BACKLOG 10     /* maximo de conexoes que podem ficar na fila  */

#define MAXLINE 500   /* Tamanho da linha recebida */

#define MAXFD 64

void mysyslog(char *progname);

void daemon_init(const char *pname);

int main()
{
    int i=0;
    int sockfd, new_fd;  /* sockfd espera por conexoes, new_fd serve para o envio de dados */
    char line[MAXLINE];
    struct sockaddr_in my_addr;    /* informacao de endereco do servidor */
    struct sockaddr_in their_addr; /* informacoes do cliente  */
    unsigned int sin_size;
    int yes=1;
    
    /* Descritores de arquivo que serão abertos no socket */
    FILE *rsock, *wsock;

    for(i=0;i<MAXLINE;i++)line[i] = '\0';

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
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

    while(1) {  /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
	
	/* Aceita novas conexoes no socket de envio, atraves dos dados recebidos pelo socket de escuta */
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        
        /* Abre ambos os descritores de arquivo no socket usado para envio e recebimento */
        if((rsock = fdopen(new_fd, "r")) == NULL){
            perror("rsock");
	    exit(1);
        }
        if((wsock = fdopen(new_fd, "w")) == NULL){
            perror("wsock");
	    exit(1);
        }
        
        fprintf(stderr, "server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));
	
	/* Cria um processo filho para cuidar do envio na conexao estabelecida em new_fd */
        if(!fork()){
	    
	    /* Loop de recebimento e ecoamento */
            while ((fgets(line, MAXLINE, rsock)) != NULL) {
                line[strlen(line)] = '\0';
                fflush(rsock);
                printf("Linha Recebida: %s", line);
                fputs(line, wsock);
                fflush(wsock);
            }
            close(new_fd);

            fprintf(stderr, "server: connection from %s closed\n",inet_ntoa(their_addr.sin_addr));
            exit(0);
        }
        
        /* Fecha o socket de envio e recebimento */
        close(new_fd);

        while(waitpid(-1, NULL, WNOHANG) > 0);


    }
    return 0;

}

void daemon_init(const char *pname)
{
    int i;
    pid_t pid;
    
    if((pid = fork()) != 0)
    {
        exit(0);
    }
    
    setsid();
    signal(SIGHUP, SIG_IGN);
    
    if((pid = fork()) != 0)
    {
        exit(0);
    }
    
    chdir("/tmp");
    
    umask(0);
    
    for(i = 0; i < MAXFD, i++)
    {
        close(i);
    }
    
    openlog(pname, LOG_PID, 0);
    
}

void mysyslog(char *progname)
{
    FILE *fp;
    char buf[64], *ctime();
    time_t time(), now;
    (void) time(&now);
    
    sprintf(buf, "%s", ctime(now));
    
    if((fp=fopen("ERROR.LOG", "a")) == 0)
    {
        exit(1);
    }
    
    fprint(fp, "%s %s\n", progname, buf);
    fclose(fp);
    
}
