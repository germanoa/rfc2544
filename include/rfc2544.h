#ifndef _INCLUDE_RFC2544_

#define IP_HEADER 20
#define UDP_HEADER 8
#define ETHER_HEADER 18
#define HEADERS IP_HEADER + UDP_HEADER + ETHER_HEADER

#define SENDER_IP 198.18.1.2
#define DUT_INPUT_IP 198.18.1.1
#define DUT_OUTPUT_IP 198.19.1.1
#define RECEIVER_IP 198.19.1.2

#define CMD_SIZE 0x20 

#define CMD_FINISH_SYN 0x2
#define CMD_FINISH_ACK 0x4
#define CMD_SETUP_SYN 0x8
#define CMD_SETUP_ACK 0x10
#define CMD_DATA 0x10

#define ONE_SECOND 1000000
//#define DELAY 100000
#define DELAY 1

#define DEBUG 0 /* 1 -> on ; 0 -> off */

#endif /* _INCLUDE_RFC_2544_ */

