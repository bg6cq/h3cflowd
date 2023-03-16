#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXLEN 		2048
#define SERV_PORT 	4000

//#define DEBUG 1

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

struct flowloghdr {
	uint8 ver;
	uint8 h1;
	uint8 h2;
	uint8 record_num;
	uint32 tm;
	uint32 h4;
	uint32 h5;
};

struct flowlog {
	uint8 proto;
	uint8 oper;		// 1-4 end, 8, new
	uint8 ipversion;
	uint8 tos;
	uint32 srcip;
	uint32 srcnatip;
	uint32 dstip;
	uint32 dstnatip;
	uint16 srcport;
	uint16 srcnatport;
	uint16 dstport;
	uint16 dstnatport;
	uint32 tm;
	uint8 x[32];
};

char *MY_INETNTOA(uint32 ip)
{
	static char b[100];
	sprintf(b, "%u.%u.%u.%u", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
	return b;
}

int main(void)
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);	/* create a socket */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind error");
		exit(1);
	}
	printf("len of uint8, uint16, uint32 = %d, %d, %d\n", sizeof(uint8), sizeof(uint16), sizeof(uint32));
	printf("len of flowloghdr %d\n", sizeof(struct flowloghdr));
	while (1) {
		socklen_t clen;
		uint8 buf[MAXLEN];
		int len;
		len = recvfrom(sockfd, buf, MAXLEN, 0, (struct sockaddr *)&cliaddr, &clen);
		if (len <= 0) {
			printf("recvfrom return 0\n");
			continue;
		}
		if (len <= 17) {
			printf("recvfrom return %d, small packet\n", len);
			continue;
		}
		int i;
#ifdef DEBUG
		for (i = 0; i < 20; i++)
			printf("%02X ", buf[i]);
		printf("\n");
#endif
		struct flowloghdr *fhdr;
		fhdr = (struct flowloghdr *)buf;
#ifdef DEBUG
		printf("len=%d, flow ver: %d, flow count: %d\n", len, fhdr->ver, fhdr->record_num);
		printf("len should be %d\n", sizeof(struct flowloghdr) + fhdr->record_num * sizeof(struct flowlog));
#endif
		int j;
		time_t rec_tm;
		struct tm *ctm;
		rec_tm = ntohl(fhdr->tm);
		ctm = localtime(&rec_tm);
		for (j = 0; j < fhdr->record_num; j++) {
			struct flowlog *fl;
			fl = (struct flowlog *)(buf + sizeof(struct flowloghdr) + sizeof(struct flowlog) * j);
#ifdef DEBUG
			printf("offset = %d\n", (uint8 *) fl - buf);
#endif
			printf("%02d:%02d:%02d ", ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
			printf("%d %d ", fl->proto, fl->oper);
			printf("%s", MY_INETNTOA(fl->srcip));
			if (fl->srcip != fl->srcnatip)
				printf("(%s)", MY_INETNTOA(fl->srcnatip));
			printf(":%u", ntohs(fl->srcport));
			if (fl->srcport != fl->srcnatport)
				printf("(%u)", ntohs(fl->srcnatport));
			printf("->%s", MY_INETNTOA(fl->dstip));
			if (fl->dstip != fl->dstnatip)
				printf("(%s)", MY_INETNTOA(fl->dstnatip));
			printf(":%u", ntohs(fl->dstport));
			if (fl->dstport != fl->dstnatport)
				printf("(%u)", ntohs(fl->dstnatport));
			printf("\n");
		}
	}
	return 0;
}