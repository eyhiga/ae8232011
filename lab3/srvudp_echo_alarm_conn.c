#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 4950

#define MAXBUFLEN 200

volatile sig_atomic_t keep_going = 1;
int sockfd;

void catch_alarm(int sig)
{
    keep_going = 0;
    signal (sig, catch_alarm);
    close(sockfd);
    fprintf(stderr, "TIMEOUT\n");
}

int main(void)
{
    
    struct sockaddr_in my_addr;	// Armazena a informacao local
    struct sockaddr_in their_addr; // Armazena a informacao do servidor
    socklen_t addr_len;

    int cont = 1;
    int numBytesSent = 0;
    int numBytesRcv = 0;
    int contChars = 0;
    int contLin = 0;
    char buf[MAXBUFLEN];
    signal(SIGALRM, catch_alarm);

    while(1){
        contChars = 0;
        contLin = 0;
        numBytesSent = 0;

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        my_addr.sin_family = AF_INET;        // host byte order
        my_addr.sin_port = htons(MYPORT);    // short, network byte order
        my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
        memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

        if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
            perror("bind");
            exit(1);
        }
        addr_len = sizeof(struct sockaddr);

        cont = 1;
        keep_going = 1;
        
        their_addr.sin_family = AF_UNSPEC;
        if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
        {
            perror("connect");
            exit(1);
        }        
        
        numBytesRcv = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
        contChars += numBytesRcv;
        contLin++;
        //printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%d@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n", numBytesRcv);
        buf[numBytesRcv] = '\0';
        printf("%s", buf);

        buf[numBytesRcv] = '\0';
        fprintf(stderr, "%s", buf);

        if(connect(sockfd, (struct sockaddr *)&their_addr, addr_len) == -1)
        {
            perror("connect");
            exit(1);
        }
        numBytesSent += send(sockfd, buf, strlen(buf), 0);

        numBytesSent += send(sockfd, buf, strlen(buf), 0);
         
		while(keep_going && (numBytesRcv = recv(sockfd, buf, MAXBUFLEN-1, 0)) != 0)
        {
            
            alarm(5);
            buf[numBytesRcv] = '\0';
            contChars += numBytesRcv;
            contLin++;

            printf("%s", buf);

            numBytesSent += send(sockfd, buf, strlen(buf), 0);

		}
        alarm(0);
        
        close(sockfd);
        
		fprintf(stderr, "Caracteres recebidos: %d\n", contChars);
		fprintf(stderr, "Linhas recebidas: %d\n", contLin);
		fprintf(stderr, "Caracteres enviados: %d\n", numBytesSent);

	}

	return 0;
}
