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

#define MYPORT 9034    /* the port users will be connecting to */

#define BACKLOG 10     /* how many pending connections queue will hold */

#define MAXLINE 500   /* Tamanho da linha recebida */

int main()
{
    int i=0;
    int sockfd, new_fd, numbytes;  /* listen on sock_fd, new connection on new_fd */
    char line[MAXLINE];
    struct sockaddr_in my_addr;    /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    unsigned int sin_size;
    
    for(i=0;i<MAXLINE;i++)line[i] = '\0';

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         /* host byte order */
    my_addr.sin_port = htons(MYPORT);     /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */
    bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while(1) {  /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        fprintf(stderr, "server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));

        if(!fork()){

            while ((numbytes = read(new_fd, line, MAXLINE)) > 0) {
                line[numbytes] = '\0';
                printf("Linha Recebida: %s", line);
                //for(i=0; i<200000000; i++);
                write(new_fd, line, numbytes);
            }
        close(new_fd);

        fprintf(stderr, "server: connection from %s closed\n",inet_ntoa(their_addr.sin_addr));
        exit(0);
        }

        while(waitpid(-1, NULL, WNOHANG) > 0);

    }
    return 0;

}

