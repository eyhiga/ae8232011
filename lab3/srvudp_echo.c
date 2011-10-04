/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 4950	// the port users will be connecting to

#define MAXBUFLEN 500

int main(void)
{
	int sockfd;
	struct sockaddr_in my_addr;	// my address information
	struct sockaddr_in their_addr; // connector's address information
	socklen_t addr_len;
	int numBytesSent = 0;
	int numBytesRcv = 0;
	char buf[MAXBUFLEN];

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;		 // host byte order
	my_addr.sin_port = htons(MYPORT);	 // short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(sockfd, (struct sockaddr *)&my_addr,
		sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	
	addr_len = sizeof(struct sockaddr);
	/*while(1)
	{*/
		/*if(!fork())
		{*/
			int contChars = 0;
			int contLin = 0;
			while((numBytesRcv = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) != 0)
			{
			    buf[numBytesRcv] = '\0';
			    printf("%s", buf);
			    //printf("got packet from %s\n",inet_ntoa(their_addr.sin_addr));
	            //printf("packet is %d bytes long\n",numBytesRcv);
				contChars += numBytesRcv;
				contLin++;
				
				numBytesSent += sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
				
			}
			fprintf(stderr, "Caracteres recebidos: %d\n", contChars);
			fprintf(stderr, "Linhas recebidas: %d\n", contLin);
			fprintf(stderr, "Caracteres enviados: %d\n", numBytesSent);
		
			/*if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}*/
		//}
		close(sockfd);
        //while(waitpid(-1, NULL, WNOHANG) > 0);
	//}

	return 0;
}