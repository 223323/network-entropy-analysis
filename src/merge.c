#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pcap/pcap.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "network_layers.h"
#include <sys/stat.h>

int parse_pcap(const char* filename, const char* output);

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("%s <pcap_in> <pcap_out>\n", argv[0]);
		return -1;
	}
	
	parse_pcap(argv[1], argc > 2 ? argv[2] : 0);

}

struct pkt_queue;

static struct pkt_queue* queue_alloc(int hdr_bytes, int payload_bytes);
static void queue_push(struct pkt_queue* q, struct pcap_pkthdr* hdr, const char* payload);
static void queue_free(struct pkt_queue* q);
static void queue_pop(struct pkt_queue* q, struct pcap_pkthdr** hdr, char** payload);
static struct pcap_pkthdr* queue_peek_header(struct pkt_queue* q);


double no_attack_delay = 40;
unsigned int num_attack_packets = 0;
unsigned int num_packets = 0;
unsigned int num_normal_packets = 0;

static void print_packet_info(unsigned int target_ip, double time, struct pcap_pkthdr* hdr, const char* pkt_data) {
	char targ_ip[20];
	char src_ip[20];
	char dst_ip[20];
	
	struct in_addr addr1;
	struct in_addr addr2;
	struct in_addr addr3;
	
	Packet* pkt = (Packet*)pkt_data;
	
	addr1.s_addr = pkt->ip.saddr.ip;
	addr2.s_addr = pkt->ip.daddr.ip;
	addr3.s_addr = target_ip;

	strcpy(src_ip, inet_ntoa(addr1));
	strcpy(dst_ip, inet_ntoa(addr2));
	strcpy(targ_ip, inet_ntoa(addr3));
	
	int is_attack_packet = target_ip == pkt->ip.daddr.ip;
	const char* color;
	if(is_attack_packet) {
		num_attack_packets++;
		color = "\x1b[33m";
	} else {
		num_normal_packets++;
		color = "";
	}
	num_packets++;
	
	printf("%d) %lf %d %d target ip: [%s] src ip: [%s] dst ip: %s[%s]\x1b[0m\n", (time > no_attack_delay) ? 2 : 1, time, hdr->ts.tv_sec, hdr->ts.tv_usec, targ_ip, src_ip, color, dst_ip);
}

int parse_pcap(const char* filename, const char* output)
{

	double time;

	int pkt_size;
	char pkt_type[5];
	char event_type;

	char errbuff[80];
	struct pcap_pkthdr pcap_hdr;
	pcap_t* p = pcap_open_offline(filename, errbuff);
	
	pcap_t* p2 = pcap_open_dead(DLT_EN10MB, 2500);
	pcap_dumper_t* pd = pcap_dump_open( p2, output ? output : "/tmp/output.pcapng" );
	
	struct pkt_queue *q1 = queue_alloc(50000000, 2000000000);
	
	const char* c_target_ip = "172.28.4.7";
	// const char* c_target_ip = "152.162.178.254";
	unsigned int target_ip = inet_addr(c_target_ip);
	
	if(!p)
	{
		return -1;
	}

	const u_char *pkt_data;
	int sec_first = 1;
	int sec_first2 = 1;
	double sec_offs = 0;
	double sec_offs2 = 0;
	
	while (pkt_data = pcap_next(p, &pcap_hdr))
	{
		Packet* pkt = (Packet*)pkt_data;
		pkt_size = pcap_hdr.len;

		if(sec_first)
		{
			sec_first = 0;
			sec_offs = pcap_hdr.ts.tv_sec + pcap_hdr.ts.tv_usec * 1e-6 - 0.01;
		}
		time = (double)(pcap_hdr.ts.tv_sec + pcap_hdr.ts.tv_usec * 1e-6) - sec_offs;
		
		
		if(time < no_attack_delay && target_ip == pkt->ip.daddr.ip) {
			// printf("pushing: %lf\n", time);
			if(sec_first2) {
				sec_first2 = 0;
				sec_offs2 = time;
			}
			queue_push(q1, &pcap_hdr, pkt_data);
		} else if((target_ip != pkt->ip.daddr.ip) || (time > no_attack_delay) ) {
			
			if(time > no_attack_delay) {
				struct pcap_pkthdr* hdr2 = queue_peek_header(q1);
				if(hdr2) {
					double time2 = (double)(hdr2->ts.tv_sec + hdr2->ts.tv_usec * 1e-6) - sec_offs + (no_attack_delay-sec_offs2);
					// printf("time2: %lf\n", time2); 
					while(hdr2 && time2 < time) {
						char* pkt_data2;
						
						queue_pop(q1, &hdr2, &pkt_data2);
						
						hdr2->ts.tv_sec = time2;
						hdr2->ts.tv_usec = (time2 - floor(time2)) * 1e6;
						
						pcap_dump( (void*)pd, hdr2,  pkt_data2 );
						print_packet_info(target_ip, time2, hdr2, pkt_data2);
						
						hdr2 = queue_peek_header(q1);
						
						if(!hdr2) break;
						time2 = (double)(hdr2->ts.tv_sec + hdr2->ts.tv_usec * 1e-6) - sec_offs + (no_attack_delay-sec_offs2);
					}
				}
			}
			
			if(target_ip == pkt->ip.daddr.ip) {
				queue_push(q1, &pcap_hdr, pkt_data);
			} else {
		
				pcap_hdr.ts.tv_sec = time;
				pcap_hdr.ts.tv_usec = (time - floor(time)) * 1e6;
				
				print_packet_info(target_ip, time, &pcap_hdr, pkt_data);
				// printf("%d) %lf %d %d target ip: [%s] src ip: [%s] dst ip: %s[%s]\x1b[0m\n", (time > no_attack_delay) ? 2 : 1, time, pcap_hdr.ts.tv_sec, pcap_hdr.ts.tv_usec, targ_ip, src_ip, color, dst_ip);
				pcap_dump( (void*)pd, &pcap_hdr,  pkt_data );
			}
		}
	}
	 
	queue_free(q1);
	
	/*
	pcap_close(p);
	p = pcap_open_offline(filename, errbuff);
	
	double time_offs = time;
	double start_time = 0;
	sec_first = 1;
	
	
	while (pkt_data = pcap_next(p, &pcap_hdr))
	{
		
		Packet* pkt = (Packet*)pkt_data;
		
		pkt_size = pcap_hdr.len;
		
		struct in_addr addr1;
		struct in_addr addr2;
		addr1.s_addr = pkt->ip.saddr.ip;
		addr2.s_addr = pkt->ip.daddr.ip;
		
		struct in_addr addr3;
		addr3.s_addr = target_ip;
		
		strcpy(targ_ip, inet_ntoa(addr3));
		strcpy(src_ip, inet_ntoa(addr1));
		strcpy(dst_ip, inet_ntoa(addr2));
		
		if(target_ip == pkt->ip.daddr.ip) {
			if(sec_first)
			{
				sec_first = 0;
				sec_offs = pcap_hdr.ts.tv_sec + pcap_hdr.ts.tv_usec * 1e-6;
			}
			
			time = (double)(pcap_hdr.ts.tv_sec + pcap_hdr.ts.tv_usec * 1e-6) + (time_offs - sec_offs);
			
			pcap_hdr.ts.tv_sec = time;
			pcap_hdr.ts.tv_usec = (time - floor(time)) * 1e6;
		
			printf("2) %lf %d %d target ip: [%s] src ip: [%s]  dst ip: [%s]\n", time, pcap_hdr.ts.tv_sec, pcap_hdr.ts.tv_usec, targ_ip, src_ip, dst_ip);
			pcap_dump( (void*)pd, &pcap_hdr,  pkt_data );
		}
	}
	
	pcap_close(p);
	*/
	
	pcap_dump_close(pd);
	printf("normal packets: %d\nattack packets: %d\ntotal packets: %d\n", num_normal_packets, num_attack_packets, num_packets);

}

struct pkt_queue {
	int alloc_hdrs;
	int alloc_buffer;
	struct pcap_pkthdr* hdrs;
	struct pcap_pkthdr* hdrs_head;
	struct pcap_pkthdr* hdrs_tail;
	char* buffer;
	char* buffer_head;
	char* buffer_tail;
};


static struct pkt_queue* queue_alloc(int hdr_bytes, int payload_bytes) {
	struct pkt_queue *q = malloc(sizeof(struct pkt_queue));
	q->alloc_hdrs = hdr_bytes;
	q->alloc_buffer = payload_bytes;
	q->hdrs = (struct pcap_pkthdr*)malloc( hdr_bytes );
	q->hdrs_tail = q->hdrs;
	q->hdrs_head = q->hdrs;
	q->buffer = malloc( payload_bytes );
	q->buffer_tail = q->buffer;
	q->buffer_head = q->buffer;
	return q;
}

static void queue_free(struct pkt_queue* q) {
	free(q->hdrs);
	free(q->buffer);
	q->alloc_hdrs = 0;
	q->alloc_buffer = 0;
	free(q);
}


static void queue_push(struct pkt_queue* q, struct pcap_pkthdr* hdr, const char* payload) {
	*q->hdrs_tail = *hdr;
	q->hdrs_tail++;
	memcpy(q->buffer_tail, payload, hdr->len);
	q->buffer_tail += hdr->len;
}


static struct pcap_pkthdr* queue_peek_header(struct pkt_queue* q) {
	return (q->hdrs_head < q->hdrs_tail) ? q->hdrs_head : 0;
}

static void queue_pop(struct pkt_queue* q, struct pcap_pkthdr** hdr, char** payload) {
	*hdr = q->hdrs_head;
	*payload = q->buffer_head;
	q->buffer_head += q->hdrs_head->len;
	q->hdrs_head++;
}
