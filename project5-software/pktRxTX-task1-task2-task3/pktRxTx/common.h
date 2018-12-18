#ifndef _COMMON_H_
#define _COMMON_H_ 

#if 0
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <netinet/if_ether.h> ////Provides declarations for ethernet header
#include <net/if.h>
#endif
#include "pktheader.h"


/* default packet length (maximum bytes per packet to capture) */
#define PACKET_LEN 1514

/* flags value at payload first byte: 
 * - SYNC : client need to send acknowledgment packets.
 * - ACK : this is ack packet from client, check the ack_no
 *         if necessary to know ack for which seq_no packet.
 * - CRC : client side has CRC or others error happen 
 */
#define SYNC 0x1
#define ACK 0x2
#define ERR 0x4

/* 
 * 96 bit (12 bytes) pseudo header needed for tcp header checksum calculation 
 */
struct pseudo_header {    
	unsigned int source_address;
	unsigned int dest_address;
	unsigned char placeholder;
	unsigned char protocol;
	unsigned short tcp_length;
};

#define HEADER_LEN      (sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr))
#define PAYLOAD_OFFSET  HEADER_LEN
#define TCPHDR_OFFSET   (sizeof(struct ethhdr) + sizeof(struct iphdr))
#define IPHDR_OFFSET    sizeof(struct ethhdr)

#define PSEUDO_SIZE     sizeof(struct pseudo_header) + sizeof(struct tcphdr) + PACKET_LEN - HEADER_LEN

/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

extern void print_payload(const unsigned char * payload, int len, FILE * fp);

extern int get_dest_macaddr(const char *name, unsigned char *mac); 
extern int get_dest_ipaddr(const char *name, char *addr);
extern int get_interface_macaddr(const char *name, unsigned char *mac);
extern int get_interface_ipaddr(const char *name, int family, uint8_t *addr, int len);

#endif
