/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

void error(char *);
int main(int argc, char *argv[])
{
	int sock, length, n;
	struct sockaddr_in server;
	struct hostent *hp;
	char buffer[256];
	int seconds=0;
	int bytes=0; 
	
	if (argc != 3) {
		printf("Usage: server port\n");
	  exit(1);
	}
	sock= socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("socket");
	
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp==0) error("Unknown host");
	
	bcopy((char *)hp->h_addr,(char *)&server.sin_addr,hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	length=sizeof(struct sockaddr_in);
	
	bytes=64;
	while(seconds<60) {
	
	  //n=sendto(sock,buffer,strlen(buffer),0,&server,length);
	  n=sendto(sock,buffer,bytes,0,(const struct sockaddr *)&server,length);
	  if (n < 0) error("Sendto");

		usleep(1000000); // 1 second
		seconds++;	
	}

	return 0;
}

void error(char *msg)
{
    perror(msg);
    exit(0);
}
