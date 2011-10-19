#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 4950

#define MAXBUFLEN 500

int main(void)
{
	int sockfd;
	struct sockaddr_in my_addr;	// Armazena a informacao local
	struct sockaddr_in their_addr; // Armazena a informacao do servidor
	socklen_t addr_len;
	int numBytesRcv = 0;
    int numBytesSent = 0;
    int contChars = 0;
    int contLin = 0;
	char buf[MAXBUFLEN];


	while(1)
	{
        contChars = 0;
        contLin = 0;

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

		while((numBytesRcv = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) != 0)
		{

		    buf[numBytesRcv] = '\0';
			contChars += numBytesRcv;
			contLin++;
            printf("%s", buf);
			//numBytesSent += sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));

		}

		fprintf(stderr, "Caracteres recebidos: %d\n", contChars);
		fprintf(stderr, "Linhas recebidas: %d\n", contLin);
		//fprintf(stderr, "Caracteres enviados: %d\n", numBytesSent);

        close(sockfd);
	}

	return 0;
}
