#include <stdio.h>
#include <stdlib.h>
#include <pcap/pcap.h>


void print_hex(const char *data, int len) {
	int i;
	for(i=0;i < len;i++) {
		if(len > 0 && (len % 10) == 0) {
			printf("\n");
		}
		printf("%2x ", (int)data[i] & 0xff);
	}
}

void print_ascii(const char *data, int len) {
	int i;
	for(i=0;i < len;i++) {
		if(len > 0 && (len % 10) == 0) {
			printf("\n");
		}
		printf("%2c", data[i]);
	}
}

int main(int argc, char** argv) {
	
	
	char errbuff[20];
	// char filename[20];
	// strcpy(filename, "fajl.pcap");
	
	struct pcap_pkthdr pcap_hdr;
	const u_char *pkt_data;
	
	pcap_t* p = pcap_open_offline(argv[1], errbuff);
	
	
	if(!p) {
		printf("error can't open pcap file \"%s\"\n", argv[1]);
		exit(-1);
	}
	
	
	while(pkt_data = pcap_next(p, &pcap_hdr)) {
		
		// pcap_hdr.len
		printf("\n\npacket \n"
			   "----------------------------\n");
		print_hex(pkt_data, pcap_hdr.len);
		printf("\n---\n");
		print_ascii(pkt_data, pcap_hdr.len);
	}
	
	printf("\n\n");
	pcap_close(p);
	
	return 0;
}
