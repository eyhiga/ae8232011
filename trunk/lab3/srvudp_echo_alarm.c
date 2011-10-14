/*
** listener.c -- a datagram sockets "server" demo
*/

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

#define MYPORT 4950	// the port users will be connecting to

#define MAXBUFLEN 500

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
    
	struct sockaddr_in my_addr;	// my address information
	struct sockaddr_in their_addr; // connector's address information
	socklen_t addr_len;

    int cont = 1;

	int numBytesSent = 0;
	int numBytesRcv = 0;
    int contChars = 0;
    int contLin = 0;
	char buf[MAXBUFLEN];
    int counter = 0;
    signal(SIGALRM, catch_alarm);

	while(1)
	{
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

        if (bind(sockfd, (struct sockaddr *)&my_addr,
            sizeof(struct sockaddr)) == -1) {
            perror("bind");
            exit(1);
        }
        addr_len = sizeof(struct sockaddr);

        cont = 1;
        keep_going = 1;
        
		while(cont)
        {
            if(keep_going && (numBytesRcv = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) != 0)
            {
                //numBytesRcv = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
                alarm(5);
		        buf[numBytesRcv] = '\0';
			    contChars += numBytesRcv;
			    contLin++;

                printf("%s", buf);

			    numBytesSent += sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
            }
            else
            {
                cont = 0;
            }

		}
        alarm(0);
        
        close(sockfd);
        
		fprintf(stderr, "Caracteres recebidos: %d\n", contChars);
		fprintf(stderr, "Linhas recebidas: %d\n", contLin);
		fprintf(stderr, "Caracteres enviados: %d\n", numBytesSent);

	}

	return 0;
}
