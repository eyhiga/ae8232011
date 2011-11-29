
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

#define BACKLOG 10     /* maximo de conexoes que podem ficar na fila  */

#define MAXDATASIZE 500   /* Tamanho da linha recebida */

#define MAXFD 64

#define SERVICE_ECHO "echo"
#define SERVICE_TCP "time_tcp"
#define SERVICE_UDP "time_udp"

#define max(x, y) ((x) > (y) ? (x) : (y))

int busy_udp = 0;
int last_udp_pid;

/* Estrutura que guarda os valores relevantes do arquivo de configuração para cada serviço */
typedef struct conf
{
    char *name;
    int *port;
    char *type;
    char *protoc;
    char *wait;
    char *pathname;
    char *args;
} conf;

/* Adiciona a mensagem apontada por "message" no log do programa */
void mysyslog(char *progname, char *message)
{
    FILE *fp;
    char buf[64];
    time_t now;
    time(&now);

    sprintf(buf, "%s", ctime(&now));
    buf[strlen(buf) - 1] = '\0';

    if((fp = fopen("CONNECTION.LOG", "a")) == 0)
    {
        exit(1);
    }

    fprintf(fp, "%s\n", progname);
    fprintf(fp, "%s\n", buf);
    fprintf(fp, "%s\n", message);
    fclose(fp);

}

/* Inicializa o programa como daemon */
void daemon_init(const char *progname)
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

    for(i=0; i<MAXFD; i++)
    {
        close(i);
    }

    openlog(progname, LOG_PID, 0);

}

/* Le o arquivo de configuração e guarda em um vetor das estruturas de configuração, uma para cada linha */
void read_config(FILE *fp, conf *c)
{
    int i;
    for(i=0; i<3; i++)
    {
        c[i].name = malloc(sizeof(char) * MAXDATASIZE);
        c[i].port = malloc(sizeof(int));
        c[i].type = malloc(sizeof(char) * MAXDATASIZE);
        c[i].protoc = malloc(sizeof(char) * MAXDATASIZE);
        c[i].wait = malloc(sizeof(char) * MAXDATASIZE);
        c[i].pathname = malloc(sizeof(char) * MAXDATASIZE);
        c[i].args = malloc(sizeof(char) * MAXDATASIZE);
    }

    for(i=0; i<3; i++)
    {
        fscanf(fp, "%s", c[i].name);
        fscanf(fp, "%d", c[i].port);
        fscanf(fp, "%s", c[i].type);
        fscanf(fp, "%s", c[i].protoc);
        fscanf(fp, "%s", c[i].wait);
        fscanf(fp, "%s", c[i].pathname);
        fscanf(fp, "%s", c[i].args);

    }
}


/* Cria, configura e retorna um novo socket TCP. Não faz teste de erros */
int create_socket_tcp()
{
    int sock;
    int yes=1;
    
    /* cria socket */
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
    }
    
    /* seta opções de socket */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        return -1;
    }

    return sock;
}

/* Cria, configura e retorna um novo socket UDP. Não faz teste de erros */
int create_socket_udp()
{
    int sock;
    int yes=1;
    
    /* cria socket */
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        return -1;
    }
    
    /* seta opções de socket */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        return -1;
    }

    return sock;

}

/* Retorna o indice no vetor de estruturas "conf" que tenham a estrutura referente ao serviço apontado por */
/* service */
int get_index(conf *c, char *service)
{
    int i;
    int index;

    for(i=0; i<3; i++)
    {
        if(strcmp(c[i].name, service) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

/* Tratador do sinal gerado quando um processo filho forkeado pelo inetd morre. Se o processo tiver sido */
/* referente a um serviço UDP, desmarca uma flag especial, que quando desmarcada permite que o socket    */
/* UDP retorne para o conjunto de descritores no qual será feito o select. Para todos os serviços, chama */
/* a função de log, logando o horário em que o serviço terminou e seu PID                                */
void sig_handler(int s)
{
    int status;
    pid_t pid;
    char *message = malloc(sizeof(char) * MAXDATASIZE);

    signal(SIGCHLD, sig_handler);
    pid = wait(&status); /* pega o PID do processo filho que encerrou */
    
    /* testa se o PID do serviço encerrado é o mesmo do último serviço UDP lançado */
    if(pid == last_udp_pid)
    {
        busy_udp = 0;
        printf("udp finished\n");
    }

    sprintf(message, 
            "server: Finished service\npid:%d\n\n", pid);
    
    /* Loga mensagem de término no log do programa */
    mysyslog("", message);

}

int main(int argc, char *argv[])
{

    /* Informacoes de configuracao */
    FILE *fp = fopen("inetd.conf", "r");
    conf *c = malloc(3 * sizeof(conf));
    read_config(fp, c);

    daemon_init(argv[0]); /* inicia programa como daemon */

    /* Informacoes para o select */
    fd_set readfds;
    int nfds;

    /* Informacoes gerais */
    char buf[MAXDATASIZE];
    socklen_t addr_len = sizeof(struct sockaddr);
    socklen_t sin_size = sizeof(struct sockaddr_in);
    char *message = malloc(sizeof(char) * MAXDATASIZE);

    /* Informacoes do socket de echo */
    int sock_echo;
    struct sockaddr_in my_addr_echo, their_addr_echo;
    int port_echo = 0;
    int index_echo = 0;

    /* Informacoes do socket tcp */
    int sock_tcp;
    struct sockaddr_in my_addr_tcp, their_addr_tcp;
    int port_tcp = 0;
    int index_tcp = 0;

    /* Informacoes do socket udp */
    int sock_udp;
    struct sockaddr_in my_addr_udp, their_addr_udp;
    int port_udp = 0;
    int index_udp = 0;

    /* Configura socket e estruturas do servico de echo */
    sock_echo = create_socket_tcp();
    if(sock_echo == -1)
    {
        perror("socket");
        exit(1);
    }

    index_echo = get_index(c, SERVICE_ECHO);
    port_echo = *(c[index_echo].port);
    my_addr_echo.sin_family = AF_INET;
    my_addr_echo.sin_port = htons(port_echo);
    my_addr_echo.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr_echo.sin_zero), 8);

    if(bind(sock_echo, (struct sockaddr*) &my_addr_echo,
                sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    
    /* escuta na porta referente ao serviço de echo */
    if(listen(sock_echo, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    /* Configura socket e estruturas do servico daytime tcp */
    sock_tcp = create_socket_tcp();
    if(sock_tcp == -1)
    {
        perror("socket");
        exit(1);
    }

    index_tcp = get_index(c, SERVICE_TCP);
    port_tcp = *(c[index_tcp].port);
    my_addr_tcp.sin_family = AF_INET;
    my_addr_tcp.sin_port = htons(port_tcp);
    my_addr_tcp.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr_tcp.sin_zero), 8);

    if(bind(sock_tcp, (struct sockaddr*) &my_addr_tcp,
                sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    
    /* escuta na porta referente ao serviço daytime tcp */
    if(listen(sock_tcp, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    /* Configura socket do servico daytime udp */
    sock_udp = create_socket_udp();
    if(sock_udp == -1)
    {
        perror("socket");
        exit(1);
    }

    index_udp = get_index(c, SERVICE_UDP);
    port_udp = *(c[index_udp].port);
    my_addr_udp.sin_family = AF_INET;
    my_addr_udp.sin_port = htons(port_udp);
    my_addr_udp.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr_udp.sin_zero), '\0', 8);

    if(bind(sock_udp, (struct sockaddr *)&my_addr_udp,
                sizeof(struct sockaddr)) == -1) 
    {
        perror("bind");
        exit(1);
    }

    int ok = 1;

    while(1)
    {
        /* Configura select */
        nfds = max(sock_echo, sock_tcp);
        FD_ZERO(&readfds);

        FD_SET(sock_echo, &readfds);
        FD_SET(sock_tcp, &readfds);
        
        /* Apenas coloca o descritor de socket UDP no conjunto se o serviço UDP nao estiver sendo usado */
        if(busy_udp == 0)
        {
            FD_SET(sock_udp, &readfds);
            nfds = max(nfds, sock_udp);
            ok = 1;
        }

        nfds += 1;
        
        /* Se o select de erro, e o erro tiver sido pelo select ser sido interrompido, apenas continua */
        /* programa */
        if(select(nfds, &readfds, NULL, NULL, NULL) < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("select");
                exit(1);
            }
        }
        
        /* Se requisição tiver sido recebida no socket referente ao serviço de echo tcp */
        if(FD_ISSET(sock_echo, &readfds))
        {
            int sock_echo_new;
            int pid_echo;
            
            /* Aceita a conexão em um novo socket para envio de dados */
            if((sock_echo_new = accept(sock_echo, 
                            (struct sockaddr *)&their_addr_echo, &sin_size)) == -1)
            {
                perror("accept");
                continue;
            }
            
            /* Forkea processo, guardando PID do filho */
            pid_echo = fork();

            if(pid_echo == 0)
            {
                /* duplica o socket de envio de dados sobre a entrada e saída padrão, que serao herdados */
                /* pelo programa execed */
                dup2(sock_echo_new, 0);
                close(sock_echo_new);
                dup2(0, 1);
                
                /* Executa o servidor referente ao serviço */
                if(execl(c[index_echo].pathname,
                            c[index_echo].args, (char *)0) == -1){
                    perror("exec\n");
                    continue;
                }
            }

            sprintf(message, 
                    "server: Got connection from: %s port %d\nservice: %s\npid: %d\n\n", 
                    inet_ntoa(their_addr_echo.sin_addr), ntohs(their_addr_echo.sin_port),
                    SERVICE_ECHO, pid_echo);
            
            /* Guarda no log informações referentes a conexão */
            mysyslog(argv[0], message);
            
            /* Fecha o novo socket de envio de dados no lado do processo pai, e tira o socket de echo do */
            /* conjunto de descritores de socket */
            close(sock_echo_new);
            FD_CLR(sock_echo, &readfds);
        }
        
        /* Se requisição tiver sido recebida no socket referente ao serviço de daytime tcp */
        if(FD_ISSET(sock_tcp, &readfds))
        {

            int sock_tcp_new;
            int pid_tcp;
            
            /* Aceita a conexão em um novo socket para envio de dados */
            if((sock_tcp_new = accept(sock_tcp, 
                            (struct sockaddr *)&their_addr_tcp, &sin_size)) == -1)
            {
                perror("accept");
                continue;
            }
            
            /* Forkea processo, guardando PID do filho */
            pid_tcp = fork();

            if(pid_tcp == 0)
            {
                char *port = malloc(sizeof(char) * MAXDATASIZE);
                sprintf(port, "%d", *(c[index_tcp].port));
                
                /* duplica o socket de envio de dados sobre a entrada e saída padrão, que serao herdados */
                /* pelo programa execed */
                dup2(sock_tcp_new, 0);
                close(sock_tcp_new);
                dup2(0, 1);
                
                /* Executa o servidor referente ao serviço */
                if(execl(c[index_tcp].pathname,
                            c[index_tcp].args, (char *)0) == -1){
                    perror("exec\n");
                    continue;
                }
            }

            sprintf(message, 
                    "server: Got connection from: %s port %d\nservice: %s\npid: %d\n\n", 
                    inet_ntoa(their_addr_tcp.sin_addr), ntohs(their_addr_tcp.sin_port), 
                    SERVICE_TCP, pid_tcp);
            
            /* Guarda no log informações referentes a conexão */
            mysyslog(argv[0], message);
            
            /* Fecha o novo socket de envio de dados no lado do processo pai, e tira o socket de echo do */
            /* conjunto de descritores de socket */
            close(sock_tcp_new);
            FD_CLR(sock_tcp, &readfds);
        }
        
        /* Flag que impede que um serviço UDP seja chamado enquanto outro estiver sendo servido na mesma */
        /* porta */
        if(ok == 1)
        {
            
            /* Se requisição tiver sido recebida no socket referente ao serviço de daytime udp */
            if(FD_ISSET(sock_udp, &readfds))
            {
                /* recebe o primeiro pacote de requisição do serviço UDP, sem retirá-lo do kernel (MSG_PEEK) */
                recvfrom(sock_udp, buf, MAXDATASIZE-1, MSG_PEEK, 
                   (struct sockaddr *)&their_addr_udp, &addr_len);
                
                /* Seta flags que impedem que o outro serviço UDP seja aceito enquanto este é servido */
                busy_udp = 1;
                ok = 0;
                
                /* Forkea processo, guardando PID do filho */
                last_udp_pid = fork();

                if(last_udp_pid == 0)
                {
                    char *port = malloc(sizeof(char) * MAXDATASIZE);
                    sprintf(port, "%d", *(c[index_tcp].port));
                    
                    /* duplica o socket de envio de dados sobre a entrada e saída padrão, que serao herdados */
                    /* pelo programa execed */
                    dup2(sock_udp, 0);
                    close(sock_udp);
                    dup2(0, 1);

                    printf("%s\n", c[index_udp].pathname);
                    
                    /* Executa o servidor referente ao serviço */
                    if(execl(c[index_udp].pathname,
                                c[index_udp].args, (char *)0) == -1){
                        perror("exec\n");
                        continue;
                    }
                }

                sprintf(message, 
                        "server: Got connection from: %s port %d\nservice: %s\npid:%d\n\n", 
                        inet_ntoa(their_addr_udp.sin_addr), ntohs(their_addr_udp.sin_port),
                        SERVICE_UDP, last_udp_pid);
                
                /* Guarda no log informações referentes a conexão */
                mysyslog(argv[0], message);
                
                /* Retira o socket UDP do conjunto de descritores de arquivo */
                FD_CLR(sock_udp, &readfds);
            }
        }
        signal(SIGCHLD, sig_handler);

    }

    return 0;

}
