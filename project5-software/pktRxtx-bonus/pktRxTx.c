
/**
* (C) Copyright 2018 ICT.
* Wenqing Wu <wuwenqing@ict.ac.cn>
*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "pcap.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#else
#include <Windows.h>
#include <time.h>
#include <process.h>
#endif

#include "common.h"

/* Use Fix MAC address */
#define MAC_ADDR_FIX    1

#define ECHO_MODE       1
#define LISTENING_MODE  2
#define BENCHMARK_MODE  3

struct iphdr iph;
struct tcphdr tcph;

pcap_t *handle;
char pkt[PACKET_LEN];
char pseudo_gram[PSEUDO_SIZE];

int rxtx_exit = 0;

/* Expect to receive acknowlegment from client */
char insert_iphdr = 1, insert_tcphdr = 1;

#ifndef WIN32
pthread_t rx_thread;
#else
unsigned int rx_thread;
#endif

/* Log packet to file (unused for now!!) */
char log_file = 0;
char tfname[50], rfname[50];
char fbinary = 0;
FILE *tx_f = NULL;    // tx file
int tx_fidx;          // file index
FILE *rx_f = NULL;    // rx file
int rx_fidx;          // file index


char errbuf[PCAP_ERRBUF_SIZE];
char dev[55];          /*network adaper */

int tx_cnt = 0;        /* number of sent packets, used for increasing TCP sequence number */
int rx_cnt = 0;
int mode = ECHO_MODE;          /* running mode, default: echo mode */
int interval = 1;      /* interval  time between two sending action*/
int sendnum = 3;       /* number of packets for a sending request*/

					   /* fixed MAC address for now */
unsigned char src_mac_addr[6] = { 0x80, 0xfa, 0x5b, 0x33, 0x56, 0xef };
unsigned char dest_mac_addr[6] = { 0x00, 0x0e, 0xc6, 0xd7, 0x77, 0x7b };

static void log_packets(FILE ** fp, char *fname, int *fidx, const u_char * buf,
	int len);

void sig_handler(int signo)
{
#ifndef WIN32
	if ((signo == SIGINT) && (rx_thread != pthread_self())) {
#else
	if (signo == SIGINT) {
#endif
		printf("received SIGINT\n");

		if (handle)
			pcap_close(handle);

		if (tx_f)
			fclose(tx_f);

		if (rx_f)
			fclose(rx_f);

		rxtx_exit = 1;
		exit(EXIT_SUCCESS);
	}
	}

/*
* Generic checksum calculation function
*/
unsigned short
csum(unsigned short *ptr, int nbytes)
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum = 0;
	while (nbytes>1) {
		sum += *ptr++;
		nbytes -= 2;
	}
	if (nbytes == 1) {
		oddbyte = 0;
		*((u_char*)&oddbyte) = *(u_char*)ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);
	answer = (short)~sum;

	return(answer);
}

static void
log_packets(FILE ** fp, char *fname, int *fidx, const u_char * buf, int len)
{
	char outf[80];
	int pos;
	struct tcphdr *hdr;

	if (log_file) {
		if (*fp) {
			pos = ftell(*fp);

			if ((pos + len) > 0x1000000) {
				fclose(*fp);
				goto open_file;
			}

			/* Only log in binary mode need to adjust to 16 bytes align */
			if (fbinary) {
				pos = pos & 0xf;
				if (pos != 0) {
					//printf("advanced %d bytes\n", (int)(0x10-pos));
					fseek(*fp, 0x10 - pos, SEEK_CUR);
				}
			}
		}
		else {
		open_file:
			sprintf(outf, fname, (*fidx)++);
			hdr = (struct tcphdr *) (buf + TCPHDR_OFFSET);
			printf("Open %s to log packets, seq 0x%04x ... \n",
				outf, hdr->seq);
			*fp = fopen(outf, "wb");
			if (*fp == NULL) {
				perror("fopen");
				exit(EXIT_FAILURE);
			}

			if (*fidx > 10)
				*fidx = 1;
		}

	}
	else {
		*fp = NULL;
	}

	if (fbinary && *fp)
		fwrite(buf, 1, len, *fp);
	else
		print_payload(buf, len, *fp);

	if (*fp)
		fflush(*fp);
}
/* *
* @ str    : content of first "len" bytes of payload
* @ len    : length of str
* */
void
send_packet(char *str, int len)
{
	struct pseudo_header psh;
	unsigned short payload_len;
	int i = 0;
	long ran;

	/* fiexed payload length for now */
	payload_len = 65;

	/* *
	* Ip checksum
	* */
	iph.tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + payload_len);
	iph.check = 0;      //Set to 0 before calculating checksum
	iph.check = csum((unsigned short *)&iph, sizeof(struct iphdr));
	//printf("IP checksum: 0x%04x\n", iph.check);
	memcpy((pkt + IPHDR_OFFSET), (char *)&iph, sizeof(struct iphdr));

	tcph.check = 0;
	tcph.seq = htonl(++tx_cnt);
	/* *
	* Now the TCP checksum
	* i.  craft a pseudo header
	* ii. calculate TCP checksum
	* */
	psh.source_address = iph.saddr;
	psh.dest_address = iph.daddr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr) + payload_len);
	int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + payload_len;

	memcpy(pseudo_gram, (char*)&psh, sizeof(struct pseudo_header));
	memcpy(pseudo_gram + sizeof(struct pseudo_header), (char*)&tcph, sizeof(struct tcphdr));
	memcpy(pseudo_gram + sizeof(struct pseudo_header) + sizeof(struct tcphdr), &pkt[PAYLOAD_OFFSET], payload_len);

	tcph.check = csum((unsigned short*)pseudo_gram, psize);

	memcpy((pkt + TCPHDR_OFFSET), (char *)&tcph, sizeof(struct tcphdr));

	/* set payload:
	* payload format: 'str' 0x11 0x11 0x11 ..."
	* */
	memcpy((pkt + PAYLOAD_OFFSET), str, len);
	for (i = (PAYLOAD_OFFSET + len); i < (PAYLOAD_OFFSET + payload_len); i++)
		pkt[i] = 0x11;

	/* Send down the packet */
	if (pcap_sendpacket(handle, (const unsigned char *)& pkt, (HEADER_LEN + payload_len)) != 0) {
		fprintf(stderr, "Error sending the packet: %s\n", pcap_geterr(handle));
	}
}

#ifndef WIN32
void *
receive_packet(void *arg)
#else
int WINAPI
receive_packet(void *arg)
#endif
{
	struct pcap_pkthdr *pkthdr;
	const u_char *pkt_data;
	int res;
	char *rcvdata = (char *)malloc(7);

	while (handle) {
		res = pcap_next_ex(handle, &pkthdr, &pkt_data);
		if (res < 0) {
			fprintf(stderr, "pcap_next_ex failed.\n");
			break;
		}
		if (res == 0)
			continue;

		/* this packet is not for me!*/
//		if (memcmp((void *)pkt_data, (void *)src_mac_addr, 6))
//			continue;
#if 0
		memcpy(rcvdata, (pkt_data + PAYLOAD_OFFSET), 6);
		rcvdata[6] = '\0';
		printf("rcv: %s\n", rcvdata);
		/* give a response */
//		if (memcmp((void*)rcvdata, (void *)"ackPkt", 6))
//			send_packet("ackPkt", 6);
#endif
		/* echo mode */
		if (mode == ECHO_MODE) {
			u_char tmp_mac_addr[6];
			/* exchange dest_mac_addr and src_mac_addr */
			memcpy((void *)tmp_mac_addr, (const void *)pkt_data, 6);
			memcpy((void *)pkt_data, (const void *)(pkt_data + 6), 6);
			memcpy((void *)(pkt_data + 6), (const void *)tmp_mac_addr, 6);

			/* Send down the packet */
			if (pcap_sendpacket(handle, (const unsigned char *)pkt_data, pkthdr->len) != 0) {
				fprintf(stderr, "Error sending the packet: %s\n", pcap_geterr(handle));
			}
			continue;
		}
		/* listening mode */
		rx_cnt++;
		if (rx_cnt >= 10000) {
			printf("10000 packets arrive.\n");
			rx_cnt = 0;
		}
	}
#ifdef WIN32
	return 1;
#endif
}

/* Initialize headers */
static void
prepare_header(const char *name)
{
	struct ethhdr *eth = (struct ethhdr *) pkt;
	unsigned long src_addr;
	char dest_addr[16];
	int i;

#if MAC_ADDR_FIX
	for (i = 0; i< 6; i++) {
		pkt[i] = dest_mac_addr[i];
		pkt[i + 6] = src_mac_addr[i];
	}
#endif
	if (insert_iphdr)
		eth->h_proto = htons(0x0800);
	else
		eth->h_proto = 0x87a; // arbitary number prevent known ethertype

							  /* Fill in the IP Header */
	memset(&iph, 0, sizeof(struct iphdr));
	iph.ihl = 5;
	iph.version = 4;
	iph.tos = 0;
	iph.id = htons(54321); //Id of this packet
	iph.frag_off = 0;
	iph.ttl = 255;

	/* give a fixed IP addr for now */
	iph.daddr = inet_addr("10.0.0.67");
	iph.daddr = inet_addr("10.0.0.68");

	if (insert_tcphdr)
		iph.protocol = IPPROTO_TCP;
	else
		iph.protocol = ~IPPROTO_TCP;
	memcpy((pkt + IPHDR_OFFSET), (char *)&iph, sizeof(struct iphdr));

	/* Fill in TCP Header */
	memset(&tcph, 0, sizeof(struct tcphdr));
	tcph.source = htons(46930);
	tcph.dest = htons(50001);
	tcph.seq = 0;
	tcph.ack_seq = 0;
	tcph.doff = 5;  //tcp header size
	tcph.fin = 0;
	tcph.syn = 0;
	tcph.rst = 0;
	tcph.psh = 0;
	tcph.ack = 1;
	tcph.urg = 0;
	tcph.window = htons(5840); /* maximum allowed window size */
	tcph.check = 0;             //leave checksum 0 now, filled later by pseudo header
	tcph.urg_ptr = 0;
	memcpy((pkt + TCPHDR_OFFSET), (char *)&tcph, sizeof(struct tcphdr));
}
/* *
* delay for xx microseconds
* */
void
time_wait(int us) {
#ifndef WIN32
	struct timespec t_s, t_e;
	clock_gettime(CLOCK_REALTIME, &t_s);
	while (!rxtx_exit) {
		clock_gettime(CLOCK_REALTIME, &t_e);
		if ((t_e.tv_sec - t_s.tv_sec) * 1000000 + (t_e.tv_nsec - t_s.tv_nsec) / 1000 > us)
			break;
	}

#else
	LARGE_INTEGER m_liPerfFreq = {0};
	if (!QueryPerformanceFrequency(&m_liPerfFreq)) {
		printf("QueryPerformanceFrequency not support for this machine.\n");
		return ;
	}
	LARGE_INTEGER m_liPerfStart = { 0 };
	QueryPerformanceCounter(&m_liPerfStart);

	LARGE_INTEGER liPerfNow = { 0 };

	for (;;)
	{
		QueryPerformanceCounter(&liPerfNow);
		double time = (((liPerfNow.QuadPart -
			m_liPerfStart.QuadPart) * 1000000) / (double)m_liPerfFreq.QuadPart);
		if (time >= us)
			break;
	}
#endif
}
/* print usage information of applicatioin */
static void
usage(void)
{
	printf("\n"
		"usage: txPckt [OPTION] [OPTION ifname]\n"
		"\n"
		"[OPTION]\n"
		"\t-h : print usage information\n"
		"\t-m <mode>: \n"
		"\t\t1 for echo mode, send received packets back to peers.\n "
		"\t\t2 for listening mode\n"
		"\t\t3 for benchmark mode\n"
		"\t-t <seconds>: time interval between two serial sending, default:1\n"
		"\t-i <interface>: network interface to send/receive packets\n "
		"\t-n <number packets>: number of packets for a sending action, default:3\n"
		"\t-b : log packet content to file as binary mode\n"
		"\t-f <filenameprefix> : may include directory, write packets content\n"
		"\t\tto ./dir/<filename>-tx-xxx.log\n\n");
}

int get_options(int argc, char *argv[])
{
	int inum, i;
	int setdev = 0;

#ifndef WIN32
	while ((i = getopt(argc, argv, "bvhf:t:n:i:m:")) != -1) {
		switch (i) {
		case 'm':
			mode = atoi(optarg);
			break;
		case 't':
			interval = atoi(optarg);
			break;
		case 'n':
			sendnum = atoi(optarg);
		case 'b':
			fbinary = 1;
			break;
		case 'f':
			strcpy(tfname, optarg);
			strcat(tfname, "-tx-%03d.log");
			strcpy(rfname, optarg);
			strcat(rfname, "-rx-%03d.log");
			log_file = 1;
			break;
		case 'i':
			strcpy(dev, optarg);
			setdev = 1;
			break;
		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}
#else
	unsigned char *p;
	for (i = 1; i < argc; i++) {
		p = (unsigned char *)argv[i];
		if (*p++ != '-') {
			fprintf(stderr, "invalid option.\n");
			exit(1);
		}
		while (*p) {
			switch (*p++) {
			case 'm':
				if (*p) {
					mode = atoi((const char*)p);
					goto next;
				}
				if (argv[++i]) {
					mode = atoi((const char*)argv[i]);
					goto next;
				}
			case 't':
				if (*p) {
					interval = atoi((const char*)p);
					goto next;
				}
				if (argv[++i]) {
					interval = atoi((const char*)argv[i]);
					goto next;
				}
			case 'n':
				if (*p) {
					sendnum = atoi((const char*)p);
					goto next;
				}
				if (argv[++i]) {
					sendnum = atoi((const char*)argv[i]);
					goto next;
				}
			case 'b':
				if (*p) {
					fbinary = atoi((const char*)p);
					goto next;
				}
				if (argv[++i]) {
					fbinary = atoi((const char*)argv[i]);
					goto next;
				}
			case 'f':
				if (*p) {
					strcpy(tfname, (const char*)p);
					strcat(tfname, "-tx-%03d.log");
					strcpy(rfname, (const char*)p);
					strcat(rfname, "-rx-%03d.log");
					log_file = 1;
					goto next;
				}
				if (argv[++i]) {
					strcpy(tfname, (const char*)argv[i]);
					strcat(tfname, "-tx-%03d.log");
					strcpy(rfname, (const char*)argv[i]);
					strcat(rfname, "-rx-%03d.log");
					log_file = 1;
					goto next;
				}
			case 'i':
				if (*p) {
					strcpy(dev, (const char*)p);
					setdev = 1;
					goto next;
				}
				if (argv[++i]) {
					strcpy(dev, (const char*)argv[i]);
					setdev = 1;
					goto next;
				}
			case 'h':
				usage();
				exit(0);
			default:
				usage();
				exit(EXIT_FAILURE);
			}
		}
	next:
		continue;
	}
#endif
	/* if user did not give a specific adapter, list all the adapters available,
	* and let the user choose one to use.
	* */
	if (!setdev) {
		pcap_if_t *alldevs;
		pcap_if_t *d;

		if (pcap_findalldevs(&alldevs, errbuf) == -1) {
			fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
			exit(EXIT_FAILURE);
		}

		/* Print the list */
		i = 0;
		for (d = alldevs; d; d = d->next) {
			if (d->description)
				printf("%d. %s\n", ++i, d->description);
			else
				printf("%d. %s (No description available)\n", ++i, d->name);
		}

		if (i == 0) {
			fprintf(stderr, "\nNo interfaces found!\n");
			exit(EXIT_FAILURE);
		}

		printf("Enter the interface number (1-%d):", i);
		if (scanf("%d", &inum) <= 0) {
			exit(EXIT_FAILURE);
		}

		if (inum < 1 || inum > i) {
			fprintf(stderr, "Interface number out of range.\n");
			/* Free the device list */
			pcap_freealldevs(alldevs);
			exit(EXIT_FAILURE);
		}

		/* Jump to the selected adapter */
		for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);

		strcpy(dev, d->name);

	}
	else {  // if (!argv[optind])
	}
	return 1;
}


int main(int argc, char *argv[])
{
	int i;

	printf("pktRxTx: build at %s, %s\n", __DATE__, __TIME__);
#ifndef WIN32
	if (getuid() != 0) {
		fprintf(stderr, "%s: root privelidges needed\n", *(argv + 0));
		exit(EXIT_FAILURE);
	}
#else
	HANDLE hThread;
#endif
	/* load configuration */
	get_options(argc, argv);
	rxtx_exit = 0;

	/* Open the adapter */
	if ((handle = pcap_open_live(dev,	/* name of the interface(device) */
		65535,	                        /* portion of the packet to capture */
		1,	                            /* promiscuous mode (nonzero means promiscuous) */
		1000,	                        /* read timeout */
		errbuf)) == NULL) {
		fprintf(stderr, "Could not open %s: %s\n", dev, errbuf);
		exit(EXIT_FAILURE);
	}

	printf("%02x:%02x:%02x:%02x:%02x:%02x ... listening on %s\n",
		src_mac_addr[0], src_mac_addr[1], src_mac_addr[2],
		src_mac_addr[3], src_mac_addr[4] & 0xff, src_mac_addr[5], dev);

	printf("send packets to %02x:%02x:%02x:%02x:%02x:%02x\n",
		dest_mac_addr[0], dest_mac_addr[1], dest_mac_addr[2],
		dest_mac_addr[3], dest_mac_addr[4] & 0xff, dest_mac_addr[5]);

#ifndef WIN32
	if (pthread_create(&rx_thread, NULL, receive_packet, NULL))
#else
	if ((hThread = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall *)(void *))receive_packet, NULL, 0, &rx_thread)) == 0)
#endif
	{
		printf("Rx Thread creation failed\n");
		goto outdoor;
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");

	/* *
	* listening mode
	* keep receiving command from user
	* */
	if (mode == LISTENING_MODE) {
		char cmd[8];
		int num;

		prepare_header((const char *)dev);

		printf("\nInput command('quit' for exit)\nFor example, 'send 5' for sending 5 packets.\n");
		do {
			printf("> ");
			scanf("%s", cmd);
			if (!memcmp((void *)cmd, (void *)"quit", 4))
				goto outdoor;

			scanf("%d", &num);

			while (num--) {
				send_packet("normal", 6);
				time_wait(200);           // wait for 200 microseconds 
			}

		} while (!rxtx_exit);
	}
	/* *
	* benchmark mode
	* keep receiving command from user
	* */
	if (mode == BENCHMARK_MODE) {
		char cmd[8];
		int time;

		prepare_header((const char *)dev);

		printf("\nInput command('quit' for exit)\nFor example, 'test 60' means benchmarking for 60 seconds.\n");
		do {
			printf("> ");
			scanf("%s", cmd);
			if (!memcmp((void *)cmd, (void *)"quit", 4))
				goto outdoor;

			scanf("%d", &time);

#ifndef WIN32
			struct timespec t_s, t_e;
			clock_gettime(CLOCK_REALTIME, &t_s);
			while (!rxtx_exit) {
				clock_gettime(CLOCK_REALTIME, &t_e);
				if ((t_e.tv_sec - t_s.tv_sec) * 1000000 + (t_e.tv_nsec - t_s.tv_nsec) / 1000 >= time * 1000000)
					break;

				send_packet("normal", 6);
				time_wait(50);           // wait for 50 microseconds 
			}

#else
			clock_t t1, t2;
			t1 = clock();
			while (!rxtx_exit) {
				t2 = clock();
				if (t2 - t1 > time * 1000)
					break;
				send_packet("normal", 6);
				time_wait(50);

			}

#endif
		} while (!rxtx_exit);
	}

#ifndef WIN32    
	pthread_join(rx_thread, NULL);
#else
	WaitForSingleObject(hThread, INFINITE);
#endif    

outdoor:
	pcap_close(handle);
	return 0;
}
