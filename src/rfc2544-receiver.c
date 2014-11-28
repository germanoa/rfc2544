/* udp server rfc2544 */
/* https://tools.ietf.org/html/rfc2544 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

#include "rfc2544.h"

int bytes;
long send_frames, rcv_frames, sum;
int first_report=1;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void clear_bench()
{
	rcv_frames=0;
	sum=0;
}

void inc_bench(int size)
{
	rcv_frames++;
	sum+=size;
	//sum += size + HEADERS;
}

void report_bench()
{
	if (first_report) {
		fprintf(stdout,"#bytes,Bps,pps\n");
		first_report=0;
	}
	fprintf(stdout,"%d,%lu,%lu\n",bytes,sum,rcv_frames);
}

int main(int argc, char *argv[])
{
	int n, sock, length, fromlen,data[2];
	struct sockaddr_in server;
	struct sockaddr_in from;
	int rcv_buf[1024], send_buf[1024];
	struct timeval tv;
	
	if (argc < 2) {
		fprintf(stderr, "Usage: port\n");
		exit(0);
	}
	
	sock=socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		error("Opening socket");
	}

	tv.tv_sec = 0;
	tv.tv_usec = 10;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	    perror("Error");
	}

	length = sizeof(server);
	bzero(&server,length);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(atoi(argv[1]));
	if (bind(sock,(struct sockaddr *)&server,length)<0) {
		error("binding");
	}
	fromlen = sizeof(struct sockaddr_in);

	bzero(rcv_buf,1024);
	bzero(send_buf,1024);

	while (1) {
		n = recvfrom(sock,rcv_buf,1024,0,(struct sockaddr *)&from,
		 			(socklen_t *)&fromlen);
		if (n<0) {
			//if (DEBUG) fprintf(stderr,"recvfrom error\n");
		}
		else {
			data[0] = rcv_buf[0];
			data[1] = rcv_buf[1];

			if (data[0] == CMD_DATA) {
				inc_bench(n);
				//TODO: data[1] pode ter frame seq.			
			} else if (data[0] == CMD_SETUP_SYN) {
				bytes = data[1];
				clear_bench();
				if (DEBUG) fprintf(stdout,"Setup syn received.\n");
				//sprintf(send_buf,"%x",CMD_SETUP_ACK);
				send_buf[0]=CMD_SETUP_ACK;
				sendto(sock, send_buf, CMD_SIZE, 0,
					(struct sockaddr*) &from,fromlen);
				usleep(DELAY);

			} else if (data[0] == CMD_FINISH_SYN) { 
				send_frames = data[1];
				if (DEBUG) fprintf(stdout,"Finish syn received.\n");
				//sprintf(send_buf,"%x",CMD_FINISH_ACK);
				send_buf[0]=CMD_FINISH_ACK;
				send_buf[1]=sum;
				sendto(sock,send_buf, CMD_SIZE, 0,
					(struct sockaddr*) &from,fromlen);

				report_bench();
				clear_bench();

				usleep(DELAY);
			} else {
				fprintf(stderr,"CMD unavailable: %d\n",data[0]);
			}
		}
	}
}

