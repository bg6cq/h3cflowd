#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXLEN 		2048
#define SERV_PORT 	4000

// #define DEBUG 1

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
	uint32 start_tm;
	uint32 end_tm;
	uint32 in_total_pkt;
	uint32 in_total_byte;
	uint32 out_total_pkt;
	uint32 out_total_byte;
	uint8 x[12];
};

char *MY_INETNTOA(uint32 ip)
{
	static char b[100];
	sprintf(b, "%u.%u.%u.%u", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
	return b;
}

FILE *fp = NULL;

void changefile(struct tm *ctm)
{
#ifdef DEBUG
	if (fp)
		return;
	else
		fp = stdout;
#else
	char fnbuf[MAXLEN];
	if (fp)
		pclose(fp);
	snprintf(fnbuf, MAXLEN, "gzip > /natlog/%04d.%02d.%02d.%02d%02d%02d.gz",
		 ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday, ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
	fp = popen(fnbuf, "w");
#endif
}

int main(void)
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
	int lastday = 0;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);	/* create a socket */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind error");
		exit(1);
	}
#ifdef DEBUG
	printf("len of uint8, uint16, uint32 = %lu, %lu, %lu\n", sizeof(uint8), sizeof(uint16), sizeof(uint32));
	printf("len of flowloghdr %lu\n", sizeof(struct flowloghdr));
#endif
	int count = 0;
	while (1) {
		socklen_t clen;
		uint8 buf[MAXLEN];
		int len;
		len = recvfrom(sockfd, buf, MAXLEN, 0, (struct sockaddr *)&cliaddr, &clen);
		if (len <= 0) {
			printf("recvfrom return %d\n", len);
			continue;
		}
		if (len <= 17) {
			printf("recvfrom return %d, small packet\n", len);
			continue;
		}
#ifdef DEBUG
		int i;
		for (i = 0; i < 20; i++)
			printf("%02X ", buf[i]);
		printf("\n");
#endif
		struct flowloghdr *fhdr;
		fhdr = (struct flowloghdr *)buf;
#ifdef DEBUG
		printf("len=%d, flow ver: %d, flow count: %d\n", len, fhdr->ver, fhdr->record_num);
		printf("len should be %lu\n", sizeof(struct flowloghdr) + fhdr->record_num * sizeof(struct flowlog));
#endif
		int j;
		time_t rec_tm;
		struct tm *ctm;
		rec_tm = ntohl(fhdr->tm);
		ctm = localtime(&rec_tm);
		if (ctm->tm_mday != lastday) {
			changefile(ctm);
			lastday = ctm->tm_mday;
		}
		if (count == 0) {
			printf("%02d:%02d:%02d ", ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
			fflush(stdout);
		}
		if (count % 100 == 1) {
			printf(".");
			fflush(stdout);
		}
		if (count >= 2000) {
			printf("\n");
			count = -1;
		}
		count++;
		for (j = 0; j < fhdr->record_num; j++) {
			struct flowlog *fl;
			fl = (struct flowlog *)(buf + sizeof(struct flowloghdr) + sizeof(struct flowlog) * j);
#ifdef DEBUG
			printf("offset = %ld\n", (uint8 *) fl - buf);
#endif
			fprintf(fp, "%02d:%02d:%02d", ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
			if (fl->proto == 6)
				fprintf(fp, " tcp %d %s", fl->oper, MY_INETNTOA(fl->srcip));
			else if (fl->proto == 17)
				fprintf(fp, " udp %d %s", fl->oper, MY_INETNTOA(fl->srcip));
			else if (fl->proto == 1)
				fprintf(fp, " icmp %d %s", fl->oper, MY_INETNTOA(fl->srcip));
			else
				fprintf(fp, " %d %d %s", fl->proto, fl->oper, MY_INETNTOA(fl->srcip));
			if (fl->srcip != fl->srcnatip)
				fprintf(fp, "(%s)", MY_INETNTOA(fl->srcnatip));
			fprintf(fp, ":%u", ntohs(fl->srcport));
			if (fl->srcport != fl->srcnatport)
				fprintf(fp, "(%u)", ntohs(fl->srcnatport));
			fprintf(fp, "->%s", MY_INETNTOA(fl->dstip));
			if (fl->dstip != fl->dstnatip)
				fprintf(fp, "(%s)", MY_INETNTOA(fl->dstnatip));
			fprintf(fp, ":%u", ntohs(fl->dstport));
			if (fl->dstport != fl->dstnatport)
				fprintf(fp, "(%u)", ntohs(fl->dstnatport));
			if (fl->end_tm != 0)
				fprintf(fp, " TIME:%u", ntohl(fl->end_tm) - ntohl(fl->start_tm));
			fprintf(fp, "\n");
		}
	}
	return 0;
}
