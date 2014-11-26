/* udp client rfc2544 */
/* https://tools.ietf.org/html/rfc2544 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

#include "rfc2544.h"

void error(char *);
int main(int argc, char *argv[])
{
	int sock, length, n,data[2],sending,bytes,it;
	long send_frames,rcv_frames;
	struct sockaddr_in server;
	struct hostent *hp;
	int rcv_buf[1024],send_buf[1024];
	int ok=0;
	int asc=1;
	float udelay;
	
	if (argc != 4) {
		printf("Usage: server port bytes\n");
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

	bytes = atoi(argv[3]);

	bzero(rcv_buf,1024);
	bzero(send_buf,1024);
	
	sending=0;
	udelay=ONE_SECOND;

	while(!ok) {

		if (!sending) {
		
			//setup
			//sprintf(send_buf,"%x%x",CMD_SETUP_SYN,send_frames);
			send_buf[0] = CMD_SETUP_SYN;
			send_buf[1] = bytes;
			sendto(sock, send_buf, CMD_SIZE, 0,
				(const struct sockaddr*) &server,length);
			if (DEBUG) fprintf(stdout,"Setup syn sent.\n");
			usleep(DELAY);

                	n = recvfrom(sock,rcv_buf,1024,0,(struct sockaddr *)&server,
                	                        (socklen_t *)&length);
                	if (n<0) {
                	        fprintf(stderr,"recvfrom error\n");
				exit(1);
                	}

                	//data[0] = atoi(&rcv_buf[0]);
                	data[0] = rcv_buf[0];
                	if (data[0]==CMD_SETUP_ACK) {
				sending=1;
			}
		} else {

			//sending
			send_frames=0;
			it = ONE_SECOND / udelay;

			while(it--) { /* 1 second? */
				//sprintf(send_buf,"%x",CMD_DATA);
				send_buf[0] = CMD_DATA;
				n = sendto(sock,send_buf,bytes,0,
					(const struct sockaddr *)&server,length);
				if (n < 0) {
					error("Sendto");
				} else {
					send_frames++;
				}
				usleep(udelay);
			}

			//begin finish
			//sprintf(send_buf,"%x",CMD_FINISH_SYN);
			send_buf[0]=CMD_FINISH_SYN;
			send_buf[1]=send_frames;

			sendto(sock, send_buf, CMD_SIZE, 0,
				(const struct sockaddr*) &server,length);
			if (DEBUG) fprintf(stdout,"Hey receiver, we finished.\n");
			usleep(DELAY);

                	n = recvfrom(sock,rcv_buf,1024,0,(struct sockaddr *)&server,
                	                        (socklen_t *)&length);
                	if (n<0) {
                	        fprintf(stderr,"recvfrom error\n");
				exit(1);
                	}

                	//data[0] = atoi(&rcv_buf[0]);
                	//data[1] = atoi(&rcv_buf[1]);
                	data[0] = rcv_buf[0];
                	data[1] = rcv_buf[1];
                	if (data[0]==CMD_FINISH_ACK) {
				sending=0;
				rcv_frames = data[1];	
			}
			//end finish

			fprintf(stdout,"pps:%lu udelay:%f\n",send_frames,udelay);

			if (asc) {
				if (rcv_frames < send_frames) {
					if (DEBUG) fprintf(stdout,"go to !asc\n");
					asc = 0;
				} else {
					udelay = udelay / 2.0;
				}
			} else {
				if (rcv_frames == send_frames) {
					ok = 1;
					fprintf(stdout,"THAT'S THE POINT!%f\n",udelay);
				} else {
					udelay = udelay * 2.0;
				}
			}

		}	

	} /* end while */

	return 0;
}

void error(char *msg)
{
    perror(msg);
    exit(0);
}
