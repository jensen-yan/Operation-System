#ifndef __PKTHEADER_H__
#define __PKTHEADER_H__

#include <stdint.h>

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6       /* length of MAC address(bytes) */
#endif


typedef uint32_t 	tcp_seq;
typedef uint32_t 	tcp_cc;

 
/* Ethernet header */
struct ethhdr
{
	uint8_t h_dest[ETHER_ADDR_LEN];     	/* destination host hwaddr */
	uint8_t h_source[ETHER_ADDR_LEN];		/* source host hwaddr */
	uint16_t h_proto;						/* IP?, ARP?, RARP? etc. */
};


/*IP header*/ 
struct iphdr
{
	uint8_t ihl:4,
            version:4;					/* version << 4 | header length >> 2 */
	uint8_t tos;						/* type of service */
	uint16_t tot_len;					/* total length */
	uint16_t id;						/* indetification */
	uint16_t frag_off;					/* fragment offset field */
	#define IP_RF 0x8000				/* reserved fragment flag */
	#define IP_DF 0x4000				/* dont fragment flag */
	#define IP_MF 0x2000				/* more fragment flag */
	#define IP_OFFMASK 0x1fff			/* mask of fragmenting bits */
	uint8_t ttl;						/* time to live */
	uint8_t protocol;				    /* protocol */
	uint16_t check;					    /* IP checksum */
	uint32_t saddr;
    uint32_t daddr;		                /* source and dest address */
};

/* TCP header */
struct tcphdr {
    uint16_t source;                    /* source port */
    uint16_t dest;                      /* destination port */
    tcp_seq seq;                        /* sequence number */
    tcp_seq ack_seq;                    /* acknowledgement number */
    uint16_t res1:4, 
            doff:4,                     /* data offset, rsvd */
            fin:1,
            syn:1,
            rst:1,
            psh:1,
            ack:1,
            urg:1,
            ece:1,
            cwr:1;
    uint16_t window;                    /* window */
    uint16_t check;                     /* checksum */
    uint16_t urg_ptr;                   /* urgent pointer */
};

/*UDP header*/
struct udphdr
{
	uint16_t source;
	uint16_t dest;
	uint16_t len;
	uint16_t check;
};
 
#endif
