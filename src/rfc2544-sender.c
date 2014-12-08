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
#include <math.h>
#include <sys/time.h>

#include "rfc2544.h"

void error(char *);
int main(int argc, char *argv[])
{
	int i,y,sock, length, n,data[2],status,bytes,it;
	long send_frames,rcv_bytes, send_bytes;
	struct sockaddr_in server;
	struct hostent *hp;
	float rcv_buf[1024],send_buf[1024];
	int ok=0;
	float udelay;
	struct timeval tv,now;
	float avg_between_send = 0.0;
	float avg_between_send_rcv = 0.0;
	float last_send_time;
	float now_send_time;
	
	if (argc != 4) {
		printf("Usage: server port bytes\n");
	  exit(1);
	}
	sock= socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("socket");
	
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	    if (DEBUG) fprintf(stderr,"setsockopt error\n");;
	}

	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp==0) error("Unknown host");
	
	bcopy((char *)hp->h_addr,(char *)&server.sin_addr,hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	length=sizeof(struct sockaddr_in);

	bytes = atoi(argv[3]);
	bytes -= HEADERS;
	//printf("%d - %d = %d\n",atoi(argv[3]),HEADERS, bytes);


	bzero(rcv_buf,1024);
	bzero(send_buf,1024);
	
	status=SETUP;
	udelay=ONE_SECOND;
	y=16.0;

	while(!ok) {

		if (status == SETUP) {
			send_buf[0] = CMD_SETUP_SYN;
			send_buf[1] = bytes;
			sendto(sock, send_buf, CMD_SIZE, 0,
				(const struct sockaddr*) &server,length);
			if (DEBUG) fprintf(stdout,"Setup syn sent.\n");
			usleep(DELAY);

                	n = recvfrom(sock,rcv_buf,1024,0,(struct sockaddr *)&server,
                	                        (socklen_t *)&length);
                	if (n<0) {
                	        if (DEBUG) fprintf(stderr,"recvfrom error\n");
				//exit(1);
                	}

                	//data[0] = atoi(&rcv_buf[0]);
                	data[0] = rcv_buf[0];
                	if (data[0]==CMD_SETUP_ACK) {
				status = SEND;
			}

		} else if (status == SEND) {
			send_frames = 0;
			it = ONE_SECOND / udelay;
			send_bytes = 0;
			avg_between_send = 0;
			

			for (i=0;i<it;++i) { /* 1 second? */
				//sprintf(send_buf,"%x",CMD_DATA);
				send_buf[0] = CMD_DATA;
				n = sendto(sock,send_buf,bytes,0,
					(const struct sockaddr *)&server,length);
				if (n < 0) {
					error("Sendto");
				} else {
					send_frames++;
					send_bytes+=n;
				}
				usleep(udelay);
				gettimeofday(&now,NULL);
				now_send_time = (now.tv_sec * 1000000);	
				now_send_time += (now.tv_usec);	

				if (i==0) {
					last_send_time = now_send_time;
				} else {
					avg_between_send += now_send_time;
					avg_between_send -= last_send_time;
					last_send_time = now_send_time;
				}
			}
			avg_between_send /= it;
			status = FINISH;

		} else if (status == FINISH) {
			//begin finish
			send_buf[0]=CMD_FINISH_SYN;
			send_buf[1]=send_frames;

			sendto(sock, send_buf, CMD_SIZE, 0,
				(const struct sockaddr*) &server,length);
			if (DEBUG) fprintf(stdout,"Hey receiver, we finished.\n");
			usleep(DELAY);

                	n = recvfrom(sock,rcv_buf,1024,0,(struct sockaddr *)&server,
                	                        (socklen_t *)&length);
                	if (n<0) {
                	        if (DEBUG) fprintf(stderr,"recvfrom error\n");
				//exit(1);
                	}

                	data[0] = rcv_buf[0];
                	data[1] = rcv_buf[1];
                	if (data[0]==CMD_FINISH_ACK) {
										status = SETUP;
										rcv_bytes = data[1];
										avg_between_send_rcv = rcv_buf[2];	
									}
			//end finish

			if (DEBUG) {
				fprintf(stdout,"Bps:%lu pps:%lu udelay:%f - %f,%f\n",
				send_bytes + (send_frames * HEADERS),send_frames,udelay
							,avg_between_send,avg_between_send_rcv);
			};

			if (status == SETUP) {
				if (rcv_bytes == send_bytes) {
					if (y > 1) {
						udelay = udelay / y;
					} else {
						ok = 1;
						//REPORT
						send_bytes+= (send_frames * HEADERS);
						printf("%d,%lu,%lu,%f,%f,%f\n",
							bytes+HEADERS,send_bytes,send_frames,udelay,
							avg_between_send,avg_between_send_rcv);
					}
				} else {
					udelay = udelay * y;
					y = y / 2;
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
