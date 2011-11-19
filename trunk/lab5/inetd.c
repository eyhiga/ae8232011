
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

void mysyslog(char *progname, char *address, char *service)
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
    fprintf(fp, "%s: server: got connection from %s, service: %s\n\n", buf, address, service);
    fclose(fp);

}

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

int create_socket_tcp()
{
    int sock;
    int yes=1;

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
        //perror("socket");
        //exit(1);
    }

    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        return -1;
        //perror("setsockopt");
        //exit(1);
    }

    return sock;
}

int create_socket_udp()
{
    int sock;

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        return -1;
    }

    return sock;

}

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

int main(int argc, char *argv[])
{

    //daemon_init(argc[0]);

    /* Informacoes de configuracao */
    FILE *fp = fopen("inetd.conf", "r");
    conf *c = malloc(3 * sizeof(conf));

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

    read_config(fp, c);

    /* Configura socket do servico de echo */
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

    if(listen(sock_echo, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    /* Configura socket do servico tcp */
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

    if(listen(sock_tcp, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    /* Configura socket do servico udp */
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

    printf("%d\n", port_echo);
    printf("%d\n", port_tcp);
    printf("%d\n", port_udp);

    return 0;

}
