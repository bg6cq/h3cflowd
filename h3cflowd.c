#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

#define MAXLEN 		2048

int debug = 0;
int port = 4000;
int num_printdot = 100;
char work_dir[MAXLEN] = "/natlog";

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long int uint64;

struct flowloghdr {
	uint8 ver;
	uint8 ip_version;
	uint16 record_num;
	uint32 tm;
	uint32 total_export_record_num;
	uint32 x;
};

struct flowlog {
	uint8 proto;
	uint8 oper;
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
	if (debug) {
		if (fp)
			return;
		else
			fp = stdout;
		return;
	}
	char fnbuf[MAXLEN];
	if (fp)
		pclose(fp);
	snprintf(fnbuf, MAXLEN, "gzip > %s/%04d.%02d.%02d.%02d%02d%02d.gz",
		 work_dir, ctm->tm_year + 1900, ctm->tm_mon + 1, ctm->tm_mday, ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
	fp = popen(fnbuf, "w");
	if (fp == NULL) {
		printf("popen %s error, exit\n", fnbuf);
		exit(0);
	}
}

void usage()
{
	printf("\n");
	printf(" h3cflowd v1.0 by james@ustc.edu.cn\n");
	printf("\n");
	printf("  collect H3C router/firewall NAT userlog(flowlog)\n");
	printf("\n");
	printf("  h3cflowd [ -h ] [ -d ] [ -p port ] [ -n num ] [ -w work_dir ]\n");
	printf("        -h            print help message\n");
	printf("        -d            enable debug\n");
	printf("        -p port       udp port, default is 4000\n");
	printf("        -n number     number of udp packets to print ., default is 100\n");
	printf("        -w work_dir   directory to save log file, default is /natlog\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
	int lastday = 0;
	int c;
	struct timeval last_tv;
	uint32 udp_pkts = 0, flow_records = 0;
	uint64 total_udp_pkts = 0, total_flow_records = 0;
	while ((c = getopt(argc, argv, "hdp:w:n:")) != EOF)
		switch (c) {
		case 'd':
			debug = 1;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'w':
			strncpy(work_dir, optarg, MAXLEN);
			break;
		case 'n':
			num_printdot = atoi(optarg);
			if (num_printdot <= 10)
				num_printdot = 10;
			break;
		default:
			usage();
			exit(0);
		}
	printf("udp port: %d\n", port);
	printf(" num UDP: %d print .\n", num_printdot);
	printf("work dir: %s\n", work_dir);
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind error");
		exit(1);
	}
	if (debug) {
		printf("length of struct flowloghdr %lu\n", sizeof(struct flowloghdr));
		printf("length of struct flowlog %lu\n", sizeof(struct flowlog));
	}
	gettimeofday(&last_tv, NULL);
	while (1) {
		socklen_t clen;
		uint8 buf[MAXLEN];
		int len;
		len = recvfrom(sockfd, buf, MAXLEN, 0, (struct sockaddr *)&cliaddr, &clen);
		if (len <= 0) {
			printf("recvfrom return %d\n", len);
			continue;
		}
		if (len <= 16) {
			printf("recvfrom return %d, small packet\n", len);
			continue;
		}
		struct flowloghdr *fhdr;
		fhdr = (struct flowloghdr *)buf;
		if (debug) {
			printf("len=%d, flow ver: %d, flow count: %d\n", len, fhdr->ver, ntohs(fhdr->record_num));
			printf("length should be %lu\n", sizeof(struct flowloghdr) + ntohs(fhdr->record_num) * sizeof(struct flowlog));
		}
		if (fhdr->ver != 3)
			continue;
		if (len != sizeof(struct flowloghdr) + ntohs(fhdr->record_num) * sizeof(struct flowlog)) {
			printf("Flow packet length ERROR, read %d bytes, but should be %lu bytes\n", len,
			       sizeof(struct flowloghdr) + ntohs(fhdr->record_num) * sizeof(struct flowlog));
			continue;
		}
		udp_pkts++;
		total_udp_pkts++;
		flow_records += ntohs(fhdr->record_num);
		total_flow_records += ntohs(fhdr->record_num);
		time_t rec_tm;
		struct tm *ctm;
		rec_tm = ntohl(fhdr->tm);
		ctm = localtime(&rec_tm);
		if (ctm->tm_mday != lastday) {
			changefile(ctm);
			lastday = ctm->tm_mday;
		}
		if (debug == 0) {
			if (udp_pkts == 1) {
				printf("%02d:%02d:%02d ", ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
				fflush(stdout);
			}
			if (udp_pkts % num_printdot == 1) {
				printf(".");
				fflush(stdout);
			}
			if (udp_pkts >= 30 * num_printdot) {
				struct timeval tv;
				gettimeofday(&tv, NULL);
				float tmspan = tv.tv_sec - last_tv.tv_sec + tv.tv_usec / 1000000.0 - last_tv.tv_usec / 1000000.0;
				printf(" UDP %0.2f/s Flow %0.2f/s Total UDP: %lu Total Flow: %lu",
				       udp_pkts / tmspan, flow_records / tmspan, total_udp_pkts, total_flow_records);
				printf("\n");
				udp_pkts = flow_records = 0;
				last_tv = tv;
			}
		}
		int j;
		for (j = 0; j < ntohs(fhdr->record_num); j++) {
			struct flowlog *fl;
			fl = (struct flowlog *)(buf + sizeof(struct flowloghdr) + sizeof(struct flowlog) * j);
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
			fprintf(fp, " %u/%u %u/%u", ntohl(fl->out_total_pkt), ntohl(fl->out_total_byte), ntohl(fl->in_total_pkt), ntohl(fl->in_total_byte));
			if (fl->end_tm != 0)
				fprintf(fp, " TIME:%u", ntohl(fl->end_tm) - ntohl(fl->start_tm));
			fprintf(fp, "\n");
		}
	}
	return 0;
}
